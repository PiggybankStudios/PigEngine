/*
File:   pig_main.cpp
Author: Taylor Robbins
Date:   09\06\2023
Description: 
	** Holds the main entry point for the application, and is the only cpp file
	** we actually compile (all others are #included into this one)
*/

#include "pig_playdate_version.h"

#include "gylib/gy_defines_check.h"

#define GYLIB_LOOKUP_PRIMES_10
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#include "gylib/gy.h"

// #define STB_IMAGE_IMPLEMENTATION
// #define STBI_NO_STDIO
// #define STBI_ASSERT(condition) Assert(condition)
// #pragma GCC diagnostic ignored "-Wdouble-promotion"
// #include "stb/stb_image.h"
// #pragma GCC diagnostic warning "-Wdouble-promotion"

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "sound.h"
#include "texture.h"
#include "sprite_sheet.h"
#include "font_range.h"
#include "font.h"
#include "offscreen_rendering.h"
#include "input.h"
#include "pd_api_ext.h"
#include "perf_graph.h"
#include "app_state.h"
#include "game_main.h"
#include "pig_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
PlaydateAPI* pd = nullptr;
PigEngineState_t* pig = nullptr;
MemArena_t* fixedHeap = nullptr;
MemArena_t* mainHeap = nullptr;
AppInput_t* input = nullptr;

const v2i ScreenSize = { PLAYDATE_SCREEN_WIDTH, PLAYDATE_SCREEN_HEIGHT };
const v2 ScreenSizef = { (r32)PLAYDATE_SCREEN_WIDTH, (r32)PLAYDATE_SCREEN_HEIGHT };
const reci ScreenRec = { 0, 0, PLAYDATE_SCREEN_WIDTH, PLAYDATE_SCREEN_HEIGHT };
const rec ScreenRecf = { 0.0f, 0.0f, (r32)PLAYDATE_SCREEN_WIDTH, (r32)PLAYDATE_SCREEN_HEIGHT };
u32 ProgramTime = 0;
r32 ElapsedMs = 0.0f;
r32 TimeScale = 1.0f;

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "performance.cpp"
#include "scratch.cpp"
#include "debug.cpp"
#include "pig_helpers.cpp"
#include "pig_file.cpp"
#include "sound.cpp"
#include "texture.cpp"
#include "sprite_sheet.cpp"
#include "font.cpp"
#include "offscreen_rendering.cpp"
#include "pig_render.cpp"
#include "input.cpp"
#include "playdate_input_hints.cpp"
#include "perf_graph.cpp"

#include "app_state.cpp"
#include "game_main.cpp"

// +--------------------------------------------------------------+
// |                     Menu Item Callbacks                      |
// +--------------------------------------------------------------+
void FpsToggledCallback(void* userData)
{
	bool newValue = (pd->system->getMenuItemValue(pig->fpsDisplayMenuItem) != 0);
	if (pig->fpsDisplayEnabled != newValue)
	{
		pig->fpsDisplayEnabled = newValue;
		PrintLine_I("FPS Display %s", pig->fpsDisplayEnabled ? "Enabled" : "Disabled");
		pig->perfGraph.enabled = pig->fpsDisplayEnabled; //TODO: Make this a seperate option
	}
}
void DebugToggledCallback(void* userData)
{
	bool newValue = (pd->system->getMenuItemValue(pig->debugMenuItem) != 0);
	if (pig->debugEnabled != newValue)
	{
		pig->debugEnabled = newValue;
		PrintLine_I("Debug %s", pig->debugEnabled ? "Enabled" : "Disabled");
	}
}

// +--------------------------------------------------------------+
// |                        Event Handler                         |
// +--------------------------------------------------------------+
EXTERN_C_START
int MainUpdateCallback(void* userData)
{
	PdBeginFrame();
	OffscreenRenderingBeginFrame();
	UpdateAppInput();
	UpdateSoundPool(&pig->soundPool);
	UpdatePerfGraph(&pig->perfGraph);
	
	if (!pig->firstUpdateCalled)
	{
		WriteLine_N("Running...");
		pig->programStartTimeSinceEpoch = input->timeSinceEpoch;
		pig->firstUpdateCalled = true;
	}
	
	PreUpdateGame();
	UpdateAppStateStack();
	PostUpdateGame();
	
	PreRenderGame();
	RenderAppStateStack();
	PostRenderGame();
	
	RenderPerfGraph(&pig->perfGraph);
	
	// Return a 1 to indicate we want the display to update
	// TODO: Are there any game types where we don't want to request display update every frame?
	return 1;
}
EXTERN_C_END

