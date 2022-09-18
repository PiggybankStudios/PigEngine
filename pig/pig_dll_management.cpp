/*
File:   pig_dll_management.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds a bunch of functions that help us deal with linking up globals, updating function pointers,
	** and various other consequences of being a hot-loaded DLL 
*/

void* PlatAllocFunc(u64 size)
{
	AssertNormalEntry();
	return plat->AllocateMemory(size, AllocAlignment_None);
}
void* PlatReallocFunc(void* allocPntr, u64 newSize, u64 oldSize = 0)
{
	AssertNormalEntry();
	return plat->ReallocMemory(allocPntr, newSize, oldSize, AllocAlignment_None);
}
void PlatFreeFunc(void* allocPntr)
{
	AssertNormalEntry();
	plat->FreeMemory(allocPntr, 0, nullptr);
}

void PigClearGlobals()
{
	pig = nullptr;
	mainHeap = nullptr;
	platHeap = nullptr;
}

void PigInitGlad()
{
	NotNull(plat);
	#if OPENGL_SUPPORTED
	if (!pig->initialized) { WriteLine_R("Initializing Engine GLAD..."); }
	if (!gladLoadGLLoader((GlLoadProc_f)plat->GetLoadProcAddressFunc()))
	{
		AssertMsg(false, "Could not initialize GLAD in Engine. gladLoadGLLoader failed");
	}
	if (!pig->initialized) { WriteLine_R("Engine GLAD Init Succeeded!"); }
	#endif
}

// +==============================+
// |       Pig_GetTempArena       |
// +==============================+
GYLIB_GET_TEMP_ARENA_DEF(Pig_GetTempArena)
{
	if (plat == nullptr) { return nullptr; }
	if (pig == nullptr) { return nullptr; }
	ThreadId_t thisThreadId = plat->GetThisThreadId();
	if (thisThreadId == pig->mainThreadId)
	{
		return &pig->tempArena;
	}
	else
	{
		PlatThreadPoolThread_t* threadContext = plat->GetThreadContext(thisThreadId);
		if (threadContext != nullptr && threadContext->tempArena.size > 0)
		{
			return &threadContext->tempArena;
		}
		else { return nullptr; }
	}
}

void PigEntryPoint(PigEntryPoint_t entryPoint, const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, EngineInput_t* input, EngineOutput_t* output)
{
	pigEntryPoint = entryPoint;
	platInfo = info;
	plat = api;
	pigIn = input;
	pigOut = output;
	if (memory != nullptr)
	{
		NotNull_(api);
		NotNull_(memory->persistentDataPntr);
		Assert_(memory->persistentDataSize >= sizeof(PigState_t));
		pig = (PigState_t*)memory->persistentDataPntr;
		fixedHeap = &pig->fixedHeap;
		mainHeap = &pig->mainHeap;
		platHeap = &pig->platHeap;
		rc = &pig->renderContext;
		//TODO: Update the gylib debug output pointers
		TempArena = &pig->tempArena;
		GetTempArena = Pig_GetTempArena;
		if (entryPoint != PigEntryPoint_Initialize) { TempPushMark(); }
	}
	else
	{
		PigClearGlobals();
	}
	GameUpdateGlobals();
	if (entryPoint != PigEntryPoint_Initialize && plat != nullptr)
	{
		AssertSingleThreaded();
	}
	if (entryPoint != PigEntryPoint_GetVersion && entryPoint != PigEntryPoint_GetStartupOptions && entryPoint != PigEntryPoint_Initialize)
	{
		Assert(pig->initialized);
	}
	if (pigIn != nullptr)
	{
		ProgramTime = pigIn->programTime;
		TimeScale = pigIn->elapsedMs / PIG_DEFAULT_FRAME_TIME;
		ElapsedMs = pigIn->elapsedMs;
		UnixTimestamp = pigIn->unixTime.timestamp;
		LocalTimestamp = pigIn->localTime.timestamp;
	}
}

void PigExitPoint(PigEntryPoint_t entryPoint)
{
	Assert_(entryPoint == pigEntryPoint);
	if (entryPoint != PigEntryPoint_Initialize && TempArena != nullptr)
	{
		TempPopMark();
		u64 numMarks = GetNumMarks(TempArena);
		Assert(numMarks == 0);
	}
	pigEntryPoint = PigEntryPoint_None;
	pigIn    = nullptr;
	pigOut   = nullptr;
	//NOTE: We don't clear these because the task threads and audio thread actually want access to them.
	//      This is fine because the platform layer will make sure those aren't running in the DLL during a DLL reload
	// PigClearGlobals();
	// platInfo = nullptr;
	// plat     = nullptr;
}
