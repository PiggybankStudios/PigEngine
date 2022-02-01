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
	
	HANDLE handle;
};

// +--------------------------------------------------------------+
// |                           Threads                            |
// +--------------------------------------------------------------+
typedef DWORD ThreadId_t;

#define THREAD_FUNCTION_DEF(functionName, userPntrVarName) DWORD WINAPI functionName(LPVOID userPntrVarName)
typedef THREAD_FUNCTION_DEF(PlatThreadFunction_f, userPntr);

struct PlatThread_t
{
	u64 id;
	bool active;
	bool waitingForCleanup;
	PlatThreadFunction_f* function;
	void* userPntr;
	
	HANDLE handle;
	DWORD win32_id;
};

struct PlatThreadPoolThread_t
{
	u64 id;
	PlatThread_t* threadPntr;
	//TODO: Maybe a TempArena?
	
	bool shouldClose;
	bool isClosed;
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
	
	WindowEngineInput_t prevInput;
	WindowEngineInput_t input;
	WindowEngineInput_t activeInput;
	
	GLFWwindow* handle;
};

// +--------------------------------------------------------------+
// |                            Audio                             |
// +--------------------------------------------------------------+
struct PlatAudioDevice_t
{
	MemArena_t* allocArena;
	bool isDefaultDevice;
	MyStr_t name;
	MyStr_t deviceId;
	
	IMMDevice* devicePntr;
};

// +--------------------------------------------------------------+
// |                            Other                             |
// +--------------------------------------------------------------+
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
};
struct PlatTaskResult_t
{
	bool success;
	u8 resultCode;
	
	void* resultPntr1;
	u64 resultSize1;
	void* resultPntr2;
	u64 resultSize2;
	void* resultPntr3;
	u64 resultSize3;
};
struct PlatTask_t
{
	u64 id;
	ThreadId_t threadId;
	PlatTaskInput_t input;
	PlatTaskResult_t result;
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
