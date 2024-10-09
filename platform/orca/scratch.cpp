/*
File:   scratch.cpp
Author: Taylor Robbins
Date:   10\07\2024
Description: 
	** Holds the Orca implementation for Scratch MemArenas.
	** Since Orca applications are single threaded, we don't have to worry about
	** doing multiple thread_local arena arrays, we can just have a single array
	** of arenas for the main thread.
*/

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
MemArena_t ScratchArenasGlobalArray[NUM_SCRATCH_ARENAS_PER_THREAD];

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void InitScratchArenas(MemArena_t* sourceArena, u64 scratchPageSize, u64 scratchMaxMarks, AllocAlignment_t alignment = AllocAlignment_None)
{
	AssertIf(scratchPageSize > 0, sourceArena != nullptr);
	for (u64 aIndex = 0; aIndex < ArrayCount(ScratchArenasGlobalArray); aIndex++)
	{
		if (scratchPageSize > 0)
		{
			InitMemArena_PagedStackArena(&ScratchArenasGlobalArray[aIndex], scratchPageSize, sourceArena, scratchMaxMarks, alignment);
		}
		else
		{
			ClearStruct(ScratchArenasGlobalArray[aIndex]);
		}
	}
}

// +--------------------------------------------------------------+
// |                        Main Interface                        |
// +--------------------------------------------------------------+
// Pass in 1 or 2 arenas that you DON'T want to get back from this function. i.e. you are going to use those arenas for memory that outlives your scratch memory
inline MemArena_t* GetScratchArena(MemArena_t* avoidConflictWith1, MemArena_t* avoidConflictWith2) //pre-declared in gy_scratch_arenas.h
{
	u64 scratchIndex = 0;
	while ((avoidConflictWith1 != nullptr && avoidConflictWith1 == &ScratchArenasGlobalArray[scratchIndex]) ||
	       (avoidConflictWith2 != nullptr && avoidConflictWith2 == &ScratchArenasGlobalArray[scratchIndex]))
	{
		scratchIndex++;
	}
	Assert(scratchIndex < NUM_SCRATCH_ARENAS_PER_THREAD);
	MemArena_t* result = &ScratchArenasGlobalArray[scratchIndex];
	if (!IsInitialized(result)) { return nullptr; }
	PushMemMark(result);
	return result;
}

inline void FreeScratchArena(MemArena_t* scratchArena) //pre-declared in gy_scratch_arenas.h
{
	NotNull(scratchArena);
	Assert(IsPntrInsideRange(scratchArena, &ScratchArenasGlobalArray[0], sizeof(MemArena_t) * NUM_SCRATCH_ARENAS_PER_THREAD));
	if (scratchArena->size == 0) { return; }
	PopMemMark(scratchArena);
}
