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
	//TODO: Add more tasks here
	PigTask_NumTypes, //NOTE: We can't go above GameTask_Base (currently set to 100)
};

#define TASK_CALLBACK_FUNC_DEF(functionName) void functionName(PlatTask_t* task, void* contextPntr)
typedef TASK_CALLBACK_FUNC_DEF(TaskCallbackFunc_f);

#endif //  _PIG_TASKS_H
