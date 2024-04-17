/*
File:   pig_python.cpp
Author: Taylor Robbins
Date:   04\08\2024
Description: 
	** None 
*/

#if PYTHON_SUPPORTED

//https://docs.python.org/3/extending/embedding.html
//https://docs.python.org/3/c-api/intro.html
//https://docs.python.org/3/c-api/init.html
//https://www.python.org/downloads/release/python-3119/
//https://python.readthedocs.io/en/latest/faq/extending.html

MyStr_t ToStr(MemArena_t* memArena, PyObject* pyObject, u64 depth = 0)
{
	StringBuilder_t result;
	NewStringBuilder(&result, memArena);
	if (pyObject != nullptr)
	{
		// +==============================+
		// |        Stringify None        |
		// +==============================+
		if (Py_IsNone(pyObject))
		{
			StringBuilderAppend(&result, "None");
		}
		// +==============================+
		// |        Stringify Bool        |
		// +==============================+
		else if (PyBool_Check(pyObject))
		{
			StringBuilderAppend(&result, Py_IsTrue(pyObject) ? "True" : "False");
		}
		// +==============================+
		// |        Stringify Long        |
		// +==============================+
		else if (PyLong_Check(pyObject))
		{
			long valueLong = PyLong_AsLong(pyObject);
			StringBuilderAppendPrint(&result, "%lld", valueLong);
		}
		// +==============================+
		// |       Stringify Float        |
		// +==============================+
		else if (PyFloat_Check(pyObject))
		{
			double valueDouble = PyFloat_AsDouble(pyObject);
			StringBuilderAppendPrint(&result, "%llf", valueDouble);
		}
		// // +==============================+
		// // |        Stringify Type        |
		// // +==============================+
		// else if (PyType_Check(pyObject))
		// {
		// 	PyTypeObject* pyType = _PyType_CAST(pyObject);
		// 	NotNull(pyType);
		// 	StringBuilderAppendPrint(&result, "(%s)", pyType->tp_name);
		// }
		// +==============================+
		// |      Stringify Function      |
		// +==============================+
		else if (PyFunction_Check(pyObject))
		{
			PyObject* code = PyFunction_GetCode(pyObject);
			MemArena_t* scratch = GetScratchArena(memArena);
			MyStr_t codeStr = ToStr(scratch, code, depth+1);
			StringBuilderAppendPrint(&result, "Function(%.*s)", StrPrint(codeStr));
			FreeScratchArena(scratch);
		}
		// +==============================+
		// |      Stringify Unicode       |
		// +==============================+
		else if (PyUnicode_Check(pyObject))
		{
			i64 numWideChars = PyUnicode_AsWideChar(pyObject, nullptr, 0);
			Assert(numWideChars > 0);
			MemArena_t* scratch = GetScratchArena(memArena);
			MyWideStr_t wideStr;
			wideStr.length = numWideChars-1;
			wideStr.chars = AllocArray(scratch, wchar_t, wideStr.length+1);
			NotNull(wideStr.chars);
			PyUnicode_AsWideChar(pyObject, wideStr.chars, wideStr.length+1);
			wideStr.chars[wideStr.length] = 0;
			StringBuilderAppendPrint(&result, "\"%S\"", wideStr.chars);
			FreeScratchArena(scratch);
		}
		// +==============================+
		// |        Stringify List        |
		// +==============================+
		else if (PyList_Check(pyObject))
		{
			i64 listLength = PyList_Size(pyObject);
			StringBuilderAppendPrint(&result, "List[%lld]{", listLength);
			for (i64 itemIndex = 0; itemIndex < listLength; itemIndex++)
			{
				StringBuilderAppend(&result, (itemIndex > 0) ? ", " : " ");
				PyObject* listItem = PyList_GetItem(pyObject, itemIndex);
				MemArena_t* scratch = GetScratchArena(memArena);
				MyStr_t itemStr = ToStr(scratch, listItem, depth+1);
				StringBuilderAppend(&result, itemStr);
				FreeScratchArena(scratch);
			}
			StringBuilderAppend(&result, " }");
		}
		// +==============================+
		// |        Stringify Dict        |
		// +==============================+
		else if (PyDict_Check(pyObject))
		{
			i64 dictSize = PyDict_Size(pyObject);
			StringBuilderAppendPrint(&result, "Dict[%lld]{", dictSize);
			PyObject* dictKeys = PyDict_Keys(pyObject);
			NotNull(dictKeys);
			Assert(PyList_Check(dictKeys));
			Assert(PyList_Size(dictKeys) == dictSize);
			for (i64 keyIndex = 0; keyIndex < dictSize; keyIndex++)
			{
				StringBuilderAppendLine(&result, (keyIndex > 0) ? "," : "");
				StringBuilderAppendChar(&result, '\t', depth+1);
				PyObject* key = PyList_GetItem(dictKeys, keyIndex);
				NotNull(key);
				PyObject* value = PyDict_GetItem(pyObject, key);
				NotNull(value);
				MemArena_t* scratch = GetScratchArena(memArena);
				MyStr_t keyStr = ToStr(scratch, key, depth+1);
				MyStr_t valueStr = ToStr(scratch, value, depth+1);
				StringBuilderAppend(&result, keyStr);
				StringBuilderAppend(&result, ": ");
				StringBuilderAppend(&result, valueStr);
				FreeScratchArena(scratch);
			}
			if (dictSize > 0)
			{
				StringBuilderAppendLine(&result);
				if (depth > 0) { StringBuilderAppendChar(&result, '\t', depth); }
			}
			StringBuilderAppend(&result, " }");
		}
		// +==============================+
		// |       Stringify Module       |
		// +==============================+
		else if (PyModule_Check(pyObject))
		{
			PyObject* moduleDict = PyModule_GetDict(pyObject);
			NotNull(moduleDict);
			Assert(PyDict_Check(moduleDict));
			i64 moduleDictSize = PyDict_Size(moduleDict);
			StringBuilderAppendPrint(&result, "Module{\"%s\"}[%lld]", PyModule_GetName(pyObject), moduleDictSize);
		}
		// +===============================+
		// | Stringify Unknown Python Type |
		// +===============================+
		else
		{
			PyObject* pyObjectStr = PyObject_Str(pyObject);
			MemArena_t* scratch = GetScratchArena(memArena);
			MyStr_t pyObjectMyStr = ToStr(scratch, pyObjectStr);
			StringBuilderAppendPrint(&result, "<<%s %.*s>>", pyObject->ob_type->tp_name, StrPrint(pyObjectMyStr));
			FreeScratchArena(scratch);
			Py_DECREF(pyObjectStr);
		}
	}
	else
	{
		StringBuilderAppend(&result, "nullptr");
	}
	return ToMyStr(&result);
}
MyStr_t ToStr(MemArena_t* memArena, const PyObject* pyObject, u64 depth = 0) //const-variant
{
	return ToStr(memArena, (PyObject*)pyObject, depth);
}

