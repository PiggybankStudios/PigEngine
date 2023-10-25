/*
File:   web_main.cpp
Author: Taylor Robbins
Date:   10\15\2023
Description: 
	** This is the main file compiled for the web platform layer of Pig Engine.
	** This file includes other .h and .cpp that constitute the platform.
	** This file will be linked with pig_main.cpp to create game.wasm.
*/

#if 1
#define PLATFORM_LAYER
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#include "common_includes.h"
#else
#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy_defines_check.h"
#include "gylib/gy.h"
#endif

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "web_version.h"
#include "web_defines.h"
#include "web_js_types.h"
#include "web_render_types.h"
#include "web_main.h"

#include "web_js_funcs.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
WebPlatformState_t* Platform = nullptr;

#include "gylib/gy_temp_memory.cpp"

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "web_core.cpp"
#include "web_debug.cpp"
#include "web_render_types.cpp"
#include "web_simple_shaders.cpp"

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
// void TestFileDownloaded(const char* filePath, u32 fileSize, const u8* fileContentsPntr)
FILE_DOWNLOADED_CALLBACK_DEF(TestFileDownloaded)
{
	PrintLine_D("Downloaded \"%s\" %u bytes %p", filePath, fileSize, fileContentsPntr);
	for (u64 bIndex = 0; bIndex+4 <= fileSize && bIndex < 100; bIndex += 4)
	{
		PrintLine_D("%08X: %02X %02X %02X %02X", (u32)(fileContentsPntr + bIndex), fileContentsPntr[bIndex+0], fileContentsPntr[bIndex+1], fileContentsPntr[bIndex+2], fileContentsPntr[bIndex+3]);
	}
}

// +--------------------------------------------------------------+
// |                      Exported Functions                      |
// +--------------------------------------------------------------+
// +==============================+
// |        WasmInitialize        |
// +==============================+
WASM_EXPORTED_FUNC(void, WasmInitialize)
{
	u64 initialMemUsage = jsStdGetHeapSize();
	Web_MemoryInit();
	TempPushMark();
	MemArena_t* scratch = GetScratchArena();
	
	Web_DebugInit();
	WriteLine_O("+======================================+");
	PrintLine_O("| Pig Engine (Web Assembly) v%u.%u(%02u)  |", WEB_VERSION_MAJOR, WEB_VERSION_MINOR, WEB_VERSION_BUILD);
	WriteLine_O("+======================================+");
	PrintLine_D("Initial Memory Usage: %llu (0x%08X) %llukB", initialMemUsage, initialMemUsage, initialMemUsage/1024);
	
	Platform->programTime = 0;
	
	//TODO: Call out to PigEngine functions to ask about StartupOptions, use that to determine the resolution we should request for the canvas
	Platform->canvasSize = NewVec2i(800, 600);
	jsInitRendering(Platform->canvasSize.width, Platform->canvasSize.height);
	
	Platform->testShader = Web_CreateShader(WebSimpleVertexShader, WebSimpleFragmentShader);
	
	Web_Vertex2D_t squareVerts[] = {
		{ { 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0 }, },
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 }, },
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 }, },
		
		{ { 1, 1, 0 }, { 1, 1, 1, 1 }, { 1, 1 }, },
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 }, },
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 }, },
	};
	if (!Web_CreateVertBuffer2D(&Platform->mainHeap, &Platform->squareBuffer, false, 6, &squareVerts, true))
	{
		WriteLine_E("Failed to create VertBuffer!");
	}
	
	Platform->testVao = Web_CreateVertexArrayObject(Web_VertexType_Default2D);
	
	Platform->downloadingId = jsDownloadFile("Resources/manifest_all.txt", &Platform->chunkArena, TestFileDownloaded);
	PrintLine_D("Progress[%u] at start: %f", Platform->downloadingId, jsGetAsyncInfo(Platform->downloadingId));
	
	u32 testTextureData[] = {
		TransparentWhite_Value,   PureRed_Value,          PureOrange_Value,       TransparentWhite_Value,
		MonokaiBrown_Value,       TransparentWhite_Value, TransparentWhite_Value, PureYellow_Value,
		PalPink_Value,            TransparentWhite_Value, TransparentWhite_Value, PureGreen_Value,
		TransparentWhite_Value,   PurePurple_Value,       PureBlue_Value,         TransparentWhite_Value,
	};
	Platform->testTexture = Web_CreateTexture(NewVec2i(4, 4), (u8*)testTextureData, true, false);
	
	// const char* jsString = jsGetString(scratch);
	// PrintLine_D("jsString: \"%s\" (%p)", jsString, jsString);
	// for (int bIndex = 0; bIndex < 10; bIndex++)
	// {
	// 	PrintLine_D("%08X = %02X", (u32)(jsString + bIndex), CharToU32(*(jsString + bIndex)));
	// }
	
	Platform->initialized = true;
	u64 endMemUsage = jsStdGetHeapSize();
	PrintLine_D("End Memory Usage: %llu (0x%08X) %llukB", endMemUsage, endMemUsage, endMemUsage/1024);
	FreeScratchArena(scratch);
	TempPopMark();
	AssertMsg(GetNumMarks(&Platform->tempArena) == 0, "TempArena mark count is not 0 at end of WasmInitialize!");
}

