/*
File:   win32_gfx_test.cpp
Author: Taylor Robbins
Date:   02\05\2024
Description: 
	** In order to test graphics device configuration for various APIs
	** This is a copy of win32_main.cpp that has a lot of logic ripped out
	** All other win32_ files that win32_main.cpp would have included are
	**   also included by this file but any place that needs to consider the
	**   difference will check #ifdef Win32_GFX_TEST, which is #defined at the top here
*/

#define WIN32_GFX_TEST //TODO: Once we fully move over to pig_graphics, we should remove this #define and simplify all the places that check it
#define WIN32_BLANK_RENDER_API RenderApi_Vulkan

#define WIN32_OPEN_CONSOLE_WINDOW_AT_START false //TODO: Find a better home for this?

#define PIG_GFX_OPENGL_SUPPORTED 0
#define PIG_GFX_WEBGL_SUPPORTED  0
#define PIG_GFX_VULKAN_SUPPORTED 1
#define PIG_GFX_D3D11_SUPPORTED  0
#define PIG_GFX_D3D12_SUPPORTED  0
#define PIG_GFX_METAL_SUPPORTED  0
#define PIG_GFX_GLFW_SUPPORTED   1

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define PLATFORM_LAYER
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#include "common_includes.h"

#include "win32/win32_version.h"
#include "win32/win32_types.h"
#include "win32/win32_main.h"
#include "win32/win32_controller_types.h"

// +--------------------------------------------------------------+
// |                       Platform Globals                       |
// +--------------------------------------------------------------+
Win32InitPhase_t InitPhase = Win32InitPhase_Entry;
Win32PlatformState_t* Platform = nullptr;
ThreadId_t MainThreadId = 0;

void Win32_DoMainLoopIteration(bool pollEvents); //pre-declared so win32_glfw.cpp can use it

// +--------------------------------------------------------------+
// |                Personal Library Source Files                 |
// +--------------------------------------------------------------+
#include "graphics/pig_graphics.cpp"
#include "gylib/gy_temp_memory.cpp"
#include "gylib/gy_scratch_arenas.cpp"

// +--------------------------------------------------------------+
// |                        STB Libraries                         |
// +--------------------------------------------------------------+
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(size)              AllocMem  (&Platform->stdHeap, (size))
#define STBI_REALLOC(pointer, newSize) ReallocMem(&Platform->stdHeap, (pointer), (newSize), 0, AllocAlignment_None, true) //ignoreNullptr: true
#define STBI_FREE(pointer)             FreeMem   (&Platform->stdHeap, (pointer), 0, true) //ignoreNullper: true
#define STBI_ASSERT(expression)        Assert    (expression)
#pragma warning(push)
#pragma warning(disable:4244)
#include "stb/stb_image.h"
#pragma warning(pop)

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_SORT qsort
#define STBRP_ASSERT Assert
#include "stb/stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_ifloor(x) FloorR32i(x)
#define STBTT_iceil(x)  CeilR32i(x)
#define STBTT_sqrt(x)   SqrtR32(x)
#define STBTT_pow(x, exp)    PowR32((x), (exp))
#define STBTT_fabs(x)   AbsR32(x)
#define STBTT_malloc(numBytes, user) AllocMem((MemArena_t*)(user), (numBytes))
#define STBTT_free(pointer, user)    FreeMem((MemArena_t*)(user), (pointer), 0, true) //ignoreNullper: true
#define STBTT_assert(expession)      Assert(expession)
#define STBTT_strlen(str)            MyStrLength32(str)
#define STBTT_memcpy                 MyMemCopy
#define STBTT_memset                 MyMemSet
#include "stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(size)              AllocMem  (&Platform->stdHeap, (size))
#define STBIW_REALLOC(pointer, newSize) ReallocMem(&Platform->stdHeap, (pointer), (newSize), 0, AllocAlignment_None, true) //ignoreNullptr: true
#define STBIW_FREE(pointer)             FreeMem   (&Platform->stdHeap, (pointer), 0, true) //ignoreNullper: true
#define STBIW_ASSERT(expression)        Assert    (expression)
#include "stb/stb_image_write.h"