void PrintPyObject(DbgLevel_t dbgLevel, const char* prefixStr, const PyObject* pyObject)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t stringifiedPyObject = ToStr(scratch, pyObject);
	PrintLineAt(dbgLevel, "%s%s", (prefixStr != nullptr) ? prefixStr : "", stringifiedPyObject.chars);
	FreeScratchArena(scratch);
}
void PrintPyObject(const char* prefixStr, const PyObject* pyObject)
{
	PrintPyObject(DbgLevel_Debug, prefixStr, pyObject);
}
void PrintPyObject(DbgLevel_t dbgLevel, const PyObject* pyObject)
{
	PrintPyObject(dbgLevel, nullptr, pyObject);
}
void PrintPyObject(const PyObject* pyObject)
{
	PrintPyObject(DbgLevel_Debug, nullptr, pyObject);
}

void PrintPyException(const char* prefixStr = nullptr)
{
	MemArena_t* scratch = GetScratchArena();
	PyObject* exception = nullptr;
	PyObject* exceptionValue = nullptr;
	PyObject* exceptionTraceback = nullptr;
	PyErr_Fetch(&exception, &exceptionValue, &exceptionTraceback);
	NotNull(exception);
	PyErr_NormalizeException(&exception, &exceptionValue, &exceptionTraceback);
	NotNull(exceptionValue);
	PyObject* exceptionValueStr = PyObject_Str(exceptionValue);
	if (exceptionTraceback == nullptr) { exceptionTraceback = Py_None; Py_INCREF(exceptionTraceback); }
	PrintLine_E("%s%s %s (Trace=%s)", (prefixStr != nullptr) ? prefixStr : "", ToStr(scratch, exception).chars, ToStr(scratch, exceptionValueStr).chars, ToStr(scratch, exceptionTraceback).chars);
	Py_XDECREF(exception);
	Py_XDECREF(exceptionValueStr);
	Py_XDECREF(exceptionValue);
	Py_XDECREF(exceptionTraceback);
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                   Python Memory Callbacks                    |
// +--------------------------------------------------------------+
void* PythonMallocCallback(void* ctx, size_t size)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	if (size == 0) { size = 1; } //We are required to provide a unique pntr when asked to allocate 0 bytes (https://docs.python.org/3/c-api/memory.html#c.PyMem_SetAllocator)
	void* result = AllocMem(pythonHeap, (u64)size);
	return result;
}
void* PythonCallocCallback(void* ctx, size_t nelem, size_t elsize)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	void* result = AllocMem(pythonHeap, (u64)(nelem * elsize));
	if (result != nullptr) { MyMemSet(result, 0x00, (u64)(nelem * elsize)); }
	return result;
}
void* PythonReallocCallback(void* ctx, void* ptr, size_t new_size)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	void* result = ReallocMem(pythonHeap, ptr, new_size, 0, AllocAlignment_None, true);
	return result;
}
void PythonFreeCallback(void* ctx, void* ptr)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	FreeMem(pythonHeap, ptr, 0, true);
}

