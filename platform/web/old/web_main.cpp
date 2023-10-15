
#define BASIC_DRAWING_CONTEXT false

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define PLATFORM_LAYER
#define GY_CUSTOM_STD_LIB
#define GY_WASM_STD_LIB
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#include "common_includes.h"

#include "web/web_version.h"
#include "common_defines.h"
#include "web/web_defines.h"
#include "web/web_javascript_interface.h"
#include "web/web_engine_exports.h"
#include "web/web_helpers.h"
#include "web/web_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
Web_PlatformState_t* Platform = nullptr;
u64 ProgramTime = 0;
#include "gylib/gy_temp_memory.cpp"

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "web/web_memory.cpp"
#include "web/web_debug.cpp"
#include "web/web_input.cpp"
#include "web/web_helpers.cpp"

#define EXPORTED_FUNC(returnType, functionName, ...) extern "C" returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)
#include "web/web_callbacks.cpp"

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
r32 OscillateSawBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = (SawR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
}

// +--------------------------------------------------------------+
// |                      Exported Functions                      |
// +--------------------------------------------------------------+

// +==============================+
// |          Initialize          |
// +==============================+
EXPORTED_FUNC(void, Initialize, int heapBaseAddress)
{
	Web_MemoryInit(heapBaseAddress);
	Web_DebugInit();
	
	WriteLine_N("+==========================================+");
	PrintLine_N("| Pig Engine Web Platform Layer v%u.%02u(%03u) |", WEB_VERSION_MAJOR, WEB_VERSION_MINOR, WEB_VERSION_BUILD);
	WriteLine_N("+==========================================+");
	
	PrintLine_D("initialized: %s", Platform->initialized ? "true" : "false");
	PrintLine_D("mainHeap type: %s", GetMemArenaTypeStr(Platform->mainHeap.type));
	
	Platform->testPos = NewVec2(100, 40);
	
	CreateVarArray(&Platform->drawnLines, &Platform->mainHeap, sizeof(Line_t));
	
	Web_FillEngineInput(&Platform->engineActiveInput);
	Web_CopyEngineInput(&Platform->engineInput, &Platform->engineActiveInput);
	Web_CopyEngineInput(&Platform->enginePreviousInput, &Platform->engineInput);
	
	Platform->testShader = CreateShader(simpleVertexShader, simpleFragmentShader);
	
	Vertex2D_t squareVerts[] = {
		{ { 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0 }, },
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 }, },
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 }, },
		
		{ { 1, 1, 0 }, { 1, 1, 1, 1 }, { 1, 1 }, },
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 }, },
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 }, },
	};
	if (!CreateVertBuffer2D(&Platform->mainHeap, &Platform->squareBuffer, false, 6, &squareVerts, true))
	{
		WriteLine_E("Failed to create VertBuffer!");
	}
	
	Platform->testVao = CreateVertexArrayObject(VertexType_Default2D);
	
	// u32 testTextureData[] = {
	// 	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	// 	0xFFFFFFFF, 0xFF40FFFF, 0xFFEF44FF, 0xFFFFFFFF,
	// 	0xFFFFFFFF, 0xFF40FFFF, 0xFFEF44FF, 0xFFFFFFFF,
	// 	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	// };
	u32 testTextureData[] = {
		0x00000000,         PureRed_Value,    PureOrange_Value, 0x00000000,
		MonokaiBrown_Value, 0x00000000,       0x00000000,       PureYellow_Value,
		PalPink_Value,      0x00000000,       0x00000000,       PureGreen_Value,
		0x00000000,         PurePurple_Value, PureBlue_Value,   0x00000000,
	};
	Platform->testTexture = CreateTexture(NewVec2i(4, 4), (u8*)testTextureData, true, false);
	
	glFrontFace(GL_CCW); //TODO: Change me back to clockwise once we've gotten coordinate space figured out
	glEnable(GL_CULL_FACE);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	
	// glAlphaFunc(GL_GEQUAL, 0.01f); //TODO: This isn't supported in WebGL2??
	// glEnable(GL_ALPHA_TEST); //TODO: This isn't supported in WebGL2??
}

