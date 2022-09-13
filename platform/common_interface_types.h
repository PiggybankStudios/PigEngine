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
	
	PlatApiShowMessageBox_f*      ShowMessageBox;
	PlatApiHandleAssertion_f*     HandleAssertion;
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
	u64 threadPoolTempArenasSize;
	u64 threadPoolTempArenasNumMarks;
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
	PlatApiShowMessageBox_f*         ShowMessageBox;
	PlatApiHandleAssertion_f*        HandleAssertion;
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
	PlatApiGetThreadContext_f*       GetThreadContext;
	PlatApiSleepForMs_f*             SleepForMs;
	PlatApiQueueTask_f*              QueueTask;
	PlatApiAllocateMemory_f*         AllocateMemory;
	PlatApiReallocMemory_f*          ReallocMemory;
	PlatApiFreeMemory_f*             FreeMemory;
	PlatApiChangeWindowTarget_f*     ChangeWindowTarget;
	PlatApiSwapBuffers_f*            SwapBuffers;
	PlatApiGetFullPath_f*            GetFullPath;
	PlatApiDoesFileExist_f*          DoesFileExist;
	PlatApiReadFileContents_f*       ReadFileContents;
	PlatApiFreeFileContents_f*       FreeFileContents;
	PlatApiWriteEntireFile_f*        WriteEntireFile;
	PlatApiOpenFile_f*               OpenFile;
	PlatApiWriteToFile_f*            WriteToFile;
	PlatApiMoveFileCursor_f*         MoveFileCursor;
	PlatApiSeekToOffsetInFile_f*     SeekToOffsetInFile;
	PlatApiReadFromFile_f*           ReadFromFile;
	PlatApiCloseFile_f*              CloseFile;
	PlatApiTryParseImageFile_f*      TryParseImageFile;
	PlatApiSaveImageDataToFile_f*    SaveImageDataToFile;
	PlatApiFreeImageData_f*          FreeImageData;
	PlatApiShowFile_f*               ShowFile;
	PlatApiShowSourceFile_f*         ShowSourceFile;
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

enum InputEventType_t
{
	InputEventType_None = 0,
	InputEventType_Character,
	InputEventType_Key,
	InputEventType_ControllerBtn,
	InputEventType_MouseBtn,
	InputEventType_MouseHover,
	InputEventType_WindowFocus,
	InputEventType_FileDropped,
	InputEventType_TaskCompleted,
	InputEventType_NumTypes,
};
const char* GetInputEventTypeStr(InputEventType_t eventType)
{
	switch (eventType)
	{
		case InputEventType_None:          return "None";
		case InputEventType_Character:     return "Character";
		case InputEventType_Key:           return "Key";
		case InputEventType_ControllerBtn: return "ControllerBtn";
		case InputEventType_MouseBtn:      return "MouseBtn";
		case InputEventType_MouseHover:    return "MouseHover";
		case InputEventType_WindowFocus:   return "WindowFocus";
		case InputEventType_FileDropped:   return "FileDropped";
		case InputEventType_TaskCompleted: return "TaskCompleted";
		default: return "Unknown";
	}
}

struct InputEvent_t
{
	u64 id;
	u64 index;
	InputEventType_t type;
	bool handled; //can be set by the engine
	
	//common data
	const struct PlatWindow_t* window; //can be nullptr
	bool hadFocus;
	bool mouseInsideWindow;
	v2 mousePos;
	ModifierKey_t modifiers;
	i64 pairedEventIndex;
	
	union
	{
		struct
		{
			u32 codepoint;
		} character;
		struct
		{
			Key_t key;
			bool pressed;
			bool released;
			bool repeated;
		} key;
		struct
		{
			ControllerBtn_t btn;
			bool pressed;
			bool released;
			bool repeated;
		} controllerBtn;
		struct
		{
			MouseBtn_t btn;
			bool pressed;
			bool released;
			bool repeated;
		} mouseBtn;
		struct
		{
			bool focused;
			const struct PlatWindow_t* oldWindow; //can be nullptr
			const struct PlatWindow_t* newWindow; //can be nullptr
		} windowFocus;
		struct
		{
			bool entered;
			const struct PlatWindow_t* oldWindow; //can be nullptr
			const struct PlatWindow_t* newWindow; //can be nullptr
		} mouseHover;
		struct
		{
			MyStr_t filePath;
		} droppedFile;
		struct
		{
			PlatTask_t task;
		} taskCompleted;
	};
};

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
	#if DEVELOPER_BUILD
	u64 numAudioFrameDrops;
	#endif
	
	StringFifo_t platDebugLines;
	
	#if PROCMON_SUPPORTED
	u64 nextProcmonEventId;
	StrHashDict_t processEntries; //ProcmonEntry_t
	StrHashDict_t touchedFiles; //ProcmonFile_t
	#endif
};

struct EngineOutput_t
{
	bool exit;
	PlatCursor_t cursorType;
	PlatMouseMode_t mouseMode;
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

#define PIG_POST_RELOAD_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t oldVersion, u64 programTime)
typedef PIG_POST_RELOAD_DEF(PigPostReload_f);

#define PIG_PERFORM_TASK_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, PlatThreadPoolThread_t* thread, PlatTask_t* task)
typedef PIG_PERFORM_TASK_DEF(PigPerformTask_f);

#endif //  _COMMON_INTERFACE_TYPES_H