// +--------------------------------------------------------------+
// |                    Python Initialization                     |
// +--------------------------------------------------------------+
void PigHandlePythonInitException(PyStatus status, const char* functionName)
{
	if (PyStatus_Exception(status))
	{
		PrintLine_E("Python exception %d in %s (%s): %s", status.exitcode, functionName, status.func, status.err_msg);
		MyDebugBreak();
	}
}

void PigInitPython()
{
	PerfTime_t initStartTime = GetPerfTime();
	MemArena_t* scratch = GetScratchArena();
	ClearStruct(pig->python);
	PyStatus status = {};
	
	//TODO: Change this back to our own arena! (Our arena was running rather slow during initialization of python. We should do some performance work for PagedHeap and arenas in general)
	#if 1
	pig->python.allocator.ctx     = &pig->pythonHeap;
	#else
	pig->python.allocator.ctx     = platInfo->stdHeap;
	#endif
	pig->python.allocator.malloc  = PythonMallocCallback;
	pig->python.allocator.calloc  = PythonCallocCallback;
	pig->python.allocator.realloc = PythonReallocCallback;
	pig->python.allocator.free    = PythonFreeCallback;
	#if PIG_MEM_ARENA_TEST_SET
	NewMemArenaTestSet(platInfo->stdHeap, &pig->arenaTestSet, 31000, 48000);
	pig->pythonHeap.testSetOut = &pig->arenaTestSet;
	#endif
	
	// pig->python.preConfig._config_init = 0;
	pig->python.preConfig.parse_argv = 0;
	pig->python.preConfig.isolated = 1;
	pig->python.preConfig.use_environment = 0;
	pig->python.preConfig.configure_locale = 0;
	pig->python.preConfig.coerce_c_locale = 1;
	// pig->python.preConfig.coerce_c_locale_warn = 0;
	#if WINDOWS_COMPILATION
	pig->python.preConfig.legacy_windows_fs_encoding = 0;
	#endif
	pig->python.preConfig.utf8_mode = 1;
	pig->python.preConfig.dev_mode = 1;
	pig->python.preConfig.allocator = PYMEM_ALLOCATOR_NOT_SET;
	status = Py_PreInitialize(&pig->python.preConfig);
	PigHandlePythonInitException(status, "Py_PreInitialize");
	
	PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &pig->python.allocator);
	PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &pig->python.allocator);
	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &pig->python.allocator);
	
	{
		PyConfig_InitIsolatedConfig(&pig->python.config);
		
		MyStr_t programPath = CombineStrs(scratch, platInfo->defaultDirectory, platInfo->exeFileName);
		status = PyConfig_SetString(&pig->python.config, &pig->python.config.program_name, ConvertUtf8StrToUcs2(scratch, programPath).chars);
		PigHandlePythonInitException(status, "Py_SetString");
		
		//TODO: What does this do actually?
		pig->python.config.dev_mode = 1;
		
		// int import_time; //=0
		// int code_debug_ranges; //=1
		// int show_ref_count; //=0
		// int dump_refs; //=0
		// wchar_t *dump_refs_file; //=nullptr
		// int malloc_stats; //=0
		// wchar_t *filesystem_encoding; //=nullptr
		// wchar_t *filesystem_errors; //=nullptr
		// wchar_t *pycache_prefix; //=nullptr
		// int parse_argv; //=0
		// PyWideStringList orig_argv; //=0 items
		// PyWideStringList argv; //0 items
		// PyWideStringList xoptions; //=0 items
		// PyWideStringList warnoptions; //=0 items
		// int site_import; //=1
		// int bytes_warning; //=0
		// int warn_default_encoding; //=0
		// int inspect; //=0
		// int interactive; //=0
		// int optimization_level; //=0
		// int parser_debug; //=0
		// int write_bytecode; //=0
		// int verbose; //=0
		// int quiet; //=0
		// int configure_c_stdio; //=0
		// int buffered_stdio; //=1
		// wchar_t *stdio_encoding; //=nullptr
		// wchar_t *stdio_errors; //=nullptr
		// wchar_t *check_hash_pycs_mode; //=nullptr
		// int use_frozen_modules; //=0
		// /* --- Path configuration inputs ------------ */
		// wchar_t *program_name; //="C:/gamedev/projects/Oort/data/Oort.exe"
		// wchar_t *pythonpath_env; //=nullptr
		// wchar_t *home; //=nullptr
		// wchar_t *platlibdir; //=nullptr
		// /* --- Path configuration outputs ----------- */
		// int module_search_paths_set; //=0
		// PyWideStringList module_search_paths; //=0 items
		// wchar_t *stdlib_dir; //=nullptr
		// wchar_t *executable; //=nullptr
		// wchar_t *base_executable; //=nullptr
		// wchar_t *prefix; //=nullptr
		// wchar_t *base_prefix; //=nullptr
		// wchar_t *exec_prefix; //=nullptr
		// wchar_t *base_exec_prefix; //=nullptr
		// /* --- Parameter only used by Py_Main() ---------- */
		// int skip_source_first_line; //=0
		// wchar_t *run_command; //=nullptr
		// wchar_t *run_module; //=nullptr
		// wchar_t *run_filename; //=nullptr
		// /* --- Private fields ---------------------------- */
		// // Install importlib? If equals to 0, importlib is not initialized at all.
		// // Needed by freeze_importlib.
		// int _install_importlib; //=1
		// // If equal to 0, stop Python initialization before the "main" phase.
		// int _init_main; //=1
		// // If non-zero, disallow threads, subprocesses, and fork.
		// // Default: 0.
		// int _isolated_interpreter; //=0
		// // If non-zero, we believe we're running from a source tree.
		// int _is_python_build; //=0
	}
	
	status = Py_InitializeFromConfig(&pig->python.config);
	PigHandlePythonInitException(status, "Py_InitializeFromConfig");
	
	// FlagSet(pig->pythonHeap.flags, MemArenaFlag_TrackTime);
	// pig->pythonHeap.totalTimeSpentAllocating = {};
	// pig->pythonHeap.totalTimedAllocationActions = 0;
	// ...
	// FlagUnset(pig->pythonHeap.flags, MemArenaFlag_TrackTime);
	// PrintLine_D("Python Init took %s total (%llu alloc actions took %s)",
	// 	FormatMillisecondsNt((u64)GetPerfTimeDiff(&initStartTime, &initEndTime), scratch),
	// 	pig->pythonHeap.totalTimedAllocationActions,
	// 	FormatMillisecondsNt((u64)GetPerfTimeTotal(&pig->pythonHeap.totalTimeSpentAllocating), scratch)
	// );
	
	status = _Py_InitializeMain();
	PigHandlePythonInitException(status, "_Py_InitializeMain");
	
	#if PIG_MEM_ARENA_TEST_SET
	{
		pig->pythonHeap.testSetOut = nullptr;
		PrintLine_O("Captured %llu allocation action%s during python initialization (%llu unique allocation pntr%s)", pig->arenaTestSet.actions.length, Plural(pig->arenaTestSet.actions.length, "s"), pig->arenaTestSet.allocations.length, Plural(pig->arenaTestSet.allocations.length, "s"));
		MemArena_t* scratch2 = GetScratchArena();
		VarArrayLoop(&pig->arenaTestSet.actions, aIndex)
		{
			VarArrayLoopGet(MemArenaTestAction_t, action, &pig->arenaTestSet.actions, aIndex);
			PushMemMark(scratch2);
			switch (action->type)
			{
				case MemArenaTestActionType_Alloc: PrintLine_D("\tAlloc %s[%llu] (%s)", GetStandardRockName(action->allocIndex), (action->allocIndex / GYLIB_NUM_STANDARD_ROCK_NAMES), FormatBytesNt(action->size, scratch2)); break;
				case MemArenaTestActionType_Realloc:
					if (action->oldAllocIndex < pig->arenaTestSet.allocations.length) { PrintLine_D("\tRealloc %s[%llu] to %s[%llu] (%s %s -> %s)", GetStandardRockName(action->oldAllocIndex), (action->oldAllocIndex / GYLIB_NUM_STANDARD_ROCK_NAMES), GetStandardRockName(action->allocIndex), (action->allocIndex / GYLIB_NUM_STANDARD_ROCK_NAMES), (action->size >= action->oldSize ? "Grow" : "Shrink"), FormatBytesNt(action->oldSize, scratch2), FormatBytesNt(action->size, scratch2)); break; }
					else { PrintLine_D("\treAlloc %s[%llu] (%s)", GetStandardRockName(action->allocIndex), (action->allocIndex / GYLIB_NUM_STANDARD_ROCK_NAMES), FormatBytesNt(action->size, scratch2)); break; }
				case MemArenaTestActionType_Free: PrintLine_D("\tFree %s[%llu] (%s)", GetStandardRockName(action->allocIndex), (action->allocIndex / GYLIB_NUM_STANDARD_ROCK_NAMES), FormatBytesNt(action->size, scratch2)); break;
				default: Unimplemented();
			}
			PopMemMark(scratch2);
		}
		FreeScratchArena(scratch2);
	}
	#endif
	
	pig->python.mainModuleName = PyUnicode_FromString("__main__");
	pig->python.mainFunctionName = PyUnicode_FromString("main");
	
	FreeScratchArena(scratch);
	PerfTime_t initEndTime = GetPerfTime();
	PrintLine_D("PigInitPython took %s total", FormatMillisecondsNt((u64)GetPerfTimeDiff(&initStartTime, &initEndTime), scratch));
}