// +==============================+
// |           TestFunc           |
// +==============================+
EXPORTED_FUNC(void, TestFunc, r64 testArg)
{
	PrintLine_D("This is at test print %llf (0x%016llX) at Debug level", testArg, *(u64*)&testArg);
	
	const char* tests[] = {
		"1.0",
		"1.1",
		"1.5",
		"0.1",
		"0.0",
		"0",
		"100",
		"125",
		"1234567890",
		"0.00001",
		"0.000025",
		"0.100025",
		".0",
		"0.",
		"1234.5678",
	};
	for (int testIndex = 0; testIndex < ArrayCount(tests); testIndex++)
	{
		const char* testStr = tests[testIndex];
		double testValue = strtod(testStr, nullptr);
		PrintLine_I("Test[%d]: \"%s\" -> %f (%.g)", testIndex, testStr, testValue, testValue);
	}
}

// +==============================+
// |         GetStackBase         |
// +==============================+
EXPORTED_FUNC(size_t, GetStackBase)
{
	int var1 = 10;
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wreturn-stack-address"
	return (size_t)&var1;
	#pragma clang diagnostic pop
}

// +==============================+
// |       AllocWasmMemory        |
// +==============================+
EXPORTED_FUNC(void*, AllocWasmMemory, int numBytes)
{
	Assert(numBytes >= 0);
	void* result = AllocMem(&Platform->mainHeap, (u64)numBytes);
	PrintLine_D("Allocation %llu bytes in mainHeap: %p", (u64)numBytes, result);
	return result;
}
// +==============================+
// |        FreeWasmMemory        |
// +==============================+
EXPORTED_FUNC(void, FreeWasmMemory, void* allocPntr, int allocSize)
{
	Assert(allocSize >= 0);
	PrintLine_D("Freeing %llu bytes in mainHeap: %p", (u64)allocSize, allocPntr);
	FreeMem(&Platform->mainHeap, allocPntr, (u64)allocSize);
}

// +==============================+
// |      FileLoadedCallback      |
// +==============================+
EXPORTED_FUNC(void, FileLoadedCallback, const char* filePath, const void* fileContents, int fileSize)
{
	NotNull(filePath);
	Assert(fileSize >= 0);
	Assert(fileSize == 0 || fileContents != nullptr);
	
	PrintLine_D("Got file %s contents for \"%s\"", FormatBytes((u64)fileSize, TempArena).pntr, filePath);
	
	//TODO: Implement me!
}

