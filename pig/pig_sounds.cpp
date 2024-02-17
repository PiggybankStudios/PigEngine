/*
File:   pig_sounds.cpp
Author: Taylor Robbins
Date:   01\14\2022
Description: 
	** Holds the code that manages SoundInstances_t and provides easy to call
	** functions to start, stop, and otherwise manage these instances
*/

// +--------------------------------------------------------------+
// |                  Sound_t Structure Handling                  |
// +--------------------------------------------------------------+
void FreeSound(Sound_t* sound)
{
	NotNull(sound);
	AssertIf(sound->data != nullptr, sound->allocArena != nullptr);
	if (sound->data != nullptr)
	{
		FreeMem(sound->allocArena, sound->data, sound->dataSize);
	}
	ClearPointer(sound);
}
void CreateSoundFromWavAudioData(const WavAudioData_t* wavData, PlatAudioFormat_t outFormat, Sound_t* soundOut, MemArena_t* memArena)
{
	AssertSingleThreaded(); //TODO: This doesn't have to be single threaded if we make the nextSoundId thread safe
	NotNull(wavData);
	NotNull(soundOut);
	NotNull(memArena);
	Assert(wavData->totalNumFrames > 0);
	NotNull(wavData->firstChunk);
	Assert((wavData->format.bitsPerSample % 8) == 0);
	Assert((outFormat.bitsPerSample % 8) == 0);
	
	ClearPointer(soundOut);
	soundOut->allocArena = memArena;
	soundOut->id = pig->nextSoundId;
	pig->nextSoundId++;
	soundOut->format = outFormat;
	soundOut->numFrames = wavData->totalNumFrames;
	
	u64 outFormatFrameSize = (outFormat.bitsPerSample/8) * outFormat.numChannels;
	u64 wavDataFrameSize = (wavData->format.bitsPerSample/8) * wavData->format.numChannels;
	soundOut->dataSize = soundOut->numFrames * outFormatFrameSize;
	soundOut->data = AllocMem(memArena, soundOut->dataSize);
	NotNull(soundOut->data);
	
	u64 sampleIndex = 0;
	const WavAudioDataChunk_t* chunk = wavData->firstChunk;
	for (u64 cIndex = 0; cIndex < wavData->numChunks; cIndex++)
	{
		NotNull(chunk);
		Assert((chunk->dataSize % wavDataFrameSize) == 0);
		Assert(sampleIndex + chunk->dataSize <= soundOut->dataSize);
		const u8* samples = (const u8*)(chunk + 1);
		if (wavData->format == outFormat)
		{
			Assert(chunk->dataSize == chunk->numFrames * outFormatFrameSize);
			MyMemCopy(&soundOut->dataI8[sampleIndex], samples, chunk->dataSize);
		}
		else
		{
			if (outFormat.samplesPerSecond == wavData->format.samplesPerSecond)
			{
				for (u64 fIndex = 0; fIndex < chunk->numFrames; fIndex++)
				{
					u64 inNumChannels = wavData->format.numChannels;
					r64 inSamples[2];
					for (u64 chIndex = 0; chIndex < inNumChannels; chIndex++)
					{
						switch (wavData->format.bitsPerSample)
						{
							case  8: { i8 sampleI8   = ((i8*)samples)[(fIndex * inNumChannels) + chIndex];  inSamples[chIndex] = ConvertSampleI8ToR64(sampleI8);   } break;
							case 16: { i16 sampleI16 = ((i16*)samples)[(fIndex * inNumChannels) + chIndex]; inSamples[chIndex] = ConvertSampleI16ToR64(sampleI16); } break;
							case 32: { i32 sampleI32 = ((i32*)samples)[(fIndex * inNumChannels) + chIndex]; inSamples[chIndex] = ConvertSampleI32ToR64(sampleI32); } break;
							default: Unimplemented(); break;
						}
					}
					for (u64 chIndex = 0; chIndex < outFormat.numChannels; chIndex++)
					{
						switch (outFormat.bitsPerSample)
						{
							case 8:  { i8*  sampleI8  = &soundOut->dataI8[sampleIndex  + fIndex * outFormat.numChannels + chIndex]; *sampleI8  = ConvertSampleR64ToI8(inSamples[chIndex  % inNumChannels]); } break;
							case 16: { i16* sampleI16 = &soundOut->dataI16[sampleIndex + fIndex * outFormat.numChannels + chIndex]; *sampleI16 = ConvertSampleR64ToI16(inSamples[chIndex % inNumChannels]); } break;
							case 32: { i32* sampleI32 = &soundOut->dataI32[sampleIndex + fIndex * outFormat.numChannels + chIndex]; *sampleI32 = ConvertSampleR64ToI32(inSamples[chIndex % inNumChannels]); } break;
							default: Unimplemented(); break;
						}
					}
				}
			}
			else
			{
				//We don't currently support transposing from one sample rate to another
				Unimplemented(); //TODO: Implement me!
			}
		}
		sampleIndex += chunk->numFrames * outFormat.numChannels;
		chunk = chunk->next;
	}
	Assert(sampleIndex == soundOut->numFrames * soundOut->format.numChannels);
}
void CreateSoundFromOggAudioData(const OggAudioData_t* oggData, PlatAudioFormat_t outFormat, Sound_t* soundOut, MemArena_t* memArena)
{
	AssertSingleThreaded(); //TODO: This doesn't have to be single threaded if we make the nextSoundId thread safe
	NotNull(oggData);
	NotNull(soundOut);
	NotNull(memArena);
	Assert(oggData->totalNumFrames > 0);
	Assert((oggData->format.bitsPerSample % 8) == 0);
	Assert((outFormat.bitsPerSample % 8) == 0);
	
	ClearPointer(soundOut);
	soundOut->allocArena = memArena;
	soundOut->id = pig->nextSoundId;
	pig->nextSoundId++;
	soundOut->format = outFormat;
	soundOut->numFrames = oggData->totalNumFrames;
	
	u64 outFormatFrameSize = (outFormat.bitsPerSample/8) * outFormat.numChannels;
	u64 oggDataFrameSize = (oggData->format.bitsPerSample/8) * oggData->format.numChannels;
	Assert((oggData->samplesSize % oggDataFrameSize) == 0);
	soundOut->dataSize = soundOut->numFrames * outFormatFrameSize;
	soundOut->data = AllocMem(memArena, soundOut->dataSize);
	NotNull(soundOut->data);
	
	const u8* samples = (const u8*)(oggData->samples);
	if (oggData->format == outFormat)
	{
		Assert(oggData->samplesSize == soundOut->dataSize);
		MyMemCopy(&soundOut->dataI8[0], samples, oggData->samplesSize);
	}
	else
	{
		if (outFormat.samplesPerSecond == oggData->format.samplesPerSecond)
		{
			for (u64 fIndex = 0; fIndex < oggData->totalNumFrames; fIndex++)
			{
				u64 inNumChannels = oggData->format.numChannels;
				r64 inSamples[2];
				for (u64 chIndex = 0; chIndex < inNumChannels; chIndex++)
				{
					switch (oggData->format.bitsPerSample)
					{
						case  8: { i8 sampleI8   = ((i8*)samples)[(fIndex * inNumChannels) + chIndex];  inSamples[chIndex] = ConvertSampleI8ToR64(sampleI8);   } break;
						case 16: { i16 sampleI16 = ((i16*)samples)[(fIndex * inNumChannels) + chIndex]; inSamples[chIndex] = ConvertSampleI16ToR64(sampleI16); } break;
						case 32: { i32 sampleI32 = ((i32*)samples)[(fIndex * inNumChannels) + chIndex]; inSamples[chIndex] = ConvertSampleI32ToR64(sampleI32); } break;
						default: Unimplemented(); break;
					}
				}
				for (u64 chIndex = 0; chIndex < outFormat.numChannels; chIndex++)
				{
					switch (outFormat.bitsPerSample)
					{
						case 8:  { i8*  sampleI8  = &soundOut->dataI8[fIndex  * outFormat.numChannels + chIndex]; *sampleI8  = ConvertSampleR64ToI8(inSamples[chIndex  % inNumChannels]); } break;
						case 16: { i16* sampleI16 = &soundOut->dataI16[fIndex * outFormat.numChannels + chIndex]; *sampleI16 = ConvertSampleR64ToI16(inSamples[chIndex % inNumChannels]); } break;
						case 32: { i32* sampleI32 = &soundOut->dataI32[fIndex * outFormat.numChannels + chIndex]; *sampleI32 = ConvertSampleR64ToI32(inSamples[chIndex % inNumChannels]); } break;
						default: Unimplemented(); break;
					}
				}
			}
		}
		else
		{
			//We don't currently support transposing from one sample rate to another
			Unimplemented(); //TODO: Implement me!
		}
	}
}

