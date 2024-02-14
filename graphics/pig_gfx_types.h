/*
File:   pig_gfx_types.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GFX_TYPES_H
#define _PIG_GFX_TYPES_H

#define PIG_GFX_DEBUG_OUTPUT_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
typedef PIG_GFX_DEBUG_OUTPUT_DEF(PigGfxDebugOutput_f);
#define PIG_GFX_DEBUG_PRINT_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
typedef PIG_GFX_DEBUG_PRINT_DEF(PigGfxDebugPrint_f);
#define PIG_GFX_INIT_FAILURE_DEF(functionName) void functionName(const char* message)
typedef PIG_GFX_INIT_FAILURE_DEF(PigGfxInitFailure_f);

struct PigGfxCallbacks_t
{
	PigGfxInitFailure_f* InitFailure;
	PigGfxDebugOutput_f* DebugOutput;
	PigGfxDebugPrint_f* DebugPrint;
};

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

// +--------------------------------------------------------------+
// |                     API Dependent Types                      |
// +--------------------------------------------------------------+
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
	PigGfxOptions_OpenGL_t gl;
	#endif
	#if PIG_GFX_VULKAN_SUPPORTED
	PigGfxOptions_Vulkan_t vk;
	#endif
};

struct PigGfxContext_t
{
	RenderApi_t renderApi;
	MemArena_t* allocArena;
	#if PIG_GFX_OPENGL_SUPPORTED
	PigGfxContext_OpenGL_t gl;
	#endif
	#if PIG_GFX_VULKAN_SUPPORTED
	PigGfxContext_Vulkan_t vk;
	#endif
};

// +--------------------------------------------------------------+
// |                      Main State Global                       |
// +--------------------------------------------------------------+
struct PigGfxState_t
{
	bool initialized;
	RenderApi_t renderApi;
	MemArena_t* mainArena;
	
	bool callbacksSet;
	PigGfxCallbacks_t callbacks;
	bool optionsSet;
	PigGfxOptions_t options;
	
	#if PIG_GFX_GLFW_SUPPORTED
	GLFWwindow* currentGlfwWindow;
	#endif
	
	bool contextCreated;
	PigGfxContext_t context;
	
	#if PIG_GFX_VULKAN_SUPPORTED
	VkTestContent_t vkTest;
	#endif
};

extern PigGfxState_t* gfx;

#endif //  _PIG_GFX_TYPES_H
