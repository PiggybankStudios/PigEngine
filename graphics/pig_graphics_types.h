/*
File:   pig_graphics_types.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GRAPHICS_TYPES_H
#define _PIG_GRAPHICS_TYPES_H

enum RenderApi_t
{
	RenderApi_None = 0,
	RenderApi_OpenGL,
	RenderApi_WebGL,
	RenderApi_Vulkan,
	RenderApi_D3D11,
	RenderApi_D3D12,
	RenderApi_Metal,
	RenderApi_NumModes,
};
const char* GetRenderApiStr(RenderApi_t enumValue);

#endif //  _PIG_GRAPHICS_TYPES_H
