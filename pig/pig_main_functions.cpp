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
	pig->reloadIndex = 1;
	
	InitMemArena_Redirect(&pig->platHeap, PlatAllocFunc, PlatFreeFunc);
	u64 totalConsoleSpaceSize = DBG_CONSOLE_BUFFER_SIZE + DBG_CONSOLE_BUILD_SPACE_SIZE;
	Assert(memory->persistentDataSize > sizeof(PigState_t) + totalConsoleSpaceSize);
	InitMemArena_FixedHeap(&pig->fixedHeap, memory->persistentDataSize - sizeof(PigState_t) - totalConsoleSpaceSize, ((u8*)memory->persistentDataPntr) + sizeof(PigState_t) + totalConsoleSpaceSize);
	InitMemArena_PagedHeapFuncs(&pig->mainHeap, PIG_MAIN_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	InitMemArena_PagedHeapFuncs(&pig->audioHeap, PIG_AUDIO_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	InitMemArena_MarkedStack(&pig->tempArena, memory->tempDataSize, memory->tempDataPntr, PIG_TEMP_MAX_MARKS);
	InitMemArena_StdHeap(&pig->stdHeap);
	TempPushMark();
	
	PerfTime_t initStartTime = plat->GetPerfTime();
	u8* consoleSpace = ((u8*)memory->persistentDataPntr) + sizeof(PigState_t);
	InitializeDebugConsole(&pig->debugConsole, DBG_CONSOLE_BUFFER_SIZE, consoleSpace + DBG_CONSOLE_BUILD_SPACE_SIZE, DBG_CONSOLE_BUILD_SPACE_SIZE, consoleSpace);
	InitializePigPerfGraph(&pig->perfGraph);
	InitializePigMemGraph(&pig->memGraph);
	PigMemGraphAddArena(&pig->memGraph, &pig->platHeap,  NewStr("platHeap"),  Grey10);
	PigMemGraphAddArena(&pig->memGraph, &pig->fixedHeap, NewStr("fixedHeap"), MonokaiGreen);
	PigMemGraphAddArena(&pig->memGraph, &pig->mainHeap,  NewStr("mainHeap"),  MonokaiOrange);
	PigMemGraphAddArena(&pig->memGraph, &pig->tempArena, NewStr("tempArena"), MonokaiBlue);
	PigMemGraphAddArena(&pig->memGraph, &pig->stdHeap,   NewStr("stdHeap"),   Grey10);
	PigMemGraphAddArena(&pig->memGraph, &pig->audioHeap, NewStr("audioHeap"), MonokaiPurple);
	InitializePigDebugOverlay(&pig->debugOverlay);
	InitPigAudioOutGraph(&pig->audioOutGraph);
	PigInitNotifications(&pig->notificationsQueue);
	
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	WriteLine_N("+==============================+");
	PrintLine_N("|    Pig engine v%u.%02u(%03u)     |", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	WriteLine_N("+==============================+");
	PrintLine_D("Running on %s", GetRenderApiStr(pig->renderApi));
	
	#if !DEBUG_BUILD && GYLIB_ASSERTIONS_ENABLED
	bool isFolder = false;
	plat->DoesFileExist(NewStr("Resources"), &isFolder);
	AssertMsg(isFolder, "Failed to find Resources directory. Please make sure that the executable is next to the Resources folder!");
	#endif
	
	SeedRand(11); //TODO: Get the timestamp and feed it in here!
	PigInitGlad();
	InitRenderContext();
	PigInitAudioOutput();
	PigInitSounds();
	Pig_InitializeInput();
	Pig_InitializeWindowStates();
	Pig_ChangeWindow(platInfo->mainWindow);
	
	pig->nextUiId = 1;
	pig->nextShaderId = 1;
	pig->nextTextureId = 1;
	pig->nextFrameBufferId = 1;
	pig->nextVectorImgId = 1;
	pig->nextVertBufferId = 1;
	pig->nextSpriteSheetId = 1;
	
	pig->firstUpdate = true;
	
	RcLoadBasicResources();
	Pig_InitResources();
	GamePinResources();
	Pig_LoadAllResources(!LOAD_ALL_RESOURCES_ON_STARTUP);
	PigInitMusicSystem(&pig->musicSystem);
	
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
	
	plat->CreateMutex(&pig->volumeMutex);
	pig->musicEnabled = true;
	pig->soundsEnabled = true;
	pig->masterVolume = 0.8f;
	pig->musicVolume  = 1.0f;
	pig->soundsVolume = 1.0f;
	if (plat->GetProgramArg(nullptr, NewStr("mute"), nullptr)) { pig->masterVolume = 0.0f; }
	if (plat->GetProgramArg(nullptr, NewStr("nomusic"), nullptr)) { pig->musicEnabled = false; }
	if (plat->GetProgramArg(nullptr, NewStr("nosounds"), nullptr)) { pig->soundsEnabled = false; }
	
	// +==============================+
	// |     Initialize AppState      |
	// +==============================+
	GameInitAppGlobals(&pig->appGlobals);
	GameAllocateAppStateStructs(&pig->appStateStructs);
	GameUpdateGlobals();
	Pig_InitializeAppStateStack();
	PushAppState(INITIAL_APP_STATE);
	Pig_HandleAppStateChanges(true);
	
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
	PigDebugOverlayCaptureMouse(&pig->debugOverlay);
	PigPerfGraphCaptureMouse(&pig->perfGraph);
	PigAudioOutGraphCaptureMouse(&pig->audioOutGraph);
	PigMemGraphCaptureMouse(&pig->memGraph);
	DebugConsoleCaptureMouse(&pig->debugConsole);
	
	PigUpdateNotifications(&pig->notificationsQueue);
	UpdatePigDebugOverlay(&pig->debugOverlay);
	UpdatePigPerfGraph(&pig->perfGraph);
	UpdatePigAudioOutGraph(&pig->audioOutGraph);
	UpdatePigMemGraph(&pig->memGraph);
	UpdateDebugConsole(&pig->debugConsole);
	Pig_HandleScreenshotHotkeys();
	
	Pig_HandleAppStateChanges(false);
	UpdateAppState(pig->currentAppState);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void PigRenderDebugOverlays()
{
	RcBindShader(&pig->resources.shaders->main2D);
	RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
	RcSetViewMatrix(Mat4_Identity);
	RcClearDepth(1.0f);
	RcSetDepth(0.0f);
	
	RenderDebugConsole(&pig->debugConsole);
	RenderPigAudioOutGraph(&pig->audioOutGraph);
	RenderPigPerfGraph(&pig->perfGraph);
	RenderPigMemGraph(&pig->memGraph);
	Pig_InputRenderDebugInfo();
	RenderPigDebugOverlay(&pig->debugOverlay);
	PigRenderNotifications(&pig->notificationsQueue);
	
	if (pig->currentWindowState->selectingSubPart || pig->currentWindowState->recordingGif)
	{
		reci selectedRec = pig->currentWindowState->screenSubPart;
		if (pig->currentWindowState->selectingSubPart)
		{
			selectedRec = NewReciBetween(pig->currentWindowState->subPartStartPos, Vec2Roundi(MousePos));
		}
		selectedRec = ReciOverlap(selectedRec, NewReci(0, 0, pig->currentWindow->input.contextResolution));
		RcDrawRectangleOutline(ToRec(selectedRec), ColorTransparent(Black, 0.5f), 1000000, true);
	}
}

void PigHandleTaskCompletedInputEvents()
{
	VarArrayLoop(&pigIn->inputEvents, eIndex)
	{
		VarArrayLoopGet(InputEvent_t, inputEvent, &pigIn->inputEvents, eIndex);
		if (inputEvent->type == InputEventType_TaskCompleted)
		{
			if (inputEvent->taskCompleted.task.input.type >= GameTask_Base)
			{
				GameHandleCompletedTask(&inputEvent->taskCompleted.task);
			}
			else
			{
				PigHandleCompletedTask(&inputEvent->taskCompleted.task);
			}
			inputEvent->handled = true;
		}
	}
}

void PigUpdate()
{
	UpdatePigPerfGraphBefore(&pig->perfGraph);
	PigHandlePlatformDebugLines(&pigIn->platDebugLines);
	PigHandleTaskCompletedInputEvents();
	Pig_UpdateWindowStates();
	Pig_UpdateInputBefore();
	PigUpdateSounds();
	PigUpdateMusicSystem(&pig->musicSystem);
	
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
				Pig_ChangeWindow(window);
				NotNull(pig->currentWindowState);
				#if 1
				FrameBuffer_t* renderBuffer = &pig->currentWindowState->frameBuffer;
				#else
				FrameBuffer_t* renderBuffer = nullptr;
				#endif
				
				if (isMainWindow)
				{
					RenderAppState(pig->currentAppState, renderBuffer);
					PigRenderDebugOverlays();
				}
				else
				{
					RcBegin(window, renderBuffer, &pig->resources.shaders->main2D, GetPredefPalColorByIndex(wIndex+4));
				}
				
				if (renderBuffer != nullptr)
				{
					PrepareFrameBufferTexture(renderBuffer);
					RcBegin(window, nullptr, &pig->resources.shaders->main2D, PalPurpleDark);
					RcBindTexture1(&renderBuffer->outTexture);
					RcDrawTexturedRectangle(NewRec(Vec2_Zero, ScreenSize), White);
				}
				Pig_UpdateCaptureHandling(pig->currentWindow, pig->currentWindowState);
				
				if (pig->currentWindowState->recordingGif)
				{
					RcBindShader(&pig->resources.shaders->main2D);
					RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
					RcSetViewMatrix(Mat4_Identity);
					RcSetDepth(0.0f);
					rec gifRecordingRec;
					gifRecordingRec.size = pig->resources.textures->gifRecording.size;
					gifRecordingRec.x = ScreenSize.width/2 - gifRecordingRec.width/2;
					gifRecordingRec.y = 10;
					RecAlign(&gifRecordingRec);
					RcBindTexture1(&pig->resources.textures->gifRecording);
					RcDrawTexturedRectangle(gifRecordingRec, White);
					
					RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
					i32 gifRecordTime = RoundR32i(pig->currentWindowState->gifFrames.count * (1000.0f / GIF_FRAMERATE));
					v2 recordingTextPos = NewVec2(gifRecordingRec.x + gifRecordingRec.width/2, gifRecordingRec.y + gifRecordingRec.height + 5 + RcGetMaxAscend());
					Vec2Align(&recordingTextPos);
					RcDrawTextPrintEx(recordingTextPos, White, TextAlignment_Center, 0, "F4 to stop (%s)", FormatMillisecondsNt(gifRecordTime, TempArena));
				}
			}
			window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
		}
		plat->SwapBuffers();
	}
	
	Pig_UpdateInputAfter();
	pig->firstUpdate = false;
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
	pig->reloadIndex++;
	
	NotifyPrint_N("Now running Pig DLL v%u.%02u(%03u)!", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	
	Pig_HandleResourcesOnReload();
	UpdateMemArenaFuncPntrs(&pig->platHeap, PlatAllocFunc, PlatFreeFunc);
	UpdateMemArenaFuncPntrs(&pig->mainHeap, PlatAllocFunc, PlatFreeFunc);
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	
	Pig_ChangeWindow(platInfo->mainWindow);
	GameHandleReload();
}

// +--------------------------------------------------------------+
// |                           Closing                            |
// +--------------------------------------------------------------+
void PigClosing()
{
	WriteLine_W("We are closing. But at least the engine DLL knows about it");
	//TODO: Implement me!
}
