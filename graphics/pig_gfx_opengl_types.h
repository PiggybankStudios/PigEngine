/*
File:   pig_gfx_opengl_types.h
Author: Taylor Robbins
Date:   02\13\2024
*/

#ifndef _PIG_GFX_OPENGL_TYPES_H
#define _PIG_GFX_OPENGL_TYPES_H

enum OpenGlProfile_t
{
	OpenGlProfile_None = 0,
	OpenGlProfile_Any,
	OpenGlProfile_Core,
	OpenGlProfile_Compat,
	OpenGlProfile_NumTypes,
};
const char* GetOpenGlProfileStr(OpenGlProfile_t enumValue);

typedef GLADloadproc GlLoadProc_f;

struct PigGfxOptions_OpenGL_t
{
	GlLoadProc_f loadProcFunction;
	int requestVersionMajor;
	int requestVersionMinor;
	OpenGlProfile_t requestProfile;
	bool requestForwardCompat;
	bool requestDebugContext;
};

struct GraphicsContext_OpenGL_t
{
	int versionMajor;
	int versionMinor;
};

#endif //  _PIG_GFX_OPENGL_TYPES_H
