/*
File:   win32_main.cpp
Author: Taylor Robbins
Date:   09\14\2021
Description: 
	** Holds the main entry point for the Win32 Platform Layer executable
*/

#include "build_config.h"

#define WIN32_OPEN_CONSOLE_WINDOW_AT_START false //TODO: Find a better home for this?

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
#include "win32/win32_box2d.h"

#include <float.h>
// #include "gylib/gy_test_floatscan.h"

// +--------------------------------------------------------------+
// |                       Platform Globals                       |
// +--------------------------------------------------------------+
Win32InitPhase_t InitPhase = Win32InitPhase_Entry;
Win32PlatformState_t* Platform = nullptr;
ThreadId_t MainThreadId = 0;

void Win32_DoMainLoopIteration(bool pollEvents); //pre-declared so win32_glfw.cpp can use it

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
// |                         Box2D Source                         |
// +--------------------------------------------------------------+
#if BOX2D_SUPPORTED
#include "b2_files.cpp"
#endif

// +--------------------------------------------------------------+
// |                         GLAD Source                          |
// +--------------------------------------------------------------+
#if OPENGL_SUPPORTED
#include "glad/glad.c"
#endif

// +--------------------------------------------------------------+
// |                    Platform Source Files                     |
// +--------------------------------------------------------------+
#include "gylib/gy_temp_memory.cpp"
#include "gylib/gy_scratch_arenas.cpp"

