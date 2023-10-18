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
#include "web_render_types.h"
#include "web_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
WebPlatformState_t* Platform = nullptr;

#include "gylib/gy_temp_memory.cpp"

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "web_js_funcs.cpp"
#include "web_core.cpp"
#include "web_debug.cpp"
// #include "web_render_types.cpp"

extern "C" uint32_t GetHeapBaseAddress();

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
	
	Web_DebugInit();
	WriteLine_O("+======================================+");
	PrintLine_O("| Pig Engine (Web Assembly) v%u.%u(%02u)  |", WEB_VERSION_MAJOR, WEB_VERSION_MINOR, WEB_VERSION_BUILD);
	WriteLine_O("+======================================+");
	PrintLine_D("Initial Memory Usage: %llu (0x%08X) %llukB", initialMemUsage, initialMemUsage, initialMemUsage/1024);
	
	Platform->programTime = 0;
	
	//Went up by 2,097,152 aka 2048kB aka 32 pages
	Platform->initialized = true;
	u64 endMemUsage = jsStdGetHeapSize();
	PrintLine_D("End Memory Usage: %llu (0x%08X) %llukB", endMemUsage, endMemUsage, endMemUsage/1024);
	TempPopMark();
	AssertMsg(GetNumMarks(&Platform->tempArena) == 0, "TempArena mark count is not 0 at end of WasmInitialize!");
	
	//Heap Base 67136 (0x10640) aka 1 page + 1,600 bytes
	//Got to 458752 (0x70000)
	// jsPrintInteger("HeapBaseAddress", GetHeapBaseAddress());
	// int startAddress = GetHeapBaseAddress();
	// if ((startAddress % WASM_MEMORY_PAGE_SIZE) != 0) { startAddress += WASM_MEMORY_PAGE_SIZE - (startAddress % WASM_MEMORY_PAGE_SIZE); }
	// for (int bIndex = startAddress; bIndex < WASM_MEMORY_PAGE_SIZE*7; bIndex += Kilobytes(1))
	// {
	// 	jsPrintInteger("Address", bIndex);
	// 	// *(u8*)(bIndex) = 0xDD;
	// 	memset((void*)(bIndex), 0xDD, Kilobytes(1)-1);
	// }
	// jsPrintString("Finished", "Up to 64 pages!");
}

// +==============================+
// |     WasmUpdateAndRender      |
// +==============================+
WASM_EXPORTED_FUNC(void, WasmUpdateAndRender)
{
	NotNull(Platform);
	TempPushMark();
	Platform->programTime += 16; //TODO: Get the time elapsed from javascript?
	
	jsGlViewport(0, 0, Platform->canvasSize.width, Platform->canvasSize.height);
	
	// jsGlClearColor(OscillateBy(Platform->programTime, 0, 1, (Platform->mouseLeftBtnDown ? 200 : 1000)), 0, 0, 1);
	// jsGlClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	// jsGlUseProgram(Platform->testShader.glId);
	// mat4 worldMatrix = Mat4RotateZ(OscillateBy(Platform->programTime, 0, TwoPi32, (Platform->mouseLeftBtnDown ? 2000 : 10000)));
	// mat4 viewMatrix = Mat4_Identity;
	// mat4 projectionMatrix = Mat4_Identity;
	// jsGlUniform4f(Platform->testShader.uniforms.color1, 1, 1, OscillateBy(Platform->programTime, 0, 1, 1000), 1);
	// jsGlUniformMatrix4fv(Platform->testShader.uniforms.worldMatrix,      1, GL_FALSE, (GLfloat*)&worldMatrix);
	// jsGlUniformMatrix4fv(Platform->testShader.uniforms.viewMatrix,       1, GL_FALSE, (GLfloat*)&viewMatrix);
	// jsGlUniformMatrix4fv(Platform->testShader.uniforms.projectionMatrix, 1, GL_FALSE, (GLfloat*)&projectionMatrix);
	// jsGlActiveTexture(GL_TEXTURE0);
	// jsGlBindTexture(GL_TEXTURE_2D, Platform->testTexture.glId);
	// jsGlUniform1i(Platform->testShader.uniforms.texture, 0);
	// jsGlUniform2f(Platform->testShader.uniforms.textureSize, Platform->testTexture.width, Platform->testTexture.height);
	// jsGlBindBuffer(GL_ARRAY_BUFFER, Platform->squareBuffer.glId);
	// BindVertexArrayObject(&Platform->testVao, &Platform->testShader, &Platform->squareBuffer);
	
	// glDrawArrays(GL_TRIANGLES, 0, (GLsizei)Platform->squareBuffer.numVertices);
	
	TempPopMark();
	AssertMsg(GetNumMarks(&Platform->tempArena) == 0, "TempArena mark count is not 0 at end of WasmUpdateAndRender!");
}