bool TryLoadSoundOggOrWav(ProcessLog_t* log, MyStr_t filePath, MemArena_t* memArena, Sound_t* soundOut)
{
	bool result = false;
	PlatFileContents_t soundFile = {};
	if (plat->ReadFileContents(filePath, nullptr, false, &soundFile))
	{
		if (StrEndsWith(filePath, ".ogg"))
		{
			OggAudioData_t oggData = {};
			if (TryDeserOggFile(soundFile.size, soundFile.data, log, &oggData, &pig->largeAllocHeap))
			{
				CreateSoundFromOggAudioData(&oggData, platInfo->audioFormat, soundOut, memArena);
				result = true;
			}
			else
			{
				LogPrintLine_E(log, "Failed to deserialize ogg sound at \"%.*s\"", StrPrint(filePath));
			}
			FreeOggAudioData(&oggData);
		}
		else if (StrEndsWith(filePath, ".wav"))
		{
			WavAudioData_t wavData = {};
			if (TryDeserWavFile(soundFile.size, soundFile.data, log, &wavData, &pig->largeAllocHeap))
			{
				CreateSoundFromWavAudioData(&wavData, platInfo->audioFormat, soundOut, memArena);
				result = true;
			}
			else
			{
				LogPrintLine_E(log, "Failed to deserialize wav sound at \"%.*s\"", StrPrint(filePath));
			}
			FreeWavAudioData(&wavData);
		}
		else
		{
			LogPrintLine_E(log, "Unknown file format extension found on sound path: \"%.*s\"", StrPrint(filePath));
			log->hadErrors = true;
			log->errorCode = OggError_UnknownExtension;
		}
		
		plat->FreeFileContents(&soundFile);
	}
	else
	{
		LogPrintLine_E(log, "Couldn't find/open sound file at \"%.*s\"", StrPrint(filePath));
		log->hadErrors = true;
		log->errorCode = OggError_MissingFile;
	}
	return result;
}

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void PigInitSounds()
{
	ClearArray(pig->soundInstances);
	pig->nextSoundId = 1;
	pig->nextSoundInstanceId = 1;
	plat->CreateMutex(&pig->soundInstancesMutex);
}