void HandleSystemEvent(PDSystemEvent event, uint32_t arg)
{
	// if (pig != nullptr) { PrintLine_D("Event: %s %u", GetPDSystemEventStr(event)); }
	
	switch (event)
	{
		// +==============================+
		// |             Init             |
		// +==============================+
		case kEventInit:
		{
			GyLibDebugOutputFunc = GyLibOutputHandler;
			GyLibDebugPrintFunc = GyLibPrintHandler;
			
			PigEngineState_t* pigEngineState = nullptr;
			{
				MemArena_t stdHeapOnStack;
				InitMemArena_StdHeap(&stdHeapOnStack);
				FlagUnset(stdHeapOnStack.flags, MemArenaFlag_TelemetryEnabled);
				pigEngineState = AllocStruct(&stdHeapOnStack, PigEngineState_t);
				NotNull(pigEngineState);
				ClearPointer(pigEngineState);
				pigEngineState->initialized = true;
				pigEngineState->stdHeap = stdHeapOnStack;
			}
			void* fixedHeapPntr = AllocMem(&pigEngineState->stdHeap, FIXED_HEAP_SIZE);
			NotNull(fixedHeapPntr);
			//NOTE: On the playdate device, all of our structures need to be properly aligned.
			//      To help encourage this, we made all our main arnas default to 8-byte alignment.
			InitMemArena_FixedHeap(&pigEngineState->fixedHeap, FIXED_HEAP_SIZE, fixedHeapPntr, AllocAlignment_8Bytes);
			InitMemArena_PagedHeapArena(&pigEngineState->mainHeap, MAIN_HEAP_PAGE_SIZE, &pigEngineState->stdHeap, MAIN_HEAP_MAX_NUM_PAGES, AllocAlignment_8Bytes);
			pigEngineState->initialized = true;
			
			Assert(pig == nullptr);
			pig = pigEngineState;
			fixedHeap = &pigEngineState->fixedHeap;
			mainHeap = &pigEngineState->mainHeap;
			
			InitScratchArenas(&pig->stdHeap, SCRATCH_ARENA_SIZE, SCRATCH_ARENA_MAX_NUM_MARKS, AllocAlignment_8Bytes);
			
			WriteLine_O("+==============================+");
			PrintLine_O("|     %s v%u.%u(%0u)     |", "Pig Engine", PIG_VERSION_MAJOR, PIG_VERSION_MINOR, PIG_VERSION_BUILD);
			WriteLine_O("+==============================+");
			
			InitOffscreenRendering();
			
			pig->fpsDisplayMenuItem = pd->system->addCheckmarkMenuItem("FPS", 1, FpsToggledCallback, nullptr);
			NotNull(pig->fpsDisplayMenuItem);
			pd->system->setMenuItemValue(pig->fpsDisplayMenuItem, pig->fpsDisplayEnabled ? 1 : 0);
			
			pig->debugMenuItem = pd->system->addCheckmarkMenuItem("Debug", 1, DebugToggledCallback, nullptr);
			NotNull(pig->debugMenuItem);
			pd->system->setMenuItemValue(pig->debugMenuItem, pig->debugEnabled ? 1 : 0);
			
			pig->debugFont = LoadFont(NewStr(DEBUG_FONT_PATH));
			Assert(pig->debugFont.isValid);
			
			u8 whiteDotPixels[4] = { 0xFF };
			whiteDotTexture = CreateTexture(NewVec2i(1, 1), ArrayCount(whiteDotPixels), whiteDotPixels, nullptr);
			Assert(whiteDotTexture.isValid);
			u8 blackDotPixels[4] = { 0x00 };
			blackDotTexture = CreateTexture(NewVec2i(1, 1), ArrayCount(blackDotPixels), blackDotPixels, nullptr);
			Assert(blackDotTexture.isValid);
			
			pig->crankHintBackSheet = LoadSpriteSheet(NewStr("Resources/Sheets/crank_hint_back"), 1);
			Assert(pig->crankHintBackSheet.isValid);
			pig->crankHintSheet = LoadSpriteSheet(NewStr("Resources/Sheets/crank_hint"), 8);
			Assert(pig->crankHintSheet.isValid);
			
			WriteLine_N("Initializing...");
			CreateRandomSeries(&pig->random);
			#if 1
			SeedRandomSeriesU64(&pig->random, pd->system->getSecondsSinceEpoch(nullptr));
			#else
			SeedRandomSeriesU64(&pig->random, 1);
			#endif
			input = &pig->input;
			InitializeAppInput();
			InitSoundPool(&pig->soundPool);
			InitPerfGraph(&pig->perfGraph);
			
			AppState_t firstAppState = InitGame();
			if (firstAppState != AppState_None)
			{
				StartFirstAppState(firstAppState);
			}
			
			WriteLine_D("Registering MainUpdateCallback...");
			pd->system->setUpdateCallback(MainUpdateCallback, nullptr);
		} break;
		
		// +==============================+
		// |           InitLua            |
		// +==============================+
		case kEventInitLua:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |             Lock             |
		// +==============================+
		case kEventLock:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |            Unlock            |
		// +==============================+
		case kEventUnlock:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |            Pause             |
		// +==============================+
		case kEventPause:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |            Resume            |
		// +==============================+
		case kEventResume:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |          Terminate           |
		// +==============================+
		case kEventTerminate:
		{
			ClearAppStateStackBeforeClose();
		} break;
		
		// +==============================+
		// |          KeyPressed          |
		// +==============================+
		case kEventKeyPressed:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |         KeyReleased          |
		// +==============================+
		case kEventKeyReleased:
		{
			//TODO: Implement me!
		} break;
		
		// +==============================+
		// |           LowPower           |
		// +==============================+
		case kEventLowPower:
		{
			//TODO: Implement me!
		} break;
	}
}

EXTERN_C_START
#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandlerShim(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if (event == kEventInit) { pdrealloc = playdate->system->realloc; }
	pd = playdate;
	HandleSystemEvent(event, arg);
	return 0;
}
EXTERN_C_END

// +--------------------------------------------------------------+
// |                      Assertion Callback                      |
// +--------------------------------------------------------------+
void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	if (pd == nullptr) { return; }
	
	if (pig != nullptr && pig->initialized)
	{
		//TODO: Shorten path to just fileName
		if (messageStr != nullptr && messageStr[0] != '\0')
		{
			pd->system->error("Assertion Failure! %s (Expression: %s) in %s %s:%d", messageStr, expressionStr, funcName, filePath, lineNumber);
		}
		else
		{
			pd->system->error("Assertion Failure! (%s) is not true in %s %s:%d", expressionStr, funcName, filePath, lineNumber);
		}
	}
	else
	{
		MyBreak();
	}
}
