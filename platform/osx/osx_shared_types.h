/*
File:   osx_shared_types.h
Author: Taylor Robbins
Date:   10\02\2021
*/

#ifndef _OSX_SHARED_TYPES_H
#define _OSX_SHARED_TYPES_H

// +--------------------------------------------------------------+
// |                            Files                             |
// +--------------------------------------------------------------+
struct PlatFileContents_t
{
	u64 id;
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
};

struct PlatOpenFile_t
{
	u64 id;
	bool isOpen;
	bool openedForWriting;
	u64 cursorIndex;
	u64 fileSize;
	
	// HANDLE handle; //TODO: Implement me!
};

struct PlatWatchedFile_t
{
	u64 id;
	MyStr_t path;
	MyStr_t fullPath;
	bool changed;
	
	// FILETIME lastWriteTime; //TODO: Implement me!
};

// +--------------------------------------------------------------+
// |                           Threads                            |
// +--------------------------------------------------------------+
typedef u32 ThreadId_t; //TODO: Figure out what this should be

// #define THREAD_FUNCTION_DEF(functionName, userPntrVarName) DWORD WINAPI functionName(LPVOID userPntrVarName)
// typedef THREAD_FUNCTION_DEF(Win32ThreadFunction_f, userPntr);

struct PlatThread_t
{
	u64 id;
	// Win32ThreadFunction_f* function; //TODO: Implement me!
	void* userPntr;
	
	// HANDLE handle; //TODO: Implement me!
	// DWORD win32_id; //TODO: Implement me!
};

struct PlatMutex_t
{
	u64 id;
	
	// HANDLE handle; //TODO: Implement me!
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
	
	bool minimizedChanged;
	bool minimized;
	
	bool resized;
	v2i previousResolution;
	v2i pixelResolution; //the actual number of pixels we think we have to work with
	v2i contextResolution; //the size that OpenGL or whatever wants us to talk in when doing stuff like glViewport
	v2 renderResolution; //the size used for all of our render logic. The "effective" resolution
	
	bool moved;
	v2i position;
	
	GLFWwindow* handle;
};

// +--------------------------------------------------------------+
// |                            Other                             |
// +--------------------------------------------------------------+
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

#endif //  _OSX_SHARED_TYPES_H
