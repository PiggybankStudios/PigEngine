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
	info->stdHeap = &Platform->stdHeapRedirect;
	
	info->defaultAudioDeviceIndex = Platform->defaultAudioDeviceIndex;
	MyMemCopy(&info->audioDevices, &Platform->audioDevices, sizeof(VarArray_t));
	
	info->monitors = &Platform->monitors;
	
	info->GetScratchArena      = Win32_GetScratchArena;
	info->FreeScratchArena     = Win32_FreeScratchArena;
	info->ShowMessageBox       = Win32_ShowMessageBox;
	info->HandleAssertion      = Win32_HandleAssertion;
	info->DebugOutput          = Win32_DebugOutput;
	info->GetProgramArg        = Win32_GetProgramArg;
	info->GetThisThreadId      = Win32_GetThisThreadId;
	info->DoesFileExist        = Win32_DoesFileExist;
	info->ReadFileContents     = Win32_ReadFileContents;
	info->FreeFileContents     = Win32_FreeFileContents;
	info->GetSpecialFolderPath = Win32_GetSpecialFolderPath;
	info->GetMonitorVideoMode  = Win32_GetMonitorVideoMode;
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
		memory->tempDataSize = tempSize;
		memory->tempDataPntr = VirtualAlloc(0, tempSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if (memory->tempDataPntr == nullptr)
		{
			PrintLine_E("Failed to allocate %llu bytes for temporary engine memory", tempSize);
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
	info->stdHeap = &Platform->stdHeapRedirect;
	info->workingDirectory = StrReplace(Platform->workingDirectory, "\\", "/", &Platform->mainHeap);
	info->exeDirectory = StrReplace(Platform->exeDirectory, "\\", "/", &Platform->mainHeap);
	info->defaultDirectory = StrReplace(Platform->defaultDirectory, "\\", "/", &Platform->mainHeap);
	info->exeFileName = AllocString(&Platform->mainHeap, &Platform->exeFileName);
	info->renderApi = Platform->renderApi;
	if (Platform->renderApi == RenderApi_OpenGL)
	{
		#if OPENGL_SUPPORTED
		info->opengl.version.major = (u32)GLVersion.major;
		info->opengl.version.minor = (u32)GLVersion.minor;
		info->opengl.version.build = 0; //Unused in OpenGL as far as I can tell
		GLint numExtensions = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
		Win32_AssertNoOpenGlError("glGetIntegerv(GL_NUM_EXTENSIONS)");
		PrintLine_I("There are %d supported OpenGL extensions", numExtensions);
		for (u32 sIndex = 0; sIndex < (u32)numExtensions; sIndex++)
		{
			const GLubyte* extensionStr = glGetStringi(GL_EXTENSIONS, sIndex);
			OpenGlExtension_t extension = OpenGlExtension_NumExtensions;
			if (TryParseEnum(NewStr((const char*)extensionStr), &extension, OpenGlExtension_NumExtensions, GetOpenGlExtensionStr))
			{
				// PrintLine_I("OpenGlExtension_%s is supported", GetOpenGlExtensionStr(extension));
				info->opengl.extensionSupported[extension] = true;
			}
			// else { PrintLine_E("Unknown extension name \"%s\"", extensionStr); }
		}
		#endif
	}
	else { AssertMsg(false, "Unimplemented RenderApi in Win32_FillPlatformInfo"); }
	info->audioFormat = Platform->audioFormat;
	info->programStartPerfTime = programStartPerfTime;
	info->mainWindow = Platform->mainWindow;
	info->windows = &Platform->windows;
	info->monitors = &Platform->monitors;
	
	info->mainThreadId.internalId = 0;
	info->mainThreadId.osId = MainThreadId;
	info->fileWatchingThreadId.internalId = ((Platform->fileWatchingThread != nullptr) ? Platform->fileWatchingThread->id : 0);
	info->fileWatchingThreadId.osId = ((Platform->fileWatchingThread != nullptr) ? (ThreadId_t)Platform->fileWatchingThread->win32_id : 0);
	info->audioThreadId.internalId = ((Platform->audioThread != nullptr) ? Platform->audioThread->id : 0);
	info->audioThreadId.osId = ((Platform->audioThread != nullptr) ? (ThreadId_t)Platform->audioThread->win32_id : 0);
	u64 filledThreadIndex = 0;
	for (u64 tIndex = 0; tIndex < PLAT_MAX_THREADPOOL_SIZE; tIndex++)
	{
		if (Platform->threadPool[tIndex].threadPntr != nullptr)
		{
			info->threadPoolIds[filledThreadIndex].internalId = Platform->threadPool[tIndex].threadPntr->id;
			info->threadPoolIds[filledThreadIndex].osId = (ThreadId_t)Platform->threadPool[tIndex].threadPntr->win32_id;
			filledThreadIndex++;
		}
	}
	info->numThreadPoolThreads = filledThreadIndex;
	
	#if STEAM_BUILD
	info->steamAppId = Platform->steamAppId;
	info->steamUserId = Platform->steamUserId;
	info->steamFriendsList = &Platform->steamFriendsList;
	#endif
}

void Win32_FillPlatformApi(PlatformApi_t* api)
{
	NotNull(api);
	ClearPointer(api);
	api->GetScratchArena           = Win32_GetScratchArena;
	api->FreeScratchArena          = Win32_FreeScratchArena;
	api->ShowMessageBox            = Win32_ShowMessageBox;
	api->HandleAssertion           = Win32_HandleAssertion;
	api->DebugOutput               = Win32_DebugOutput;
	api->GetProgramArg             = Win32_GetProgramArg;
	api->CreateMutex               = Win32_CreateMutex;
	api->DestroyMutex              = Win32_DestroyMutex;
	api->LockMutex                 = Win32_LockMutex;
	api->UnlockMutex               = Win32_UnlockMutex;
	api->CreateSemaphore           = Win32_CreateSemaphore;
	api->DestroySemaphore          = Win32_DestroySemaphore;
	api->WaitOnSemaphore           = Win32_WaitOnSemaphore;
	api->TriggerSemaphore          = Win32_TriggerSemaphore;
	api->CreateInterlockedInt      = Win32_CreateInterlockedInt;
	api->InterlockedExchange       = Win32_InterlockedExchange;
	api->DestroyInterlockedInt     = Win32_DestroyInterlockedInt;
	api->GetThisThreadId           = Win32_GetThisThreadId;
	api->GetThreadContext          = Win32_GetThreadContext;
	api->SleepForMs                = Win32_SleepForMs;
	api->QueueTask                 = Win32_QueueTask;
	api->AllocateMemory            = Win32_AllocateMemory;
	api->ReallocMemory             = Win32_ReallocMemory;
	api->FreeMemory                = Win32_FreeMemory;
	api->GetRapidClickMaxTime      = Win32_GetRapidClickMaxTime;
	api->ChangeWindowTarget        = Win32_ChangeWindowTarget;
	api->SwapBuffers               = Win32_SwapBuffers;
	api->GetNativeWindowPntr       = Win32_GetNativeWindowPntr;
	api->GetFullPath               = Win32_GetFullPath;
	api->DoesFileExist             = Win32_DoesFileExist;
	api->StartEnumeratingFiles     = Win32_StartEnumeratingFiles;
	api->EnumerateFiles            = Win32_EnumerateFiles;
	api->CreateFolder              = Win32_CreateFolder;
	api->ReadFileContents          = Win32_ReadFileContents;
	api->FreeFileContents          = Win32_FreeFileContents;
	api->WriteEntireFile           = Win32_WriteEntireFile;
	api->OpenFile                  = Win32_OpenFile;
	api->WriteToFile               = Win32_WriteToFile;
	api->MoveFileCursor            = Win32_MoveFileCursor;
	api->SeekToOffsetInFile        = Win32_SeekToOffsetInFile;
	api->ReadFromFile              = Win32_ReadFromFile;
	api->CloseFile                 = Win32_CloseFile;
	api->ReadFileContentsStream    = Win32_ReadFileContentsStream;
	api->OpenFileStream            = Win32_OpenFileStream;
	api->TryParseImageFile         = Win32_TryParseImageFile;
	api->SaveImageDataToFile       = Win32_SaveImageDataToFile;
	api->FreeImageData             = Win32_FreeImageData;
	api->ShowSourceFile            = Win32_ShowSourceFile;
	api->ShowFile                  = Win32_ShowFile;
	api->GetSpecialFolderPath      = Win32_GetSpecialFolderPath;
	api->IsFileWatched             = Win32_IsFileWatched;
	api->WatchFile                 = Win32_WatchFile;
	api->UnwatchFile               = Win32_UnwatchFile;
	api->GetFileIconId             = Win32_GetFileIconId;
	api->GetFileIconImageData      = Win32_GetFileIconImageData;
	#ifndef WIN32_GFX_TEST
	api->GetLoadProcAddressFunc    = Win32_GetLoadProcAddressFunc;
	api->RenderLoadingScreen       = Win32_RenderLoadingScreen;
	#endif
	api->FreeFontData              = Win32_FreeFontData;
	api->ReadPlatformFont          = Win32_ReadPlatformFont;
	api->BakeFont                  = Win32_BakeFont;
	#ifndef WIN32_GFX_TEST
	api->DebugReadout              = Win32_DebugReadout;
	#endif
	api->CopyTextToClipboard       = Win32_CopyTextToClipboard;
	api->PasteTextFromClipboard    = Win32_PasteTextFromClipboard;
	api->GetProgramTime            = Win32_GetProgramTime;
	api->GetMonitorVideoMode       = Win32_GetMonitorVideoMode;
	#if STEAM_BUILD
	api->GetSteamFriendInfoById    = Win32_GetSteamFriendInfoById;
	api->GetSteamFriendGroupById   = Win32_GetSteamFriendGroupById;
	api->GetSteamFriendPresenceStr = Win32_GetSteamFriendPresenceStr;
	api->StartSteamFriendsQuery    = Win32_StartSteamFriendsQuery;
	api->UpdateSteamFriendStatus   = Win32_UpdateSteamFriendStatus;
	api->RequestSteamFriendAvatar  = Win32_RequestSteamFriendAvatar;
	#endif
	api->CheckRunningProcess       = Win32_CheckRunningProcess;
	api->StartProcess              = Win32_StartProcess;
	api->CloseRunningProcess       = Win32_CloseRunningProcess;
	#if BOX2D_SUPPORTED
	api->InitPhysicsEngine         = Win32_InitPhysicsEngine;
	api->DestroyPhysicsEngine      = Win32_DestroyPhysicsEngine;
	api->PhysicsTick               = Win32_PhysicsTick;
	api->CreatePhysicsBody         = Win32_CreatePhysicsBody;
	api->DestroyPhysicsBody        = Win32_DestroyPhysicsBody;
	api->SetPhysicsBodyVelocity    = Win32_SetPhysicsBodyVelocity;
	api->GetPhysicsBodyState       = Win32_GetPhysicsBodyState;
	#endif
}