#include "win32/win32_func_defs.h"
#include "win32/win32_debug.cpp"
#include "win32/win32_program_args.cpp"
#include "win32/win32_performance.cpp"
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
#include "win32/win32_procmon.cpp"
#include "win32/win32_render_basic.cpp"
#include "win32/win32_overlays.cpp"
#include "win32/win32_loading.cpp"
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
	PerfTime_t initStartTime = GetPerfTime();
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
	// |       LoadingEngineDll       |
	// +==============================+
	PerfSection("LoadingEngineDll");
	InitPhase = Win32InitPhase_AudioInitialized;
	
	Win32_DllLoadingInit();
	if (!Win32_LoadEngineDll(Platform->engineDllPath, Platform->engineDllTempPath, &Platform->engine))
	{
		Win32_InitError("Failed to load the engine DLL. Make sure that the DLL exists next to the executable and that it's the correct version. This could also be caused by missing a glew32.dll");
	}
	PrintLine_I("Loaded engine DLL v%u.%02u(%03u)", Platform->engine.version.major, Platform->engine.version.minor, Platform->engine.version.build);
	
	// +==============================+
	// |       InitializingGlfw       |
	// +==============================+
	PerfSection("InitializingGlfw");
	InitPhase = Win32InitPhase_EngineDllLoaded;
	
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
	
	WriteLine_N("Calling Pig_GetStartupOptions...");
	Platform->engine.GetStartupOptions(&Platform->startupInfo, &Platform->startupOptions);
	WriteLine_N("Pig_GetStartupOptions Complete!");
	
	AssertMsg(Platform->startupOptions.numWindows > 0, "The engine DLL requested 0 windows. Is the engine DLL corrupt?");
	NotNullMsg(Platform->startupOptions.windowOptions, "The engine DLL did not fill out the windowsOptions array. Is the engine DLL corrupt?");
	AssertMsg(Platform->startupOptions.render.api != RenderApi_None, "The engine DLL did not choose a valid Render API to use. Is the engine DLL corrupt?");
	NotEmptyStr(&Platform->startupOptions.loadingImagePath);
	AssertMsg(Platform->startupOptions.audioDeviceIndex < Platform->startupInfo.audioDevices.length, "Engine chose an invalid audio device index!");
	AssertMsg(Platform->startupOptions.threadPoolSize <= PLAT_MAX_NUM_THREADS, "Engine wanted too many threads in it's thread pool!");
	AssertIfMsg(Platform->startupOptions.threadPoolTempArenasSize > 0, Platform->startupOptions.threadPoolTempArenasNumMarks > 0, "TempArenas for thread pools cannot have 0 marks");
	
	// +===============================+
	// | Initializing Sockets and HTTP |
	// +===============================+
	PerfSection("InitializingSocketsAndHttp");
	InitPhase = Win32InitPhase_StartupOptionsObtained;
	
	#if SOCKETS_SUPPORTED
	if (!InitializeSockets())
	{
		Win32_InitError("Failed to initialize sockets!");
	}
	#endif
	
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
	
	#if PROCMON_SUPPORTED
	// +==============================+
	// |     ProcmonDriverLoading     |
	// +==============================+
	PerfSection("ProcmonDriverLoading");
	InitPhase = Win32InitPhase_ThreadPoolsCreated;
	
	Win32_ProcmonInit(Platform->startupOptions.threadPoolTempArenasSize, Platform->startupOptions.threadPoolTempArenasNumMarks);
	#endif
	
	// +==============================+
	// |        WindowOpening         |
	// +==============================+
	PerfSection("WindowOpening");
	#if PROCMON_SUPPORTED
	InitPhase = Win32InitPhase_ProcmonDriverLoaded;
	#else
	InitPhase = Win32InitPhase_ThreadPoolsCreated;
	#endif
	
	if (Platform->startupOptions.openDebugConsole)
	{
		Win32_OpenConsoleWindow();
		WriteLine_I("Opened console window that was requested by engine DLL");
	}
	
	//TODO: Add support for other APIs and then add else ifs here
	if (Platform->startupOptions.render.api == RenderApi_OpenGL && OPENGL_SUPPORTED)
	{
		Platform->renderApi = RenderApi_OpenGL;
	}
	else
	{
		PrintLine_E("The Win32 platform layer does not support %s as a render API yet", GetRenderApiStr(Platform->startupOptions.render.api));
		Win32_InitError("Unsupported win32 render API chosen by the engine");
	}
	
	for (u64 wIndex = 0; wIndex < Platform->startupOptions.numWindows; wIndex++)
	{
		NotEmptyStr(&Platform->startupOptions.windowOptions[wIndex].create.windowTitle);
		PlatWindow_t* newWindow = Win32_GlfwCreateWindow(&Platform->startupOptions.render, &Platform->startupOptions.windowOptions[wIndex].create);
		if (newWindow == nullptr)
		{
			Win32_InitError("Failed to create application window through GLFW. This is usually caused by your graphics drivers not supporting the minimum version of OpenGL that we require.");
		}
		glfwMakeContextCurrent(newWindow->handle);
		Win32_ApplyWindowOptions(newWindow, &Platform->startupOptions.windowOptions[wIndex]);
		Win32_LoadWindowIcon(newWindow, Platform->startupOptions.numIconFiles, Platform->startupOptions.iconFilePaths);
		if (wIndex == 0)
		{
			Platform->mainWindow = newWindow;
			newWindow->activeInput.isFocused = true;
			Win32_GladInit();
			
			if (Platform->startupOptions.render.api == RenderApi_OpenGL)
			{
				PrintLine_I("Successfully created %s v%d.%d graphics context!", GetRenderApiStr(Platform->startupOptions.render.api), GLVersion.major, GLVersion.minor);
			}
		}
	}
	NotNull(Platform->mainWindow);
	glfwMakeContextCurrent(Platform->mainWindow->handle);
	
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
	
	Win32_LoadBasicResources(&Platform->startupOptions);
	Win32_InitBasicRendering();
	Win32_InitOverlays();
	Platform->loadingPercent = 0.0f;
	Win32_RenderLoadingScreen(0.0f);
	
	// +==============================+
	// |     SteamInitialization      |
	// +==============================+
	PerfSection("SteamInitialization");
	InitPhase = Win32InitPhase_ResourcesLoaded;
	
	Win32_SteamInit();
	
	Platform->loadingPercent = 0.5f;
	// Win32_RenderLoadingScreen(0.0f);
	
	// +==================================+
	// | FileWatchingAndInterfaceFilling  |
	// +==================================+
	PerfSection("FileWatchingAndInterfaceFilling");
	InitPhase = Win32InitPhase_SteamInitialized;
	
	//TODO: Initialize DirectSound or other audio library
	//TODO: Initialize WinHTTP
	Win32_InitProcessManagement();
	Win32_InitFileWatching();
	#if DEBUG_BUILD
	Win32_WatchEngineDll(Platform->engineDllPath, Platform->engineDllTempPath, &Platform->engine);
	#endif
	
	Win32_FillPlatformInfo(&Platform->info, initStartTime);
	Win32_FillPlatformApi(&Platform->api);
	Win32_FillEngineMemory(&Platform->engineMemory, Platform->startupOptions.mainMemoryRequest, Platform->startupOptions.tempMemoryRequest);
	Win32_FillEngineOutput(&Platform->engineOutput);
	
	u64 initProgramTime = Win32_GetProgramTime(nullptr, true);
	u64 initUnixTimestamp = Win32_GetCurrentTimestamp(false);
	u64 initLocalTimestamp = Win32_GetCurrentTimestamp(true);
	
	Platform->loadingPercent = 1.0f;
	Win32_RenderLoadingScreen(0.0f);
	
	// +==============================+
	// |     EngineInitialization     |
	// +==============================+
	PerfSection("EngineInitialization");
	InitPhase = Win32InitPhase_EngineInitialization;
	
	PerfTime_t initEndTime = GetPerfTime();
	PrintLine_N("Calling Pig_Initialize... (Win32 platform took %.1lfms to init)", GetPerfTimeDiff(&initStartTime, &initEndTime));
	Platform->callingEngineInitialize = true;
	Platform->engine.Initialize(&Platform->info, &Platform->api, &Platform->engineMemory, initProgramTime, initUnixTimestamp, initLocalTimestamp);
	Platform->callingEngineInitialize = false;
	if (Platform->mainWindow == nullptr || Platform->mainWindow->closed || glfwWindowShouldClose(Platform->mainWindow->handle))
	{
		WriteLine_E("Exited during initialization");
		Win32_GlfwCleanup();
		return 0;
	}
	WriteLine_N("Pig_Initialize Complete");
	Platform->firstUpdateStartTime = GetPerfTime();
	
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
	
	#if PROCMON_SUPPORTED
	Win32_ProcmonShutdown();
	#endif
	
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
	#if STEAM_BUILD
	Win32_UpdateSteamStuff();
	#endif
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
	#if STEAM_BUILD
	Win32_UpdateEngineInputSteamInfo(&Platform->enginePreviousInput, &Platform->engineInput);
	#endif
	Win32_PassDebugLinesToEngineInput(&Platform->engineInput);
	Win32_PassCompletedTasksToEngineInput(&Platform->engineInput);
	#if DEVELOPER_BUILD
	Platform->engineInput.numAudioFrameDrops = Platform->numAudioFrameDrops;
	#endif
	Win32_ClearEngineOutput(&Platform->engineOutput);
	
	if (InitPhase < Win32InitPhase_PostFirstUpdate) { PrintLine_N("Calling first Pig_Update..."); }
	Platform->engine.Update(&Platform->info, &Platform->api, &Platform->engineMemory, &Platform->engineInput, &Platform->engineOutput);
	if (InitPhase < Win32InitPhase_PostFirstUpdate)
	{
		PerfTime_t firstUpdateEndTime = GetPerfTime();
		PrintLine_N("First Pig_Update Complete! (Took %.1lfms)", GetPerfTimeDiff(&Platform->firstUpdateStartTime, &firstUpdateEndTime));
	}
	Win32_CheckForStableFramerate(Platform->engineInput.uncappedElapsedMs);
	
	DestroyStringFifo(&Platform->engineInput.platDebugLines);
	Win32_ProcessEngineOutput(&Platform->engineOutput);
	Win32_OverlaysPostUpdateCleanup();
	
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