// +==============================+
// |     WasmUpdateAndRender      |
// +==============================+
WASM_EXPORTED_FUNC(void, WasmUpdateAndRender)
{
	NotNull(Platform);
	TempPushMark();
	Platform->programTime += 16; //TODO: Get the time elapsed from javascript?
	
	if (Platform->downloadingId != 0)
	{
		r32 progress = jsGetAsyncInfo(Platform->downloadingId);
		PrintLine_D("Progress[%u]: %f", Platform->downloadingId, progress);
		if (progress >= 1.0f || progress <= -1.0f) { Platform->downloadingId = 0; }
	}
	
	jsGlViewport(0, 0, Platform->canvasSize.width, Platform->canvasSize.height);
	jsGlEnable(GL_BLEND);
	
	Color_t clearColor = MonokaiBack;
	clearColor.r = ColorChannelToU8(OscillateBy(Platform->programTime, 0, 1, (Platform->mouseLeftBtnDown ? 200 : 3000)));
	jsGlClearColor(ColorChannelToR32(clearColor.r), ColorChannelToR32(clearColor.g), ColorChannelToR32(clearColor.b), ColorChannelToR32(clearColor.a));
	jsGlClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	jsGlUseProgram(Platform->testShader.glId); Web_AssertNoOpenGlError();
	mat4 worldMatrix = Mat4RotateZ(OscillateBy(Platform->programTime, 0, TwoPi32, (Platform->mouseLeftBtnDown ? 2000 : 10000)));
	mat4 viewMatrix = Mat4_Identity;
	mat4 projectionMatrix = Mat4_Identity;
	jsGlUniform4f(Platform->testShader.uniforms.color1, 1, 1, OscillateBy(Platform->programTime, 0, 1, 1000), 1); Web_AssertNoOpenGlError();
	jsGlUniformMatrix4fv(Platform->testShader.uniforms.worldMatrix,      1, GL_FALSE, (GLfloat*)&worldMatrix); Web_AssertNoOpenGlError();
	jsGlUniformMatrix4fv(Platform->testShader.uniforms.viewMatrix,       1, GL_FALSE, (GLfloat*)&viewMatrix); Web_AssertNoOpenGlError();
	jsGlUniformMatrix4fv(Platform->testShader.uniforms.projectionMatrix, 1, GL_FALSE, (GLfloat*)&projectionMatrix); Web_AssertNoOpenGlError();
	jsGlActiveTexture(GL_TEXTURE0);
	jsGlBindTexture(GL_TEXTURE_2D, Platform->testTexture.glId);
	jsGlUniform1i(Platform->testShader.uniforms.texture, 0);
	jsGlUniform2f(Platform->testShader.uniforms.textureSize, Platform->testTexture.width, Platform->testTexture.height);
	jsGlBindBuffer(GL_ARRAY_BUFFER, Platform->squareBuffer.glId);
	Web_BindVertexArrayObject(&Platform->testVao, &Platform->testShader, &Platform->squareBuffer);
	
	jsGlDrawArrays(GL_TRIANGLES, 0, (GLsizei)Platform->squareBuffer.numVertices);
	
	TempPopMark();
	AssertMsg(GetNumMarks(&Platform->tempArena) == 0, "TempArena mark count is not 0 at end of WasmUpdateAndRender!");
}

// +==============================+
// |         WasmAllocMem         |
// +==============================+
WASM_EXPORTED_FUNC(void*, WasmAllocMem, MemArena_t* memArenaPntr, uint32_t numBytes)
{
	void* result = AllocMem(memArenaPntr, numBytes);
	PrintLine_D("WasmAllocMem(%p, %u) => %p", memArenaPntr, numBytes, result);
	return result;
}
// +==============================+
// |         WasmFreeMem          |
// +==============================+
WASM_EXPORTED_FUNC(void, WasmFreeMem, MemArena_t* memArenaPntr, void* allocPntr, uint32_t allocSize)
{
	PrintLine_D("FreeMem(%p, %p, %u)", memArenaPntr, allocPntr, allocSize);
	FreeMem(memArenaPntr, allocPntr, allocSize);
}

// +==============================+
// |   WasmFileFinishedDownload   |
// +==============================+
WASM_EXPORTED_FUNC(void, WasmFileFinishedDownload, const char* filePath, u32 fileSize, const u8* fileContentsPntr, FileDownloadedCallback_f* callbackPntr)
{
	NotNull(callbackPntr);
	PrintLine_D("WasmFileFinishedDownload(\"%s\", %u, %p, %p)", filePath, fileSize, fileContentsPntr, callbackPntr);
	if (callbackPntr != nullptr)
	{
		callbackPntr(filePath, fileSize, fileContentsPntr);
	}
}
