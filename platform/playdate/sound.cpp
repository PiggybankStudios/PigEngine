/*
File:   sound.cpp
Author: Taylor Robbins
Date:   09\09\2023
Description: 
	** Functions for Sound_t structure (which wraps AudioSample* and SamplePlayer*)
*/

Sound_t LoadSound(MyStr_t path)
{
	MemArena_t* scratch = GetScratchArena();
	Sound_t result = {};
	
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	result.sample = pd->sound->sample->load(path.chars);
	if (result.sample != nullptr)
	{
		result.isValid = true;
		
		result.duration = pd->sound->sample->getLength(result.sample) * NUM_MS_PER_SECOND;
		
		u8* sampleData = nullptr;
		pd->sound->sample->getData(result.sample, &sampleData, &result.format, &result.samplesPerSec, &result.byteLength);
		result.isStereo = SoundFormatIsStereo(result.format);
		result.is16bit = SoundFormatIs16bit(result.format);
		result.bytesPerFrame = SoundFormat_bytesPerFrame(result.format);
		result.numSamples = (result.byteLength / result.bytesPerFrame);
	}
	else
	{
		PrintLine_E("Couldn't load sound %s", pathNullTerm.chars);
	}
	
	FreeScratchArena(scratch);
	return result;
}

SoundInstance_t NewSoundInstance(Sound_t* source)
{
	NotNull(source);
	SoundInstance_t result = {};
	result.source = source;
	
	result.player = pd->sound->sampleplayer->newPlayer();
	if (result.player != nullptr)
	{			
		pd->sound->sampleplayer->setSample(result.player, source->sample);
		result.isValid = true;
	}
	else
	{
		AssertMsg(false, "Couldn't create SamplePlayer");
	}
	
	return result;
}

void StartSoundInstance(SoundInstance_t* instance, int repeatCount = 1, r32 rate = 1.0f)
{
	NotNull(instance);
	if (!instance->isValid) { return; }
	
	pd->sound->sampleplayer->play(instance->player, repeatCount, rate);
	instance->isPlaying = true;
	instance->repeatCount = repeatCount;
	instance->rate = rate;
}

void UpdateSoundInstance(SoundInstance_t* instance)
{
	if (!instance->isValid) { return; }
	if (!instance->isPlaying) { return; }
	instance->isPlaying = (pd->sound->sampleplayer->isPlaying(instance->player) != 0);
}

void InitSoundPool(SoundPool_t* pool)
{
	NotNull(pool);
	ClearPointer(pool);
}

void UpdateSoundPool(SoundPool_t* pool)
{
	NotNull(pool);
	for (u64 iIndex = 0; iIndex < MAX_SOUND_INSTANCES; iIndex++)
	{
		SoundInstance_t* instance = &pool->instances[iIndex];
		UpdateSoundInstance(instance);
		if (instance->isValid && !instance->isPlaying)
		{
			instance->source->numInstances--;
			ClearPointer(instance);
		}
	}
}

SoundInstance_t* AllocSoundInstance(SoundPool_t* pool)
{
	NotNull(pool);
	for (u64 iIndex = 0; iIndex < MAX_SOUND_INSTANCES; iIndex++)
	{
		SoundInstance_t* instance = &pool->instances[iIndex];
		if (!instance->isValid)
		{
			return instance;
		}
	}
	return nullptr;
}

SoundInstance_t* PlaySound(SoundPool_t* pool, Sound_t* source, int repeatCount = 1, r32 rate = 1.0f)
{
	NotNull(pool);
	SoundInstance_t* instance = AllocSoundInstance(pool);
	if (instance == nullptr) { return nullptr; }
	*instance = NewSoundInstance(source);
	StartSoundInstance(instance, repeatCount, rate);
	source->numInstances++;
	return instance;
}
SoundInstance_t* PlaySound(Sound_t* source, int repeatCount = 1, r32 rate = 1.0f)
{
	NotNull(pig);
	return PlaySound(&pig->soundPool, source, repeatCount, rate);
}