void PigUpdateSounds()
{
	//Clean up finished sound instances
	for (u64 sIndex = 0; sIndex < PIG_MAX_SOUND_INSTANCES; sIndex++)
	{
		SoundInstance_t* instance = &pig->soundInstances[sIndex];
		if (instance->type != SoundInstanceType_None && instance->numLoops >= 1 && !instance->playing)
		{
			instance->type = SoundInstanceType_None;
		}
	}
}

SoundInstance_t* NewSoundInstance(SoundInstanceType_t type, bool isMusic)
{
	AssertSingleThreaded();
	SoundInstance_t* result = nullptr;
	if (plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE))
	{
		for (u64 sIndex = 0; sIndex < PIG_MAX_SOUND_INSTANCES; sIndex++)
		{
			SoundInstance_t* instance = &pig->soundInstances[sIndex];
			if (instance->type == SoundInstanceType_None)
			{
				result = instance;
				break;
			}
		}
		
		if (result != nullptr)
		{
			ClearPointer(result);
			result->id = pig->nextSoundInstanceId;
			pig->nextSoundInstanceId++;
			result->type = type;
			result->isMusic = isMusic;
			result->playing = false;
			result->format = platInfo->audioFormat;
			result->volume = 1.0f;
			result->nextInstanceToStart = nullptr;
			result->playOnAlignmentSize = 0;
		}
		
		plat->UnlockMutex(&pig->soundInstancesMutex);
	}
	return result;
}

SoundInstanceHandle_t NewSoundInstanceHandle(SoundInstance_t* instance)
{
	SoundInstanceHandle_t result = {};
	result.id = ((instance != nullptr) ? instance->id : 0);
	result.instancePntr = instance;
	return result;
}
SoundInstance_t* GetSoundInstanceFromHandle(SoundInstanceHandle_t handle)
{
	if (handle.instancePntr == nullptr) { return nullptr; }
	if (handle.instancePntr->id != handle.id) { return nullptr; }
	if (handle.instancePntr->type == SoundInstanceType_None) { return nullptr; }
	return handle.instancePntr;
}
bool IsSoundInstanceHandleDonePlaying(SoundInstanceHandle_t handle)
{
	SoundInstance_t* instance = GetSoundInstanceFromHandle(handle);
	if (instance == nullptr) { return true; }
	if (instance->playing && instance->numLoops >= 1) { return true; }
	return false;
}
#define SoundInstanceHandle_Empty NewSoundInstanceHandle(nullptr)

