/*
File:   win32_core.cpp
Author: Taylor Robbins
Date:   09\23\2021
Description: 
	** This file holds a bunch of core functionality for the windows platform layer
*/

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

MyStr_t Win32_GetExecutablePath(MemArena_t* memArena)
{
	NotNull(memArena);
	AssertSingleThreaded(); //TODO: Make this thread safe?
	
	// TempPushMark(); //TODO: Move this back to the temporary memory arena
	u32 tempBufferSize = 1024;
	char* tempBuffer = AllocArray(&Platform->mainHeap, char, tempBufferSize);
	NotNull(tempBuffer);
	DWORD pathLength = GetModuleFileNameA(0, tempBuffer, tempBufferSize);
	FreeMem(&Platform->mainHeap, tempBuffer);
	// TempPopMark();
	if (pathLength <= 0) { return MyStr_Empty; }
	if (pathLength >= tempBufferSize-1)
	{
		PrintLine_W("Our executable path is %u characters or more. Please put the game in a shorter directory", tempBufferSize-1);
		return MyStr_Empty;
	}
	
	char* resultBuffer = AllocArray(memArena, char, pathLength+1);
	NotNull(resultBuffer);
	DWORD resultLength = GetModuleFileNameA(0, resultBuffer, pathLength+1);
	Assert(resultLength == pathLength);
	resultBuffer[resultLength] = '\0';
	
	return NewStr(resultLength, resultBuffer);
}
MyStr_t Win32_GetWorkingDirectory(MemArena_t* memArena)
{
	NotNull(memArena);
	
	DWORD pathLength = GetCurrentDirectory(0, nullptr);
	if (pathLength <= 0) { return MyStr_Empty; }
	
	char* resultBuffer = AllocArray(memArena, char, pathLength+2);
	NotNull(resultBuffer);
	DWORD resultLength = GetCurrentDirectoryA(pathLength+1, resultBuffer);
	Assert(resultLength <= pathLength); //TODO: Should this just be == ?
	if (resultBuffer[resultLength] != '\\' && resultBuffer[resultLength] != '/') { resultBuffer[resultLength] = '\\'; resultLength++; }
	resultBuffer[resultLength] = '\0';
	return NewStr(resultLength, resultBuffer);
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
	if (DEBUG_BUILD) { Win32_OpenConsoleWindow(); }
	
	printf("[Win32 Platform Core is Initializing...]\n");
	
	InitMemArena_StdHeap(&Platform->stdHeap);
	
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
	Win32_CreateMutex(&Platform->threadSafeHeapMutex);
	
	#if PROCMON_SUPPORTED
	InitMemArena_PagedHeapFuncs(&Platform->procmonHeap, Megabytes(1), ProcmonAllocate, ProcmonFree);
	#endif
}

// +--------------------------------------------------------------+
// |                   Secondary Init Functions                   |
// +--------------------------------------------------------------+
void Win32_FindExePathAndWorkingDir()
{
	NotNull(Platform);
	AssertSingleThreaded();
	TempPushMark();
	MyStr_t tempExePath = Win32_GetExecutablePath(GetTempArena());
	if (IsEmptyStr(tempExePath))
	{
		Win32_InitError("Failed to get executable directory. This could be due to the exe being in a location with too long of a path.");
	}
	Platform->exeDirectory = MyStr_Empty;
	Platform->exeFileName = MyStr_Empty;
	SplitFilePath(tempExePath, &Platform->exeDirectory, &Platform->exeFileName);
	Platform->exeDirectory = AllocString(&Platform->mainHeap, &Platform->exeDirectory);
	Platform->exeFileName = AllocString(&Platform->mainHeap, &Platform->exeFileName);
	StrReplaceInPlace(Platform->exeDirectory, "\\", "/");
	StrReplaceInPlace(Platform->exeFileName, "\\", "/");
	PrintLine_I("Our exe is in folder \"%s\" named \"%s\"", Platform->exeDirectory.pntr, Platform->exeFileName.pntr);
	TempPopMark();
	
	Platform->workingDirectory = Win32_GetWorkingDirectory(&Platform->mainHeap);
	if (IsEmptyStr(Platform->workingDirectory))
	{
		Win32_InitError("Failed to get working directory");
	}
	StrReplaceInPlace(Platform->workingDirectory, "\\", "/");
	PrintLine_I("Our working directory is \"%s\"", Platform->workingDirectory.pntr);
	
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
	//TODO: Does this need to change based off which render API we are using?
	Platform->winGraphicsContext = wglGetCurrentContext();
	if (Platform->winGraphicsContext == NULL)
	{
		Win32_InitError("Failed to get the Windows graphics context created by GLFW");
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
