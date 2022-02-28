/*
File:   win32_interface_filling.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds functions that help us take information that we have in the platform layer and fill it into
	** structures that the engine DLL will understand (defined in common_interface_types.h)
*/

void Win32_FillStartupInfo(StartupInfo_t* info)
{
	NotNull(info);
	ClearPointer(info); //TODO: Should we deallocate anything?
	info->type = PlatType_Windows;
	info->version.major = WIN32_VERSION_MAJOR;
	info->version.minor = WIN32_VERSION_MINOR;
	info->version.build = WIN32_VERSION_BUILD;
	info->glfwVersion = Platform->glfwVersion;
	info->mainThreadId = MainThreadId;
	info->platTempArena = &Platform->tempArena;
	
	info->defaultAudioDeviceIndex = Platform->defaultAudioDeviceIndex;
	MyMemCopy(&info->audioDevices, &Platform->audioDevices, sizeof(VarArray_t));
	
	info->ShowMessageBox      = Win32_ShowMessageBox;
	info->HandleAssertion     = Win32_HandleAssertion;
	info->DebugOutput         = Win32_DebugOutput;
	info->GetProgramArg       = Win32_GetProgramArg;
	info->GetThisThreadId     = Win32_GetThisThreadId;
	info->DoesFileExist       = Win32_DoesFileExist;
	info->ReadFileContents    = Win32_ReadFileContents;
	info->FreeFileContents    = Win32_FreeFileContents;
	info->GetMonitorVideoMode = Win32_GetMonitorVideoMode;
}

void Win32_FillEngineMemory(EngineMemory_t* memory, u64 mainSize, u64 tempSize)
{
	NotNull(memory);
	ClearPointer(memory);
	if (mainSize > 0)
	{
		memory->persistentDataSize = mainSize;
		memory->persistentDataPntr = VirtualAlloc(0, mainSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if (memory->persistentDataPntr == nullptr)
		{
			PrintLine_E("Failed to allocate %llu bytes for main engine memory", mainSize);
			Win32_InitError("Failed to allocate main memory block for engine. Does your system have enough RAM to run this application?");
		}
	}
	if (tempSize > 0)
	{
		memory->tempDataSize = mainSize;
		memory->tempDataPntr = VirtualAlloc(0, mainSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if (memory->tempDataPntr == nullptr)
		{
			PrintLine_E("Failed to allocate %llu bytes for temporary engine memory", mainSize);
			Win32_InitError("Failed to allocate temporary memory block for engine. Does your system have enough RAM to run this application?");
		}
	}
}

void Win32_FillPlatformInfo(PlatformInfo_t* info, PerfTime_t programStartPerfTime)
{
	NotNull(info);
	ClearPointer(info); //TODO: Should we deallocate anything?
	info->type = PlatType_Windows;
	info->version.major = WIN32_VERSION_MAJOR;
	info->version.minor = WIN32_VERSION_MINOR;
	info->version.build = WIN32_VERSION_BUILD;
	info->glfwVersion = Platform->glfwVersion;
	info->renderApi = Platform->renderApi;
	info->mainThreadId = MainThreadId;
	info->audioFormat = Platform->audioFormat;
	info->programStartPerfTime = programStartPerfTime;
	info->mainWindow = Platform->mainWindow;
	info->windows = &Platform->windows;
	info->monitors = &Platform->monitors;
}

void Win32_FillPlatformApi(PlatformApi_t* api)
{
	NotNull(api);
	ClearPointer(api);
	api->ShowMessageBox         = Win32_ShowMessageBox;
	api->HandleAssertion        = Win32_HandleAssertion;
	api->DebugOutput            = Win32_DebugOutput;
	api->GetProgramArg          = Win32_GetProgramArg;
	api->CreateMutex            = Win32_CreateMutex;
	api->DestroyMutex           = Win32_DestroyMutex;
	api->LockMutex              = Win32_LockMutex;
	api->UnlockMutex            = Win32_UnlockMutex;
	api->CreateSemaphore        = Win32_CreateSemaphore;
	api->DestroySemaphore       = Win32_DestroySemaphore;
	api->WaitOnSemaphore        = Win32_WaitOnSemaphore;
	api->TriggerSemaphore       = Win32_TriggerSemaphore;
	api->CreateInterlockedInt   = Win32_CreateInterlockedInt;
	api->InterlockedExchange    = Win32_InterlockedExchange;
	api->DestroyInterlockedInt  = Win32_DestroyInterlockedInt;
	api->GetThisThreadId        = Win32_GetThisThreadId;
	api->GetThreadContext       = Win32_GetThreadContext;
	api->SleepForMs             = Win32_SleepForMs;
	api->QueueTask              = Win32_QueueTask;
	api->AllocateMemory         = Win32_AllocateMemory;
	api->FreeMemory             = Win32_FreeMemory;
	api->ChangeWindowTarget     = Win32_ChangeWindowTarget;
	api->SwapBuffers            = Win32_SwapBuffers;
	api->GetFullPath            = Win32_GetFullPath;
	api->DoesFileExist          = Win32_DoesFileExist;
	api->ReadFileContents       = Win32_ReadFileContents;
	api->FreeFileContents       = Win32_FreeFileContents;
	api->WriteEntireFile        = Win32_WriteEntireFile;
	api->OpenFile               = Win32_OpenFile;
	api->WriteToFile            = Win32_WriteToFile;
	api->MoveFileCursor         = Win32_MoveFileCursor;
	api->SeekToOffsetInFile     = Win32_SeekToOffsetInFile;
	api->ReadFromFile           = Win32_ReadFromFile;
	api->CloseFile              = Win32_CloseFile;
	api->TryParseImageFile      = Win32_TryParseImageFile;
	api->SaveImageDataToFile    = Win32_SaveImageDataToFile;
	api->FreeImageData          = Win32_FreeImageData;
	api->ShowSourceFile         = Win32_ShowSourceFile;
	api->ShowFile               = Win32_ShowFile;
	api->IsFileWatched          = Win32_IsFileWatched;
	api->WatchFile              = Win32_WatchFile;
	api->UnwatchFile            = Win32_UnwatchFile;
	api->GetLoadProcAddressFunc = Win32_GetLoadProcAddressFunc;
	api->RenderLoadingScreen    = Win32_RenderLoadingScreen;
	api->FreeFontData           = Win32_FreeFontData;
	api->ReadPlatformFont       = Win32_ReadPlatformFont;
	api->BakeFont               = Win32_BakeFont;
	api->DebugReadout           = Win32_DebugReadout;
	api->CopyTextToClipboard    = Win32_CopyTextToClipboard;
	api->PasteTextFromClipboard = Win32_PasteTextFromClipboard;
	api->GetPerfTime            = Win32_GetPerfTime;
	api->GetPerfTimeDiff        = Win32_GetPerfTimeDiff;
	api->GetProgramTime         = Win32_GetProgramTime;
	api->GetMonitorVideoMode    = Win32_GetMonitorVideoMode;
}

void Win32_FillEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	output->cursorType = PlatCursor_Default;
}

void Win32_ClearEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	output->cursorType = PlatCursor_Default;
}

void Win32_ProcessEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	//TODO: Does this work for multiple windows? Do we need to set the cursor in each window?
	if (output->cursorType != Platform->currentCursorType)
	{
		Assert(output->cursorType < PlatCursor_NumCursors);
		glfwSetCursor(Platform->currentWindow->handle, Platform->glfwCursors[output->cursorType]);
		Platform->currentCursorType = output->cursorType;
	}
}
