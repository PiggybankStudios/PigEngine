/*
File:   win32_core_shared_types.h
Author: Taylor Robbins
Date:   03\17\2024
Description:
	** These types get #included before any external libraries (which includes GyLib)
	** so they could be used by those libraries (For example PlatMutex_t the backing type for GyMutex_t)
*/

#ifndef _WIN_32_CORE_SHARED_TYPES_H
#define _WIN_32_CORE_SHARED_TYPES_H

struct PlatMutex_t
{
	unsigned long long id;
	
	HANDLE handle;
};

struct PlatSemaphore_t
{
	unsigned long long id;
	
	HANDLE handle;
};

struct PlatInterlockedInt_t
{
	unsigned long long id;
	
	volatile LONG value;
};

#endif //  _WIN_32_CORE_SHARED_TYPES_H