void PigPythonHandleReload()
{
	pig->python.allocator.malloc  = PythonMallocCallback;
	pig->python.allocator.calloc  = PythonCallocCallback;
	pig->python.allocator.realloc = PythonReallocCallback;
	pig->python.allocator.free    = PythonFreeCallback;
	PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &pig->python.allocator);
	PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &pig->python.allocator);
	PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &pig->python.allocator);
}

PyObject* PythonDebugOutputCallback(PyObject* arguments, PyObject* kwArguments)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t argumentsStr = ToStr(scratch, arguments);
	MyStr_t kwArgumentsStr = ToStr(scratch, kwArguments);
	PrintLine_I("PythonDebugOutputCallback(%.*s, %.*s)", StrPrint(argumentsStr), StrPrint(kwArgumentsStr));
	FreeScratchArena(scratch);
	return Py_None;
}

void FreePythonLocalDict(PyObject* localDict)
{
	NotNull(localDict);
	Assert(PyDict_Check(localDict));
	i64 localDictSize = PyDict_Size(localDict);
	PyObject* localDictKeys = PyDict_Keys(localDict);
	NotNull(localDictKeys);
	Assert(PyList_Check(localDictKeys));
	Assert(PyList_Size(localDictKeys) == localDictSize);
	for (i64 keyIndex = 0; keyIndex < localDictSize; keyIndex++)
	{
		PyObject* key = PyList_GetItem(localDictKeys, keyIndex);
		NotNull(key);
		PyDict_DelItem(localDict, key);
	}
	// Assert(Py_REFCNT(localDict) == 1); //TODO: Can we figure out why this isn't always going to 0 references??
	Py_DECREF(localDict);
}

