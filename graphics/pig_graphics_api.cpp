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

void PigGfx_FillDefaultOptions(PigGfxOptions_t* optionsOut)
{
	NotNull(optionsOut);
	ClearPointer(optionsOut);
	optionsOut->doubleBuffered = true;
	optionsOut->bitDepth = 32;
	optionsOut->depthBufferBitDepth = 8;
	optionsOut->stencilBufferBitDepth = 8;
	optionsOut->numAntialiasingSamples = 4;
	#if PIG_GFX_OPENGL_SUPPORTED
	#if PIG_GFX_GLFW_SUPPORTED
	optionsOut->opengl_loadProcFunction = (GlLoadProc_f)glfwGetProcAddress;
	#endif
	optionsOut->opengl_RequestVersionMajor = 3;
	optionsOut->opengl_RequestVersionMinor = 3;
	optionsOut->opengl_RequestProfile = OpenGlProfile_Core;
	optionsOut->opengl_RequestForwardCompat = true;
	optionsOut->opengl_requestDebugContext = false;
	#endif
}

void PigGfx_SetOptions(const PigGfxOptions_t* options)
{
	NotNull(gfx);
	Assert(gfx->initialized);
	NotNull(options);
	//TODO: Any validation of the options needed here?
	MyMemCopy(&gfx->options, options, sizeof(PigGfxOptions_t));
	gfx->optionsSet = true;
}

#if PIG_GFX_GLFW_SUPPORTED

void PigGfx_SetGlfwWindowHints()
{
	NotNull(gfx);
	Assert(gfx->initialized);
	Assert(gfx->optionsSet);
	
	switch(gfx->renderApi)
	{
		#if PIG_GFX_OPENGL_SUPPORTED
		case RenderApi_OpenGL: PigGfx_SetGlfwWindowHints_OpenGL(); break;
		#endif
		#if PIG_GFX_WEBGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_SetGlfwWindowHints_WebGL();
		#endif
		#if PIG_GFX_VULKAN_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_SetGlfwWindowHints_Vulkan();
		#endif
		#if PIG_GFX_D3D11_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_SetGlfwWindowHints_D3D11();
		#endif
		#if PIG_GFX_D3D12_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_SetGlfwWindowHints_D3D12();
		#endif
		#if PIG_GFX_METAL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_SetGlfwWindowHints_Metal();
		#endif
		default: AssertMsg(false, "Unsupported renderApi in PigGfx_SetGlfwWindowHints!"); break;
	}
	
	glfwWindowHint(GLFW_DOUBLEBUFFER, gfx->options.doubleBuffered ? GL_TRUE : GL_FALSE);
	if (gfx->options.bitDepth == 32)
	{
		glfwWindowHint(GLFW_RED_BITS,   8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS,  8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
	}
	else if (gfx->options.bitDepth == 24)
	{
		glfwWindowHint(GLFW_RED_BITS,   8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS,  8);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
	}
	else { AssertMsg(false, "Unsupported value passed for bitDepth in PigGfx_SetOptions!"); }
	glfwWindowHint(GLFW_DEPTH_BITS, gfx->options.depthBufferBitDepth);
	glfwWindowHint(GLFW_STENCIL_BITS, gfx->options.stencilBufferBitDepth);
	glfwWindowHint(GLFW_SAMPLES, (int)gfx->options.numAntialiasingSamples);
}

void PigGfx_SwitchToGlfwWindow(GLFWwindow* glfwWindowPntr)
{
	NotNull(gfx);
	Assert(gfx->initialized);
	gfx->currentGlfwWindow = glfwWindowPntr;
	glfwMakeContextCurrent(glfwWindowPntr);
}
#endif

GraphicsContext_t* PigGfx_CreateContext()
{
	NotNull(gfx);
	Assert(gfx->initialized);
	switch (gfx->renderApi)
	{
		#if PIG_GFX_OPENGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_OpenGL();
		#endif
		#if PIG_GFX_WEBGL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_WebGL();
		#endif
		#if PIG_GFX_VULKAN_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_Vulkan();
		#endif
		#if PIG_GFX_D3D11_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_D3D11();
		#endif
		#if PIG_GFX_D3D12_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_D3D12();
		#endif
		#if PIG_GFX_METAL_SUPPORTED
		case RenderApi_OpenGL: return PigGfx_CreateContext_Metal();
		#endif
		default: return false;
	}
}

// +--------------------------------------------------------------+
// |                       Render Functions                       |
// +--------------------------------------------------------------+
void PigGfx_BeginRendering(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue)
{
	NotNull(gfx);
	Assert(gfx->initialized);
	Assert(gfx->contextCreated);
	switch (gfx->renderApi)
	{
		#if PIG_GFX_OPENGL_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_OpenGL(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		#if PIG_GFX_WEBGL_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_WebGL(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		#if PIG_GFX_VULKAN_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_Vulkan(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		#if PIG_GFX_D3D11_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_D3D11(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		#if PIG_GFX_D3D12_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_D3D12(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		#if PIG_GFX_METAL_SUPPORTED
		case RenderApi_OpenGL: PigGfx_BeginRendering_Metal(doClearColor, clearColor, doClearDepth, clearDepth, doClearStencil, clearStencilValue); break;
		#endif
		default: AssertMsg(false, "Unsupported renderApi in PigGfx_BeginRendering!"); break;
	}
}
