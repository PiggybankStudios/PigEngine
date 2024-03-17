/*
File:   win32_core.cpp
Author: Taylor Robbins
Date:   09\23\2021
Description: 
	** This file holds a bunch of core functionality for the windows platform layer
*/

#define WIN32_FILETIME_SEC_OFFSET           11644473600ULL //11,644,473,600 seconds between Jan 1st 1601 and Jan 1st 1970

// #define MAIN_SCRATCH_ARENA_PAGE_SIZE       Kilobytes(512)
#define MAIN_SCRATCH_ARENA_MAX_SIZE        Gigabytes(1)
#define MAIN_SCRATCH_ARENA_MAX_NUM_MARKS   256

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
const char* Win32_GetErrorCodeStr(DWORD windowsErrorCode, bool printUnknownValue = false)
{
	switch (windowsErrorCode)
	{
		case ERROR_FILE_NOT_FOUND:    return "ERROR_FILE_NOT_FOUND";    //2
		case ERROR_FILE_EXISTS:       return "ERROR_FILE_EXISTS";       //80
		case ERROR_ALREADY_EXISTS:    return "ERROR_ALREADY_EXISTS";    //183
		case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION"; //?
		case ERROR_PIPE_BUSY:         return "ERROR_PIPE_BUSY";         //?
		case ERROR_ACCESS_DENIED:     return "ERROR_ACCESS_DENIED";     //?
		case ERROR_DIRECTORY:         return "ERROR_DIRECTORY";         //267
		default: return (printUnknownValue ? TempPrint("(0x%08X)", windowsErrorCode) : "UNKNOWN");
	}
}

// +==============================+
// |     Win32_ShowMessageBox     |
// +==============================+
// void ShowMessageBox(const char* title, const char* message)
PLAT_API_SHOW_MESSAGE_BOX_DEFINITION(Win32_ShowMessageBox)
{
	NotNull_(title);
	NotNull_(message);
	int messageBoxResult = MessageBoxA(
		NULL,    // hWnd,
		message, // lpText,
		title,   // lpCaption,
		MB_OK    // uType
	);
	UNUSED(messageBoxResult);
}

void Win32_FatalError(const char* errorMessage, const char* messageBoxTitle)
{
	Win32_ShowMessageBox(messageBoxTitle, errorMessage);
	exit(1); //TODO: Turn this into a win32 specific call
}
void Win32_InitError(const char* errorMessage) //pre-declared in win32_func_defs.cpp
{
	Win32_FatalError(errorMessage, "Initialization Error Encountered!");
}

#if PROCMON_SUPPORTED
//TODO: We should probably think a little harder about where we get the procmon memory from. A VirtualAlloc call might be better?
void* ProcmonAllocate(u64 numBytes)
{
	return malloc(numBytes);
}
void ProcmonFree(void* memPntr)
{
	return free(memPntr);
}
#endif

void* Win32_StdAllocate(u64 numBytes)
{
	return malloc(numBytes);
}
void Win32_StdFree(void* alloc)
{
	return free(alloc);
}

// +--------------------------------------------------------------+
// |                          Functions                           |
// +--------------------------------------------------------------+
void Win32_OpenConsoleWindow()
{
	NotNull(Platform);
	AssertSingleThreaded();
	if (!Platform->consoleWindowIsOpen)
	{
		BOOL allocResult = AllocConsole();
		if (allocResult == 0)
		{
			DWORD errorCode = GetLastError(); //TODO: Can we dump this error code somewhere?
			const char* errorCodeName = Win32_GetErrorCodeStr(errorCode);
			UNUSED(errorCode);
			UNUSED(errorCodeName);
			AssertMsg(false, "Error opening console window");
		}
		FILE* stdinResult  = freopen("CONIN$",  "r", stdin);
		FILE* stdoutResult = freopen("CONOUT$", "w", stdout);
		FILE* stderrResult = freopen("CONOUT$", "w", stderr);
		if (stdinResult == nullptr || stdoutResult == nullptr || stderrResult == nullptr)
		{
			AssertMsg(false, "Error rerouting std file handles after opening console window");
		}
		Platform->consoleWindowIsOpen = true;
	}
}