void FreePythonScript(PythonScript_t* script)
{
	NotNull(script);
	NotNull(script->allocArena);
	if (script->localDict != nullptr) { FreePythonLocalDict(script->localDict); }
	FreeString(script->allocArena, &script->debugOutputCallbackNameStr);
	FreeString(script->allocArena, &script->debugOutputCallbackDescriptionStr);
	FreeMem(script->allocArena, script, sizeof(PythonScript_t));
}

PythonScript_t* CreatePythonScript(MemArena_t* memArena)
{
	NotNull(memArena);
	PythonScript_t* result = AllocStruct(memArena, PythonScript_t);
	NotNull(result);
	result->allocArena = memArena;
	result->localDict = PyDict_New();
	result->debugOutputCallbackNameStr = NewStringInArenaNt(memArena, "PigDebugOutput");
	result->debugOutputCallbackDescriptionStr = NewStringInArenaNt(memArena, "Write string(s) to the debug output");
	result->debugOutputCallbackDef = { result->debugOutputCallbackNameStr.chars, PythonDebugOutputCallback, METH_VARARGS, result->debugOutputCallbackDescriptionStr.chars }; //TODO: Flatten this!
	PyObject* callbackName = PyUnicode_FromString(result->debugOutputCallbackNameStr.chars);
	PyObject* callback = PyCFunction_New(&result->debugOutputCallbackDef, nullptr); //TODO: Pass result here instead of nullptr?
	PyDict_SetItem(result->localDict, callbackName, callback);
	Py_DECREF(callback);
	Py_DECREF(callbackName);
	return result;
}

