/*
File:   win32_dll_loading.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds functions that help us load up the Engine DLL and find function exports and such
*/

#if DEMO_BUILD
#define ENGINE_DLL_NAME            PROJECT_NAME_SAFE "_Demo.dll"
#define ENGINE_DLL_TEMP_NAME       PROJECT_NAME_SAFE "_Demo_TEMP.dll"
#define ENGINE_DLL_TEMP_FORMAT_STR PROJECT_NAME_SAFE "_Demo_TEMP_Reload%llu.dll"
#else
#define ENGINE_DLL_NAME            PROJECT_NAME_SAFE ".dll"
#define ENGINE_DLL_TEMP_NAME       PROJECT_NAME_SAFE "_TEMP.dll"
#define ENGINE_DLL_TEMP_FORMAT_STR PROJECT_NAME_SAFE "_TEMP__Reload%llu.dll"
#endif

#define ENGINE_DLL_RELOAD_WAIT_TIME  400 //ms

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_DllLoadingInit()
{
	Platform->engineDllPath     = CombineStrs(&Platform->mainHeap, Platform->exeDirectory, NewStr(ENGINE_DLL_NAME));
	Platform->engineDllTempPath = CombineStrs(&Platform->mainHeap, Platform->exeDirectory, NewStr(ENGINE_DLL_TEMP_NAME));
	Platform->engineDllTempFormatStr = CombineStrs(&Platform->mainHeap, Platform->exeDirectory, NewStr(ENGINE_DLL_TEMP_FORMAT_STR));
}

// +--------------------------------------------------------------+
// |                             Load                             |
// +--------------------------------------------------------------+
bool Win32_LoadEngineDll(MyStr_t filePath, MyStr_t tempPath, EngineDll_t* dllOut)
{
	NotEmptyStr(&filePath);
	AssertNullTerm(&filePath);
	NotEmptyStr(&tempPath);
	AssertNullTerm(&tempPath);
	NotNull(dllOut);
	
	//TODO: Check if the file exists first
	
	ClearPointer(dllOut);
	// dllOut->lastWriteTime = Win32_GetFileWriteTime(filePath); //TODO: Re-enable me!
	
	if (DEBUG_BUILD)
	{
		u32 copyTries = 0;
		while (CopyFileA(filePath.pntr, tempPath.pntr, false) == 0) //failIfExists: false
		{
			DWORD error = GetLastError();
			if (error != ERROR_SHARING_VIOLATION)
			{
				PrintLine_E("CopyFileA error: %u", GetLastError());
				copyTries++;
				
				if (copyTries >= 100) 
				{
					PrintLine_E("Could not copy DLL from \"%s\" to \"%s\".", filePath.pntr, tempPath.pntr);
					return false;
				}
			}
		}
		//TODO: Check to make sure the new temp dll exists
		// PrintLine_E("Tried to copy %u times", copyTries);
		dllOut->handle = LoadLibraryA(tempPath.pntr);
	}
	else
	{
		dllOut->handle = LoadLibraryA(filePath.pntr);
	}
	
	if (dllOut->handle == nullptr)
	{
		DWORD loadError = GetLastError();
		PrintLine_E("Failed to load the engine DLL. Error: %u", loadError);
		return false;
	}
	
	dllOut->GetVersion        = (PigGetVersion_f*)        GetProcAddress(dllOut->handle, "Pig_GetVersion");
	dllOut->GetStartupOptions = (PigGetStartupOptions_f*) GetProcAddress(dllOut->handle, "Pig_GetStartupOptions");
	dllOut->Initialize        = (PigInitialize_f*)        GetProcAddress(dllOut->handle, "Pig_Initialize");
	dllOut->Update            = (PigUpdate_f*)            GetProcAddress(dllOut->handle, "Pig_Update");
	dllOut->AudioService      = (PigAudioService_f*)      GetProcAddress(dllOut->handle, "Pig_AudioService");
	dllOut->ShouldWindowClose = (PigShouldWindowClose_f*) GetProcAddress(dllOut->handle, "Pig_ShouldWindowClose");
	dllOut->Closing           = (PigClosing_f*)           GetProcAddress(dllOut->handle, "Pig_Closing");
	dllOut->PreReload         = (PigPreReload_f*)         GetProcAddress(dllOut->handle, "Pig_PreReload");
	dllOut->PostReload        = (PigPostReload_f*)        GetProcAddress(dllOut->handle, "Pig_PostReload");
	dllOut->PerformTask       = (PigPerformTask_f*)       GetProcAddress(dllOut->handle, "Pig_PerformTask");
	
	dllOut->isValid = (
		dllOut->GetVersion        != nullptr &&
		dllOut->GetStartupOptions != nullptr &&
		dllOut->Initialize        != nullptr &&
		dllOut->Update            != nullptr &&
		dllOut->AudioService      != nullptr &&
		dllOut->ShouldWindowClose != nullptr &&
		dllOut->Closing           != nullptr &&
		dllOut->PreReload         != nullptr &&
		dllOut->PostReload        != nullptr &&
		dllOut->PerformTask       != nullptr
	);
	
	if (dllOut->isValid)
	{
		dllOut->version = dllOut->GetVersion();
	}
	else
	{
		BOOL freeResult = FreeLibrary(dllOut->handle);
		AssertMsg(freeResult, "Failed to free engine DLL after unsuccessful load");
		dllOut->handle = NULL;
	}
	return dllOut->isValid;
}