u64 Win32_GetCurrentTimestamp(bool local, i64* timezoneOffsetOut = nullptr, bool* timezoneDoesDstOut = nullptr, MyStr_t* timezoneNameOut = nullptr)
{
	u64 result = 0;
	if (local)
	{
		u64 unixTimestamp = Win32_GetCurrentTimestamp(false);
		TIME_ZONE_INFORMATION timezoneInfo = {};
		DWORD timezoneResult = GetTimeZoneInformation(&timezoneInfo);
		DebugAssertAndUnusedMsg(timezoneResult != TIME_ZONE_ID_INVALID, timezoneResult, "GetTimeZoneInformation failed and gave TIME_ZONE_ID_INVALID");
		i64 localTimezoneOffset = -((i64)timezoneInfo.Bias * NUM_SEC_PER_MINUTE);
		if (timezoneOffsetOut != nullptr) { *timezoneOffsetOut = localTimezoneOffset; }
		result = unixTimestamp + localTimezoneOffset;
		bool localTimezoneDoesDst = (timezoneInfo.DaylightBias != 0); //TODO: It's possible that DaylightBias isn't -60 minutes. Should we handle that?
		if (timezoneDoesDstOut != nullptr) { *timezoneDoesDstOut = localTimezoneDoesDst; }
		if (timezoneNameOut != nullptr)
		{
			*timezoneNameOut = ConvertUcs2StrToUtf8Nt(TempArena, &timezoneInfo.StandardName[0]);
		}
	}
	else
	{
		//TODO: We could use the higher accuracy from this file time to determine sub-second timestamp values
		FILETIME systemFileTime = {};
		GetSystemTimeAsFileTime(&systemFileTime);
		ULARGE_INTEGER systemLargeIntegerTime = {};
		systemLargeIntegerTime.HighPart = systemFileTime.dwHighDateTime;
		systemLargeIntegerTime.LowPart = systemFileTime.dwLowDateTime;
		//NOTE: FILETIME value is number of 100-nanosecond intervals since Jan 1st 1601 UTC
		//      We want number of seconds since Jan 1st 1970 UTC so divide by 10,000,000 and subtract off 369 years
		result = (u64)(systemLargeIntegerTime.QuadPart/10000000ULL);
		if (result >= WIN32_FILETIME_SEC_OFFSET) { result -= WIN32_FILETIME_SEC_OFFSET; }
	}
	return result;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_CoreInit(bool usedWinMainEntryPoint)
{
	Platform = (Win32PlatformState_t*)malloc(sizeof(Win32PlatformState_t)); //TODO: Should we allocate this in another way?
	if (Platform == nullptr)
	{
		Win32_InitError("Failed to allocate data for the Win32PlatformState_t");
	}
	ClearPointer(Platform);
	Platform->initialized = true;
	Platform->version.major = WIN32_VERSION_MAJOR;
	Platform->version.minor = WIN32_VERSION_MINOR;
	Platform->version.build = WIN32_VERSION_BUILD;
	Platform->consoleWindowIsOpen = !usedWinMainEntryPoint;
	
	printf("[Win32 Platform Core is Initializing...]\n");
	
	InitMemArena_StdHeap(&Platform->stdHeap);
	InitMemArena_Redirect(&Platform->stdHeapRedirect, Win32_StdAllocate, Win32_StdFree);
	// InitThreadLocalScratchArenasPaged(&Platform->stdHeapRedirect, MAIN_SCRATCH_ARENA_PAGE_SIZE, MAIN_SCRATCH_ARENA_MAX_NUM_MARKS);
	InitThreadLocalScratchArenasVirtual(MAIN_SCRATCH_ARENA_MAX_SIZE, MAIN_SCRATCH_ARENA_MAX_NUM_MARKS);
	
	void* mainHeapMem = malloc(PLAT_MAIN_HEAP_SIZE);
	if (mainHeapMem == nullptr)
	{
		Win32_InitError("Failed to allocate memory for Platform Main Heap");
	}
	InitMemArena_FixedHeap(&Platform->mainHeap, PLAT_MAIN_HEAP_SIZE, mainHeapMem); //TODO: Change this to a Paged Heap?
	
	u64 tempArenaSize = PLAT_TEMP_ARENA_SIZE;
	void* tempArenaMem = malloc(PLAT_TEMP_ARENA_SIZE); //TODO: Should we use VirtualAlloc?
	if (tempArenaMem == nullptr)
	{
		Win32_InitError("Failed to allocate memory for TempArena");
	}
	
	InitMemArena_MarkedStack(&Platform->tempArena, tempArenaSize, tempArenaMem, PLAT_TEMP_ARENA_MAX_MARKS);
	TempArena = &Platform->tempArena;
	
	void* threadSafeHeapMem = malloc(PLAT_THREAD_SAFE_HEAP_SIZE);
	if (threadSafeHeapMem == nullptr)
	{
		Win32_InitError("Failed to allocate memory for Platform Thread Safe Heap");
	}
	InitMemArena_FixedHeap(&Platform->threadSafeHeap, PLAT_THREAD_SAFE_HEAP_SIZE, threadSafeHeapMem);
	Win32_CreateMutex(&Platform->threadSafeHeap.mutex);
	
	#if DEBUG_BUILD
	Platform->mainHeap.debugName        = NewStringInArenaNt(&Platform->mainHeap, "win32_mainHeap").chars;
	Platform->stdHeap.debugName         = NewStringInArenaNt(&Platform->mainHeap, "win32_stdHeap").chars;
	Platform->stdHeapRedirect.debugName = NewStringInArenaNt(&Platform->mainHeap, "win32_stdHeapRedirect").chars;
	Platform->tempArena.debugName       = NewStringInArenaNt(&Platform->mainHeap, "win32_tempArena").chars;
	Platform->threadSafeHeap.debugName  = NewStringInArenaNt(&Platform->mainHeap, "win32_threadSafeHeap").chars;
	CompileAssert(NUM_SCRATCH_ARENAS_PER_THREAD == 3);
	ThreadLocalScratchArenas[0].debugName = NewStringInArenaNt(&Platform->mainHeap, "Scratch1").chars;
	ThreadLocalScratchArenas[1].debugName = NewStringInArenaNt(&Platform->mainHeap, "Scratch2").chars;
	ThreadLocalScratchArenas[2].debugName = NewStringInArenaNt(&Platform->mainHeap, "Scratch3").chars;
	#endif
	
	#if PROCMON_SUPPORTED
	InitMemArena_PagedHeapFuncs(&Platform->procmonHeap, Megabytes(1), ProcmonAllocate, ProcmonFree);
	#endif
	
	#if BOX2D_SUPPORTED
	InitMemArena_PagedHeapArena(&Platform->box2dHeap, Kilobytes(512), &Platform->stdHeap);
	#endif
}

// +--------------------------------------------------------------+
// |                   Secondary Init Functions                   |
// +--------------------------------------------------------------+
void Win32_FindExePathAndWorkingDir()
{
	NotNull(Platform);
	AssertSingleThreaded();
	OsError_t osError = OsError_None;
	MemArena_t* scratch = GetScratchArena();
	
	MyStr_t tempExePath = OsGetExecutablePath(scratch, &osError);
	if (osError != OsError_None || IsEmptyStr(tempExePath))
	{
		Win32_InitError("Failed to get executable directory. This could be due to the exe being in a location with too long of a path.");
	}
	Platform->exeDirectory = MyStr_Empty;
	Platform->exeFileName = MyStr_Empty;
	SplitFilePath(tempExePath, &Platform->exeDirectory, &Platform->exeFileName);
	Platform->exeDirectory = AllocString(&Platform->mainHeap, &Platform->exeDirectory);
	Platform->exeFileName = AllocString(&Platform->mainHeap, &Platform->exeFileName);
	PrintLine_I("Our exe is in folder \"%s\" named \"%s\"", Platform->exeDirectory.pntr, Platform->exeFileName.pntr);
	
	Platform->workingDirectory = OsGetWorkingDirectory(&Platform->mainHeap, &osError);
	if (osError != OsError_None || IsEmptyStr(Platform->workingDirectory))
	{
		Win32_InitError("Failed to get working directory");
	}
	PrintLine_I("Our working directory is \"%s\"", Platform->workingDirectory.pntr);
	
	FreeScratchArena(scratch);
}

void Win32_ChooseDefaultDirectory()
{
	bool loadFromWorkingDirectory = false;
	if (DEBUG_BUILD) { loadFromWorkingDirectory = true; }
	else if (Win32_GetProgramArg(nullptr, NewStr("w"), nullptr))
	{
		WriteLine_D("User passed \"-w\" command line argument");
		loadFromWorkingDirectory = true;
	}
	if (Win32_GetProgramArg(nullptr, NewStr("exe"), nullptr))
	{
		WriteLine_D("User passed \"-exe\" command line argument");
		loadFromWorkingDirectory = false;
	}
	if (loadFromWorkingDirectory)
	{
		Platform->defaultDirectory = AllocString(&Platform->mainHeap, &Platform->workingDirectory);
		PrintLine_D("Will load resources from working directory: \"%s\"", Platform->defaultDirectory.pntr);
	}
	else
	{
		Platform->defaultDirectory = AllocString(&Platform->mainHeap, &Platform->exeDirectory);
		PrintLine_D("Will load resources from executable directory: \"%s\"", Platform->defaultDirectory.pntr);
	}
	StrReplaceInPlace(Platform->defaultDirectory, "\\", "/");
}

void Win32_GetContextAndWindowHandles()
{
	if (Platform->renderApi == RenderApi_OpenGL)
	{
		Platform->winGraphicsContext = wglGetCurrentContext();
		if (Platform->winGraphicsContext == NULL)
		{
			Win32_InitError("Failed to get the Windows graphics context created by GLFW");
		}
	}
	
	//TODO: Should we do this for each window we create?
	Platform->windowHandle = GetForegroundWindow();
	if (Platform->windowHandle == NULL) { Platform->windowHandle = GetDesktopWindow(); }
	if (Platform->windowHandle == NULL)
	{
		Win32_InitError("Failed to get the Window Handle");
	}
	
	Platform->winDeviceContext = GetDC(Platform->windowHandle);
	if (Platform->winDeviceContext == NULL)
	{
		Win32_InitError("Failed to get the window's Handle to Device Context (HDC)");
	}
	
	Platform->processHandle = GetCurrentProcess();
}

void Win32_LoadCursors()
{
	NotNull(Platform);
	NotNull(Platform->mainWindow);
	Platform->glfwCursors[PlatCursor_Default]          = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	Platform->glfwCursors[PlatCursor_TextIBeam]        = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	Platform->glfwCursors[PlatCursor_Pointer]          = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	Platform->glfwCursors[PlatCursor_ResizeHorizontal] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	Platform->glfwCursors[PlatCursor_ResizeVertical]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	glfwSetCursor(Platform->mainWindow->handle, Platform->glfwCursors[PlatCursor_Default]);
	Platform->currentCursorType = PlatCursor_Default;
	
	//TODO: Should we find a proper home for this?
	Platform->currentMouseMode = PlatMouseMode_Default;
}

// +--------------------------------------------------------------+
// |                     Memory API Functions                     |
// +--------------------------------------------------------------+
// +==============================+
// |     Win32_AllocateMemory     |
// +==============================+
// void* AllocateMemory(u64 size, AllocAlignment_t alignOverride)
PLAT_API_ALLOCATE_MEMORY_DEF(Win32_AllocateMemory)
{
	//TODO: Should we track the programs allocations somehow?
	return AllocMem(&Platform->stdHeap, size, alignOverride);
}

// +==============================+
// |     Win32_ReallocMemory      |
// +==============================+
// void* ReallocMemory(void* allocPntr, u64 newSize, u64 oldSize, AllocAlignment_t alignOverride)
PLAT_API_REALLOC_MEMORY_DEF(Win32_ReallocMemory)
{
	//TODO: Should we track the programs allocations somehow?
	return ReallocMem(&Platform->stdHeap, allocPntr, newSize, oldSize, alignOverride);
}

// +==============================+
// |       Win32_FreeMemory       |
// +==============================+
// void FreeMemory(void* allocPntr, u64 oldSize, u64* oldSizeOut)
PLAT_API_FREE_MEMORY_DEF(Win32_FreeMemory)
{
	//TODO: Should we track the programs allocations somehow?
	FreeMem(&Platform->stdHeap, allocPntr, oldSize, false, oldSizeOut);
}