//If this returns false, you can call PrintPyException to print the error
bool ParsePythonScriptStr(PythonScript_t* script, MyStr_t codeStr)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t allocatedCodeStr = AllocString(scratch, &codeStr);
	PyCompilerFlags flags = {};
	flags.cf_flags = PyCF_SOURCE_IS_UTF8;
	flags.cf_feature_version = PY_MINOR_VERSION;
	
	PyObject* parseResult = PyRun_StringFlags(allocatedCodeStr.chars, Py_file_input, script->localDict, script->localDict, &flags);
	
	Py_XDECREF(parseResult);
	FreeScratchArena(scratch);
	return (parseResult != nullptr);
}

//If this returns RunPythonFunctionResult_Exception, you can call PrintPyException to print the error
RunPythonFunctionResult_t RunPythonFunctionInDict(PyObject* pyDict, MyStr_t functionName, PyObject** returnOut)
{
	PyObject* functionNameObject = PyUnicode_FromStringAndSize(functionName.chars, functionName.length);
	PyObject* functionObject = PyDict_GetItem(pyDict, functionNameObject);
	Py_DECREF(functionNameObject);
	if (functionObject == nullptr) { return RunPythonFunctionResult_FunctionMissing; }
	Py_INCREF(functionObject);
	
	if (!PyFunction_Check(functionObject)) { Py_DECREF(functionObject); return RunPythonFunctionResult_NotAFunction; }
	
	PyObject* mainResult = PyObject_CallNoArgs(functionObject);
	if (mainResult == nullptr) { Py_DECREF(functionObject); return RunPythonFunctionResult_Exception; }
	
	if (returnOut != nullptr)
	{
		*returnOut = mainResult;
	}
	else { Py_DECREF(mainResult); }
	
	return RunPythonFunctionResult_Success;
}
RunPythonFunctionResult_t RunPythonFunctionInScript(PythonScript_t* script, MyStr_t functionName, PyObject** returnOut)
{
	NotNull2(script, script->localDict);
	return RunPythonFunctionInDict(script->localDict, functionName, returnOut);
}

#endif //PYTHON_SUPPORTED
