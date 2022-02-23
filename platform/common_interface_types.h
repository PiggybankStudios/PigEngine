/*
File:   common_interface_types.h
Author: Taylor Robbins
Date:   09\23\2021
Description:
	** These structures are used to facilitate communication between the platform layer and engine
*/

#ifndef _COMMON_INTERFACE_TYPES_H
#define _COMMON_INTERFACE_TYPES_H

struct StartupInfo_t
{
	PlatType_t type;
	Version_t version;
	GlfwVersion_t glfwVersion;
	
	ThreadId_t mainThreadId;
	
	MemArena_t* platTempArena;
	
	u64 defaultAudioDeviceIndex;
	VarArray_t audioDevices;
	
	PlatApiDebugOutput_f*         DebugOutput;
	PlatApiGetProgramArg_f*       GetProgramArg;
	PlatApiGetThisThreadId_f*     GetThisThreadId;
	PlatApiDoesFileExist_f*       DoesFileExist;
	PlatApiReadFileContents_f*    ReadFileContents;
	PlatApiFreeFileContents_f*    FreeFileContents;
	PlatApiGetMonitorVideoMode_f* GetMonitorVideoMode;
};

struct StartupOptions_t
{
	u64 mainMemoryRequest;
	u64 tempMemoryRequest;
	bool openDebugConsole;
	
	RenderApi_t renderApi;
	
	u64 numWindows;
	PlatWindowOptions_t* windowOptions;
	
	u64 numIconFiles;
	MyStr_t* iconFilePaths;
	
	Color_t loadingBackgroundColor;
	Color_t loadingBarColor;
	MyStr_t loadingImagePath;
	MyStr_t loadingBackPath;
	bool loadingBackTiling;
	r32 loadingBackScale;
	
	u64 audioDeviceIndex;
	PlatAudioFormat_t audioOutputFormat;
	
	u64 threadPoolSize;
};

struct PlatformInfo_t
{
	PlatType_t type;
	Version_t version;
	GlfwVersion_t glfwVersion;
	RenderApi_t renderApi;
	
	ThreadId_t mainThreadId;
	PlatAudioFormat_t audioFormat;
	PerfTime_t programStartPerfTime;
	PerfSectionBundle_t initPerfSectionBundle; //not filled until just before first update
	
	const PlatWindow_t* mainWindow;
	const LinkedList_t* windows;
	const PlatMonitorList_t* monitors;
};

struct PlatformApi_t
{
	PlatApiDebugOutput_f*            DebugOutput;
	PlatApiGetProgramArg_f*          GetProgramArg;
	PlatApiCreateMutex_f*            CreateMutex;
	PlatApiDestroyMutex_f*           DestroyMutex;
	PlatApiLockMutex_f*              LockMutex;
	PlatApiUnlockMutex_f*            UnlockMutex;
	PlatApiCreateSemaphore_f*        CreateSemaphore;
	PlatApiDestroySemaphore_f*       DestroySemaphore;
	PlatApiWaitOnSemaphore_f*        WaitOnSemaphore;
	PlatApiTriggerSemaphore_f*       TriggerSemaphore;
	PlatApiCreateInterlockedInt_f*   CreateInterlockedInt;
	PlatApiInterlockedExchange_f*    InterlockedExchange;
	PlatApiDestroyInterlockedInt_f*  DestroyInterlockedInt;
	PlatApiGetThisThreadId_f*        GetThisThreadId;
	PlatApiSleepForMs_f*             SleepForMs;
	PlatApiAllocateMemory_f*         AllocateMemory;
	PlatApiFreeMemory_f*             FreeMemory;
	PlatApiChangeWindowTarget_f*     ChangeWindowTarget;
	PlatApiSwapBuffers_f*            SwapBuffers;
	PlatApiGetFullPath_f*            GetFullPath;
	PlatApiDoesFileExist_f*          DoesFileExist;
	PlatApiReadFileContents_f*       ReadFileContents;
	PlatApiFreeFileContents_f*       FreeFileContents;
	PlatApiTryParseImageFile_f*      TryParseImageFile;
	PlatApiSaveImageDataToFile_f*    SaveImageDataToFile;
	PlatApiFreeImageData_f*          FreeImageData;
	PlatApiShowFile_f*               ShowFile;
	PlatApiShowSourceFile_f*         ShowSourceFile;
	PlatApiWriteEntireFile_f*        WriteEntireFile;
	PlatApiIsFileWatched_f*          IsFileWatched;
	PlatApiWatchFile_f*              WatchFile;
	PlatApiUnwatchFile_f*            UnwatchFile;
	PlatApiGetLoadProcAddressFunc_f* GetLoadProcAddressFunc;
	PlatApiRenderLoadingScreen_f*    RenderLoadingScreen;
	PlatApiFreeFontData_f*           FreeFontData;
	PlatApiReadPlatformFont_f*       ReadPlatformFont;
	PlatApiBakeFont_f*               BakeFont;
	PlatApiDebugReadout_f*           DebugReadout;
	PlatApiCopyTextToClipboard_f*    CopyTextToClipboard;
	PlatApiPasteTextFromClipboard_f* PasteTextFromClipboard;
	PlatApiGetPerfTime_f*            GetPerfTime;
	PlatApiGetPerfTimeDiff_f*        GetPerfTimeDiff;
	PlatApiGetProgramTime_f*         GetProgramTime;
	PlatApiGetMonitorVideoMode_f*    GetMonitorVideoMode;
};

