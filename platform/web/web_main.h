/*
File:   web_main.h
Author: Taylor Robbins
Date:   10\15\2023
*/

#ifndef _WEB_MAIN_H
#define _WEB_MAIN_H

struct WebPlatformState_t
{
	bool initialized;
	
	MemArena_t stdHeap;
	MemArena_t mainHeap;
	MemArena_t tempArena;
	MemArena_t scratchArenas[NUM_SCRATCH_ARENAS_PER_THREAD];
	
	v2i canvasSize;
	u64 programTime;
	bool mouseLeftBtnDown;
	
	Web_Shader_t testShader;
	Web_VertBuffer_t squareBuffer;
	Web_Texture_t testTexture;
	Web_VertexArrayObject_t testVao;
};

#endif //  _WEB_MAIN_H
