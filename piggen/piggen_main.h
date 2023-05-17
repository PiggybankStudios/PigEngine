/*
File:   piggen_main.h
Author: Taylor Robbins
Date:   05\09\2023
*/

#ifndef _PIGGEN_MAIN_H
#define _PIGGEN_MAIN_H

#define PIGGEN_TEMP_ARENA_SIZE       Kilobytes(512)
#define PIGGEN_TEMP_ARENA_MAX_MARKS  256

#define EXIT_CODE_STARTUP_FAILURE  1
#define EXIT_CODE_ASSERTION_FAILED 2

struct ProgramArguments_t
{
	u64 count;
	MyStr_t* args;
};

struct FileEnumerator_t
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
struct FileContents_t
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

struct ExclusionPattern_t
{
	MyStr_t patternStr;
};

struct FileToProcess_t
{
	MyStr_t path;
};

struct PigGenState_t
{
	MemArena_t stdHeap;
	MemArena_t mainHeap;
	MemArena_t tempArena;
	
	bool verboseEnabled;
	
	u64 nextFileContentsId;
	
	ProgramArguments_t programArgs;
	VarArray_t exclusionPatterns; //ExclusionPattern_t
	
	MyStr_t defaultDirectory;
	MyStr_t targetPath;
};

#endif //  _PIGGEN_MAIN_H
