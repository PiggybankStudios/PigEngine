/*
File:   pig_gfx_vulkan.h
Author: Taylor Robbins
Date:   02\05\2024
*/

#ifndef _PIG_GFX_VULKAN_H
#define _PIG_GFX_VULKAN_H

bool PigGfx_Init_Vulkan();
void PigGfx_SetGlfwWindowHints_Vulkan();
void PigGfx_DestroyContext_Vulkan(GraphicsContext_t* context);
GraphicsContext_t* PigGfx_CreateContext_Vulkan(MemArena_t* memArena);

void PigGfx_BeginRendering_Vulkan(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue);

#endif //  _PIG_GFX_VULKAN_H
