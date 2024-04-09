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

void* PythonMallocCallback(void* ctx, size_t size)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	if (size == 0) { size = 1; } //We are required to provide a unique pntr when asked to allocate 0 bytes (https://docs.python.org/3/c-api/memory.html#c.PyMem_SetAllocator)
	return AllocMem(pythonHeap, (u64)size);
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
	return ReallocMem(pythonHeap, ptr, new_size, 0, AllocAlignment_None, true);
}

void PythonFreeCallback(void* ctx, void* ptr)
{
	NotNull(ctx);
	MemArena_t* pythonHeap = (MemArena_t*)ctx;
	FreeMem(pythonHeap, ptr, 0, true);
}

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
	MemArena_t* scratch = GetScratchArena();
	ClearStruct(pig->python);
	PyStatus status = {};
	
	pig->python.allocator.ctx     = &pig->pythonHeap;
	pig->python.allocator.malloc  = PythonMallocCallback;
	pig->python.allocator.calloc  = PythonCallocCallback;
	pig->python.allocator.realloc = PythonReallocCallback;
	pig->python.allocator.free    = PythonFreeCallback;
	
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
	
	PyConfig_InitIsolatedConfig(&pig->python.config);
	MyStr_t programPath = CombineStrs(scratch, platInfo->defaultDirectory, platInfo->exeFileName);
	MyWideStr_t programPathWide = ConvertUtf8StrToUcs2(mainHeap, programPath); //TODO: Do we need to allocate this on mainHeap?
	status = PyConfig_SetString(&pig->python.config, &pig->python.config.program_name, programPathWide.chars);
	PigHandlePythonInitException(status, "Py_SetString");
	
	status = Py_InitializeFromConfig(&pig->python.config);
	PigHandlePythonInitException(status, "Py_InitializeFromConfig");
	
	// WriteLine_I("Running test.py");
	// wchar_t* program = Py_DecodeLocale("test.py", NULL);
	// WriteLine_I("Done with test.py!");
	
	FreeScratchArena(scratch);
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

#endif //PYTHON_SUPPORTED
