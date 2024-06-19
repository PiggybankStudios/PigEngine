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
	pig->dontExitOnAssert = DEBUG_BUILD;
	
	u64 totalConsoleSpaceSize = DBG_CONSOLE_BUFFER_SIZE + DBG_CONSOLE_BUILD_SPACE_SIZE;
	u64 permanentMemoryNeeded = sizeof(PigState_t) + totalConsoleSpaceSize;
	Assert_(memory->persistentDataSize > permanentMemoryNeeded);
	UNUSED(permanentMemoryNeeded);
	InitMemArena_FixedHeap(&pig->fixedHeap, memory->persistentDataSize - sizeof(PigState_t) - totalConsoleSpaceSize, ((u8*)memory->persistentDataPntr) + sizeof(PigState_t) + totalConsoleSpaceSize);
	InitMemArena_PagedHeapFuncs(&pig->mainHeap, PIG_MAIN_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	InitMemArena_PagedHeapFuncs(&pig->threadSafeHeap, PIG_THREAD_SAFE_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	plat->CreateMutex(&pig->threadSafeHeap.mutex);
	InitMemArena_PagedHeapFuncs(&pig->audioHeap, PIG_AUDIO_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	InitMemArena_PagedHeapFuncs(&pig->largeAllocHeap, PIG_LARGE_ALLOC_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	#if LUA_SUPPORTED
	InitMemArena_PagedHeapFuncs(&pig->luaHeap, PIG_LUA_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	#endif
	#if PYTHON_SUPPORTED
	InitMemArena_PagedHeapFuncs(&pig->pythonHeap, PIG_LUA_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc, 0, AllocAlignment_8Bytes);
	FlagSet(pig->pythonHeap.flags, MemArenaFlag_CacheFreeOffset);
	#endif
	InitMemArena_MarkedStack(&pig->tempArena, memory->tempDataSize, memory->tempDataPntr, PIG_TEMP_MAX_MARKS);
	#if PIG_MAIN_ARENA_DEBUG
	InitMemArena_PagedHeapFuncs(&pig->mainHeapDebug, PIG_MAIN_ARENA_DEBUG_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	pig->mainHeap.debugArena = &pig->mainHeapDebug;
	pig->mainHeapDebug.debugName = NewStringInArenaNt(&pig->mainHeap, "mainHeapDebug").chars;
	#endif
	#if DEBUG_BUILD
	pig->mainHeap.debugName       = NewStringInArenaNt(&pig->mainHeap, "mainHeap").chars;
	pig->threadSafeHeap.debugName = NewStringInArenaNt(&pig->mainHeap, "threadSafeHeap").chars;
	pig->fixedHeap.debugName      = NewStringInArenaNt(&pig->mainHeap, "fixedHeap").chars;
	pig->audioHeap.debugName      = NewStringInArenaNt(&pig->mainHeap, "audioHeap").chars;
	pig->largeAllocHeap.debugName = NewStringInArenaNt(&pig->mainHeap, "largeAllocHeap").chars;
	#if LUA_SUPPORTED
	pig->luaHeap.debugName        = NewStringInArenaNt(&pig->mainHeap, "luaHeap").chars;
	#endif
	#if PYTHON_SUPPORTED
	pig->pythonHeap.debugName     = NewStringInArenaNt(&pig->mainHeap, "pythonHeap").chars;
	#endif
	pig->tempArena.debugName      = NewStringInArenaNt(&pig->mainHeap, "tempArena").chars;
	#endif
	TempPushMark();
	
	PerfTime_t initStartTime = GetPerfTime();
	u8* consoleSpace = ((u8*)memory->persistentDataPntr) + sizeof(PigState_t);
	InitializeDebugConsole(&pig->debugConsole, DBG_CONSOLE_BUFFER_SIZE, consoleSpace + DBG_CONSOLE_BUILD_SPACE_SIZE, DBG_CONSOLE_BUILD_SPACE_SIZE, consoleSpace);
	GameLoadSettings(&pig->settings, mainHeap);
	InitializePigPerfGraph(&pig->perfGraph);
	InitializePigMemGraph(&pig->memGraph);
	{
		MemArena_t* scratch1 = GetScratchArena();
		MemArena_t* scratch2 = GetScratchArena(scratch1);
		MemArena_t* scratch3 = GetScratchArena(scratch1, scratch2);
		PigMemGraphAddArena(&pig->memGraph, platInfo->stdHeap,    NewStr("stdHeap"),        Grey10);
		PigMemGraphAddArena(&pig->memGraph, &pig->fixedHeap,      NewStr("fixedHeap"),      MonokaiGreen);
		PigMemGraphAddArena(&pig->memGraph, &pig->mainHeap,       NewStr("mainHeap"),       MonokaiOrange);
		//TODO: Add the pig->threadSafeHeap here once we figure out how to do thread safe inspection
		PigMemGraphAddArena(&pig->memGraph, &pig->imguiHeap,      NewStr("imguiHeap"),      MonokaiYellow);
		#if LUA_SUPPORTED
		PigMemGraphAddArena(&pig->memGraph, &pig->luaHeap,        NewStr("luaHeap"),        MonokaiYellow);
		#endif
		#if PYTHON_SUPPORTED
		PigMemGraphAddArena(&pig->memGraph, &pig->pythonHeap,     NewStr("pythonHeap"),     MonokaiYellow);
		#endif
		PigMemGraphAddArena(&pig->memGraph, &pig->largeAllocHeap, NewStr("largeAllocHeap"), MonokaiBrown);
		PigMemGraphAddArena(&pig->memGraph, &pig->tempArena,      NewStr("tempArena"),      MonokaiBlue);
		PigMemGraphAddArena(&pig->memGraph, scratch1,             NewStr("scratch1"),       MonokaiBlue);
		PigMemGraphAddArena(&pig->memGraph, scratch2,             NewStr("scratch2"),       MonokaiBlue);
		PigMemGraphAddArena(&pig->memGraph, scratch3,             NewStr("scratch3"),       MonokaiBlue);
		PigMemGraphAddArena(&pig->memGraph, &pig->audioHeap,      NewStr("audioHeap"),      MonokaiPurple);
		FreeScratchArena(scratch1);
		FreeScratchArena(scratch2);
		FreeScratchArena(scratch3);
	}
	InitializePigDebugOverlay(&pig->debugOverlay);
	InitPigAudioOutGraph(&pig->audioOutGraph);
	PigInitNotifications(&pig->notificationsQueue);
	PigInitConfirmDialogs();
	
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	WriteLine_N("+==============================+");
	PrintLine_N("|    Pig engine v%u.%02u(%03u)     |", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	WriteLine_N("+==============================+");
	
	#if !DEBUG_BUILD && GYLIB_ASSERTIONS_ENABLED
	bool isFolder = false;
	plat->DoesFileExist(NewStr("Resources"), &isFolder);
	AssertMsg(isFolder, "Failed to find Resources directory. Please make sure that the executable is next to the Resources folder!");
	#endif
	
	#if OPENGL_SUPPORTED
	PigInitGlad();
	const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
	PrintLine_D("Using %s API on %s %s", GetRenderApiStr(pig->renderApi), vendor, renderer);
	#endif
	
	SeedRand((u32)LocalTimestamp);
	CreateRandomSeries(&pig->random);
	SeedRandomSeriesU64(&pig->random, LocalTimestamp);
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
	Pig_LoadResourceAtStartup(!LOAD_ALL_RESOURCES_ON_STARTUP, BEFORE_RESOURCES_LOAD_PERCENT, RESOURCES_LOAD_PERCENT);
	#if STEAM_BUILD
	Pig_InitializeSteamAvatars();
	#endif
	PigInitMusicSystem(&pig->musicSystem);
	PigInitDebugBindings(&pig->sessionDebugBindings, fixedHeap);
	PigInitDebugBindings(&pig->debugBindings, fixedHeap);
	GameLoadDebugBindings(&pig->debugBindings);
	PigInitImgui();
	#if LUA_SUPPORTED
	PigInitLua();
	#endif
	#if PYTHON_SUPPORTED
	PigInitPython();
	#endif
	
	plat->CreateMutex(&pig->volumeMutex);
	pig->musicEnabled = true;
	pig->soundsEnabled = true;
	pig->masterVolume = 1.0f;
	pig->musicVolume  = 1.0f;
	pig->soundsVolume = 1.0f;
	
	// +==============================+
	// |      Game General Init       |
	// +==============================+
	GameInitAppGlobals(&pig->appGlobals);
	GameAllocateAppStateStructs(&pig->appStateStructs);
	GameUpdateGlobals();
	Pig_InitializeAppStateStack();
	GameGeneralInit();
	
	pig->initialized = true;
	PerfTime_t initEndTime = GetPerfTime();
	PrintLine_D("Initialization took %.1lfms (%.1lfms total startup time)", GetPerfTimeDiff(&initStartTime, &initEndTime), GetPerfTimeDiff(&platInfo->programStartPerfTime, &initEndTime));
	TempPopMark();
	u64 numMarks = GetNumMarks(TempArena);
	Assert(numMarks == 0);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void PigUpdateMainWindow()
{
	CheckScratchNumMarksDuringUpdate();
	PigUpdateImguiBefore();
	Pig_UpdateResources();
	
	PigNotificationsCaptureMouse(&pig->notificationsQueue);
	PigDebugOverlayCaptureMouse(&pig->debugOverlay);
	PigPerfGraphCaptureMouse(&pig->perfGraph);
	PigAudioOutGraphCaptureMouse(&pig->audioOutGraph);
	PigMemGraphCaptureMouse(&pig->memGraph);
	DebugConsoleCaptureMouse(&pig->debugConsole);
	PigCaptureMouseConfirmDialogs();
	
	PigUpdateNotifications(&pig->notificationsQueue);
	UpdatePigDebugOverlay(&pig->debugOverlay);
	UpdatePigPerfGraph(&pig->perfGraph);
	UpdatePigAudioOutGraph(&pig->audioOutGraph);
	UpdatePigMemGraph(&pig->memGraph);
	UpdateDebugConsole(&pig->debugConsole);
	PigUpdateConfirmDialogs();
	Pig_HandleScreenshotHotkeys();
	Pig_HandleDebugBindings(&pig->sessionDebugBindings);
	Pig_HandleDebugBindings(&pig->debugBindings);
	#if STEAM_BUILD
	Pig_UpdateSteamAvatars();
	#endif
	PigImguiHandleInputEventsAndCaptureMouse();
	
	GameGeneralUpdate();
	Pig_HandleAppStateChanges(false);
	UpdateAppState(pig->currentAppState);
	
	PigUpdateImguiAfter();
	CheckScratchNumMarksDuringUpdate();
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void PigRenderForcedOverlays()
{
	// +==============================+
	// |   Render Capture Area Rec    |
	// +==============================+
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
	
	// +==============================+
	// | Render Time Scale Indicator  |
	// +==============================+
	if (pigOut->fixedTimeScaleEnabled)
	{
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
		v2 scaleTextPos = NewVec2(5, ScreenSize.height - 5 - RcGetMaxDescend());
		Vec2Align(&scaleTextPos);
		MyStr_t timeScaleStr = TempPrintStr("Time %lgx (%lgfps)", pigOut->fixedTimeScale, 1000.0 / (PIG_DEFAULT_FRAME_TIME * pigOut->fixedTimeScale));
		RcDrawText(timeScaleStr, scaleTextPos + NewVec2(0, 2), Black);
		RcDrawText(timeScaleStr, scaleTextPos, White);
	}
}
void PigRenderOutOfScreenshotOverlays()
{
	RcBindShader(&pig->resources.shaders->main2D);
	RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
	RcSetViewMatrix(Mat4_Identity);
	RcSetDepth(0.0f);
	
	// +================================+
	// | Render Recording GIF Indicator |
	// +================================+
	if (pig->currentWindowState->recordingGif)
	{
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
void PigRenderDebugOverlays()
{
	RcBindShader(&pig->resources.shaders->main2D);
	RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
	RcSetViewMatrix(Mat4_Identity);
	RcClearDepth(1.0f); //TODO: Do we need this?
	RcSetDepth(0.0f);
	
	PigRenderImgui();
	RenderPigDebugOverlayBelowConsole(&pig->debugOverlay);
	PigRenderConfirmDialogs();
	RenderDebugConsole(&pig->debugConsole);
	RenderPigAudioOutGraph(&pig->audioOutGraph);
	RenderPigPerfGraph(&pig->perfGraph);
	RenderPigMemGraph(&pig->memGraph);
	Pig_InputRenderDebugInfo();
	RenderPigDebugOverlay(&pig->debugOverlay);
	PigRenderNotifications(&pig->notificationsQueue);
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

#if DEBUG_BUILD
static u64 numMarksOnUpdateEntry1 = 0;
static u64 numMarksOnUpdateEntry2 = 0;
static u64 numMarksOnUpdateEntry3 = 0;
void CheckScratchNumMarksDuringUpdate(u64 numExtraMarks1, u64 numExtraMarks2, u64 numExtraMarks3) //pre-declared in app_func_defs.h
{
	MemArena_t* scratch1 = GetScratchArena();
	MemArena_t* scratch2 = GetScratchArena(scratch1);
	MemArena_t* scratch3 = GetScratchArena(scratch1, scratch2);
	u64 numMarksAtExit1 = GetNumMarks(scratch1);
	u64 numMarksAtExit2 = GetNumMarks(scratch2);
	u64 numMarksAtExit3 = GetNumMarks(scratch3);
	Assert(numMarksAtExit1 == numMarksOnUpdateEntry1 + numExtraMarks1);
	Assert(numMarksAtExit2 == numMarksOnUpdateEntry2 + numExtraMarks2);
	Assert(numMarksAtExit3 == numMarksOnUpdateEntry3 + numExtraMarks3);
	FreeScratchArena(scratch1);
	FreeScratchArena(scratch2);
	FreeScratchArena(scratch3);
}
#else
void CheckScratchNumMarksDuringUpdate(u64 numExtraMarks1, u64 numExtraMarks2, u64 numExtraMarks3) { UNUSED(numExtraMarks1); UNUSED(numExtraMarks2); UNUSED(numExtraMarks3); }
#endif

void PigUpdate()
{
	#if DEBUG_BUILD
	{
		MemArena_t* scratch1 = GetScratchArena();
		MemArena_t* scratch2 = GetScratchArena(scratch1);
		MemArena_t* scratch3 = GetScratchArena(scratch1, scratch2);
		numMarksOnUpdateEntry1 = GetNumMarks(scratch1);
		numMarksOnUpdateEntry2 = GetNumMarks(scratch2);
		numMarksOnUpdateEntry3 = GetNumMarks(scratch3);
		FreeScratchArena(scratch1);
		FreeScratchArena(scratch2);
		FreeScratchArena(scratch3);
	}
	#endif
	CheckScratchNumMarksDuringUpdate();
	
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
					u64 firstRenderingAppStateIndex = pig->appStateStackSize-1;
					for (u64 aIndex = pig->appStateStackSize; aIndex > 0; aIndex--)
					{
						firstRenderingAppStateIndex = aIndex-1;
						if (DoesAppStateCoverBelow(pig->appStateStack[aIndex-1])) { break; }
					}
					for (u64 aIndex = firstRenderingAppStateIndex; aIndex < pig->appStateStackSize; aIndex++)
					{
						RenderAppState(pig->appStateStack[aIndex], renderBuffer, (aIndex == firstRenderingAppStateIndex));
					}
					PigRenderDebugOverlays();
					PigRenderForcedOverlays();
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
				PigRenderOutOfScreenshotOverlays();
			}
			window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
		}
		plat->SwapBuffers();
	}
	
	Pig_UpdateInputAfter();
	CheckScratchNumMarksDuringUpdate();
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
	UpdateMemArenaFuncPntrs(&pig->mainHeap, PlatAllocFunc, PlatFreeFunc);
	UpdateMemArenaFuncPntrs(&pig->threadSafeHeap, PlatAllocFunc, PlatFreeFunc);
	UpdateMemArenaFuncPntrs(&pig->largeAllocHeap, PlatAllocFunc, PlatFreeFunc);
	#if LUA_SUPPORTED
	UpdateMemArenaFuncPntrs(&pig->luaHeap, PlatAllocFunc, PlatFreeFunc);
	#endif
	#if PYTHON_SUPPORTED
	UpdateMemArenaFuncPntrs(&pig->pythonHeap, PlatAllocFunc, PlatFreeFunc);
	#endif
	UpdateMemArenaFuncPntrs(&pig->audioHeap, PlatAllocFunc, PlatFreeFunc);
	GyLibDebugOutputFunc = Pig_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Pig_GyLibDebugPrintHandler;
	
	DebugConsoleClearRegisteredCommands(&pig->debugConsole);
	
	Pig_ChangeWindow(platInfo->mainWindow);
	PigImguiHandleReload();
	#if LUA_SUPPORTED
	PigLuaHandleReload();
	#endif
	#if PYTHON_SUPPORTED
	PigPythonHandleReload();
	#endif
	GameHandleReload();
}

// +--------------------------------------------------------------+
// |                           Closing                            |
// +--------------------------------------------------------------+
void PigClosing()
{
	WriteLine_W("We are closing. But at least the engine DLL knows about it");
	GamePrepareForClose();
	for (u64 sIndex = pig->appStateStackSize; sIndex > 0; sIndex--)
	{
		StopAppState(pig->appStateStack[sIndex-1], AppState_None, true, true);
	}
	PigImguiHandleClosing();
	pig->appStateStackSize = 0;
}
