/*
File:   pig_graphics_api.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GRAPHICS_API_H
#define _PIG_GRAPHICS_API_H

struct PigGfxState_t
{
	bool initialized;
	RenderApi_t renderApi;
	#if PIG_GFX_GLFW_SUPPORTED
	GLFWwindow* glfwWindowPntr;
	#endif
};

extern PigGfxState_t* gfx;

u64 PigGfx_GetSupportedRenderApis(MemArena_t* memArena, RenderApi_t** apisOut);
bool PigGfx_Init(MemArena_t* pigGraphicsStateAllocArena, RenderApi_t renderApi);
#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowPntr(GLFWwindow* glfwWindowPntr);
#endif
bool PigGfx_CreateContextInWindow();

#endif //  _PIG_GRAPHICS_API_H
