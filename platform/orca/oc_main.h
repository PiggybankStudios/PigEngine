/*
File:   oc_main.h
Author: Taylor Robbins
Date:   10\08\2024
*/

#ifndef _OC_MAIN_H
#define _OC_MAIN_H

struct PlatformState_t
{
	MemArena_t stdHeap;
	MemArena_t mainHeap;
	OC_Arena_t ocArena;
	
	OC_Surface_t surface;
	OC_CanvasRenderer_t renderer;
	OC_CanvasContext_t canvasContext;
	OC_UiContext_t ui;
	
	AppInput_t appInput;
	
	OC_Font_t debugFont;
};

#endif //  _OC_MAIN_H