// +==============================+
// |         RenderFrame          |
// +==============================+
EXPORTED_FUNC(void, RenderFrame, r64 canvasWidth, r64 canvasHeight)
{
	// PrintLine_D("Rendering to %gx%g canvas", canvasWidth, canvasHeight);
	ProgramTime = (u64)ModR64(js_GetTime(), 60000);
	
	Web_CopyEngineInput(&Platform->enginePreviousInput, &Platform->engineInput);
	Web_CopyEngineInput(&Platform->engineInput, &Platform->engineActiveInput);
	Web_ResetEngineInput(&Platform->engineActiveInput);
	Web_UpdateEngineInputTimeInfo(&Platform->enginePreviousInput, &Platform->engineInput);
	
	#if 1
	if (Platform->mouseMoved && Platform->mouseLeftBtnDown)
	{
		Platform->testPos.x += 10;
		
		Line_t* newLine = VarArrayAdd(&Platform->drawnLines, Line_t);
		NotNull(newLine);
		newLine->start = Platform->prevMousePos;
		newLine->end = Platform->mousePos;
	}
	if (Platform->mouseLeftBtnPressed)
	{
		// PrintLine_D("ProgramTime: %llf", ProgramTime);
		// js_TestFunc();
		// js_LoadFile("Resources/Text/gamecontrollerdb.txt");
	}
	
	// +==============================+
	// |     Basic Drawing Tests      |
	// +==============================+
	#if BASIC_DRAWING_CONTEXT
	{
		js_ClearCanvas();
		js_DrawRectangle(OscillateSawBy(Platform->engineInput.programTime, 10, 90, 1000), 10, 100, 200, ((Platform->engineInput.programTime/10) % 255), 0, 0, 255);
		
		rec graphRec = NewRec(Platform->testPos.x, Platform->testPos.y, 400, 400);
		js_DrawRectangle(graphRec.x, graphRec.y, graphRec.width, graphRec.height, 0, 0, 0, 255);
		
		r32 startX = -10;
		r32 endX = 10;
		r32 startY = -10;
		r32 endY = 10;
		u64 numIterations = 100;
		v2 previousPos = Vec2_Zero;
		for (u64 xIndex = 0; xIndex <= numIterations; xIndex++)
		{
			r32 lerpValue = ((r32)xIndex / (r32)numIterations);
			r32 x = startX + (lerpValue * (endX - startX));
			r32 y = roundf(x);
			r32 lerpValueY = (y - startY) / (endY - startY);
			v2 newPos = NewVec2(graphRec.x + (lerpValue * graphRec.width), graphRec.y + graphRec.height - (lerpValueY * graphRec.height));
			Vec2Align(&newPos);
			if (xIndex > 0)
			{
				js_DrawLine(previousPos.x, previousPos.y, newPos.x, newPos.y, 1, 255, RoundR32i(LerpR32(200, 80, lerpValue)), 100, 255);
			}
			previousPos = newPos;
		}
		
		VarArrayLoop(&Platform->drawnLines, lIndex)
		{
			VarArrayLoopGet(Line_t, line, &Platform->drawnLines, lIndex);
			js_DrawLine(line->start.x, line->start.y, line->end.x, line->end.y, 1, 0, 255, 255, 255);
		}
	}
	// +==============================+
	// |     WebGL Drawing Tests      |
	// +==============================+
	#else
	{
		glViewport(0, 0, canvasWidth, canvasHeight);
		
		glClearColor(OscillateBy(ProgramTime, 0, 1, (Platform->mouseLeftBtnDown ? 200 : 1000)), 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		
		glUseProgram(Platform->testShader.glId);
		mat4 worldMatrix = Mat4RotateZ(OscillateBy(ProgramTime, 0, TwoPi32, (Platform->mouseLeftBtnDown ? 2000 : 10000)));
		mat4 viewMatrix = Mat4_Identity;
		mat4 projectionMatrix = Mat4_Identity;
		glUniform4f(Platform->testShader.uniforms.color1, 1, 1, OscillateBy(ProgramTime, 0, 1, 1000), 1);
		glUniformMatrix4fv(Platform->testShader.uniforms.worldMatrix,      1, GL_FALSE, (GLfloat*)&worldMatrix);
		glUniformMatrix4fv(Platform->testShader.uniforms.viewMatrix,       1, GL_FALSE, (GLfloat*)&viewMatrix);
		glUniformMatrix4fv(Platform->testShader.uniforms.projectionMatrix, 1, GL_FALSE, (GLfloat*)&projectionMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Platform->testTexture.glId);
		glUniform1i(Platform->testShader.uniforms.texture, 0);
		glUniform2f(Platform->testShader.uniforms.textureSize, Platform->testTexture.width, Platform->testTexture.height);
		glBindBuffer(GL_ARRAY_BUFFER, Platform->squareBuffer.glId);
		BindVertexArrayObject(&Platform->testVao, &Platform->testShader, &Platform->squareBuffer);
		
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)Platform->squareBuffer.numVertices);
	}
	#endif
	
	Platform->mouseMoved = false;
	Platform->mouseLeftBtnPressed = false;
	Platform->mouseMiddleBtnPressed = false;
	Platform->mouseRightBtnPressed = false;
	Platform->mouseLeftBtnReleased = false;
	Platform->mouseMiddleBtnReleased = false;
	Platform->mouseRightBtnReleased = false;
	Platform->prevMousePos = Platform->mousePos;
	#endif
}

// +--------------------------------------------------------------+
// |                   Assert Failure Function                    |
// +--------------------------------------------------------------+
static bool insideAssertFailure = false;
void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	if (insideAssertFailure) { return; }
	
	if (messageStr != nullptr && messageStr[0] != '\0')
	{
		PrintLine_E("Assertion Failure! %s (Expression: %s) in %s %s:%d", messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
	}
	else
	{
		PrintLine_E("Assertion Failure! (%s) is not true in %s %s:%d", expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
	}
	
	MyBreak();
}