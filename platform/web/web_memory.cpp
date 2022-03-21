/*
File:   web_memory.cpp
Author: Taylor Robbins
Date:   03\18\2022
Description: 
	** Holds an implementation of malloc that works in our web assembly environment
	** where the memory is all stored in one contiguous block that can only grow
*/

// +--------------------------------------------------------------+
// |                       malloc function                        |
// +--------------------------------------------------------------+
u8* heapBase;
int heapUsed;
int heapSize;
void* malloc(size_t bytes)
{
	if (heapUsed + bytes > heapSize)
	{
		int numNewPagesNeeded = CeilDivI32((heapUsed + bytes) - heapSize, WASM_MEM_PAGE_SIZE);
		Assert(numNewPagesNeeded > 0);
		js_GrowHeap(numNewPagesNeeded);
		heapSize += numNewPagesNeeded * WASM_MEM_PAGE_SIZE;
	}
	Assert(heapUsed + bytes <= heapSize);
	void* address = (void*)(heapBase + heapUsed);
	heapUsed += (int)bytes;
	return address;
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
void* Web_ArenaAllocateCallback(u64 numBytes)
{
	return malloc((size_t)numBytes);
}

void Web_ArenaFreeCallback(void* memPntr)
{
	// NOTE: Web Assembly memory model doesn't actually support freeing anything.
	//       Our memory only grows, never shrinks.
	Assert_(false);
}

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void Web_MemoryInit(int heapBaseAddress)
{
	heapBase = (u8*)heapBaseAddress;
	heapUsed = 0;
	heapSize = js_GetHeapSize();
	
	u8* tempArenaSpace = (u8*)malloc(PLAT_TEMP_ARENA_SIZE);
	NotNull_(tempArenaSpace);
	
	Platform = (Web_PlatformState_t*)malloc(sizeof(Web_PlatformState_t));
	NotNull_(Platform);
	ClearPointer(Platform);
	Platform->initialized = true;
	
	InitMemArena_PagedHeapFuncs(&Platform->mainHeap, PLAT_MAIN_HEAP_PAGE_SIZE, Web_ArenaAllocateCallback, Web_ArenaFreeCallback);
	InitMemArena_MarkedStack(&Platform->tempArena, PLAT_TEMP_ARENA_SIZE, tempArenaSpace, PLAT_TEMP_ARENA_MAX_MARKS);
	TempArena = &Platform->tempArena;
}

