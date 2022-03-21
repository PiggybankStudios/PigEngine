/*
File:   web_shared_types.h
Author: Taylor Robbins
Date:   03\13\2022
*/

#ifndef _WEB_SHARED_TYPES_H
#define _WEB_SHARED_TYPES_H

//TODO: Copy the shared type structures here when we are ready to implement them

// +--------------------------------------------------------------+
// |                            Files                             |
// +--------------------------------------------------------------+
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
	
	//TODO: Put platform specific stuff here
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
	
	//TODO: Put platform specific stuff here
};

// +--------------------------------------------------------------+
// |                           Threads                            |
// +--------------------------------------------------------------+
typedef int ThreadId_t;

#define THREAD_FUNCTION_DEF(functionName, userPntrVarName) int functionName(void* userPntrVarName)
typedef THREAD_FUNCTION_DEF(PlatThreadFunction_f, userPntr);

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
};

struct PlatThreadPoolThread_t
{
	u64 id;
	PlatThread_t* threadPntr;
	MemArena_t tempArena;
	
	bool shouldClose;
	bool isClosed;
	bool isAwake;
};

struct PlatMutex_t
{
	u64 id;
};

struct PlatSemaphore_t
{
	u64 id;
};

struct PlatInterlockedInt_t
{
	u64 id;
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
	
	//TODO: Put platform specific stuff here
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
struct PlatAudioDevice_t
{
	MemArena_t* allocArena;
	PlatAudioDeviceState_t state;
	bool isDefaultDevice;
	MyStr_t name;
	MyStr_t deviceId;
	
	//TODO: Put platform specific stuff here
};

// +--------------------------------------------------------------+
// |                            Other                             |
// +--------------------------------------------------------------+
#define GL_LOAD_PROC_DEFINITION(functionName) void functionName()
typedef GL_LOAD_PROC_DEFINITION(GlLoadProcBase_f);
typedef GlLoadProcBase_f* GlLoadProc_f;

//TODO: Is this even a thing on web? Do we have any controller support at all?
#define MAX_NUM_CONTROLLERS  4 //this is 16 controllers right now

struct PlatWatchedFile_t
{
	u64 id;
	MyStr_t path;
	MyStr_t fullPath;
	PlatInterlockedInt_t changed;
	
	//TODO: Put platform specific stuff here
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
	//TODO: Put platform specific stuff here
	u64 placeholder;
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

#endif //  _WEB_SHARED_TYPES_H
