/*
File:   pig_audio_mixer.cpp
Author: Taylor Robbins
Date:   01\14\2022
Description: 
	** Holds the function that services the audio output buffer when needed
	** This function runs on the audio output thread and gets called
	** according to the needs of the platform layer in order to keep the
	** audio output buffer filled with valid samples in front of the read cursor
*/

void PigInitAudioOutput()
{
	//NOTE: This happens on the main thread
	AssertSingleThreaded();
	
	plat->CreateMutex(&pig->audioOutSamplesMutex);
	pig->audioOutSamples = AllocArray(&pig->largeAllocHeap, r64, PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
	NotNull(pig->audioOutSamples);
	MyMemSet(pig->audioOutSamples, 0x00, sizeof(r64) * PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
	pig->audioOutWriteIndex = 0;
}

r64 GetSoundInstanceCurrentVolume(SoundInstance_t* instance, PlatAudioFormat_t format, r64 instanceTime, r32 masterVolume, r32 musicVolume, r32 soundsVolume)
{
	r64 instanceLengthMs = ((r64)instance->numFrames / (r64)format.samplesPerSecond) * 1000.0;
	r64 result = (r64)instance->volume;
	if (instance->attackTime > 0 && (instanceTime * 1000.0) < (r64)instance->attackTime)
	{
		r64 attackTime = (instanceTime * 1000.0)  / (r64)instance->attackTime;
		result *= Ease(instance->attackCurve, (r32)attackTime);
	}
	if (instance->falloffTime > 0 && (instanceTime * 1000.0) > instanceLengthMs - (r64)instance->falloffTime)
	{
		r64 falloffTime = ((instanceTime * 1000.0) - (instanceLengthMs - (r64)instance->falloffTime)) / (r64)instance->falloffTime;
		result *= 1.0f - Ease(instance->falloffCurve, (r32)falloffTime);
	}
	result *= masterVolume * (instance->isMusic ? musicVolume : soundsVolume);
	return result;
}

//TODO: Add support for multi-channel audio!
r64 GetSoundInstanceSample(SoundInstance_t* instance, PlatAudioFormat_t format, u8 channelIndex, bool advanceFrameIndex, u64 globalSampleIndex, r32 masterVolume, r32 musicVolume, r32 soundsVolume)
{
	NotNull(instance);
	r64 result = 0;
	if (instance->playOnAlignmentSize != 0)
	{
		if ((globalSampleIndex % instance->playOnAlignmentSize) == 0)
		{
			instance->playOnAlignmentSize = 0;
		}
		else
		{
			return 0.0;
		}
	}
	switch (instance->type)
	{
		case SoundInstanceType_SineWave:
		{
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result = SinR64(instanceTime * TwoPi64 * instance->frequency) * GetSoundInstanceCurrentVolume(instance, format, instanceTime, masterVolume, musicVolume, soundsVolume);
		} break;
		
		case SoundInstanceType_SquareWave:
		{
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result = ((DecimalPartR64(instanceTime / (1 / instance->frequency)) >= 0.5f) ? 1.0f : -1.0f) * GetSoundInstanceCurrentVolume(instance, format, instanceTime, masterVolume, musicVolume, soundsVolume);
		} break;
		
		case SoundInstanceType_SawWave:
		{
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result = SawR64(instanceTime * TwoPi64 * instance->frequency) * GetSoundInstanceCurrentVolume(instance, format, instanceTime, masterVolume, musicVolume, soundsVolume);
		} break;
		
		case SoundInstanceType_Samples:
		{
			NotNull(instance->sound);
			NotNull(instance->sound->data);
			Assert(instance->sound->format.samplesPerSecond == format.samplesPerSecond);
			
			if (instance->sound->format.bitsPerSample == 8)
			{
				i8* framePntr = &instance->sound->dataI8[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI8ToR64(framePntr[channelIndex % instance->sound->format.numChannels]);
			}
			else if (instance->sound->format.bitsPerSample == 16)
			{
				i16* framePntr = &instance->sound->dataI16[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI16ToR64(framePntr[channelIndex % instance->sound->format.numChannels]);
			}
			else if (instance->sound->format.bitsPerSample == 32)
			{
				i32* framePntr = &instance->sound->dataI32[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI32ToR64(framePntr[channelIndex % instance->sound->format.numChannels]);
			}
			else { AssertMsg(false, "We don't support a sound's bitsPerSample in the audio mixer!"); }
			
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result *= GetSoundInstanceCurrentVolume(instance, format, instanceTime, masterVolume, musicVolume, soundsVolume);
		} break;
		
		default: AssertMsg(false, "Unhandled SoundInstanceType in GetSoundInstanceSample"); break;
	}
	if (advanceFrameIndex)
	{
		instance->frameIndex++;
		if (instance->frameIndex >= instance->numFrames)
		{
			instance->numLoops++;
			instance->frameIndex = 0;
			if (!instance->repeating)
			{
				instance->playing = false;
				if (instance->nextInstanceToStart != nullptr)
				{
					instance->nextInstanceToStart->playing = true;
					instance->nextInstanceToStart = nullptr;
				}
			}
			else
			{
				instance->attackTime = 0;
				instance->attackCurve = EasingStyle_None;
			}
		}
	}
	return result;
}

void PigAudioService(AudioServiceInfo_t* audioInfo)
{
	//NOTE: This function runs on the audio thread
	NotNull_(platInfo);
	NotNull_(plat);
	NotNull_(pig);
	NotNull_(audioInfo);
	
	PerfTime_t serviceStartTime = GetPerfTime();
	
	plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE);
	r32 masterVolume = pig->masterVolume;
	r32 musicVolume  = (pig->musicEnabled ? pig->musicVolume : 0.0f);
	r32 soundsVolume = (pig->soundsEnabled ? pig->soundsVolume : 0.0f);
	plat->UnlockMutex(&pig->volumeMutex);
	
	plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE);
	plat->LockMutex(&pig->audioOutSamplesMutex, MUTEX_LOCK_INFINITE);
	
	u64 outputByteIndex = 0;
	for (u64 frameIndex = 0; frameIndex < audioInfo->numFramesNeeded; frameIndex++)
	{
		u64 globalFrameIndex = audioInfo->audioFrameIndex + frameIndex;
		globalFrameIndex = (globalFrameIndex % (audioInfo->format.samplesPerSecond * 100ULL));
		
		r64 sampleValueR64[PIG_MAX_AUDIO_CHANNELS] = { 0.0f, 0.0f };
		DebugAssert(audioInfo->format.numChannels <= PIG_MAX_AUDIO_CHANNELS);
		for (u64 iIndex = 0; iIndex < PIG_MAX_SOUND_INSTANCES; iIndex++)
		{
			SoundInstance_t* instance = &pig->soundInstances[iIndex];
			if (instance->type != SoundInstanceType_None && instance->playing)
			{
				for (u8 channelIndex = 0; channelIndex < audioInfo->format.numChannels; channelIndex++)
				{
					bool advanceFrameIndex = (channelIndex == audioInfo->format.numChannels-1);
					sampleValueR64[channelIndex] += GetSoundInstanceSample(instance, audioInfo->format, channelIndex, advanceFrameIndex, globalFrameIndex, masterVolume, musicVolume, soundsVolume);
				}
			}
		}
		for (u8 channelIndex = 0; channelIndex < audioInfo->format.numChannels; channelIndex++)
		{
			// if (sampleValueR64 > 1.0 || sampleValueR64 < -1.0) { MyDebugBreak(); }
			#if DEVELOPER_BUILD
			if (sampleValueR64[channelIndex] >  1.0f) { sampleValueR64[channelIndex] =  1.0f; pig->numAudioClips++; }
			if (sampleValueR64[channelIndex] < -1.0f) { sampleValueR64[channelIndex] = -1.0f; pig->numAudioClips++; }
			#else
			sampleValueR64[channelIndex] = ClampR64(sampleValueR64[channelIndex], -1, 1);
			#endif
		}
		
		#if 0
		// +==============================+
		// |     Sine Wave Generation     |
		// +==============================+
		const r64 volume = 0.2;
		r64 sampleTime = (r64)globalFrameIndex / (r64)audioInfo->format.samplesPerSecond;
		sampleValueR64 = SinR64(sampleTime * TwoPi64 * FREQUENCY_MIDDLE_C) * volume;
		#endif
		
		if (!pig->audioOutGraph.paused)
		{
			//TODO: Feed 2 channel audio data to the debug display!
			pig->audioOutSamples[pig->audioOutWriteIndex] = sampleValueR64[0];
			pig->audioOutWriteIndex = (pig->audioOutWriteIndex + 1) % PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH;
		}
		
		if (audioInfo->format.bitsPerSample == 32)
		{
			i32 sampleValueI32[PIG_MAX_AUDIO_CHANNELS];
			sampleValueI32[0] = (i32)RoundR64i(sampleValueR64[0] * (r64)INT32_MAX);
			sampleValueI32[1] = (i32)RoundR64i(sampleValueR64[1] * (r64)INT32_MAX);
			if (audioInfo->format.numChannels == 2)
			{
				*(i32*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i32)*0] = sampleValueI32[0];
				*(i32*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i32)*1] = sampleValueI32[1];
			}
			else if (audioInfo->format.numChannels == 1)
			{
				*(i32*)&audioInfo->bufferPntr[outputByteIndex] = sampleValueI32[0];
			}
			else { AssertMsg_(false, "Unhandled value for numChannels in PigAudioService"); }
		}
		else if (audioInfo->format.bitsPerSample == 16)
		{
			i16 sampleValueI16[PIG_MAX_AUDIO_CHANNELS];
			sampleValueI16[0] = (i16)RoundR64i(sampleValueR64[0] * (r64)INT16_MAX);
			sampleValueI16[1] = (i16)RoundR64i(sampleValueR64[1] * (r64)INT16_MAX);
			if (audioInfo->format.numChannels == 2)
			{
				*(i16*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i16)*0] = sampleValueI16[0];
				*(i16*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i16)*1] = sampleValueI16[1];
			}
			else if (audioInfo->format.numChannels == 1)
			{
				*(i16*)&audioInfo->bufferPntr[outputByteIndex] = sampleValueI16[0];
			}
			else { AssertMsg_(false, "Unhandled value for numChannels in PigAudioService"); }
		}
		else { AssertMsg_(false, "Unhandled value for bitsPerSample in PigAudioService"); }
		
		outputByteIndex += (audioInfo->format.numChannels * (audioInfo->format.bitsPerSample/8));
	}
	
	audioInfo->numFramesFilled = audioInfo->numFramesNeeded;
	audioInfo->fillWithSilence = false; //TODO: Remove me!
	plat->UnlockMutex(&pig->audioOutSamplesMutex);
	plat->UnlockMutex(&pig->soundInstancesMutex);
	
	PerfTime_t serviceEndTime = GetPerfTime();
	pig->audioMixerTime = GetPerfTimeDiff(&serviceStartTime, &serviceEndTime);
}
