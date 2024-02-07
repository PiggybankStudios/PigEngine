/*
File:   pig_graphics_api.cpp
Author: Taylor Robbins
Date:   02\06\2024
Description: 
	** Holds functions that will be the primary method by which the application interfaces with Pig Graphics
	** Most of these methods are wrappers for each of the underlying implementation methods
*/

PigGfxState_t* gfx = nullptr;

u64 PigGfx_GetSupportedRenderApis(MemArena_t* memArena, RenderApi_t** apisOut)
{
	u64 resultCount = 0
		+ (PIG_GFX_OPENGL_SUPPORTED  ? 1 : 0)
		+ (PIG_GFX_WEBGL_SUPPORTED   ? 1 : 0)
		+ (PIG_GFX_VULKAN_SUPPORTED  ? 1 : 0)
		+ (PIG_GFX_D3D11_SUPPORTED   ? 1 : 0)
		+ (PIG_GFX_D3D12_SUPPORTED   ? 1 : 0)
		+ (PIG_GFX_METAL_SUPPORTED   ? 1 : 0);
	
	if (memArena == nullptr || resultCount == 0) { *apisOut = nullptr; return resultCount; }
	*apisOut = AllocArray(memArena, RenderApi_t, resultCount);
	if (*apisOut == nullptr) { return resultCount; }
	
	u64 resultIndex = 0;
	if (PIG_GFX_OPENGL_SUPPORTED)  { (*apisOut)[resultIndex] = RenderApi_OpenGL;  resultIndex++; }
	if (PIG_GFX_WEBGL_SUPPORTED)   { (*apisOut)[resultIndex] = RenderApi_WebGL;   resultIndex++; }
	if (PIG_GFX_VULKAN_SUPPORTED)  { (*apisOut)[resultIndex] = RenderApi_Vulkan;  resultIndex++; }
	if (PIG_GFX_D3D11_SUPPORTED)   { (*apisOut)[resultIndex] = RenderApi_D3D11;   resultIndex++; }
	if (PIG_GFX_D3D12_SUPPORTED)   { (*apisOut)[resultIndex] = RenderApi_D3D12;   resultIndex++; }
	if (PIG_GFX_METAL_SUPPORTED)   { (*apisOut)[resultIndex] = RenderApi_Metal;   resultIndex++; }
	Assert(resultIndex == resultCount);
	
	return resultCount;
}

bool PigGfx_Init(MemArena_t* pigGraphicsStateAllocArena, RenderApi_t renderApi)
{
	NotNull(pigGraphicsStateAllocArena);
	
	gfx = AllocStruct(pigGraphicsStateAllocArena, PigGfxState_t);
	NotNull(gfx);
	ClearPointer(gfx);
	gfx->renderApi = renderApi;
	gfx->initialized = true;
	
	switch (renderApi)
	{
		#if PIG_GFX_OPENGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_Init_OpenGL();
		#endif
		#if PIG_GFX_WEBGL_SUPPORTED
		case RenderApi_WebGL: return PigGfx_Init_WebGL();
		#endif
		#if PIG_GFX_VULKAN_SUPPORTED
		case RenderApi_Vulkan: return PigGfx_Init_Vulkan();
		#endif
		#if PIG_GFX_D3D11_SUPPORTED
		case RenderApi_D3D11: return PigGfx_Init_D3D11();
		#endif
		#if PIG_GFX_D3D12_SUPPORTED
		case RenderApi_D3D12: return PigGfx_Init_D3D12();
		#endif
		#if PIG_GFX_METAL_SUPPORTED
		case RenderApi_Metal: return PigGfx_Init_Metal();
		#endif
		default: AssertMsg(false, "Unsupported RenderApi passed to PigGfx_Init! Make sure you are asking for an API that was reported by PigGfx_GetSupportedRenderApis"); return false;
	}
}

#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowPntr(GLFWwindow* glfwWindowPntr)
{
	NotNull(gfx);
	Assert(gfx->initialized);
	gfx->glfwWindowPntr = glfwWindowPntr;
}
#endif

bool PigGfx_CreateContextInWindow()
{
	NotNull(gfx);
	Assert(gfx->initialized);
	switch (gfx->renderApi)
	{
		#if PIG_GFX_OPENGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_OpenGL();
		#endif
		#if PIG_GFX_WEBGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_WebGL();
		#endif
		#if PIG_GFX_VULKAN_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_Vulkan();
		#endif
		#if PIG_GFX_D3D11_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_D3D11();
		#endif
		#if PIG_GFX_D3D12_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_D3D12();
		#endif
		#if PIG_GFX_METAL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContextInWindow_Metal();
		#endif
		default: return false;
	}
}
