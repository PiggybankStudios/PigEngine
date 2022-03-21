
// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define PLATFORM_LAYER
#include "stb/stb_sprintf.h"
#define vsnprintf stbsp_vsnprintf
#define GY_CUSTOM_STD_LIB
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#include "common_includes.h"

#include "web/web_version.h"
#include "common_defines.h"
#include "web/web_defines.h"
#include "web/web_javascript_interface.h"
#include "web/web_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
Web_PlatformState_t* Platform = nullptr;
#include "gylib/gy_temp_memory.cpp"

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "web/web_memory.cpp"
#include "web/web_debug.cpp"

// +--------------------------------------------------------------+
// |                       Library Includes                       |
// +--------------------------------------------------------------+
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

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
#define EXPORTED_FUNC(returnType, functionName, ...) extern "C" returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)

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
}

// +==============================+
// |           TestFunc           |
// +==============================+
EXPORTED_FUNC(void, TestFunc, r64 testArg)
{
	PrintLine_D("This is at test print %llf (0x%016llX) at Debug level", testArg, *(u64*)&testArg);
}

// +==============================+
// |         GetStackBase         |
// +==============================+
EXPORTED_FUNC(int, GetStackBase)
{
	int var1 = 10;
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wreturn-stack-address"
	return (int)&var1;
	#pragma clang diagnostic pop
}

// +==============================+
// |         RenderFrame          |
// +==============================+
EXPORTED_FUNC(void, RenderFrame, r64 canvasWidth, r64 canvasHeight)
{
	static int frameNum = 0;
	frameNum++;
	
	// PrintLine_D("Rendering to %gx%g canvas", canvasWidth, canvasHeight);
	
	// if (Platform->mouseLeftBtnPressed)
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
		js_TestFunc();
	}
	
	js_ClearCanvas();
	js_DrawRectangle(OscillateSawBy(frameNum, 10, 90, 1000), 10, 100, 200, (frameNum % 255), 0, 0, 255);
	
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
	
	Platform->mouseMoved = false;
	Platform->mouseLeftBtnPressed = false;
	Platform->mouseMiddleBtnPressed = false;
	Platform->mouseRightBtnPressed = false;
	Platform->mouseLeftBtnReleased = false;
	Platform->mouseMiddleBtnReleased = false;
	Platform->mouseRightBtnReleased = false;
	Platform->prevMousePos = Platform->mousePos;
}

// +--------------------------------------------------------------+
// |                       Input Functions                        |
// +--------------------------------------------------------------+
// +==============================+
// |          MouseMoved          |
// +==============================+
EXPORTED_FUNC(void, MouseMoved, r64 mouseX, r64 mouseY)
{
	if (Platform->mousePos.x != mouseX || Platform->mousePos.y != mouseY)
	{
		Platform->mouseMoved = true;
		Platform->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	}
}

// +==============================+
// |       MouseBtnChanged        |
// +==============================+
EXPORTED_FUNC(void, MouseBtnChanged, int button, bool isDown, r64 mouseX, r64 mouseY)
{
	if (button == 0) //left
	{
		Platform->mouseLeftBtnDown = isDown;
		if (isDown) { Platform->mouseLeftBtnPressed = true; }
		else { Platform->mouseLeftBtnReleased = true; }
	}
	else if (button == 1) //middle
	{
		Platform->mouseMiddleBtnDown = isDown;
		if (isDown) { Platform->mouseMiddleBtnPressed = true; }
		else { Platform->mouseMiddleBtnReleased = true; }
	}
	else if (button == 2) //right
	{
		Platform->mouseRightBtnDown = isDown;
		if (isDown) { Platform->mouseRightBtnPressed = true; }
		else { Platform->mouseRightBtnReleased = true; }
	}
	else
	{
		PrintLine_E("WARNING: Unknown mouse button index %d in MouseBtnChanged!", button);
	}
	if (Platform->mousePos.x != mouseX || Platform->mousePos.y != mouseY)
	{
		Platform->mouseMoved = true;
		Platform->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	}
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