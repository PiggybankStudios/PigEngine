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
	ClearArray(pig->audioOutSamples);
	pig->audioOutWriteIndex = 0;
}

r64 GetSoundInstanceCurrentVolume(SoundInstance_t* instance, PlatAudioFormat_t format, r64 instanceTime)
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
	return result;
}

//TODO: Add support for multi-channel audio!
r64 GetSoundInstanceSample(SoundInstance_t* instance, PlatAudioFormat_t format, u64 globalSampleIndex)
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
			result = SinR64(instanceTime * TwoPi64 * instance->frequency) * GetSoundInstanceCurrentVolume(instance, format, instanceTime);
		} break;
		
		case SoundInstanceType_SquareWave:
		{
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result = ((DecimalPartR64(instanceTime / (1 / instance->frequency)) >= 0.5f) ? 1.0f : -1.0f) * GetSoundInstanceCurrentVolume(instance, format, instanceTime);
		} break;
		
		case SoundInstanceType_SawWave:
		{
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result = SawR64(instanceTime * TwoPi64 * instance->frequency) * GetSoundInstanceCurrentVolume(instance, format, instanceTime);
		} break;
		
		case SoundInstanceType_Samples:
		{
			NotNull(instance->sound);
			NotNull(instance->sound->data);
			Assert(instance->sound->format.samplesPerSecond == format.samplesPerSecond);
			
			if (instance->sound->format.bitsPerSample == 8)
			{
				i8* framePntr = &instance->sound->dataI8[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI8ToR64(framePntr[0]);
			}
			else if (instance->sound->format.bitsPerSample == 16)
			{
				i16* framePntr = &instance->sound->dataI16[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI16ToR64(framePntr[0]);
			}
			else if (instance->sound->format.bitsPerSample == 32)
			{
				i32* framePntr = &instance->sound->dataI32[instance->frameIndex * instance->sound->format.numChannels];
				result = ConvertSampleI32ToR64(framePntr[0]);
			}
			else { AssertMsg(false, "We don't support a sound's bitsPerSample in the audio mixer!"); }
			
			r64 instanceTime = (r64)instance->frameIndex / (r64)format.samplesPerSecond;
			result *= GetSoundInstanceCurrentVolume(instance, format, instanceTime);
		} break;
		
		default: AssertMsg(false, "Unhandled SoundInstanceType in GetSoundInstanceSample"); break;
	}
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
	
	PerfTime_t serviceStartTime = plat->GetPerfTime();
	
	plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE);
	plat->LockMutex(&pig->audioOutSamplesMutex, MUTEX_LOCK_INFINITE);
	
	u64 outputByteIndex = 0;
	for (u64 frameIndex = 0; frameIndex < audioInfo->numFramesNeeded; frameIndex++)
	{
		u64 globalFrameIndex = audioInfo->audioFrameIndex + frameIndex;
		globalFrameIndex = (globalFrameIndex % (audioInfo->format.samplesPerSecond * 100ULL));
		
		r64 sampleValueR64 = 0;
		for (u64 iIndex = 0; iIndex < PIG_MAX_SOUND_INSTANCES; iIndex++)
		{
			SoundInstance_t* instance = &pig->soundInstances[iIndex];
			if (instance->type != SoundInstanceType_None && instance->playing)
			{
				sampleValueR64 += GetSoundInstanceSample(instance, audioInfo->format, globalFrameIndex);
			}
		}
		// if (sampleValueR64 > 1.0 || sampleValueR64 < -1.0) { MyDebugBreak(); }
		#if DEBUG_BUILD
		if (sampleValueR64 >  1.0f) { sampleValueR64 =  1.0f; pig->numAudioClips++; }
		if (sampleValueR64 < -1.0f) { sampleValueR64 = -1.0f; pig->numAudioClips++; }
		#else
		sampleValueR64 = ClampR64(sampleValueR64, -1, 1);
		#endif
		
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
			pig->audioOutSamples[pig->audioOutWriteIndex] = sampleValueR64;
			pig->audioOutWriteIndex = (pig->audioOutWriteIndex + 1) % PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH;
		}
		
		if (audioInfo->format.bitsPerSample == 32)
		{
			i32 sampleValueI32 = (i32)RoundR64i(sampleValueR64 * (r64)INT32_MAX);
			if (audioInfo->format.numChannels == 2)
			{
				*(i32*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i32)*0] = sampleValueI32;
				*(i32*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i32)*1] = sampleValueI32;
			}
			else if (audioInfo->format.numChannels == 1)
			{
				*(i32*)&audioInfo->bufferPntr[outputByteIndex] = sampleValueI32;
			}
			else { AssertMsg_(false, "Unhandled value for numChannels in PigAudioService"); }
		}
		else if (audioInfo->format.bitsPerSample == 16)
		{
			i16 sampleValueI16 = (i16)RoundR64i(sampleValueR64 * (r64)INT16_MAX);
			if (audioInfo->format.numChannels == 2)
			{
				*(i16*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i16)*0] = sampleValueI16;
				*(i16*)&audioInfo->bufferPntr[outputByteIndex + sizeof(i16)*1] = sampleValueI16;
			}
			else if (audioInfo->format.numChannels == 1)
			{
				*(i16*)&audioInfo->bufferPntr[outputByteIndex] = sampleValueI16;
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
	
	PerfTime_t serviceEndTime = plat->GetPerfTime();
	pig->audioMixerTime = plat->GetPerfTimeDiff(&serviceStartTime, &serviceEndTime);
}
