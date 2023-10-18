/*
File:   web_core.cpp
Author: Taylor Robbins
Date:   10\16\2023
Description: 
	** Holds some of the most basic functions that the web platform layer depends on
	** (Like memory arena initialization)
*/

// Currently, this takes 32 wasm pages right off the bat for tempArena and scratchArenas.
// And 1 page more the instant anything else is allocated on mainHeap.
void Web_MemoryInit()
{
	MemArena_t tempStdHeap;
	InitMemArena_StdHeap(&tempStdHeap);
	
	Platform = AllocStruct(&tempStdHeap, WebPlatformState_t);
	NotNull(Platform);
	ClearPointer(Platform);
	
	MyMemCopy(&Platform->stdHeap, &tempStdHeap, sizeof(MemArena_t));
	
	InitMemArena_PagedHeapArena(&Platform->mainHeap, PLAT_MAIN_HEAP_PAGE_SIZE, &Platform->stdHeap);
	FlagUnset(Platform->mainHeap.flags, MemArenaFlag_AutoFreePages); //Wasm can't free memory
	
	//At 512kB, this takes 8 wasm pages
	u8* tempArenaSpace = AllocArray(&Platform->stdHeap, u8, PLAT_TEMP_ARENA_SIZE);
	NotNull(tempArenaSpace);
	InitMemArena_MarkedStack(&Platform->tempArena, PLAT_TEMP_ARENA_SIZE, tempArenaSpace, PLAT_TEMP_ARENA_MAX_MARKS);
	TempArena = &Platform->tempArena;
	
	//At 256kB each, 3 arenas, this takes 24 wasm pages
	for (u64 aIndex = 0; aIndex < ArrayCount(Platform->scratchArenas); aIndex++)
	{
		InitMemArena_PagedStackArena(&Platform->scratchArenas[aIndex], WEB_SCRATCH_ARENA_PAGE_SIZE, &Platform->mainHeap, WEB_SCRATCH_ARENA_MAX_NUM_MARKS);
	}
}

// Pass in 1 or 2 arenas that you DON'T want to get back from this function. i.e. you are going to use those arenas for memory that outlives your scratch memory
static MemArena_t* GetScratchArena(MemArena_t* avoidConflictWith1, MemArena_t* avoidConflictWith2) //pre-declared in gy_scratch_arenas.h
{
	u64 scratchIndex = 0;
	while ((avoidConflictWith1 != nullptr && avoidConflictWith1 == &Platform->scratchArenas[scratchIndex]) ||
	       (avoidConflictWith2 != nullptr && avoidConflictWith2 == &Platform->scratchArenas[scratchIndex]))
	{
		scratchIndex++;
	}
	Assert(scratchIndex < NUM_SCRATCH_ARENAS_PER_THREAD);
	MemArena_t* result = &Platform->scratchArenas[scratchIndex];
	if (!IsInitialized(result)) { return nullptr; }
	PushMemMark(result);
	return result;
}

static void FreeScratchArena(MemArena_t* scratchArena) //pre-declared in gy_scratch_arenas.h
{
	NotNull(scratchArena);
	Assert(IsPntrInsideRange(scratchArena, &Platform->scratchArenas[0], sizeof(MemArena_t) * NUM_SCRATCH_ARENAS_PER_THREAD));
	if (scratchArena->size == 0) { return; }
	PopMemMark(scratchArena);
}
