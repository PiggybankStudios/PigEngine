/*
File:   pig_input.h
Author: Taylor Robbins
Date:   10\30\2021
*/

#ifndef _PIG_INPUT_H
#define _PIG_INPUT_H

struct MouseHitInfo_t
{
	u64 priority; //0 means nothing is hovered, greater numbers are higher priority
	const PlatWindow_t* window;
	MyStr_t name;
	const void* pntr;
	u64 index;
};

struct BtnHandlingInfo_t
{
	bool pressHandled;
	bool releaseHandled;
	bool extendedHandled;
	#if DEBUG_BUILD
	MyStr_t filePath;
	u64 lineNumber;
	#endif
};

#endif //  _PIG_INPUT_H
