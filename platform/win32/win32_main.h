/*
File:   win32_main.h
Author: Taylor Robbins
Date:   09\23\2021
Description:
	** Contains the definition of the structure where we keep all of our Win32 platform state information
*/

#ifndef _WIN_32_MAIN_H
#define _WIN_32_MAIN_H

struct Win32PlatformState_t
{
	bool initialized;
	bool exitRequested;
	Version_t version;
	
	MemArena_t stdHeap;
	MemArena_t mainHeap;
	MemArena_t tempArena;
	#if BOX2D_SUPPORTED
	MemArena_t box2dHeap;
	#endif
	PlatMutex_t threadSafeHeapMutex;
	MemArena_t threadSafeHeap;
	
	PlatMutex_t debugLineMutex;
	StringFifo_t debugLineFifo;
	
	bool consoleWindowIsOpen;
	ProgramArguments_t programArgs;
	MyStr_t workingDirectory; //contains trailing \ or is empty
	MyStr_t exeDirectory; //contains trailing \ or is empty
	MyStr_t exeFileName;
	MyStr_t defaultDirectory; //contains trailing \ or is empty
	MyStr_t engineDllPath;
	MyStr_t engineDllTempPath;
	MyStr_t engineDllTempFormatStr;
	MyStr_t localTimezoneName;
	
	RenderApi_t renderApi;
	GlfwVersion_t glfwVersion;
	
	u64 nextMonitorId;
	u64 nextMonitorVideoModeId;
	u64 nextFileContentsId;
	u64 nextOpenFileId;
	u64 nextWatchedFileId;
	u64 nextThreadId;
	u64 nextMutexId;
	u64 nextSemaphoreId;
	u64 nextInterlockedIntId;
	u64 nextAllocationId;
	u64 nextWindowId;
	u64 nextTaskId;
	u64 nextImageId;
	u64 nextFontDataId;
	u64 nextPackContextId;
	u64 nextInputEventId;
	
	HDC winDeviceContext;
	HGLRC winGraphicsContext;
	HWND windowHandle; //TODO: Should we have one for each window we create?
	HANDLE processHandle;
	
	PlatMonitorList_t monitors;
	
	i64 perfCountFrequency;
	PerfTime_t firstUpdateStartTime;
	r64 timeSpentOnSwapBuffers;
	
	u64 numOpenWindows;
	LinkedList_t windows;
	PlatWindow_t* mainWindow;
	PlatWindow_t* currentWindow;
	
	StartupInfo_t startupInfo;
	StartupOptions_t startupOptions;
	PlatformInfo_t info;
	PlatformApi_t api;
	EngineMemory_t engineMemory;
	EngineInput_t enginePreviousInput;
	EngineInput_t engineInput;
	EngineInput_t engineActiveInput;
	EngineOutput_t engineOutput;
	bool hasReachedStableFramerate;
	u64 numStableElapsedMsInARow;
	
	bool callingEngineInitialize;
	EngineDll_t engine;
	u64 engineReloadCount;
	
	PlatThread_t* fileWatchingThread;
	FileWatchingContext_t fileWatchingContext;
	
	r32 loadingPercent;
	
	PlatCursor_t currentCursorType;
	PlatMouseMode_t currentMouseMode;
	GLFWcursor* glfwCursors[PlatCursor_NumCursors];
	
	//Debug Output
	LinkedList_t debugReadoutLines;
	
	//Threading
	PlatThread_t threads[PLAT_MAX_NUM_THREADS];
	u64 threadPoolSize;
	PlatSemaphore_t threadPoolSemaphore;
	PlatThreadPoolThread_t threadPool[PLAT_MAX_THREADPOOL_SIZE];
	u64 numQueuedTasks;
	PlatTask_t queuedTasks[PLAT_MAX_NUM_TASKS];
	
	//Audio
	IMMDeviceEnumerator* audioDeviceEnumerator;
	AudioCallbackClass_c audioDeviceCallback;
	u64 defaultAudioDeviceIndex;
	VarArray_t audioDevices; //PlatAudioDevice_t
	IAudioClient* audioClient;
	IAudioRenderClient* audioRenderClient;
	PlatAudioFormat_t audioFormat;
	u64 audioFrameIndex;
	PlatThread_t* audioThread;
	#if DEVELOPER_BUILD
	u64 numAudioFrameDrops;
	#endif
	PlatMutex_t audioOutputMutex;
	AudioServiceInfo_t audioServiceInfo;
	bool audioWaitForFirstUpdateAfterReload;
	
	//Basic Rendering
	VarArray_t vertexArrayObjs;
	Win32_Texture_t dotTexture;
	Win32_Texture_t loadingImage;
	Win32_Texture_t loadingBackground;
	Win32_VertBuffer_t squareBuffer;
	Win32_Shader_t loadingShader;
	Win32_Font_t debugFont;
	Win32_Font_t hexFont;
	
	Win32_VertexArrayObject_t* boundVao;
	Win32_Shader_t* boundShader;
	Win32_Texture_t* boundTexture;
	Win32_VertBuffer_t* boundBuffer;
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	
};

#endif //  _WIN_32_MAIN_H