void SetSoundVolume(SoundInstance_t* instance, r32 newVolume)
{
	NotNull(instance);
	if (instance->playing && instance->volume != newVolume)
	{
		if (plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE))
		{
			instance->volume = newVolume;
			plat->UnlockMutex(&pig->soundInstancesMutex);
		}
	}
}

void StopSoundInstance(SoundInstance_t* instance)
{
	if (instance != nullptr && instance->playing)
	{
		if (plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE))
		{
			instance->type = SoundInstanceType_None;
			instance->playing = false;
			if (instance->nextInstanceToStart != nullptr)
			{
				instance->nextInstanceToStart->type = SoundInstanceType_None;
			}
			plat->UnlockMutex(&pig->soundInstancesMutex);
		}
	}
}
void StopSoundInstance(SoundInstanceHandle_t handle)
{
	StopSoundInstance(GetSoundInstanceFromHandle(handle));
}

void StopAllSoundInstancesForSound(const Sound_t* sound)
{
	for (u64 sIndex = 0; sIndex < PIG_MAX_SOUND_INSTANCES; sIndex++)
	{
		SoundInstance_t* instance = &pig->soundInstances[sIndex];
		if (instance->type == SoundInstanceType_Samples && instance->sound == sound)
		{
			StopSoundInstance(instance);
		}
	}
}

void StartSoundInstance(SoundInstance_t* instance)
{
	NotNull(instance);
	Assert(instance->id != 0);
	Assert(instance->type != SoundInstanceType_None);
	Assert(instance->numFrames > 0);
	AssertIf(IsSoundInstanceTypeGenerated(instance->type), instance->frequency > 0);
	AssertIf(!IsSoundInstanceTypeGenerated(instance->type), instance->sound != nullptr);
	
	//TODO: For now the audio mixer only handles mixing audio of the same format as the output buffer.
	Assert(instance->format.bitsPerSample == platInfo->audioFormat.bitsPerSample);
	Assert(instance->format.numChannels == platInfo->audioFormat.numChannels);
	Assert(instance->format.samplesPerSecond == platInfo->audioFormat.samplesPerSecond);
	
	if (plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE))
	{
		instance->playing = true;
		instance->frameIndex = 0;
		instance->numLoops = 0;
		plat->UnlockMutex(&pig->soundInstancesMutex);
	}
}
void StartSoundInstanceAfterOtherInstance(SoundInstance_t* otherInstance, SoundInstance_t* newInstance)
{
	NotNull(otherInstance);
	NotNull(newInstance);
	Assert(otherInstance != newInstance);
	Assert(newInstance->id != 0);
	Assert(newInstance->type != SoundInstanceType_None);
	Assert(newInstance->numFrames > 0);
	AssertIf(IsSoundInstanceTypeGenerated(newInstance->type), newInstance->frequency > 0);
	AssertIf(!IsSoundInstanceTypeGenerated(newInstance->type), newInstance->sound != nullptr);
	
	if (plat->LockMutex(&pig->soundInstancesMutex, MUTEX_LOCK_INFINITE))
	{
		Assert(otherInstance->id != 0);
		if (otherInstance->playing == true || otherInstance->numLoops == 0)
		{
			Assert(otherInstance->type != SoundInstanceType_None);
			Assert(otherInstance->nextInstanceToStart == nullptr);
			otherInstance->nextInstanceToStart = newInstance;
			newInstance->playing = false;
			newInstance->frameIndex = 0;
			newInstance->numLoops = 0;
		}
		else
		{
			newInstance->playing = true;
			newInstance->frameIndex = 0;
			newInstance->numLoops = 0;
		}
		plat->UnlockMutex(&pig->soundInstancesMutex);
	}
}

