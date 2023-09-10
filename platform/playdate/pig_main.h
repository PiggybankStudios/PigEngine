/*
File:   pig_main.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _PIG_MAIN_H
#define _PIG_MAIN_H

//PlayDate has 16 MB of memory, we should be able to make 64x 256kB pages. Reserve one for FIXED_HEAP, and 6 for scratch arenas
#define FIXED_HEAP_SIZE              Kilobytes(256)
#define MAIN_HEAP_PAGE_SIZE          Kilobytes(256)
#define MAIN_HEAP_MAX_NUM_PAGES      57
#define SCRATCH_ARENA_SIZE           Kilobytes(512)
#define SCRATCH_ARENA_MAX_NUM_MARKS  32

#define TARGET_FRAMERATE         30 //fps
#define TARGET_FRAME_TIME        (1000.0f / TARGET_FRAMERATE) //ms
#define TIME_SCALE_SNAP_PERCENT  0.1f

#define DEBUG_FONT_PATH "Resources/Fonts/pixel8"

struct PigEngineState_t
{
	bool initialized;
	bool firstUpdateCalled;
	u64 programStartTimeSinceEpoch;
	
	MemArena_t stdHeap;
	MemArena_t fixedHeap;
	MemArena_t mainHeap;
	MemArena_t scratchArenas[NUM_SCRATCH_ARENAS_PER_THREAD];
	
	AppStateStack_t appStates;
	AppInput_t input;
	SoundPool_t soundPool;
	Font_t debugFont;
	RandomSeries_t random;
	
	PDMenuItem* fpsDisplayMenuItem;
	bool fpsDisplayEnabled;
	PDMenuItem* debugMenuItem;
	bool debugEnabled;
	
	PerfGraph_t perfGraph;

};

#endif //  _PIG_MAIN_H
