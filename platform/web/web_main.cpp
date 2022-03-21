
// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define PLATFORM_LAYER
#include "stb/stb_sprintf.h"
#define vsnprintf stbsp_vsnprintf
#define GY_CUSTOM_STD_LIB
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
}

// +==============================+
// |           TestFunc           |
// +==============================+
EXPORTED_FUNC(void, TestFunc, int testArg)
{
	PrintLine_D("This is at test print %d at Debug level", testArg);
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
EXPORTED_FUNC(void, RenderFrame, float canvasWidth, float canvasHeight)
{
	static int frameNum = 0;
	frameNum++;
	
	// PrintLine_D("Rendering to %gx%g canvas", canvasWidth, canvasHeight);
	
	js_ClearCanvas();
	js_DrawRectangle(OscillateSawBy(frameNum, 10, 90, 1000), 10, 100, 200, 0, (frameNum % 255), 0, 0, 255);
	
	rec graphRec = NewRec(10, 10, 100, 75);
	js_DrawRectangle(graphRec.x, graphRec.y, graphRec.width, graphRec.height, 0, 0, 0, 0, 255);
	
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
		r32 y = absf(x);
		r32 lerpValueY = (y - startY) / (endY - startY);
		v2 newPos = NewVec2(graphRec.x + (lerpValue * graphRec.width), graphRec.y + graphRec.height - (lerpValueY * graphRec.height));
		Vec2Align(&newPos);
		if (xIndex > 0)
		{
			js_DrawLine(previousPos.x, previousPos.y, newPos.x, newPos.y, 0.6f, 255, 0, 0, 255);
		}
		previousPos = newPos;
	}
}
