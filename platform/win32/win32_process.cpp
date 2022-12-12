/*
File:   win32_process.cpp
Author: Taylor Robbins
Date:   11\08\2022
Description: 
	** Holds functions that allow us to ask questions about running processes on the computer
*/

void Win32_FreePlatRunningProcess(PlatRunningProcess_t* process)
{
	NotNull(process);
	if (process->filePath.pntr != nullptr)
	{
		NotNull(process->allocArena);
		FreeString(process->allocArena, &process->filePath);
		FreeString(process->allocArena, &process->workingDirectory);
		FreeString(process->allocArena, &process->argumentsString);
	}
	ClearPointer(process);
}

void Win32_InitProcessManagement()
{
	Win32_CreateMutex(&Platform->runningProcessesMutex);
	Platform->nextRunningProcessId = 1;
	CreateBktArray(&Platform->runningProcesses, &Platform->threadSafeHeap, sizeof(PlatRunningProcess_t));
}

void Win32_UpdateRunningProcesses()
{
	for (u64 pIndex = 0; pIndex < Platform->runningProcesses.length; pIndex++)
	{
		PlatRunningProcess_t* process = BktArrayGet(&Platform->runningProcesses, PlatRunningProcess_t, pIndex);
		if (process->readyForRemoval)
		{
			//TODO: Determine if the process can be removed from the bkt array without moving any active pointers
		}
	}
}

// +==============================+
// |      Win32_StartProcess      |
// +==============================+
// PlatRunningProcess_t* StartProcess(MyStr_t filePath, MyStr_t argumentsString, MyStr_t workingDirectory)
PLAT_API_START_PROCESS_DEFINITION(Win32_StartProcess)
{
	NotEmptyStr(&filePath);
	NotNullStr(&filePath);
	NotNullStr(&workingDirectory);
	NotNullStr(&argumentsString);
	Assert(argumentsString.length <= 32767); //limitation imposed by CreateProcessA
	
	Win32_LockMutex(&Platform->runningProcessesMutex, MUTEX_LOCK_INFINITE);
	
	MyStr_t fullFilePath = Win32_GetFullPath(GetTempArena(), filePath, true);
	MyStr_t actualWorkingDirectory = workingDirectory;
	if (IsEmptyStr(workingDirectory))
	{
		actualWorkingDirectory = GetDirectoryPart(fullFilePath);
		if (actualWorkingDirectory.length > 0 && actualWorkingDirectory.chars[actualWorkingDirectory.length-1] == '\\') { actualWorkingDirectory.length--; }
	}
	actualWorkingDirectory = AllocString(GetTempArena(), &actualWorkingDirectory);
	
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.lpReserved      = NULL;
	startupInfo.lpDesktop       = ""; // LPSTR
	startupInfo.lpTitle         = NULL;
	startupInfo.dwX             = 0; // STARTF_USEPOSITION
	startupInfo.dwY             = 0; // STARTF_USEPOSITION
	startupInfo.dwXSize         = 0; // STARTF_USESIZE
	startupInfo.dwYSize         = 0; // STARTF_USESIZE
	startupInfo.dwXCountChars   = 0; // STARTF_USECOUNTCHARS
	startupInfo.dwYCountChars   = 0; // STARTF_USECOUNTCHARS
	startupInfo.dwFillAttribute = 0; // STARTF_USEFILLATTRIBUTE
	startupInfo.dwFlags         = 0;
	startupInfo.wShowWindow     = 0; // STARTF_USESHOWWINDOW
	startupInfo.cbReserved2     = 0;
	startupInfo.lpReserved2     = NULL;
	startupInfo.hStdInput       = 0; // STARTF_USESTDHANDLES
	startupInfo.hStdOutput      = 0; // STARTF_USESTDHANDLES
	startupInfo.hStdError       = 0; // STARTF_USESTDHANDLES
	
	PROCESS_INFORMATION processInfo = {};
	
	BOOL createProcessSuccess = CreateProcessA(
		fullFilePath.pntr, // lpApplicationName
		(argumentsString.pntr != nullptr) ? argumentsString.pntr : "", // lpCommandLine 
		NULL, // lpProcessAttributes
		NULL, // lpThreadAttributes
		false, // bInheritHandles
		0, // dwCreationFlags
		NULL, // lpEnvironment
		actualWorkingDirectory.pntr, // lpCurrentDirectory
		&startupInfo, // lpStartupInfo
		&processInfo // lpProcessInformation
	);
	if (createProcessSuccess == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("Failed to create process from path: \"%.*s\" in \"%.*s\": %s", fullFilePath.length, fullFilePath.pntr, actualWorkingDirectory.length, actualWorkingDirectory.pntr, Win32_GetErrorCodeStr(errorCode, true));
		Win32_UnlockMutex(&Platform->runningProcessesMutex);
		return nullptr;
	}
	
	Win32_LockMutex(&Platform->threadSafeHeapMutex, MUTEX_LOCK_INFINITE);
	PlatRunningProcess_t* newProcess = BktArrayAdd(&Platform->runningProcesses, PlatRunningProcess_t);
	NotNull(newProcess);
	ClearPointer(newProcess);
	newProcess->id = Platform->nextRunningProcessId;
	Platform->nextRunningProcessId++;
	newProcess->handle = processInfo.hProcess;
	newProcess->threadHandle = processInfo.hThread;
	newProcess->processId = processInfo.dwProcessId;
	newProcess->threadId = processInfo.dwThreadId;
	newProcess->allocArena = &Platform->threadSafeHeap;
	newProcess->filePath = AllocString(newProcess->allocArena, &filePath);
	newProcess->workingDirectory = AllocString(newProcess->allocArena, &actualWorkingDirectory);
	newProcess->argumentsString = AllocString(newProcess->allocArena, &argumentsString);
	newProcess->isFinished = false;
	Win32_UnlockMutex(&Platform->threadSafeHeapMutex);
	
	Win32_UnlockMutex(&Platform->runningProcessesMutex);
	return newProcess;
}

// +==============================+
// |  Win32_CheckRunningProcess   |
// +==============================+
// void CheckRunningProcess(PlatRunningProcess_t* runningProcess)
PLAT_API_CHECK_RUNNING_PROCESS_DEFINITION(Win32_CheckRunningProcess)
{
	if (runningProcess->isFinished) { return; }
	
	DWORD exitCode = 0;
	BOOL getExitCodeSuccess = GetExitCodeProcess(
		runningProcess->handle, //hProcess
		&exitCode
	);
	runningProcess->lastExitCodeCheckTime = Win32_GetProgramTime(nullptr, true);
	if (getExitCodeSuccess == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("Failed to get exitCode for running process \"%.*s\": %s", runningProcess->filePath.length, runningProcess->filePath.pntr, Win32_GetErrorCodeStr(errorCode));
		runningProcess->isFinished = true;
		runningProcess->exitCode = 0;
		return;
	}
	
	if (exitCode != STILL_ACTIVE)
	{
		runningProcess->isFinished = true;
		runningProcess->exitCode = (u64)exitCode;
		CloseHandle(runningProcess->threadHandle);
		CloseHandle(runningProcess->handle);
	}
}

// +==============================+
// |  Win32_CloseRunningProcess   |
// +==============================+
// void CloseRunningProcess(PlatRunningProcess_t* runningProcess)
PLAT_API_CLOSE_RUNNING_PROCESS_DEFINITION(Win32_CloseRunningProcess)
{
	NotNull(runningProcess);
	
	if (!runningProcess->isFinished)
	{
		Unimplemented(); //TODO: Implement me!
	}
	
	runningProcess->readyForRemoval = true;
}
