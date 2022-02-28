/*
File:   pig_tasks.h
Author: Taylor Robbins
Date:   02\28\2022
*/

#ifndef _PIG_TASKS_H
#define _PIG_TASKS_H

enum PigTask_t
{
	PigTask_None = 0,
	PigTask_ReadFileContents,
	PigTask_WriteEntireFile,
	PigTask_NumTypes, //NOTE: We can't go above GameTask_Base (currently set to 100)
};

#define TASK_CALLBACK_FUNC_DEF(functionName) void functionName(PlatTask_t* task, void* contextPntr)
typedef TASK_CALLBACK_FUNC_DEF(TaskCallbackFunc_f);

#define READ_FILE_CONTENTS_CALLBACK_FUNC_DEF(functionName) void functionName(bool success, MyStr_t filePath, PlatFileContents_t* file, void* contextPntr)
typedef READ_FILE_CONTENTS_CALLBACK_FUNC_DEF(ReadFileContentsCallback_f);

#define WRITE_ENTIRE_FILE_CALLBACK_FUNC_DEF(functionName) void functionName(bool success, MyStr_t filePath, u64 contentsSize, void* contentsPntr, void* contextPntr)
typedef WRITE_ENTIRE_FILE_CALLBACK_FUNC_DEF(WriteEntireFileCallback_f);

#endif //  _PIG_TASKS_H
