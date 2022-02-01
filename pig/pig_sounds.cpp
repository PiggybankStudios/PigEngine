/*
File:   pig_sounds.cpp
Author: Taylor Robbins
Date:   01\14\2022
Description: 
	** Holds the code that manages SoundInstances_t and provides easy to call
	** functions to start, stop, and otherwise manage these instances
*/

void PigInitSounds()
{
	ClearArray(pig->soundInstances);
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

SoundInstance_t* NewSoundInstance(SoundInstanceType_t type)
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
			result->playing = false;
			result->format = platInfo->audioFormat;
			result->volume = 1.0f;
			result->nextInstanceToStart = false;
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

void StartSoundInstance(SoundInstance_t* instance)
{
	NotNull(instance);
	Assert(instance->id != 0);
	Assert(instance->type != SoundInstanceType_None);
	Assert(instance->numFrames > 0);
	AssertIf(IsSoundInstanceTypeGenerated(instance->type), instance->frequency > 0);
	AssertIf(!IsSoundInstanceTypeGenerated(instance->type), instance->samples != nullptr);
	
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
	AssertIf(!IsSoundInstanceTypeGenerated(newInstance->type), newInstance->samples != nullptr);
	
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

SoundInstanceHandle_t PlaySineNote(SoundInstanceHandle_t soundToWaitFor, r32 volume, r64 frequency, u64 durationMs,
	u64 attackTime = 0, EasingStyle_t attackCurve = EasingStyle_None, u64 falloffTime = 0, EasingStyle_t falloffCurve = EasingStyle_None)
{
	Assert(frequency > 0.0);
	Assert(durationMs > 0);
	AssertIf(falloffTime > 0, falloffCurve != EasingStyle_None);
	SoundInstanceHandle_t result = {};
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SineWave);
	if (instance == nullptr) { return result; }
	
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
	SoundInstanceHandle_t result = {};
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SquareWave);
	if (instance == nullptr) { return result; }
	
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
	SoundInstanceHandle_t result = {};
	
	SoundInstance_t* instance = NewSoundInstance(SoundInstanceType_SawWave);
	if (instance == nullptr) { return result; }
	
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
