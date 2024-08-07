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
	MemArena_t* stdHeap;
	
	u64 defaultAudioDeviceIndex;
	VarArray_t audioDevices;
	
	const PlatMonitorList_t* monitors;
	
	PlatApiGetScratchArena_f*      GetScratchArena;
	PlatApiFreeScratchArena_f*     FreeScratchArena;
	PlatApiShowMessageBox_f*       ShowMessageBox;
	PlatApiHandleAssertion_f*      HandleAssertion;
	PlatApiDebugOutput_f*          DebugOutput;
	PlatApiGetProgramArg_f*        GetProgramArg;
	PlatApiGetThisThreadId_f*      GetThisThreadId;
	PlatApiDoesFileExist_f*        DoesFileExist;
	PlatApiReadFileContents_f*     ReadFileContents;
	PlatApiFreeFileContents_f*     FreeFileContents;
	PlatApiGetSpecialFolderPath_f* GetSpecialFolderPath;
	PlatApiGetMonitorVideoMode_f*  GetMonitorVideoMode;
};

struct StartupRenderOptions_t
{
	RenderApi_t api;
	struct
	{
		int requestCoreProfile; //false = Compat profile
		int requestVersionMajor;
		int requestVersionMinor;
		int minVersionMajor;
		int minVersionMinor;
		bool forwardCompat;
		bool debugEnabled;
	} opengl;
};

struct StartupOptions_t
{
	u64 mainMemoryRequest;
	u64 tempMemoryRequest;
	u64 scratchArenaSizes;
	bool openDebugConsole;
	
	StartupRenderOptions_t render;
	
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
	u64 threadPoolScratchArenasMaxSize;
	u64 threadPoolScratchArenasNumMarks;
};

struct PlatformInfo_t
{
	PlatType_t type;
	Version_t version;
	GlfwVersion_t glfwVersion;
	MemArena_t* stdHeap;
	
	MyStr_t workingDirectory; //contains trailing / or is empty
	MyStr_t exeDirectory; //contains trailing / or is empty
	MyStr_t defaultDirectory; //contains trailing / or is empty
	MyStr_t exeFileName;
	
	RenderApi_t renderApi;
	#if OPENGL_SUPPORTED
	OpenGlInfo_t opengl;
	#endif
	
	PlatAudioFormat_t audioFormat;
	PerfTime_t programStartPerfTime;
	PerfSectionBundle_t initPerfSectionBundle; //not filled until just before first update
	
	PlatThreadIdPair_t mainThreadId;
	PlatThreadIdPair_t fileWatchingThreadId;
	PlatThreadIdPair_t audioThreadId;
	u64 numThreadPoolThreads;
	PlatThreadIdPair_t threadPoolIds[PLAT_MAX_THREADPOOL_SIZE];
	
	const PlatWindow_t* mainWindow;
	const LinkedList_t* windows; //PlatWindow_t
	const PlatMonitorList_t* monitors;
	
	#if STEAM_BUILD
	u32 steamAppId;
	CSteamID steamUserId;
	PlatSteamFriendsList_t* steamFriendsList;
	#endif
	
	bool wasRunInAdministratorMode;
};