struct EngineMemory_t
{
	void* persistentDataPntr;
	u64  persistentDataSize;
	
	void* tempDataPntr;
	u64  tempDataSize;
};

struct AudioServiceInfo_t
{
	PlatThread_t* thread;
	PlatAudioFormat_t format;
	u64 audioFrameIndex;
	
	u64 numFramesNeeded;
	u64 bufferSize;
	u8* bufferPntr;
	u64 numFramesFilled;
	bool fillWithSilence; //can be set to true by the engine
};

#define MAX_NUM_CONTROLLERS  GLFW_JOYSTICK_LAST //this is 16 controllers right now

struct EngineInput_t
{
	VarArray_t inputEvents; //InputEvent_t 
	
	bool actualElapsedMsIgnored;
	r64 programTimeF; //ms (since start of program)
	u64 programTime; //ms (since start of program)
	r64 elapsedMs; //ms
	r64 uncappedElapsedMs; //ms
	r64 avgElapsedMs; //ms
	r64 framerate; //frames/sec
	r64 avgFramerate; //frames/sec
	r64 timeDelta; //stays near 1.0f if framerate is close to targetFramerate
	r64 lastUpdateElapsedMs; //ms
	r64 timeSpentWaitingLastFrame; //ms
	
	RealTime_t localTime;
	RealTime_t unixTime;
	MyStr_t localTimezoneName;
	i64 localTimezoneOffset; //in seconds
	bool localTimezoneDoesDst;
	
	PlatBtnState_t keyStates[Key_NumKeys];
	PlatBtnState_t mouseBtnStates[MouseBtn_NumBtns];
	PlatControllerState_t controllerStates[MAX_NUM_CONTROLLERS];
	
	bool scrollChangedX;
	bool scrollChangedY;
	v2 scrollValue;
	v2 scrollDelta;
	
	//Debug
	#if DEBUG_BUILD
	u64 numAudioFrameDrops;
	#endif
	
	StringFifo_t platDebugLines;
};

struct EngineOutput_t
{
	PlatCursor_t cursorType;
};

#define PIG_GET_VERSION_DEF(functionName) Version_t functionName()
typedef PIG_GET_VERSION_DEF(PigGetVersion_f);

#define PIG_GET_STARTUP_OPTIONS_DEF(functionName) void functionName(const StartupInfo_t* info, StartupOptions_t* optionsOut)
typedef PIG_GET_STARTUP_OPTIONS_DEF(PigGetStartupOptions_f);

#define PIG_INITIALIZE_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory)
typedef PIG_INITIALIZE_DEF(PigInitialize_f);

#define PIG_UPDATE_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, EngineInput_t* input, EngineOutput_t* output)
typedef PIG_UPDATE_DEF(PigUpdate_f);

#define PIG_AUDIO_SERVICE_DEF(functionName) void functionName(AudioServiceInfo_t* audioInfo)
typedef PIG_AUDIO_SERVICE_DEF(PigAudioService_f);

#define PIG_SHOULD_WINDOW_CLOSE_DEF(functionName) bool functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, const PlatWindow_t* window)
typedef PIG_SHOULD_WINDOW_CLOSE_DEF(PigShouldWindowClose_f);

#define PIG_CLOSING_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory)
typedef PIG_CLOSING_DEF(PigClosing_f);

#define PIG_PRE_RELOAD_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t newVersion)
typedef PIG_PRE_RELOAD_DEF(PigPreReload_f);

#define PIG_POST_RELOAD_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t oldVersion)
typedef PIG_POST_RELOAD_DEF(PigPostReload_f);

#define PIG_PERFORM_TASK_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, PlatTask_t* task)
typedef PIG_PERFORM_TASK_DEF(PigPerformTask_f);

#endif //  _COMMON_INTERFACE_TYPES_H
