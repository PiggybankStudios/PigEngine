/*
File:   pig_main.cpp
Author: Taylor Robbins
Date:   09\06\2023
Description: 
	** Holds the main entry point for the application, and is the only cpp file
	** we actually compile (all others are #included into this one)
*/

#include "pig_version.h"

#include "gylib/gy_defines_check.h"

#define GYLIB_LOOKUP_PRIMES_10
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#include "gylib/gy.h"

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "sound.h"
#include "texture.h"
#include "sprite_sheet.h"
#include "font_range.h"
#include "font.h"
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

const v2i ScreenSize = { LCD_COLUMNS, LCD_ROWS };
const v2 ScreenSizef = { (r32)LCD_COLUMNS, (r32)LCD_ROWS };
u32 ProgramTime = 0;
r32 ElapsedMs = 0.0f;
r32 TimeScale = 1.0f;

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "scratch.cpp"
#include "debug.cpp"
#include "sound.cpp"
#include "texture.cpp"
#include "sprite_sheet.cpp"
#include "font.cpp"
#include "input.cpp"
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
int MainUpdateCallback(void* userData)
{
	PdBeginFrame();
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
	
	return 0;
}

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
				pigEngineState = AllocStruct(&stdHeapOnStack, PigEngineState_t);
				NotNull(pigEngineState);
				ClearPointer(pigEngineState);
				pigEngineState->initialized = true;
				pigEngineState->stdHeap = stdHeapOnStack;
			}
			void* fixedHeapPntr = MyMalloc(FIXED_HEAP_SIZE);
			NotNull(fixedHeapPntr);
			InitMemArena_FixedHeap(&pigEngineState->fixedHeap, FIXED_HEAP_SIZE, fixedHeapPntr);
			InitMemArena_PagedHeapArena(&pigEngineState->mainHeap, MAIN_HEAP_PAGE_SIZE, &pigEngineState->stdHeap, MAIN_HEAP_MAX_NUM_PAGES);
			pigEngineState->initialized = true;
			
			Assert(pig == nullptr);
			pig = pigEngineState;
			fixedHeap = &pigEngineState->fixedHeap;
			mainHeap = &pigEngineState->mainHeap;
			
			InitScratchArenas(&pig->stdHeap, SCRATCH_ARENA_SIZE, SCRATCH_ARENA_MAX_NUM_MARKS);
			
			WriteLine_O("+==============================+");
			PrintLine_O("|     %s v%u.%u(%0u)     |", "Pig Engine", PIG_VERSION_MAJOR, PIG_VERSION_MINOR, PIG_VERSION_BUILD);
			WriteLine_O("+==============================+");
			
			pig->fpsDisplayMenuItem = pd->system->addCheckmarkMenuItem("FPS", 1, FpsToggledCallback, nullptr);
			NotNull(pig->fpsDisplayMenuItem);
			pd->system->setMenuItemValue(pig->fpsDisplayMenuItem, pig->fpsDisplayEnabled ? 1 : 0);
			
			pig->debugMenuItem = pd->system->addCheckmarkMenuItem("Debug", 1, DebugToggledCallback, nullptr);
			NotNull(pig->debugMenuItem);
			pd->system->setMenuItemValue(pig->debugMenuItem, pig->debugEnabled ? 1 : 0);
			
			pig->debugFont = LoadFont(NewStr(DEBUG_FONT_PATH));
			Assert(pig->debugFont.isValid);
			
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
			StartFirstAppState(firstAppState);
			
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
			//TODO: Implement me!
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