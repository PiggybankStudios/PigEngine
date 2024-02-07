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

#if PIG_GFX_OPENGL_SUPPORTED
enum OpenGlProfile_t
{
	OpenGlProfile_None = 0,
	OpenGlProfile_Any,
	OpenGlProfile_Core,
	OpenGlProfile_Compat,
	OpenGlProfile_NumTypes,
};
const char* GetOpenGlProfileStr(OpenGlProfile_t enumValue);
#endif

typedef GLADloadproc GlLoadProc_f;

struct PigGfxOptions_t
{
	//TODO: These options COULD be different per-window, so they aren't really like "global" options like the request versions and whatnot below.
	//      For now though, we'll assume all windows share the same basic graphical settings in an application
	bool doubleBuffered;
	u8 bitDepth; //32 or 24
	u8 depthBufferBitDepth;
	u8 stencilBufferBitDepth;
	u8 numAntialiasingSamples;
	
	#if PIG_GFX_OPENGL_SUPPORTED
	GlLoadProc_f opengl_loadProcFunction;
	int opengl_RequestVersionMajor;
	int opengl_RequestVersionMinor;
	OpenGlProfile_t opengl_RequestProfile;
	bool opengl_RequestForwardCompat;
	bool opengl_requestDebugContext;
	#endif
};

struct GraphicsContext_t
{
	RenderApi_t renderApi;
	#if PIG_GFX_OPENGL_SUPPORTED
	int openglVersionMajor;
	int openglVersionMinor;
	#endif
};

#endif //  _PIG_GRAPHICS_TYPES_H
