/*
File:   win32_file_watching.cpp
Author: Taylor Robbins
Date:   09\27\2021
Description: 
	** Holds functions that help us track file changes and send out notifications of various sorts
*/

THREAD_FUNCTION_DEF(Win32_FileWatchingThreadFunc, userPntr);

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void Win32_InitFileWatching()
{
	FileWatchingContext_t* context = &Platform->fileWatchingContext;
	CreateLinkedList(&context->watchedFiles, &Platform->mainHeap, PlatWatchedFile_t);
	Win32_CreateMutex(&context->watchedFilesMutex);
	Platform->fileWatchingThread = Win32_CreateThread(Win32_FileWatchingThreadFunc, context);
}

FILETIME Win32_GetFileWriteTime(MyStr_t filePath)
{
	NotEmptyStr(&filePath);
	AssertNullTerm(&filePath);
	FILETIME lastWriteTime = {};
	WIN32_FILE_ATTRIBUTE_DATA attData;
	if (GetFileAttributesExA(filePath.pntr, GetFileExInfoStandard, &attData))
	{
		lastWriteTime = attData.ftLastWriteTime;
	}
	return lastWriteTime;
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Win32_UpdateFileWatching()
{
	//TODO: Put this on another thread?
}

// +--------------------------------------------------------------+
// |                        API Functions                         |
// +--------------------------------------------------------------+
// +==============================+
// |     Win32_IsFileWatched      |
// +==============================+
// bool IsFileWatched(MyStr_t filePath, PlatWatchedFile_t** watchedFileOut)
PLAT_API_IS_FILE_WATCHED_DEF(Win32_IsFileWatched)
{
	NotEmptyStr(&filePath);
	FileWatchingContext_t* context = &Platform->fileWatchingContext;
	
	MyStr_t fullFilePath = Win32_GetFullPath(GetTempArena(), filePath, true);
	
	PlatWatchedFile_t* filePntr = LinkedListFirst(&context->watchedFiles, PlatWatchedFile_t);
	for (u64 fIndex = 0; fIndex < context->watchedFiles.count; fIndex++)
	{
		NotNull(filePntr);
		if (StrCompareIgnoreCase(filePntr->fullPath, fullFilePath) == 0)
		{
			if (watchedFileOut != nullptr) { *watchedFileOut = filePntr; }
			return true;
		}
		filePntr = LinkedListNext(&context->watchedFiles, PlatWatchedFile_t, filePntr);
	}
	return false;
}

// +==============================+
// |       Win32_WatchFile        |
// +==============================+
// PlatWatchedFile_t* WatchFile(MyStr_t filePath)
PLAT_API_WATCH_FILE_DEF(Win32_WatchFile)
{
	NotEmptyStr(&filePath);
	AssertNullTerm(&filePath);
	FileWatchingContext_t* context = &Platform->fileWatchingContext;
	
	PlatWatchedFile_t* existingWatch = nullptr;
	if (Win32_IsFileWatched(filePath, &existingWatch)) { return existingWatch; }
	
	MyStr_t fullFilePath = Win32_GetFullPath(GetTempArena(), filePath, true);
	if (!Win32_DoesFileExist(fullFilePath, nullptr)) { return nullptr; }
	
	Win32_LockMutex(&context->watchedFilesMutex, MUTEX_LOCK_INFINITE);
	PlatWatchedFile_t* result = LinkedListAdd(&context->watchedFiles, PlatWatchedFile_t);
	NotNull(result);
	ClearPointer(result);
	result->path = AllocString(&Platform->mainHeap, &filePath);
	result->fullPath = AllocString(&Platform->mainHeap, &fullFilePath);
	result->lastWriteTime = Win32_GetFileWriteTime(result->path);
	Win32_CreateInterlockedInt(&result->changed, 0);
	Win32_UnlockMutex(&context->watchedFilesMutex);
	
	DebugAssert(Win32_IsFileWatched(filePath, nullptr));
	return result;
}

// +==============================+
// |      Win32_UnwatchFile       |
// +==============================+
// bool UnwatchFile(const PlatWatchedFile_t* watchedFile)
PLAT_API_UNWATCH_FILE_DEF(Win32_UnwatchFile)
{
	NotNull(watchedFile);
	NotEmptyStr(&watchedFile->path);
	PlatWatchedFile_t* nonConstFile = (PlatWatchedFile_t*)watchedFile;
	FileWatchingContext_t* context = &Platform->fileWatchingContext;
	
	Win32_LockMutex(&context->watchedFilesMutex, MUTEX_LOCK_INFINITE);
	u64 listIndex = 0;
	bool foundInList = IsItemInLinkedList(&context->watchedFiles, watchedFile, &listIndex);
	if (!foundInList)
	{
		return false;
	}
	
	FreeString(&Platform->mainHeap, &nonConstFile->path);
	FreeString(&Platform->mainHeap, &nonConstFile->fullPath);
	LinkedListRemove(&context->watchedFiles, PlatWatchedFile_t, nonConstFile);
	Win32_UnlockMutex(&context->watchedFilesMutex);
	
	return true;
}

// +--------------------------------------------------------------+
// |                   Watching Thread Function                   |
// +--------------------------------------------------------------+
// +==============================+
// | Win32_FileWatchingThreadFunc |
// +==============================+
THREAD_FUNCTION_DEF(Win32_FileWatchingThreadFunc, userPntr) //pre-declared at top of file
{
	NotNull_(userPntr);
	FileWatchingContext_t* context = (FileWatchingContext_t*)userPntr;
	while (true)
	{
		Win32_LockMutex(&context->watchedFilesMutex, MUTEX_LOCK_INFINITE);
		PlatWatchedFile_t* filePntr = LinkedListFirst(&context->watchedFiles, PlatWatchedFile_t);
		for (u64 fIndex = 0; fIndex < context->watchedFiles.count; fIndex++)
		{
			NotNull(filePntr);
			//TODO: Do we need to check for the file getting deleted and count that as a change?
			//TODO: Do we need to check for the file being re-created after a delete?
			FILETIME newWriteTime = Win32_GetFileWriteTime(filePntr->fullPath);
			if (CompareFileTime(&newWriteTime, &filePntr->lastWriteTime) != 0)
			{
				filePntr->lastWriteTimeChange = Win32_GetProgramTime(nullptr);
				filePntr->lastWriteTime = newWriteTime;
			}
			if (filePntr->lastWriteTimeChange != 0 && Win32_TimeSince(filePntr->lastWriteTimeChange) >= 1000)
			{
				u32 oldChangedValue = Win32_InterlockedExchange(&filePntr->changed, 1);
				UNUSED(oldChangedValue);
				filePntr->lastWriteTimeChange = 0;
			}
			filePntr = LinkedListNext(&context->watchedFiles, PlatWatchedFile_t, filePntr);
		}
		Win32_UnlockMutex(&context->watchedFilesMutex);
		
		Win32_SleepForMs(FILE_WATCHING_SLEEP_TIME);
	}
}
