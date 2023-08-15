/*
File:   linux_main.cpp
Author: Taylor Robbins
Date:   08\11\2023
Description: 
	** This is a test file currently. It does not actually compile any of Pig Engine. It's used mostly to exercise gylib on linux (clang) through WSL
*/

#include <stdio.h>

#include "gylib/gy_defines_check.h"
#include "gylib/gy_basic_macros.h"

#define GYLIB_LOOKUP_PRIMES_10
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#include "gylib/gy.h"

#include "gylib/gy_scratch_arenas.cpp"

//NOTE: This is 

int main()
{
	InitThreadLocalScratchArenas(Gigabytes(64), 256);
	
	printf("\nHello World!\n\n");
	
	MemArena_t* scratch = GetScratchArena();
	printf("Got scratch arena %p (%lu/%lu)\n", scratch, scratch->used, scratch->size);
	
	void* alloc1 = AllocMem(scratch, Kilobytes(24));
	void* alloc2 = AllocMem(scratch, Kilobytes(1));
	printf("alloc1: %p\n", alloc1);
	printf("alloc2: %p\n", alloc2);
	printf("Arena Usage: %lu/%lu\n", scratch->used, scratch->size);
	FreeMem(scratch, alloc2, Kilobytes(1));
	printf("Arena Usage: %lu/%lu\n", scratch->used, scratch->size);
	FreeMem(scratch, alloc1, Kilobytes(24));
	printf("Arena Usage: %lu/%lu\n", scratch->used, scratch->size);
	
	u64 memPageSize = OsGetMemoryPageSize();
	printf("Memory Page Size: %s\n", FormatBytesNt(memPageSize, scratch));
	u64 reserveMemCommitted = 0;
	u64 reserveMemSize = Megabytes(1);
	u8* reserveMemPntr = (u8*)OsReserveMemory(reserveMemSize);
	printf("Reserved %s: %p\n", FormatBytesNt(reserveMemSize, scratch), reserveMemPntr);
	OsCommitReservedMemory(reserveMemPntr, 2*memPageSize); reserveMemCommitted += 2*memPageSize;
	printf("Committed %s: up to %p\n", FormatBytesNt(reserveMemCommitted, scratch), reserveMemPntr + reserveMemCommitted);
	MyMemSet(reserveMemPntr, 0x11, reserveMemCommitted + Kilobytes(256));
	printf("Wrote some bytes...\n");
	
	FreeScratchArena(scratch);
	
	printf("\n");
	return 0;
}