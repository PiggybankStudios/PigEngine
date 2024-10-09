/*
File:   oc_main.cpp
Author: Taylor Robbins
Date:   10\08\2024
Description: 
	** Holds the main entry point for the Orca application, and is the only cpp
	** file we actually compile (all others are #included into this one).
	** Note this is the "platform" layer for Orca applications, the majority of
	** the application specific code will be in app_main.h and app_main.cpp and
	** the files they #include. So while this is the "Application" from the view
	** of the Orca runtime, it's not really the "Application" in our view
*/

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#include "build_config.h"

// +==============================+
// |          Libraries           |
// +==============================+
#define ORCA_COMPILATION
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#include "gylib/gy.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

// +==============================+
// |    Platform Header Files     |
// +==============================+
#include "debug.h"
#include "vector_img.h"
#include "oc_main.h"

// +==============================+
// |       Platform Globals       |
// +==============================+
PlatformState_t* platform = nullptr;
MemArena_t* stdHeap = nullptr;
MemArena_t* mainHeap = nullptr;
v2 MousePos = Vec2_Zero_Const;
v2 ScreenSize = Vec2_Zero_Const;
v2i ScreenSizei = Vec2i_Zero_Const;
rec ScreenRec = Rec_Zero_Const;

// +==============================+
// |    Platform Source Files     |
// +==============================+
#include "debug.cpp"
#include "scratch.cpp"
#include "vector_img.cpp"

#include "app_main.cpp"

// +--------------------------------------------------------------+
// |                      Orca Entry Points                       |
// +--------------------------------------------------------------+
// +==============================+
// |          OC_OnInit           |
// +==============================+
ORCA_EXPORT void OC_OnInit()
{
	// +==============================+
	// |   Initialize Memory Arenas   |
	// +==============================+
	{
		MemArena_t stdHeapLocal = {};
		InitMemArena_StdHeap(&stdHeapLocal);
		platform = AllocStruct(&stdHeapLocal, PlatformState_t);
		NotNull(platform);
		ClearPointer(platform);
		MyMemCopy(&platform->stdHeap, &stdHeapLocal, sizeof(MemArena_t));
		stdHeap = &platform->stdHeap;
		InitMemArena_PagedHeapArena(&platform->mainHeap, MAIN_HEAP_PAGE_SIZE, stdHeap);
		mainHeap = &platform->mainHeap;
		InitScratchArenas(stdHeap, SCRATCH_ARENAS_PAGE_SIZE, SCRATCH_ARENAS_MAX_MARKS);
	}
	
	MemArena_t* scratch = GetScratchArena();
	PrintLine_I("%s v%d.%d(%d) is starting...", PROJECT_NAME_STR, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);
	
	#if DEBUG_BUILD
	MyStr_t windowTitle = PrintInArenaStr(scratch, "%s %d.%d(%d)", PROJECT_NAME_STR, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);
	#else
	MyStr_t windowTitle = NewStr(PROJECT_NAME_STR);
	#endif
	OC_WindowSetTitle(windowTitle);
	
	platform->renderer = OC_CanvasRendererCreate();
	platform->surface = OC_CanvasSurfaceCreate(platform->renderer);
	platform->canvasContext = OC_CanvasContextCreate();
	OC_UiInit(&platform->ui);
	OC_UiSetContext(&platform->ui);
	
    OC_UnicodeRange_t fontRanges[] = {
        OC_UNICODE_BASIC_LATIN,
        OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
        OC_UNICODE_LATIN_EXTENDED_A,
        OC_UNICODE_LATIN_EXTENDED_B,
        OC_UNICODE_SPECIALS
    };
	platform->debugFont = OC_FontCreateFromPath(NewStr("Font/consolab.ttf"), ArrayCount(fontRanges), fontRanges);
	
	AppInit();
	
	FreeScratchArena(scratch);
}

//TODO: Should we free any memory when the application closes?

ORCA_EXPORT void OC_OnRawEvent(OC_Event_t* event)
{
	OC_UiSetContext(&platform->ui);
	OC_UiProcessEvent(event);
}

// +==============================+
// |         OC_OnResize          |
// +==============================+
ORCA_EXPORT void OC_OnResize(u32 width, u32 height)
{
	// PrintLine_I("OC_OnResize(%d, %d)!", width, height);
	ScreenSize = NewVec2((r32)width, (r32)height);
	ScreenSizei = NewVec2i((i32)width, (i32)height);
	ScreenRec = NewRec(0, 0, (r32)width, (r32)height);
}

// +==============================+
// |         OC_OnKeyDown         |
// +==============================+
ORCA_EXPORT void OC_OnKeyDown(OC_ScanCode_t scan, OC_KeyCode_t key)
{
	//TODO: Implement me!
}

// +==============================+
// |          OC_OnKeyUp          |
// +==============================+
ORCA_EXPORT void OC_OnKeyUp(OC_ScanCode_t scan, OC_KeyCode_t key)
{
	//TODO: Implement me!
}

// +==============================+
// |        OC_OnMouseMove        |
// +==============================+
ORCA_EXPORT void OC_OnMouseMove(r32 x, r32 y, r32 dx, r32 dy)
{
	MousePos.x = x;
	MousePos.y = y;
}

// +==============================+
// |      OC_OnFrameRefresh       |
// +==============================+
ORCA_EXPORT void OC_OnFrameRefresh()
{
	AppUpdateAndRender();
}
