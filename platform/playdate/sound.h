/*
File:   sound.h
Author: Taylor Robbins
Date:   09\09\2023
*/

#ifndef _SOUND_H
#define _SOUND_H

#define MAX_SOUND_INSTANCES 16

struct Sound_t
{
	bool isValid;
	
	r32 duration; //ms
	SoundFormat format;
	bool isStereo;
	bool is16bit;
	u32 samplesPerSec;
	u32 byteLength;
	u32 bytesPerFrame;
	u32 numSamples;
	
	AudioSample* sample;
	u64 numInstances;
};

struct SoundInstance_t
{
	bool isValid;
	bool isPlaying;
	int repeatCount;
	r32 rate;
	Sound_t* source;
	SamplePlayer* player;
};

struct SoundPool_t
{
	SoundInstance_t instances[MAX_SOUND_INSTANCES];
};

#endif //  _SOUND_H
