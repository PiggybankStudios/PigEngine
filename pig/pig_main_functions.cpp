/*
File:   pig_main_functions.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds the PigInitialize, PigUpdate, PigPreReload, PigPostReload, and PigClosing functions
*/

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void PigInitialize(EngineMemory_t* memory)
{
	ClearPointer(pig);
	pig->mainThreadId = plat->GetThisThreadId();
	pig->renderApi = platInfo->renderApi;
	pig->dllReloaded = false;
	
	InitMemArena_Redirect(&pig->platHeap, PlatAllocFunc, PlatFreeFunc);
	//TODO: Change this to a paged heap
	Assert(memory->persistentDataSize > sizeof(PigState_t) + DBG_CONSOLE_BUFFER_SIZE);
	InitMemArena_FixedHeap(&pig->mainHeap, memory->persistentDataSize - sizeof(PigState_t) - DBG_CONSOLE_BUFFER_SIZE, ((u8*)memory->persistentDataPntr) + sizeof(PigState_t) + DBG_CONSOLE_BUFFER_SIZE);
	InitMemArena_MarkedStack(&pig->tempArena, memory->tempDataSize, memory->tempDataPntr, PIG_TEMP_MAX_MARKS);
	InitMemArena_StdHeap(&pig->stdHeap);
	TempPushMark();
	
	PerfTime_t initStartTime = plat->GetPerfTime();
	InitializeDebugConsole(&pig->debugConsole, DBG_CONSOLE_BUFFER_SIZE, ((u8*)memory->persistentDataPntr) + sizeof(PigState_t));
	InitializePigPerfGraph(&pig->perfGraph);
	InitializePigDebugOverlay(&pig->debugOverlay);
	InitPigAudioOutGraph(&pig->audioOutGraph);
	PigInitNotifications(&pig->notificationsQueue);
	
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	WriteLine_N("+==============================+");
	PrintLine_N("|    Pig engine v%u.%02u(%03u)     |", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	WriteLine_N("+==============================+");
	PrintLine_D("Running on %s", GetRenderApiStr(pig->renderApi));
	
	SeedRand(11); //TODO: Get the timestamp and feed it in here!
	PigInitGlad();
	InitRenderContext();
	PigInitAudioOutput();
	PigInitSounds();
	
	pig->nextUiId = 1;
	pig->nextShaderId = 1;
	pig->nextTextureId = 1;
	pig->nextVectorImgId = 1;
	pig->nextVertBufferId = 1;
	pig->nextSpriteSheetId = 1;
	plat->ChangeWindowTarget(platInfo->mainWindow);
	
	RcLoadBasicResources();
	Pig_InitResources();
	Pig_LoadAllResources(); //TODO: Eventually we don't want to load ALL resources at startup 
	
	// plat->DebugReadout(NewStr("Hello from Pig Engine!"), White, 1.0f);
	#if 0
	while (true)
	{
		for (u32 loadingStep = 0; loadingStep < 100; loadingStep++)
		{
			if (plat->RenderLoadingScreen((r32)loadingStep / 100.0f)) { return; }
			// if (plat->RenderLoadingScreen(1.0f)) { return; }
			Sleep(5);
		}
	}
	#endif
	
	InPlaceNew(MyPhysRenderer_c, &pig->physRenderer); //TODO: Do we need this?
	
	// +==============================+
	// |     Initialize AppState      |
	// +==============================+
	pig->currentAppState = AppState_None;
	InitializeAppState(pig->currentAppState);
	
	pig->initialized = true;
	PerfTime_t initEndTime = plat->GetPerfTime();
	PrintLine_D("Initialization took %.1lfms (%.1lfms total startup time)", plat->GetPerfTimeDiff(&initStartTime, &initEndTime), plat->GetPerfTimeDiff(&platInfo->programStartPerfTime, &initEndTime));
	TempPopMark();
	u64 numMarks = GetNumMarks(TempArena);
	Assert(numMarks == 0);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void PigUpdateMainWindow()
{
	Pig_UpdateResources();
	
	PigNotificationsCaptureMouse(&pig->notificationsQueue);
	PigAudioOutGraphCaptureMouse(&pig->audioOutGraph);
	PigPerfGraphCaptureMouse(&pig->perfGraph);
	PigDebugOverlayCaptureMouse(&pig->debugOverlay);
	DebugConsoleCaptureMouse(&pig->debugConsole);
	
	PigUpdateNotifications(&pig->notificationsQueue);
	UpdatePigDebugOverlay(&pig->debugOverlay);
	UpdatePigPerfGraph(&pig->perfGraph);
	UpdatePigAudioOutGraph(&pig->audioOutGraph);
	UpdateDebugConsole(&pig->debugConsole);
	
	if (pig->changeAppStateRequested)
	{
		DeinitializeAppState(pig->currentAppState);
		InitializeAppState(pig->newAppState);
		pig->currentAppState = pig->newAppState;
		pig->newAppState = AppState_None;
		pig->changeAppStateRequested = false;
	}
	
	UpdateAppState(pig->currentAppState);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void PigRenderMainWindow()
{
	RenderAppState(pig->currentAppState);
	
	RcBindShader(&pig->resources.mainShader2D);
	RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
	RcSetViewMatrix(Mat4_Identity);
	RcClearDepth(1.0f);
	
	RenderPigDebugOverlay(&pig->debugOverlay);
	RenderDebugConsole(&pig->debugConsole);
	RenderPigAudioOutGraph(&pig->audioOutGraph);
	RenderPigPerfGraph(&pig->perfGraph);
	PigRenderNotifications(&pig->notificationsQueue);
	Pig_InputRenderDebugInfo();
}

void PigUpdate()
{
	PigHandlePlatformDebugLines(&pigIn->platDebugLines);
	Pig_UpdateInputBefore();
	PigUpdateSounds();
	
	Pig_ChangeWindow(platInfo->mainWindow);
	
	PigUpdateMainWindow();
	
	if (true)
	{
		const PlatWindow_t* window = LinkedListFirst(platInfo->windows, PlatWindow_t);
		bool isMainWindow = (window == platInfo->mainWindow);
		for (u32 wIndex = 0; wIndex < platInfo->windows->count; wIndex++)
		{
			NotNull(window);
			if (!window->closed)
			{
				if (isMainWindow)
				{
					PigRenderMainWindow();
				}
				else
				{
					RcBegin(window, &pig->resources.mainShader2D, GetPredefPalColorByIndex(wIndex+4));
				}
			}
			window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
		}
		plat->SwapBuffers();
	}
	
	Pig_UpdateInputAfter();
}

// +--------------------------------------------------------------+
// |                      ShouldWindowClose                       |
// +--------------------------------------------------------------+
bool PigShouldWindowClose(const PlatWindow_t* window)
{
	UNUSED(window);
	//TODO: Add any logic we want for scenarios where we want to prevent the window from closing
	return true;
}

// +--------------------------------------------------------------+
// |                          PreReload                           |
// +--------------------------------------------------------------+
void PigPreReload(Version_t newVersion)
{
	PrintLine_N("Shutting down Pig DLL v%u.%02u(%03u)...", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	UNUSED(newVersion); //TODO: Remove me!
	//TODO: Implement me!
}

// +--------------------------------------------------------------+
// |                          PostReload                          |
// +--------------------------------------------------------------+
void PigPostReload(Version_t oldVersion)
{
	UNUSED(oldVersion); //TODO: Remove me!
	PigInitGlad();
	pig->dllReloaded = true;
	
	NotifyPrint_N("Now running Pig DLL v%u.%02u(%03u)!", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	
	UpdateMemArenaFuncPntrs(&pig->platHeap, PlatAllocFunc, PlatFreeFunc);
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	
	// +==============================+
	// |  Fix Physics Related Stuff   |
	// +==============================+
	{
		q3AllocFunc = Q3AllocCallback;
		q3FreeFunc = Q3FreeCallback;
		InPlaceNew(MyPhysRenderer_c, &pig->physRenderer); //TODO: Do we need this?
	}
}

// +--------------------------------------------------------------+
// |                           Closing                            |
// +--------------------------------------------------------------+
void PigClosing()
{
	WriteLine_W("We are closing. But at least the engine DLL knows about it");
	//TODO: Implement me!
}