struct PlatformApi_t
{
	PlatApiGetScratchArena_f*            GetScratchArena;
	PlatApiFreeScratchArena_f*           FreeScratchArena;
	PlatApiShowMessageBox_f*             ShowMessageBox;
	PlatApiHandleAssertion_f*            HandleAssertion;
	PlatApiDebugOutput_f*                DebugOutput;
	PlatApiGetProgramArg_f*              GetProgramArg;
	PlatApiCreateMutex_f*                CreateMutex;
	PlatApiDestroyMutex_f*               DestroyMutex;
	PlatApiLockMutex_f*                  LockMutex;
	PlatApiUnlockMutex_f*                UnlockMutex;
	PlatApiCreateSemaphore_f*            CreateSemaphore;
	PlatApiDestroySemaphore_f*           DestroySemaphore;
	PlatApiWaitOnSemaphore_f*            WaitOnSemaphore;
	PlatApiTriggerSemaphore_f*           TriggerSemaphore;
	PlatApiCreateInterlockedInt_f*       CreateInterlockedInt;
	PlatApiInterlockedExchange_f*        InterlockedExchange;
	PlatApiDestroyInterlockedInt_f*      DestroyInterlockedInt;
	PlatApiGetThisThreadId_f*            GetThisThreadId;
	PlatApiGetThreadContext_f*           GetThreadContext;
	PlatApiSleepForMs_f*                 SleepForMs;
	PlatApiQueueTask_f*                  QueueTask;
	PlatApiAllocateMemory_f*             AllocateMemory;
	PlatApiReallocMemory_f*              ReallocMemory;
	PlatApiFreeMemory_f*                 FreeMemory;
	PlatApiGetRapidClickMaxTime_f*       GetRapidClickMaxTime;
	PlatApiChangeWindowTarget_f*         ChangeWindowTarget;
	PlatApiSwapBuffers_f*                SwapBuffers;
	PlatApiGetNativeWindowPntr_f*        GetNativeWindowPntr;
	PlatApiGetFullPath_f*                GetFullPath;
	PlatApiDoesFileExist_f*              DoesFileExist;
	PlatApiStartEnumeratingFiles_f*      StartEnumeratingFiles;
	PlatApiEnumerateFiles_f*             EnumerateFiles;
	PlatApiCreateFolder_f*               CreateFolder;
	PlatApiReadFileContents_f*           ReadFileContents;
	PlatApiFreeFileContents_f*           FreeFileContents;
	PlatApiWriteEntireFile_f*            WriteEntireFile;
	PlatApiOpenFile_f*                   OpenFile;
	PlatApiWriteToFile_f*                WriteToFile;
	PlatApiMoveFileCursor_f*             MoveFileCursor;
	PlatApiSeekToOffsetInFile_f*         SeekToOffsetInFile;
	PlatApiReadFromFile_f*               ReadFromFile;
	PlatApiCloseFile_f*                  CloseFile;
	PlatApiReadFileContentsStream_f*     ReadFileContentsStream;
	PlatApiOpenFileStream_f*             OpenFileStream;
	PlatApiTryParseImageFile_f*          TryParseImageFile;
	PlatApiSaveImageDataToFile_f*        SaveImageDataToFile;
	PlatApiFreeImageData_f*              FreeImageData;
	PlatApiShowFile_f*                   ShowFile;
	PlatApiShowSourceFile_f*             ShowSourceFile;
	PlatApiGetSpecialFolderPath_f*       GetSpecialFolderPath;
	PlatApiIsFileWatched_f*              IsFileWatched;
	PlatApiWatchFile_f*                  WatchFile;
	PlatApiUnwatchFile_f*                UnwatchFile;
	PlatApiGetFileIconId_f*              GetFileIconId;
	PlatApiGetFileIconImageData_f*       GetFileIconImageData;
	#ifndef WIN32_GFX_TEST
	PlatApiGetLoadProcAddressFunc_f*     GetLoadProcAddressFunc;
	#endif
	PlatApiRenderLoadingScreen_f*        RenderLoadingScreen;
	PlatApiFreeFontData_f*               FreeFontData;
	PlatApiReadPlatformFont_f*           ReadPlatformFont;
	PlatApiBakeFont_f*                   BakeFont;
	PlatApiDebugReadout_f*               DebugReadout;
	PlatApiCopyTextToClipboard_f*        CopyTextToClipboard;
	PlatApiPasteTextFromClipboard_f*     PasteTextFromClipboard;
	PlatApiGetProgramTime_f*             GetProgramTime;
	PlatApiGetMonitorVideoMode_f*        GetMonitorVideoMode;
	#if STEAM_BUILD
	PlatApiGetSteamFriendInfoById_f*     GetSteamFriendInfoById;
	PlatApiGetSteamFriendGroupById_f*    GetSteamFriendGroupById;
	PlatApiGetSteamFriendPresenceStr_f*  GetSteamFriendPresenceStr;
	PlatApiStartSteamFriendsQuery_f*     StartSteamFriendsQuery;
	PlatApiUpdateSteamFriendStatus_f*    UpdateSteamFriendStatus;
	PlatApiRequestSteamFriendAvatar_f*   RequestSteamFriendAvatar;
	#endif
	PlatApiCheckRunningProcess_f*        CheckRunningProcess;
	PlatApiStartProcess_f*               StartProcess;
	PlatApiCloseRunningProcess_f*        CloseRunningProcess;
	PlatApiRestartWithAdminPrivileges_f* RestartWithAdminPrivileges;
	#if BOX2D_SUPPORTED
	PlatApiInitPhysicsEngine_f*          InitPhysicsEngine;
	PlatApiDestroyPhysicsEngine_f*       DestroyPhysicsEngine;
	PlatApiPhysicsTick_f*                PhysicsTick;
	PlatApiCreatePhysicsBody_f*          CreatePhysicsBody;
	PlatApiDestroyPhysicsBody_f*         DestroyPhysicsBody;
	PlatApiSetPhysicsBodyVelocity_f*     SetPhysicsBodyVelocity;
	PlatApiGetPhysicsBodyState_f*        GetPhysicsBodyState;
	#endif
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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetInputEventTypeStr(InputEventType_t eventType);
#else
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
#endif

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
			i32 controllerIndex;
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
	
	#if STEAM_BUILD
	u64 steamIpcCallCount;
	#endif
};

struct EngineOutput_t
{
	bool exit;
	
	bool fixedTimeScaleEnabled;
	r64 fixedTimeScale;
	
	PlatCursor_t cursorType;
	
	PlatMouseMode_t mouseMode;
	
	bool moveWindow;
	u64 moveWindowId;
	reci moveWindowRec;
	
	bool changeFullscreen;
	u64 changeFullscreenWindowId;
	bool fullscreenEnabled;
	u64 fullscreenMonitorId;
	u64 fullscreenVideoModeIndex;
	u64 fullscreenFramerateIndex;
	v2i windowedResolution;
	i64 windowedFramerate;
	
	bool changeWindowMinimized;
	bool minizedWindow;
	u64 minimizeWindowId;
};

#define PIG_GET_VERSION_DEF(functionName) Version_t functionName()
typedef PIG_GET_VERSION_DEF(PigGetVersion_f);

#define PIG_GET_STARTUP_OPTIONS_DEF(functionName) void functionName(const StartupInfo_t* info, StartupOptions_t* optionsOut)
typedef PIG_GET_STARTUP_OPTIONS_DEF(PigGetStartupOptions_f);

#define PIG_INITIALIZE_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, u64 programTime, u64 unixTimestamp, u64 localTimestamp)
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

#define PIG_POST_RELOAD_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t oldVersion, u64 programTime, u64 unixTimestamp, u64 localTimestamp)
typedef PIG_POST_RELOAD_DEF(PigPostReload_f);

#define PIG_PERFORM_TASK_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, PlatThreadPoolThread_t* thread, PlatTask_t* task)
typedef PIG_PERFORM_TASK_DEF(PigPerformTask_f);

#if PROCMON_SUPPORTED
#define PIG_HANDLE_PROCMON_EVENT_DEF(functionName) void functionName(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, ProcmonEvent_t* event, MemArena_t* tempArena)
typedef PIG_HANDLE_PROCMON_EVENT_DEF(PigHandleProcmonEvent_f);
#endif

#endif //  _COMMON_INTERFACE_TYPES_H
