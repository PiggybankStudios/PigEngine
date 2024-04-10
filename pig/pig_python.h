/*
File:   pig_python.h
Author: Taylor Robbins
Date:   04\08\2024
*/

#ifndef _PIG_PYTHON_H
#define _PIG_PYTHON_H

#if PYTHON_SUPPORTED

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
