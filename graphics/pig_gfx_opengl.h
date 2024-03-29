/*
File:   pig_gfx_opengl.h
Author: Taylor Robbins
Date:   02\05\2024
*/

#ifndef _PIG_GFX_OPENGL_H
#define _PIG_GFX_OPENGL_H

bool PigGfx_Init_OpenGL();
#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowHints_OpenGL();
#endif;
void PigGfx_DestroyContext_OpenGL(PigGfxContext_t* context);
PigGfxContext_t* PigGfx_CreateContext_OpenGL(MemArena_t* memArena);

void PigGfx_BeginRendering_OpenGL(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue);

#endif //  _PIG_GFX_OPENGL_H