// +--------------------------------------------------------------+
// |                    Platform Source Files                     |
// +--------------------------------------------------------------+
#include "win32/win32_func_defs.h"
#include "win32/win32_debug.cpp"
#include "win32/win32_program_args.cpp"
#include "win32/win32_performance.cpp"
#include "common_performance.cpp"
#include "win32/win32_core.cpp"
#include "win32/win32_threading.cpp"
#include "win32/win32_files.cpp"
#include "win32/win32_file_watching.cpp"
#include "win32/win32_dll_loading.cpp"
#include "win32/win32_process.cpp"
#include "win32/win32_steam.cpp"
#include "win32/win32_box2d.cpp"
#include "win32/win32_audio.cpp"
#include "win32/win32_clipboard.cpp"
#include "win32/win32_input.cpp"
#include "win32/win32_glfw.cpp"
#include "win32/win32_monitors.cpp"
#include "win32/win32_fonts.cpp"
// #include "win32/win32_render_basic.cpp"
// #include "win32/win32_overlays.cpp"
// #include "win32/win32_loading.cpp"
#include "win32/win32_assert.cpp"

#include "win32/win32_gfx_callbacks.cpp"
#include "win32/win32_interface_filling.cpp"
#include "win32/win32_engine_output.cpp"

// +--------------------------------------------------------------+
// |                    Win32 Main Entry Point                    |
// +--------------------------------------------------------------+
#if WIN32_OPEN_CONSOLE_WINDOW_AT_START
int main(int argc, char* argv[])
#define USED_WIN_MAIN_ENTRY_POINT 0
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#define USED_WIN_MAIN_ENTRY_POINT 1
#endif
{
	#if USED_WIN_MAIN_ENTRY_POINT
	UNUSED(hInstance); //TODO: Remove me!
	UNUSED(hPrevInstance); //TODO: Remove me!
	#else
	UNUSED(argc);
	UNUSED(argv);
	#endif
	
	Win32_CoreInit((USED_WIN_MAIN_ENTRY_POINT != 0));
	Win32_PerformanceInit();
	PerfTime_t initStartTime = Win32_GetPerfTime();
	TempPushMark();
	
	// +==============================+
	// |   InitializingDebugOutput    |
	// +==============================+
	StartPerfSections(Win32InitPhase_NumPhases, "InitializingDebugOutput", true);
	InitPhase = Win32InitPhase_CoreInitialized;
	
	Win32_DebugInit();
	
	// +==============================+
	// |      ParsingProgramArgs      |
	// +==============================+
	PerfSection("ParsingProgramArgs");
	InitPhase = Win32InitPhase_DebugOutputInitialized;
	
	WriteLine_O("+==============================+");
	PrintLine_O("|     PigEngine v%u.%02u(%03u)     |", Platform->version.major, Platform->version.minor, Platform->version.build);
	WriteLine_O("+==============================+");
	
	#if 0
	//TODO: Remove this test
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
		// double testValue = strtod(testStr, nullptr);
		double testValue = (double)strtox(testStr, nullptr, FloatSize_64);
		PrintLine_I("Test[%d]: \"%s\" -> %f (%.g)", testIndex, testStr, testValue, testValue);
	}
	#endif
	
	#if USED_WIN_MAIN_ENTRY_POINT
	Win32_ProcessProgramArguments(lpCmdLine, nCmdShow);
	#else
	Win32_ProcessProgramArguments(argc, argv);
	#endif
	PrintLine_D("Passed %u program argument%s%s", Platform->programArgs.count, (Platform->programArgs.count == 1) ? "" : "s", (Platform->programArgs.count > 0) ? ":" : "");
	for (u32 aIndex = 0; aIndex < Platform->programArgs.count; aIndex++)
	{
		MyStr_t* argument = &Platform->programArgs.args[aIndex];
		PrintLine_D(" [%u]: \"%.*s\"", aIndex, StrPntrPrint(argument));
	}
	
	// +======================================+
	// | InitializingThreadingAndDirectories  |
	// +======================================+
	PerfSection("InitializingThreadingAndDirectories");
	InitPhase = Win32InitPhase_ProgramArgsParsed;
	
	Win32_FindExePathAndWorkingDir();
	Win32_ChooseDefaultDirectory();
	
	Win32_InitThreading();
	
	// +==============================+
	// |     AudioInitialization      |
	// +==============================+
	PerfSection("AudioInitialization");
	InitPhase = Win32InitPhase_ThreadingInitialized;
	
	Win32_AudioInit();
	
	// +==============================+
	// |       InitializingGlfw       |
	// +==============================+
	PerfSection("InitializingGlfw");
	InitPhase = Win32InitPhase_AudioInitialized;
	
	Win32_GlfwInit();
	Win32_LoadGameControllerDbFile(NewStr(SDL_CONTROLLER_DB_PATH));
	Win32_InitMonitors();
	Win32_FillMonitorInfo();
	
	// +==============================+
	// |    GettingStartupOptions     |
	// +==============================+
	PerfSection("GettingStartupOptions");
	InitPhase = Win32InitPhase_GlfwInitialized;
	
	Win32_FillStartupInfo(&Platform->startupInfo);
	{
		Platform->startupOptions.mainMemoryRequest = Kilobytes(256);
		Platform->startupOptions.tempMemoryRequest = Kilobytes(256);
		Platform->startupOptions.scratchArenaSizes = Kilobytes(256);
		Platform->startupOptions.openDebugConsole = true;
		Platform->startupOptions.renderApi = WIN32_BLANK_RENDER_API;
		Platform->startupOptions.numWindows = 1;
		PlatWindowOptions_t windowOptions = {};
		windowOptions.create.resizableWindow = true;
		windowOptions.create.topmostWindow = false;
		windowOptions.create.decoratedWindow = true;
		windowOptions.create.antialiasingNumSamples = 4;
		windowOptions.create.autoIconify = false;
		windowOptions.create.windowTitle = NewStr(PROJECT_NAME);
		windowOptions.create.fullscreen = false;
		windowOptions.create.fullscreenMonitor = nullptr;
		windowOptions.create.fullscreenVideoMode = nullptr;
		windowOptions.create.fullscreenFramerateIndex = 0;
		windowOptions.create.windowedResolution = NewVec2i(1600, 900);
		windowOptions.create.windowedLocation = NewVec2i(20, 50);
		windowOptions.create.windowedMaximized = false;
		windowOptions.create.windowedFramerate = 60;
		windowOptions.enforceMinSize = true;
		windowOptions.minWindowSize = NewVec2i(400, 200);
		windowOptions.enforceMaxSize = false;
		windowOptions.maxWindowSize = Vec2i_Zero;
		windowOptions.forceAspectRatio = false;
		windowOptions.aspectRatio = NewVec2i(0, 0);
		Platform->startupOptions.windowOptions = &windowOptions;
		MyStr_t iconPaths[6];
		iconPaths[0] = NewStr("Resources/icon16.png");
		iconPaths[1] = NewStr("Resources/icon24.png");
		iconPaths[2] = NewStr("Resources/icon32.png");
		iconPaths[3] = NewStr("Resources/icon64.png");
		iconPaths[4] = NewStr("Resources/icon120.png");
		iconPaths[5] = NewStr("Resources/icon256.png");
		Platform->startupOptions.numIconFiles = ArrayCount(iconPaths);
		Platform->startupOptions.iconFilePaths = &iconPaths[0];
		Platform->startupOptions.loadingBackgroundColor = White;
		Platform->startupOptions.loadingBarColor = Black;
		Platform->startupOptions.loadingImagePath = NewStr("Resources/Sprites/pig_loading_image.png");
		Platform->startupOptions.loadingBackPath = NewStr("Resources/Textures/pig_checker_blue.png");
		Platform->startupOptions.loadingBackTiling = true;
		Platform->startupOptions.loadingBackScale = 2.0f;
		Platform->startupOptions.audioDeviceIndex = 0;
		Platform->startupOptions.audioOutputFormat;
		Platform->startupOptions.audioOutputFormat.bitsPerSample = 32;
		Platform->startupOptions.audioOutputFormat.numChannels = 2;
		Platform->startupOptions.audioOutputFormat.samplesPerSecond = 44100;
		Platform->startupOptions.threadPoolSize = 0;
		Platform->startupOptions.threadPoolTempArenasSize = 0;
		Platform->startupOptions.threadPoolTempArenasNumMarks = 0;
		Platform->startupOptions.threadPoolScratchArenasMaxSize = 0;
		Platform->startupOptions.threadPoolScratchArenasNumMarks = 0;
	}
	
	Platform->renderApi = Platform->startupOptions.renderApi;
	
	AssertMsg(Platform->startupOptions.numWindows > 0, "The engine DLL requested 0 windows. Is the engine DLL corrupt?");
	NotNullMsg(Platform->startupOptions.windowOptions, "The engine DLL did not fill out the windowsOptions array. Is the engine DLL corrupt?");
	AssertMsg(Platform->startupOptions.renderApi != RenderApi_None, "The engine DLL did not choose a valid Render API to use. Is the engine DLL corrupt?");
	NotEmptyStr(&Platform->startupOptions.loadingImagePath);
	AssertMsg(Platform->startupOptions.audioDeviceIndex < Platform->startupInfo.audioDevices.length, "Engine chose an invalid audio device index!");
	AssertMsg(Platform->startupOptions.threadPoolSize <= PLAT_MAX_NUM_THREADS, "Engine wanted too many threads in it's thread pool!");
	AssertIfMsg(Platform->startupOptions.threadPoolTempArenasSize > 0, Platform->startupOptions.threadPoolTempArenasNumMarks > 0, "TempArenas for thread pools cannot have 0 marks");
	
	// +===============================+
	// | Initializing Sockets and HTTP |
	// +===============================+
	PerfSection("InitializingSocketsAndHttp");
	InitPhase = Win32InitPhase_StartupOptionsObtained;
	
	// +==============================+
	// |      ThreadPoolCreation      |
	// +==============================+
	PerfSection("ThreadPoolCreation");
	InitPhase = Win32InitPhase_SocketsInitialized;
	
	Win32_InitThreadPool(
		Platform->startupOptions.threadPoolSize,
		Platform->startupOptions.threadPoolTempArenasSize,
		Platform->startupOptions.threadPoolTempArenasNumMarks,
		Platform->startupOptions.threadPoolScratchArenasMaxSize,
		Platform->startupOptions.threadPoolScratchArenasNumMarks
	);
	
	// +==============================+
	// |        Graphics Init         |
	// +==============================+
	PigGfxContext_t pigGfxContext = {};
	pigGfxContext.InitFailure = Win32_PigGfxInitFailure;
	pigGfxContext.DebugOutput = Win32_PigGfxDebugOutput;
	pigGfxContext.DebugPrint = Win32_PigGfxDebugPrint;
	if (!PigGfx_Init(&pigGfxContext, &Platform->mainHeap, &Platform->mainHeap, Platform->startupOptions.renderApi))
	{
		PrintLine_E("The Win32 platform layer does not support %s as a render API yet", GetRenderApiStr(Platform->startupOptions.renderApi));
		Win32_InitError("Unsupported win32 render API chosen by the engine");
	}
	
	PigGfxOptions_t gfxOptions = {};
	PigGfx_FillDefaultOptions(&gfxOptions);
	gfxOptions.numAntialiasingSamples = 4; //TODO: This should really source from windowOptions.create.antialiasingNumSamples!
	#if PIG_GFX_OPENGL_SUPPORTED
	gfxOptions.opengl_RequestVersionMajor = OPENGL_REQUEST_VERSION_MAJOR;
	gfxOptions.opengl_RequestVersionMinor = OPENGL_REQUEST_VERSION_MINOR;
	gfxOptions.opengl_RequestProfile = OPENGL_REQUEST_PROFILE;
	gfxOptions.opengl_RequestForwardCompat = OPENGL_FORCE_FORWARD_COMPAT;
	gfxOptions.opengl_requestDebugContext = OPENGL_DEBUG_CONTEXT;
	#endif
	#if PIG_GFX_VULKAN_SUPPORTED
	gfxOptions.vulkan_ApplicationName = PROJECT_NAME_SAFE;
	gfxOptions.vulkan_ApplicationVersionInt = VK_MAKE_VERSION(1, 0, 0);
	gfxOptions.vulkan_EngineName = "PigEngineWin32";
	gfxOptions.vulkan_EngineVersionInt = VK_MAKE_VERSION(WIN32_VERSION_MAJOR, WIN32_VERSION_MINOR, WIN32_VERSION_BUILD);
	gfxOptions.vulkan_RequestVersionMajor = 1;
	gfxOptions.vulkan_RequestVersionMinor = 3;
	#endif
	PigGfx_SetOptions(&gfxOptions);
	
	// +==============================+
	// |        WindowOpening         |
	// +==============================+
	PerfSection("WindowOpening");
	InitPhase = Win32InitPhase_ThreadPoolsCreated;
	
	if (Platform->startupOptions.openDebugConsole)
	{
		Win32_OpenConsoleWindow();
		WriteLine_I("Opened console window that was requested by engine DLL");
	}
	
	for (u64 wIndex = 0; wIndex < Platform->startupOptions.numWindows; wIndex++)
	{
		NotEmptyStr(&Platform->startupOptions.windowOptions[wIndex].create.windowTitle);
		PigGfx_SetGlfwWindowHints();
		PlatWindow_t* newWindow = Win32_GlfwCreateWindow(&Platform->startupOptions.windowOptions[wIndex].create);
		if (newWindow == nullptr)
		{
			Win32_InitError("Failed to create application window through GLFW. This is usually caused by your graphics drivers not supporting the minimum version of OpenGL that we require.");
		}
		PigGfx_SwitchToGlfwWindow(newWindow->handle);
		Win32_ApplyWindowOptions(newWindow, &Platform->startupOptions.windowOptions[wIndex]);
		Win32_LoadWindowIcon(newWindow, Platform->startupOptions.numIconFiles, Platform->startupOptions.iconFilePaths);
		if (wIndex == 0)
		{
			Platform->mainWindow = newWindow;
			newWindow->activeInput.isFocused = true;
			Platform->gfxContext = PigGfx_CreateContext(&Platform->mainHeap);
			if (Platform->gfxContext == nullptr)
			{
				Win32_InitError("Failed to create graphics context after window creation!");
			}
			#if PIG_GFX_OPENGL_SUPPORTED
			if (Platform->gfxContext->renderApi == RenderApi_OpenGL)
			{
				PrintLine_I("Successfully created %s v%d.%d graphics context!", GetRenderApiStr(Platform->gfxContext->renderApi), Platform->gfxContext->openglVersionMajor, Platform->gfxContext->openglVersionMinor);
			}
			#endif
			if (Platform->gfxContext->renderApi != RenderApi_OpenGL)
			{
				PrintLine_I("Successfully created %s graphics context!", GetRenderApiStr(Platform->gfxContext->renderApi));
			}
		}
	}
	NotNull(Platform->mainWindow);
	PigGfx_SwitchToGlfwWindow(Platform->mainWindow->handle);
	
	Win32_GetContextAndWindowHandles();
	Win32_LoadCursors();
	
	// +==============================+
	// |       StartAudioOutput       |
	// +==============================+
	PerfSection("StartAudioOutput");
	InitPhase = Win32InitPhase_WindowOpened;
	
	Win32_StartAudioOutput(Platform->startupOptions.audioDeviceIndex, &Platform->startupOptions.audioOutputFormat);
	
	// +==============================+
	// |    LoadingBasicResources     |
	// +==============================+
	PerfSection("LoadingBasicResources");
	InitPhase = Win32InitPhase_AudioOutputStarted;
	
	// Win32_LoadBasicResources(&Platform->startupOptions);
	// Win32_InitBasicRendering();
	// Win32_InitOverlays();
	Platform->loadingPercent = 0.0f;
	// Win32_RenderLoadingScreen(0.0f);
	
	// +==================================+
	// | FileWatchingAndInterfaceFilling  |
	// +==================================+
	PerfSection("FileWatchingAndInterfaceFilling");
	InitPhase = Win32InitPhase_ResourcesLoaded;
	
	Win32_InitProcessManagement();
	Win32_InitFileWatching();
	
	Win32_FillPlatformInfo(&Platform->info, initStartTime);
	Win32_FillPlatformApi(&Platform->api);
	Win32_FillEngineMemory(&Platform->engineMemory, Platform->startupOptions.mainMemoryRequest, Platform->startupOptions.tempMemoryRequest);
	Win32_FillEngineOutput(&Platform->engineOutput);
	
	u64 initProgramTime = Win32_GetProgramTime(nullptr, true);
	u64 initUnixTimestamp = Win32_GetCurrentTimestamp(false);
	u64 initLocalTimestamp = Win32_GetCurrentTimestamp(true);
	
	Platform->loadingPercent = 1.0f;
	// Win32_RenderLoadingScreen(0.0f);
	
	// +==============================+
	// |     EngineInitialization     |
	// +==============================+
	PerfSection("EngineInitialization");
	InitPhase = Win32InitPhase_EngineInitialization;
	
	PerfTime_t initEndTime = Win32_GetPerfTime();
	PrintLine_N("Blank platform took %.1lfms to init", Win32_GetPerfTimeDiff(&initStartTime, &initEndTime));
	if (Platform->mainWindow == nullptr || Platform->mainWindow->closed || glfwWindowShouldClose(Platform->mainWindow->handle))
	{
		WriteLine_E("Exited during initialization");
		Win32_GlfwCleanup();
		return 0;
	}
	WriteLine_N("Pig_Initialize Complete");
	Platform->firstUpdateStartTime = Win32_GetPerfTime();
	
	Win32_CheckForThreadAssertions();
	
	// +==============================+
	// |      EngineInputFilling      |
	// +==============================+
	PerfSection("EngineInputFilling");
	InitPhase = Win32InitPhase_Initialized;
	
	Win32_FillEngineInput(&Platform->engineActiveInput);
	Win32_CopyEngineInput(&Platform->engineInput, &Platform->engineActiveInput);
	Win32_CopyEngineInput(&Platform->enginePreviousInput, &Platform->engineInput);
	Platform->hasReachedStableFramerate = false;
	Platform->numStableElapsedMsInARow = 0;
	
	// +==============================+
	// |    Perf Sections Printout    |
	// +==============================+
	EndPerfSections();
	BundlePerfSections(&Platform->mainHeap, &Platform->info.initPerfSectionBundle);
	PrintLine_I("Platform init sections took %.1lfms total", GetTotalPerfSectionsTime());
	for (u64 sIndex = 0; sIndex < GetNumPerfSections(); sIndex++)
	{
		PrintLine_D("  %s: %.1lfms", GetPerfSectionName(sIndex), GetPerfSectionTimeByIndex(sIndex));
	}
	
	// +--------------------------------------------------------------+
	// |                       Main Window Loop                       |
	// +--------------------------------------------------------------+
	TempPopMark();
	InitPhase = Win32InitPhase_DoingFirstUpdate;
	while (Platform->mainWindow->handle != nullptr &&
		!glfwWindowShouldClose(Platform->mainWindow->handle) &&
		!Platform->exitRequested)
	{
		Win32_DoMainLoopIteration(true);
	}
	
	// +==============================+
	// |           Closing            |
	// +==============================+
	InitPhase = Win32InitPhase_Closing;
	
	if (Platform->engine.isValid)
	{
		Platform->engine.Closing(&Platform->info, &Platform->api, &Platform->engineMemory);
	}
	
	Win32_GlfwCleanup();
	
	return 0;
}

