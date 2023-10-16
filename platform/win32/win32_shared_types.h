/*
File:   win32_shared_types.h
Author: Taylor Robbins
Date:   09\23\2021
Description:
	** This file defines the format for a bunch of common types that the engine
	** expects to know about and use to communicate to the platform layer.
	** These types may contain different data on each platform but they must have the same name
	** and some members are expected to exist so the engine can look at or manipulate them.
*/

#ifndef _WIN32_SHARED_TYPES_H
#define _WIN32_SHARED_TYPES_H

// +--------------------------------------------------------------+
// |                            Files                             |
// +--------------------------------------------------------------+
struct PlatFileEnumerator_t
{
	MyStr_t folderPath;
	MyStr_t folderPathWithWildcard;
	bool enumerateFiles;
	bool enumerateFolders;
	
	bool finished;
	u64 index;
	u64 nextIndex;
	
	WIN32_FIND_DATAA findData;
	HANDLE handle;
};

struct PlatFileContents_t
{
	u64 id;
	bool readSuccess;
	MyStr_t path;
	union
	{
		u64 size;
		u64 length;
	};
	union
	{
		u8* data;
		char* chars;
	};
	
	DWORD errorCode;
};

struct PlatOpenFile_t
{
	u64 id;
	bool isOpen;
	bool openedForWriting;
	u64 cursorIndex;
	u64 fileSize;
	MyStr_t path;
	MyStr_t fullPath;
	
	HANDLE handle;
};

// +--------------------------------------------------------------+
// |                           Threads                            |
// +--------------------------------------------------------------+
typedef DWORD ThreadId_t;

#define THREAD_FUNCTION_DEF(functionName, userPntrVarName) DWORD WINAPI functionName(LPVOID userPntrVarName)
typedef THREAD_FUNCTION_DEF(PlatThreadFunction_f, userPntr);

struct PlatThreadIdPair_t
{
	u64 internalId;
	ThreadId_t osId;
};

struct PlatThread_t
{
	u64 id;
	bool active;
	bool waitingForCleanup;
	PlatThreadFunction_f* function;
	void* userPntr;
	
	bool assertionFailed;
	const char* assertionFailedFilepath;
	int assertionFailedLineNum;
	const char* assertionFailedFuncName;
	const char* assertionFailedMessage;
	const char* assertionFailedExpression;
	
	HANDLE handle;
	DWORD win32_id;
};

struct PlatThreadPoolThread_t
{
	u64 id;
	PlatThread_t* threadPntr;
	MemArena_t tempArena;
	u64 scratchArenasMaxSize;
	u64 scratchArenasMarkCount;
	
	bool shouldClose;
	bool isClosed;
	bool isAwake;
};

struct PlatMutex_t
{
	u64 id;
	
	HANDLE handle;
};

struct PlatSemaphore_t
{
	u64 id;
	
	HANDLE handle;
};

struct PlatInterlockedInt_t
{
	u64 id;
	
	volatile LONG value;
};

// +--------------------------------------------------------------+
// |                          Processes                           |
// +--------------------------------------------------------------+
struct PlatRunningProcess_t
{
	u64 id;
	MemArena_t* allocArena;
	MyStr_t filePath;
	MyStr_t workingDirectory;
	MyStr_t argumentsString;
	
	bool isFinished;
	u64 lastExitCodeCheckTime;
	u64 exitCode;
	bool readyForRemoval;
	
	HANDLE handle;
	HANDLE threadHandle;
	DWORD  processId;
	DWORD  threadId;
};

// +--------------------------------------------------------------+
// |                            Memory                            |
// +--------------------------------------------------------------+
struct PlatAllocation_t
{
	u64 id;
	u64 size;
	void* base;
};

// +--------------------------------------------------------------+
// |                           Windows                            |
// +--------------------------------------------------------------+
struct PlatWindow_t
{
	u64 id;
	bool closed;
	PlatWindowOptions_t options;
	
