/*
File:   pig_graphics_types.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GRAPHICS_TYPES_H
#define _PIG_GRAPHICS_TYPES_H

#define PIG_GFX_DEBUG_OUTPUT_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
typedef PIG_GFX_DEBUG_OUTPUT_DEF(PigGfxDebugOutput_f);
#define PIG_GFX_DEBUG_PRINT_DEF(functionName) void functionName(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
typedef PIG_GFX_DEBUG_PRINT_DEF(PigGfxDebugPrint_f);
#define PIG_GFX_INIT_FAILURE_DEF(functionName) void functionName(const char* message)
typedef PIG_GFX_INIT_FAILURE_DEF(PigGfxInitFailure_f);

struct PigGfxContext_t
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

#if PIG_GFX_OPENGL_SUPPORTED
typedef GLADloadproc GlLoadProc_f;
#endif

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
	
	#if PIG_GFX_VULKAN_SUPPORTED
	const char* vulkan_ApplicationName;
	u32 vulkan_ApplicationVersionInt;
	const char* vulkan_EngineName;
	u32 vulkan_EngineVersionInt;
	int vulkan_RequestVersionMajor;
	int vulkan_RequestVersionMinor;
	#endif
};

struct GraphicsContext_t
{
	RenderApi_t renderApi;
	MemArena_t* allocArena;
	#if PIG_GFX_OPENGL_SUPPORTED
	int openglVersionMajor;
	int openglVersionMinor;
	#endif
	#if PIG_GFX_VULKAN_SUPPORTED
	VkAllocationCallbacks vkAllocatorStruct;
	VkAllocationCallbacks* vkAllocator;
	VkInstance vkInstance;
	VkSurfaceFormatKHR vkSurfaceFormat;
	VkSurfaceKHR vkSurface;
	VkPhysicalDevice vkPhysicalDevice;
	u32 queueFamilyIndex;
	VkExtent2D swapExtent;
	VkDevice vkDevice;
	VkQueue vkQueue;
	VkSwapchainKHR vkSwapchain;
	u32 vkSwapImageCount;
	VkImageView* vkSwapImageViews;
	#endif
};

#endif //  _PIG_GRAPHICS_TYPES_H
