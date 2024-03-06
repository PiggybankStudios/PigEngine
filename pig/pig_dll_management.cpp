/*
File:   pig_dll_management.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds a bunch of functions that help us deal with linking up globals, updating function pointers,
	** and various other consequences of being a hot-loaded DLL 
*/

void PigClearGlobals()
{
	pig = nullptr;
	mainHeap = nullptr;
	platHeap = nullptr;
	logGlobals = nullptr;
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

// +--------------------------------------------------------------+
// |               Custom Scratch Arenas Interface                |
// +--------------------------------------------------------------+
inline MemArena_t* GetScratchArena(MemArena_t* avoidConflictWith1, MemArena_t* avoidConflictWith2) //pre-declared in gy_scratch_arenas.h
{
	DebugAssert(plat != nullptr || startup != nullptr);
	if (startup != nullptr)
	{
		DebugAssert(startup->GetScratchArena != nullptr);
		return startup->GetScratchArena(avoidConflictWith1, avoidConflictWith2);
	}
	else
	{
		DebugAssert(plat->GetScratchArena != nullptr);
		return plat->GetScratchArena(avoidConflictWith1, avoidConflictWith2);
	}
}
inline void FreeScratchArena(MemArena_t* scratchArena) //pre-declared in gy_scratch_arenas.h
{
	DebugAssert(plat != nullptr || startup != nullptr);
	if (startup != nullptr)
	{
		DebugAssert(startup->FreeScratchArena != nullptr);
		startup->FreeScratchArena(scratchArena);
	}
	else
	{
		DebugAssert(plat->FreeScratchArena != nullptr);
		plat->FreeScratchArena(scratchArena);
	}
}

u64 LogGlobals_GetPreciseProgramTime()
{
	return ((plat != nullptr) ? plat->GetProgramTime(nullptr, true) : 0);
}
u64 LogGlobals_GetThreadNumber()
{
	return (u64)((plat != nullptr) ? plat->GetThisThreadId() : 0);
}

void PigEntryPoint(PigEntryPoint_t entryPoint, const StartupInfo_t* startupInfo, const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, EngineInput_t* input, EngineOutput_t* output)
{
	startup = startupInfo;
	pigEntryPoint = entryPoint;
	platInfo = info;
	plat = api;
	pigIn = input;
	pigOut = output;
	#if GAME_USES_FUNC_TABLE
	if (entryPoint == PigEntryPoint_Initialize) { FillGlobalFuncTable(); }
	#endif
	if (memory != nullptr)
	{
		NotNull_(api);
		NotNull_(memory->persistentDataPntr);
		u64 pigStateSize = sizeof(PigState_t);
		Assert_(memory->persistentDataSize >= pigStateSize);
		UNUSED(pigStateSize);
		pig = (PigState_t*)memory->persistentDataPntr;
		fixedHeap = &pig->fixedHeap;
		mainHeap = &pig->mainHeap;
		platHeap = &pig->platHeap;
		rc = &pig->renderContext;
		//TODO: Update the gylib debug output pointers
		TempArena = &pig->tempArena;
		GetTempArena = Pig_GetTempArena;
		if (entryPoint != PigEntryPoint_Initialize) { TempPushMark(); }
		logGlobals = &pig->logGlobals;
		pig->logGlobals.getPreciseProgramTime = LogGlobals_GetPreciseProgramTime;
		pig->logGlobals.getThreadNumber = LogGlobals_GetThreadNumber;
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
		ElapsedMs = MinR64(pigIn->elapsedMs, PIG_MAX_ELAPSED_MS);
		TimeScale = ElapsedMs / PIG_DEFAULT_FRAME_TIME;
		UncappedElapsedMs = pigIn->elapsedMs;
		UnixTimestamp = pigIn->unixTime.timestamp;
		LocalTimestamp = pigIn->localTime.timestamp;
		pig->logGlobals.timestamp = LocalTimestamp;
		pig->logGlobals.programTime = ProgramTime;
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
	startup  = nullptr;
	//NOTE: We don't clear these because the task threads and audio thread actually want access to them.
	//      This is fine because the platform layer will make sure those aren't running in the DLL during a DLL reload
	// PigClearGlobals();
	// platInfo = nullptr;
	// plat     = nullptr;
}
