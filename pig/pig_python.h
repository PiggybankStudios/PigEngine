/*
File:   pig_python.h
Author: Taylor Robbins
Date:   04\08\2024
*/

#ifndef _PIG_PYTHON_H
#define _PIG_PYTHON_H

#if PYTHON_SUPPORTED

enum RunPythonFunctionResult_t
{
	RunPythonFunctionResult_None = 0,
	RunPythonFunctionResult_Success,
	RunPythonFunctionResult_FunctionMissing,
	RunPythonFunctionResult_NotAFunction,
	RunPythonFunctionResult_Exception,
	RunPythonFunctionResult_NumResults,
};
const char* GetRunPythonFunctionResultStr(RunPythonFunctionResult_t enumValue)
{
	switch (enumValue)
	{
		case RunPythonFunctionResult_None:            return "None";
		case RunPythonFunctionResult_Success:         return "Success";
		case RunPythonFunctionResult_FunctionMissing: return "FunctionMissing";
		case RunPythonFunctionResult_NotAFunction:    return "NotAFunction";
		case RunPythonFunctionResult_Exception:       return "Exception";
		default: return "Unknown";
	}
}

struct PythonScript_t
{
	MemArena_t* allocArena;
	PyObject* localDict;
	MyStr_t debugOutputCallbackNameStr;
	MyStr_t debugOutputCallbackDescriptionStr;
	PyMethodDef debugOutputCallbackDef;
};

struct PythonState_t
{
	PyMemAllocatorEx allocator;
	PyPreConfig preConfig;
	PyConfig config;
	
	PyObject* mainModuleName; //Unicode
	PyObject* mainFunctionName; //Unicode
};

#endif //PYTHON_SUPPORTED

#endif //  _PIG_PYTHON_H
