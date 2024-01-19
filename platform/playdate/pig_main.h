/*
File:   pig_main.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _PIG_MAIN_H
#define _PIG_MAIN_H

// Playdate has just shy of 16 MB of memory (67,108,864 bytes), we should be able to make 64x 256kB pages.
// Reserve 1 for FIXED_HEAP, and 6 for scratch arenas, that leaves 57 pages or exactly 6MB for the mainHeap
// In testing, we can only allocate 55 pages before the pdrealloc function starts telling us no.
// Not sure what the last 2 pages are used for. Maybe other accidental allocations we've done?
// The simulator tells us about 176,413 bytes worth of allocations that we don't really know about internally. That's 1 page's worth
#define FIXED_HEAP_SIZE              Kilobytes(256)
#define MAIN_HEAP_PAGE_SIZE          (Kilobytes(256) - sizeof(HeapPageHeader_t))
#define MAIN_HEAP_MAX_NUM_PAGES      55
#define MAIN_HEAP_MAX_SIZE           (MAIN_HEAP_PAGE_SIZE * MAIN_HEAP_MAX_NUM_PAGES)
#define SCRATCH_ARENA_SIZE           Kilobytes(512)
#define SCRATCH_ARENA_MAX_NUM_MARKS  32

#define TARGET_FRAMERATE         30 //fps
#define TARGET_FRAME_TIME        (1000.0f / TARGET_FRAMERATE) //ms
#define TIME_SCALE_SNAP_PERCENT  0.1f

#define PLAYDATE_SCREEN_WIDTH  400
#define PLAYDATE_SCREEN_HEIGHT 240

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
	SpriteSheet_t crankHintBackSheet;
	SpriteSheet_t crankHintSheet;
	
	PDMenuItem* fpsDisplayMenuItem;
	bool fpsDisplayEnabled;
	PDMenuItem* debugMenuItem;
	bool debugEnabled;
	bool debugOutputDisabledLevels[DbgLevel_NumLevels];
	
	PerfGraph_t perfGraph;
	
	OffscreenRenderState_t offscreenRenderState;
};

#endif //  _PIG_MAIN_H
