/*
File:   pig_music.cpp
Author: Taylor Robbins
Date:   07\10\2022
Description: 
	** Handles tracking the playing music as well as effects like cross-fading when we want to change music
*/

void PigInitMusicSystem(MusicSystemState_t* music)
{
	NotNull(music);
	ClearPointer(music);
	music->currentMusic = SoundInstanceHandle_Empty;
	music->previousMusic = SoundInstanceHandle_Empty;
	music->currentFade = MusicFade_None;
}

void GetMusicVolumesForFade(MusicFade_t fade, r32 progress, r32* currentVolumeOut, r32* previousVolumeOut)
{
	r32 current = 1.0f;
	r32 previous = 0.0f;
	if (progress < 0.0f) { current = 0.0f; previous = 1.0f; }
	else if (progress > 1.0f) { current = 1.0f; previous = 0.0f; }
	else
	{
		r32 invProgress = (1.0f - progress);
		bool halfway = (progress >= 0.5f);
		bool third = (progress >= (1.0f / 3.0f));
		bool twoThird = (progress >= (2.0f / 3.0f));
		switch (fade)
		{
			case MusicFade_Instant:          current = 1.0f; previous = 0.0f; break;
			case MusicFade_Step:             current = (halfway ? 1.0f : 0.0f); previous = (halfway ? 0.0f : 1.0f); break;
			case MusicFade_LinearCrossfade:  current = progress; previous = invProgress; break;
			case MusicFade_FadeOutThenIn:    current = (halfway ? ((progress - 0.5f) * 2) : 0.0f); previous = (halfway ? 0.0f : (1 - (2*progress))); break;
			case MusicFade_FadeInThenOut:    current = (halfway ? 1.0f : (2*progress)); previous = (halfway ? (1 - ((progress - 0.5f) * 2)) : 1.0f); break;
			case MusicFade_PartialCrossFade: current = (third ? ((1.5f*progress) - 0.5f) : 0.0f); previous = (twoThird ? 0.0f : (1.5f * (1-progress))); break;
			default: DebugAssert(false); break;
		}
	}
	if (currentVolumeOut != nullptr) { *currentVolumeOut = current; }
	if (previousVolumeOut != nullptr) { *previousVolumeOut = previous; }
}

void PigUpdateMusicSystem(MusicSystemState_t* music)
{
	NotNull(music);
	
	// +==============================+
	// |     Update Fade Progress     |
	// +==============================+
	if (music->currentFade != MusicFade_None)
	{
		if (music->currentFadeDuration == 0) { music->currentFadeProgress = 1.0f; }
		else { UpdateAnimationUp(&music->currentFadeProgress, music->currentFadeDuration); }
		
		// +==============================+
		// |        Fade Completed        |
		// +==============================+
		if (music->currentFadeProgress == 1.0f)
		{
			music->currentFade = MusicFade_None;
			StopSoundInstance(music->previousMusic);
			music->previousMusic = SoundInstanceHandle_Empty;
		}
	}
	
	// +==============================+
	// |   Calculate Music Volumes    |
	// +==============================+
	r32 currentVolume = 1;
	r32 previousVolume = 0;
	if (music->currentFade != MusicFade_None)
	{
		GetMusicVolumesForFade(music->currentFade, music->currentFadeProgress, &currentVolume, &previousVolume);
	}
	
	// +==============================+
	// |     Apply Music Volumes      |
	// +==============================+
	SoundInstance_t* currentInstance = GetSoundInstanceFromHandle(music->currentMusic);
	SoundInstance_t* previousInstance = GetSoundInstanceFromHandle(music->previousMusic);
	if (currentInstance != nullptr) { SetSoundVolume(currentInstance, ConvertLoudnessToVolume(currentVolume)); }
	if (previousInstance != nullptr) { SetSoundVolume(previousInstance, ConvertLoudnessToVolume(previousVolume)); }
}

SoundInstanceHandle_t PlayMusic(Sound_t* sound, MusicFade_t fade = MusicFade_Instant, r32 fadeDuration = 0)
{
	NotNull(pig);
	Assert(fade != MusicFade_None && fade < MusicFade_NumTypes);
	MusicSystemState_t* music = &pig->musicSystem;
	
	if (music->currentFade != MusicFade_None)
	{
		music->currentFade = MusicFade_None;
		StopSoundInstance(music->previousMusic);
	}
	
	music->previousMusic = music->currentMusic;
	music->currentMusic = SoundInstanceHandle_Empty;
	music->currentFade = fade;
	music->currentFadeDuration = fadeDuration;
	music->currentFadeProgress = 0.0f;
	
	if (sound != nullptr)
	{
		SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_Samples, true);
		if (instance == nullptr) { return SoundInstanceHandle_Empty; }
		
		instance->repeating = true;
		instance->numFrames = sound->numFrames;
		instance->format = sound->format;
		instance->sound = sound;
		instance->volume = 0.0f;
		
		StartSoundInstance(instance);
		
		music->currentMusic = NewSoundInstanceHandle(instance);
	}
	
	return music->currentMusic;
}

SoundInstanceHandle_t SoftPlayMusic(Sound_t* sound, MusicFade_t fade = MusicFade_Instant, r32 fadeDuration = 0)
{
	NotNull(pig);
	Assert(fade != MusicFade_None && fade < MusicFade_NumTypes);
	MusicSystemState_t* music = &pig->musicSystem;
	
	SoundInstance_t* currentInstance = GetSoundInstanceFromHandle(music->currentMusic);
	if (currentInstance == nullptr && sound == nullptr) { return music->currentMusic; }
	if (currentInstance != nullptr && currentInstance->sound != nullptr && sound != nullptr && currentInstance->sound->id == sound->id) { return music->currentMusic; }
	
	return PlayMusic(sound, fade, fadeDuration);
}