// +--------------------------------------------------------------+
// |                            Watch                             |
// +--------------------------------------------------------------+
void Win32_WatchEngineDll(MyStr_t filePath, MyStr_t tempPath, EngineDll_t* engine)
{
	NotEmptyStr(&filePath);
	AssertNullTerm(&filePath);
	NotEmptyStr(&tempPath);
	AssertNullTerm(&tempPath);
	NotNull(engine);
	
	engine->watch = Win32_WatchFile(filePath);
	NotNull(engine->watch);
}

bool Win32_CheckForEngineDllChange(EngineDll_t* engine)
{
	NotNull(engine);
	if (engine->watch == nullptr) { return false; }
	u32 watchChanged = Win32_InterlockedExchange(&engine->watch->changed, 0);
	if (watchChanged != 0)
	{
		engine->lastChangeProgramTime = Win32_GetProgramTime(nullptr, true);
		engine->needToReload = true;
	}
	if (engine->needToReload && Platform->engineInput.programTime >= engine->lastChangeProgramTime + ENGINE_DLL_RELOAD_WAIT_TIME)
	{
		return true;
	}
	return false;
}

void Win32_ReloadEngineDll(MyStr_t filePath, MyStr_t tempFormatStr, EngineDll_t* engine)
{
	NotNull(engine);
	
	EngineDll_t newDll = {};
	
	TempPushMark();
	MyStr_t tempPath = PrintInArenaStr(GetTempArena(), tempFormatStr.pntr, Platform->engineReloadCount+1);
	NotNullStr(&tempPath);
	Platform->engineReloadCount++;
	
	if (!Win32_LoadEngineDll(filePath, tempPath, &newDll))
	{
		PrintLine_E("Failed to load the new engine DLL from \"%s\" copied to \"%s\"", filePath.pntr, tempPath.pntr);
		TempPopMark();
		return;
	}
	
	PrintLine_N("Calling PreReload on v%u.%02u(%03u)...", engine->version.major, engine->version.minor, engine->version.build);
	engine->PreReload(&Platform->info, &Platform->api, &Platform->engineMemory, newDll.version);
	
	PrintLine_N("Releasing old Engine DLL v%u.%02u(%03u)...", engine->version.major, engine->version.minor, engine->version.build);
	Version_t oldVersion = engine->version;
	newDll.watch = engine->watch;
	if (newDll.watch != nullptr) { Win32_InterlockedExchange(&newDll.watch->changed, 0); }
	newDll.needToReload = false;
	BOOL freeResult = FreeLibrary(engine->handle);
	AssertMsg(freeResult, "Failed to free old engine DLL after successful reload");
	MyMemCopy(engine, &newDll, sizeof(EngineDll_t));
	
	u64 unixTime = Win32_GetCurrentTimestamp(false);
	u64 localTime = Win32_GetCurrentTimestamp(true);
	
	PrintLine_N("Calling PostReload on v%u.%02u(%03u)...", engine->version.major, engine->version.minor, engine->version.build);
	engine->PostReload(&Platform->info, &Platform->api, &Platform->engineMemory, oldVersion, Win32_GetProgramTime(nullptr, false), unixTime, localTime);
	
	TempPopMark();
}
