/*
File:   web_main.h
Author: Taylor Robbins
Date:   03\18\2022
*/

#ifndef _WEB_MAIN_H
#define _WEB_MAIN_H

struct Web_PlatformState_t
{
	bool initialized;
	MemArena_t mainHeap;
	MemArena_t tempArena;
	
};

#endif //  _WEB_MAIN_H