// +--------------------------------------------------------------+
// |                        Play Functions                        |
// +--------------------------------------------------------------+
SoundInstanceHandle_t PlaySineNote(SoundInstanceHandle_t soundToWaitFor, r32 volume, r64 frequency, u64 durationMs,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None, u64 falloffTime = 0, EasingStyle_t falloffCurve = EasingStyle_None)
{
	Assert(frequency > 0.0);
	Assert(durationMs > 0);
	AssertIf(falloffTime > 0, falloffCurve != EasingStyle_None);
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SineWave, false);
	if (instance == nullptr) { return SoundInstanceHandle_Empty; }
	
	instance->frequency = frequency;
	instance->numFrames = (platInfo->audioFormat.samplesPerSecond * durationMs) / 1000ULL;
	instance->attackTime = attackTime;
	instance->attackCurve = attackCurve;
	instance->falloffTime = falloffTime;
	instance->falloffCurve = falloffCurve;
	instance->volume = volume;
	
	SoundInstance_t* instanceToWaitFor = GetSoundInstanceFromHandle(soundToWaitFor);
	if (instanceToWaitFor != nullptr)
	{
		StartSoundInstanceAfterOtherInstance(instanceToWaitFor, instance);
	}
	else
	{
		StartSoundInstance(instance);
	}
	
	return NewSoundInstanceHandle(instance);
}

SoundInstanceHandle_t PlaySquareNote(r32 volume, r64 frequency, u64 durationMs,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None, u64 falloffTime = 0, EasingStyle_t falloffCurve = EasingStyle_None)
{
	Assert(frequency > 0.0);
	Assert(durationMs > 0);
	AssertIf(falloffTime > 0, falloffCurve != EasingStyle_None);
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SquareWave, false);
	if (instance == nullptr) { return SoundInstanceHandle_Empty; }
	
	instance->frequency = frequency;
	instance->numFrames = (platInfo->audioFormat.samplesPerSecond * durationMs) / 1000ULL;
	instance->attackTime = attackTime;
	instance->attackCurve = attackCurve;
	instance->falloffTime = falloffTime;
	instance->falloffCurve = falloffCurve;
	instance->volume = volume;
	
	StartSoundInstance(instance);
	
	return NewSoundInstanceHandle(instance);
}

SoundInstanceHandle_t PlaySawNote(r32 volume, r64 frequency, u64 durationMs,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None, u64 falloffTime = 0, EasingStyle_t falloffCurve = EasingStyle_None)
{
	Assert(frequency > 0.0);
	Assert(durationMs > 0);
	AssertIf(falloffTime > 0, falloffCurve != EasingStyle_None);
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SawWave, false);
	if (instance == nullptr) { return SoundInstanceHandle_Empty; }
	
	instance->frequency = frequency;
	instance->numFrames = (platInfo->audioFormat.samplesPerSecond * durationMs) / 1000ULL;
	instance->attackTime = attackTime;
	instance->attackCurve = attackCurve;
	instance->falloffTime = falloffTime;
	instance->falloffCurve = falloffCurve;
	instance->volume = volume;
	
	StartSoundInstance(instance);
	
	return NewSoundInstanceHandle(instance);
}

//TODO: Rename this because windows has PlaySound as a function name
SoundInstanceHandle_t PlaySound(const Sound_t* sound, r32 volume,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None, u64 falloffTime = 0, EasingStyle_t falloffCurve = EasingStyle_None)
{
	NotNull(sound);
	NotNull(sound->data);
	AssertIf(attackTime > 0, attackCurve != EasingStyle_None);
	AssertIf(falloffTime > 0, falloffCurve != EasingStyle_None);
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_Samples, false);
	if (instance == nullptr) { return SoundInstanceHandle_Empty; }
	
	instance->numFrames = sound->numFrames;
	instance->format = sound->format;
	instance->sound = sound;
	instance->volume = volume;
	instance->attackTime = attackTime;
	instance->attackCurve = attackCurve;
	instance->falloffTime = falloffTime;
	instance->falloffCurve = falloffCurve;
	
	StartSoundInstance(instance);
	
	return NewSoundInstanceHandle(instance);
}

//TODO: This interface should probably be much better so we don't have 2 music instances at once and so we can do cross-fades and similar effects
#if 0
SoundInstanceHandle_t PlayMusic(const Sound_t* sound, r32 volume,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None)
{
	NotNull(sound);
	NotNull(sound->data);
	
	SoundInstanceHandle_t result = {};
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_Samples, true);
	if (instance == nullptr) { return result; }
	
	instance->repeating = true;
	instance->numFrames = sound->numFrames;
	instance->format = sound->format;
	instance->sound = sound;
	instance->volume = volume;
	instance->attackTime = attackTime;
	instance->attackCurve = attackCurve;
	
	StartSoundInstance(instance);
	
	return NewSoundInstanceHandle(instance);
}
#endif