void Win32_DoMainLoopIteration(bool pollEvents) //pre-declared above
{
	//NOTE: This could cause us to recurse once so it should happen outside the TempPushMark
	if (pollEvents) { Win32_PollEventsAndCheckWindows(); }
	if (Platform->mainWindow->closed) { return; }
	
	TempPushMark();
	
	Win32_CheckForThreadAssertions();
	Win32_UpdateRunningProcesses();
	Win32_UpdateAudio();
	Win32_UpdateFileWatching();
	Win32_CheckControllerInputs(&Platform->engineActiveInput);
	
	Win32_CopyEngineInput(&Platform->enginePreviousInput, &Platform->engineInput);
	Win32_CopyEngineInput(&Platform->engineInput, &Platform->engineActiveInput);
	Win32_ResetEngineInput(&Platform->engineActiveInput);
	bool windowInteractionOccurred = false;
	PlatWindow_t* window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		NotNull(window);
		if (!window->closed)
		{
			Win32_CopyWindowEngineInput(&window->prevInput, &window->input);
			Win32_CopyWindowEngineInput(&window->input, &window->activeInput);
			Win32_ResetWindowEngineInput(&window->activeInput);
			if (window->input.windowInteractionOccurred)
			{
				windowInteractionOccurred = true;
			}
		}
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
	Win32_UpdateEngineInputTimeInfo(&Platform->enginePreviousInput, &Platform->engineInput, windowInteractionOccurred);
	Win32_PassDebugLinesToEngineInput(&Platform->engineInput);
	Win32_PassCompletedTasksToEngineInput(&Platform->engineInput);
	#if DEVELOPER_BUILD
	Platform->engineInput.numAudioFrameDrops = Platform->numAudioFrameDrops;
	#endif
	Win32_ClearEngineOutput(&Platform->engineOutput);
	
	{
		static u64 simpleTime = 0;
		Color_t clearColor = ColorLerp(MonokaiPurple, MonokaiOrange, OscillateBy(simpleTime, 0, 1, 5000));
		PigGfx_BeginRendering(true, clearColor, true, 1.0f, true, 0);
		//TODO: Add some test graphics logic here
		Win32_SwapBuffers();
		simpleTime += 16;
	}
	
	Win32_CheckForStableFramerate(Platform->engineInput.uncappedElapsedMs);
	
	DestroyStringFifo(&Platform->engineInput.platDebugLines);
	Win32_ProcessEngineOutput(&Platform->engineOutput);
	// Win32_OverlaysPostUpdateCleanup();
	
	#if DEBUG_BUILD
	if (Platform->audioWaitForFirstUpdateAfterReload)
	{
		Win32_LockMutex(&Platform->audioOutputMutex, MUTEX_LOCK_INFINITE);
		Platform->audioWaitForFirstUpdateAfterReload = false;
		Win32_UnlockMutex(&Platform->audioOutputMutex);
	}
	if (Platform->numQueuedTasks == 0)
	{
		if (Win32_CheckForEngineDllChange(&Platform->engine))
		{
			if (Win32_LockMutex(&Platform->audioOutputMutex, MUTEX_LOCK_INFINITE))
			{
				//TODO: Make sure all of our worker threads are empty/idle before we attempt a reload
				WriteLine_N("Engine DLL has changed. Attempting to reload....");
				Win32_ReloadEngineDll(Platform->engineDllPath, Platform->engineDllTempFormatStr, &Platform->engine);
				Platform->audioWaitForFirstUpdateAfterReload = true;
				Win32_UnlockMutex(&Platform->audioOutputMutex);
			}
		}
	}
	#endif
	
	InitPhase = Win32InitPhase_PostFirstUpdate;
	TempPopMark();
	u64 numMarks = GetNumMarks(TempArena);
	Assert(numMarks == 0);
}

