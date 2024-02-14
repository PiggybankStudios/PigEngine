/*
File:   pig_graphics_api.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GRAPHICS_API_H
#define _PIG_GRAPHICS_API_H

u64 PigGfx_GetSupportedRenderApis(MemArena_t* memArena, RenderApi_t** apisOut);
bool PigGfx_Init(const PigGfxContext_t* context, MemArena_t* stateAllocArena, MemArena_t* mainAllocArena, RenderApi_t renderApi);
void PigGfx_FillDefaultOptions(PigGfxOptions_t* optionsOut);
void PigGfx_SetOptions(const PigGfxOptions_t* options);
#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowHints();
void PigGfx_SwitchToGlfwWindow(GLFWwindow* glfwWindowPntr);
#endif
void PigGfx_DestroyContext(GraphicsContext_t* context);
GraphicsContext_t* PigGfx_CreateContext(MemArena_t* memArena);

// +==============================+
// |       Render Functions       |
// +==============================+
void PigGfx_BeginRendering(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue);

#endif //  _PIG_GRAPHICS_API_H