	WindowEngineInput_t prevInput;
	WindowEngineInput_t input;
	WindowEngineInput_t activeInput;
	
	GLFWwindow* handle;
};

// +--------------------------------------------------------------+
// |                            Audio                             |
// +--------------------------------------------------------------+
enum PlatAudioDeviceState_t
{
	PlatAudioDeviceState_Unknown = 0,
	PlatAudioDeviceState_Unplugged,
	PlatAudioDeviceState_NotPresent,
	PlatAudioDeviceState_Disabled,
	PlatAudioDeviceState_Active,
	PlatAudioDeviceState_NumStates,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetAudioDeviceStateStr(PlatAudioDeviceState_t deviceState);
#else
const char* GetAudioDeviceStateStr(PlatAudioDeviceState_t deviceState)
{
	switch (deviceState)
	{
		case PlatAudioDeviceState_Unknown:    return "Unknown";
		case PlatAudioDeviceState_Unplugged:  return "Unplugged";
		case PlatAudioDeviceState_NotPresent: return "NotPresent";
		case PlatAudioDeviceState_Disabled:   return "Disabled";
		case PlatAudioDeviceState_Active:     return "Active";
		default: return "Unknown";
	}
}
#endif
struct PlatAudioDevice_t
{
	MemArena_t* allocArena;
	PlatAudioDeviceState_t state;
	bool isDefaultDevice;
	MyStr_t name;
	MyStr_t deviceId;
	
	IMMDevice* devicePntr;
};

// +--------------------------------------------------------------+
// |                            Other                             |
// +--------------------------------------------------------------+
typedef GLADloadproc GlLoadProc_f;

#define MAX_NUM_CONTROLLERS  GLFW_JOYSTICK_LAST //this is 16 controllers right now

struct PlatWatchedFile_t
{
	u64 id;
	MyStr_t path;
	MyStr_t fullPath;
	PlatInterlockedInt_t changed;
	
	u64 lastWriteTimeChange;
	FILETIME lastWriteTime;
};

struct PlatTaskInput_t
{
	MemArena_t* memArena;
	u64 type; //to be filled with enum by application
	u64 id; //to be filled with some ID by application
	
	u64 contextSize;
	void* contextPntr;
	MemArena_t* contextArena;
	
	MyStr_t inputStr;
	MemArena_t* inputStrArena;
	
	u64 inputSize1;
	void* inputPntr1;
	MemArena_t* inputArena1;
	
	u64 inputSize2;
	void* inputPntr2;
	MemArena_t* inputArena2;
	
	u64 inputSize3;
	void* inputPntr3;
	MemArena_t* inputArena3;
	
	void* callbackFunc;
	void* callbackContext;
};
struct PlatTaskResult_t
{
	bool success;
	u8 resultCode;
	
	u64 resultSize1;
	void* resultPntr1;
	u64 resultSize2;
	void* resultPntr2;
	u64 resultSize3;
	void* resultPntr3;
};
struct PlatTask_t
{
	u64 id; //used as a filled flag, 0 meaning not filled
	PlatInterlockedInt_t claimId; //thread interlock point
	ThreadId_t threadId;
	u64 poolId;
	PlatTaskInput_t input;
	PlatTaskResult_t result;
	bool finished;
};

struct PerfTime_t
{
	LARGE_INTEGER perfCount;
	u64 cycleCount;
};

struct PerfSection_t
{
	const char* name;
	PerfTime_t time;
};

struct PerfSectionBundle_t
{
	MemArena_t* allocArena;
	VarArray_t sections; //PerfSection_t
};

struct PlatDebugLine_t
{
	u8 flags;
	u64 programTime;
	u64 preciseProgramTime;
	u64 timestamp;
	ThreadId_t thread;
	u64 fileLineNumber;
	DbgLevel_t dbgLevel;
};

struct PlatRectPackContext_t
{
	u64 id;
	v2i packSize;
	
	stbrp_context stbContext;
};

#endif //  _WIN32_SHARED_TYPES_H
