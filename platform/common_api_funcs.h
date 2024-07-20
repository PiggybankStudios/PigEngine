/*
File:   common_api_funcs.h
Author: Taylor Robbins
Date:   09\23\2021
Description:
	** Contains a bunch of #defines and typedefs that set up the format for all API functions provided by the platform layer to the application in PlatformApi_t
*/

#ifndef _COMMON_API_FUNCS_H
#define _COMMON_API_FUNCS_H

#define PLAT_API_GET_SCRATCH_ARENA_DEFINITION(functionName) MemArena_t* functionName(MemArena_t* avoidConflictWith1, MemArena_t* avoidConflictWith2)
typedef PLAT_API_GET_SCRATCH_ARENA_DEFINITION(PlatApiGetScratchArena_f);

#define PLAT_API_FREE_SCRATCH_ARENA_DEFINITION(functionName) void functionName(MemArena_t* scratchArena)
typedef PLAT_API_FREE_SCRATCH_ARENA_DEFINITION(PlatApiFreeScratchArena_f);

#define PLAT_API_SHOW_MESSAGE_BOX_DEFINITION(functionName) void functionName(const char* title, const char* message)
typedef PLAT_API_SHOW_MESSAGE_BOX_DEFINITION(PlatApiShowMessageBox_f);

#define PLAT_API_HANDLE_ASSERTION_DEFINITION(functionName) void functionName(bool shouldExit, const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
typedef PLAT_API_HANDLE_ASSERTION_DEFINITION(PlatApiHandleAssertion_f);

#define PLAT_API_DEBUG_OUTPUT_DEF(functionName) void functionName(MyStr_t messageStr, bool newLine)
typedef PLAT_API_DEBUG_OUTPUT_DEF(PlatApiDebugOutput_f);

#define PLAT_API_GET_PROGRAM_ARG_DEF(functionName) bool functionName(MemArena_t* memArena, MyStr_t argName, MyStr_t* valueOut)
typedef PLAT_API_GET_PROGRAM_ARG_DEF(PlatApiGetProgramArg_f);

#define PLAT_API_CREATE_MUTEX_DEF(functionName) void functionName(PlatMutex_t* mutex)
typedef PLAT_API_CREATE_MUTEX_DEF(PlatApiCreateMutex_f);

#define PLAT_API_DESTROY_MUTEX_DEF(functionName) void functionName(PlatMutex_t* mutex)
typedef PLAT_API_DESTROY_MUTEX_DEF(PlatApiDestroyMutex_f);

#define PLAT_API_LOCK_MUTEX_DEF(functionName) bool functionName(PlatMutex_t* mutex, u64 timeoutMs)
typedef PLAT_API_LOCK_MUTEX_DEF(PlatApiLockMutex_f);

#define PLAT_API_UNLOCK_MUTEX_DEF(functionName) void functionName(PlatMutex_t* mutex)
typedef PLAT_API_UNLOCK_MUTEX_DEF(PlatApiUnlockMutex_f);

#define PLAT_API_CREATE_SEMAPHORE_DEF(functionName) void functionName(PlatSemaphore_t* semaphore, u64 startCount, u64 maxCount)
typedef PLAT_API_CREATE_SEMAPHORE_DEF(PlatApiCreateSemaphore_f);

#define PLAT_API_DESTROY_SEMAPHORE_DEF(functionName) void functionName(PlatSemaphore_t* semaphore)
typedef PLAT_API_DESTROY_SEMAPHORE_DEF(PlatApiDestroySemaphore_f);

#define PLAT_API_WAIT_ON_SEMAPHORE_DEF(functionName) bool functionName(PlatSemaphore_t* semaphore, u64 timeout)
typedef PLAT_API_WAIT_ON_SEMAPHORE_DEF(PlatApiWaitOnSemaphore_f);

#define PLAT_API_TRIGGER_SEMAPHORE_DEF(functionName) bool functionName(PlatSemaphore_t* semaphore, u64 count, u64* previousCountOut)
typedef PLAT_API_TRIGGER_SEMAPHORE_DEF(PlatApiTriggerSemaphore_f);

#define PLAT_API_CREATE_INTERLOCKED_INT(functionName) void functionName(PlatInterlockedInt_t* interlockedInt, u32 initialValue)
typedef PLAT_API_CREATE_INTERLOCKED_INT(PlatApiCreateInterlockedInt_f);

#define PLAT_API_INTERLOCKED_EXCHANGE(functionName) u32 functionName(PlatInterlockedInt_t* interlockedInt, u32 newValue)
typedef PLAT_API_INTERLOCKED_EXCHANGE(PlatApiInterlockedExchange_f);

#define PLAT_API_DESTROY_INTERLOCKED_INT(functionName) void functionName(PlatInterlockedInt_t* interlockedInt)
typedef PLAT_API_DESTROY_INTERLOCKED_INT(PlatApiDestroyInterlockedInt_f);

#define PLAT_API_GET_THIS_THREAD_ID_DEF(functionName) ThreadId_t functionName()
typedef PLAT_API_GET_THIS_THREAD_ID_DEF(PlatApiGetThisThreadId_f);

#define PLAT_API_GET_THREAD_CONTEXT(functionName) PlatThreadPoolThread_t* functionName(ThreadId_t threadId)
typedef PLAT_API_GET_THREAD_CONTEXT(PlatApiGetThreadContext_f);

#define PLAT_API_SLEEP_FOR_MS_DEF(functionName) void functionName(u64 numMs)
typedef PLAT_API_SLEEP_FOR_MS_DEF(PlatApiSleepForMs_f);

#define PLAT_API_QUEUE_TASK_DEFINITION(functionName) PlatTask_t* functionName(PlatTaskInput_t* taskInput)
typedef PLAT_API_QUEUE_TASK_DEFINITION(PlatApiQueueTask_f);

#define PLAT_API_ALLOCATE_MEMORY_DEF(functionName) void* functionName(u64 size, AllocAlignment_t alignOverride)
typedef PLAT_API_ALLOCATE_MEMORY_DEF(PlatApiAllocateMemory_f);

#define PLAT_API_REALLOC_MEMORY_DEF(functionName) void* functionName(void* allocPntr, u64 newSize, u64 oldSize, AllocAlignment_t alignOverride)
typedef PLAT_API_REALLOC_MEMORY_DEF(PlatApiReallocMemory_f);

#define PLAT_API_FREE_MEMORY_DEF(functionName) void functionName(void* allocPntr, u64 oldSize, u64* oldSizeOut)
typedef PLAT_API_FREE_MEMORY_DEF(PlatApiFreeMemory_f);

#define PLAT_API_GET_RAPID_CLICK_MAX_TIME_DEF(functionName) u64 functionName()
typedef PLAT_API_GET_RAPID_CLICK_MAX_TIME_DEF(PlatApiGetRapidClickMaxTime_f);

#define PLAT_API_CHANGE_WINDOW_TARGET_DEF(functionName) void functionName(const PlatWindow_t* window)
typedef PLAT_API_CHANGE_WINDOW_TARGET_DEF(PlatApiChangeWindowTarget_f);

#define PLAT_API_SWAP_BUFFERS_DEF(functionName) void functionName()
typedef PLAT_API_SWAP_BUFFERS_DEF(PlatApiSwapBuffers_f);

#define PLAT_API_GET_NATIVE_WINDOW_PNTR_DEF(functionName) void* functionName(const PlatWindow_t* window)
typedef PLAT_API_GET_NATIVE_WINDOW_PNTR_DEF(PlatApiGetNativeWindowPntr_f);

#define PLAT_API_GET_FULL_PATH_DEF(functionName) MyStr_t functionName(MemArena_t* memArena, MyStr_t relativePath, bool giveBackslashes)
typedef PLAT_API_GET_FULL_PATH_DEF(PlatApiGetFullPath_f);

#define PLAT_API_DOES_FILE_EXIST_DEF(functionName) bool functionName(MyStr_t filePath, bool* isFolderOut)
typedef PLAT_API_DOES_FILE_EXIST_DEF(PlatApiDoesFileExist_f);

#define PLAT_API_START_ENUMERATING_FILES_DEF(functionName) PlatFileEnumerator_t functionName(MyStr_t folderPath, bool enumerateFiles, bool enumerateFolders)
typedef PLAT_API_START_ENUMERATING_FILES_DEF(PlatApiStartEnumeratingFiles_f);

#define PLAT_API_ENUMERATE_FILES_DEF(functionName) bool functionName(PlatFileEnumerator_t* enumerator, MyStr_t* pathOut, MemArena_t* pathOutArena, bool giveFullPath)
typedef PLAT_API_ENUMERATE_FILES_DEF(PlatApiEnumerateFiles_f);

#define PLAT_API_CREATE_FOLDER(functionName) bool functionName(MyStr_t folderPath)
typedef PLAT_API_CREATE_FOLDER(PlatApiCreateFolder_f);

#define PLAT_API_READ_FILE_CONTENTS_DEF(functionName) bool functionName(MyStr_t filePath, MemArena_t* memArena, bool convertNewLines, PlatFileContents_t* contentsOut)
typedef PLAT_API_READ_FILE_CONTENTS_DEF(PlatApiReadFileContents_f);

#define PLAT_API_FREE_FILE_CONTENTS_DEF(functionName) void functionName(PlatFileContents_t* fileContents)
typedef PLAT_API_FREE_FILE_CONTENTS_DEF(PlatApiFreeFileContents_f);

//TODO: Add PLAT_API to this
#define WRITE_ENTIRE_FILE_DEFINITION(functionName) bool functionName(MyStr_t filePath, const void* memory, u64 memorySize)
typedef WRITE_ENTIRE_FILE_DEFINITION(PlatApiWriteEntireFile_f);

#define PLAT_API_OPEN_FILE_DEFINITION(functionName) bool functionName(MyStr_t filePath, OpenFileMode_t mode, bool calculateSize, PlatOpenFile_t* openFileOut)
typedef PLAT_API_OPEN_FILE_DEFINITION(PlatApiOpenFile_f);

#define PLAT_API_READ_FILE_CONTENTS_STREAM_DEFINITION(functionName) Stream_t functionName(MyStr_t filePath, MemArena_t* memArena, bool convertNewLines)
typedef PLAT_API_READ_FILE_CONTENTS_STREAM_DEFINITION(PlatApiReadFileContentsStream_f);

#define PLAT_API_OPEN_FILE_STREAM_DEFINITION(functionName) Stream_t functionName(MyStr_t filePath, MemArena_t* memArena, bool convertNewLines)
typedef PLAT_API_OPEN_FILE_STREAM_DEFINITION(PlatApiOpenFileStream_f);

#define PLAT_API_WRITE_TO_FILE_DEFINITION(functionName) bool functionName(PlatOpenFile_t* openFile, u64 numBytes, const void* bytesPntr, bool convertNewLines)
typedef PLAT_API_WRITE_TO_FILE_DEFINITION(PlatApiWriteToFile_f);

#define PLAT_API_READ_FROM_FILE_DEFINITION(functionName) u64 functionName(PlatOpenFile_t* openFile, u64 numBytes, void* bufferPntr, bool convertNewLines, u64* rawNumBytesReadOut)
typedef PLAT_API_READ_FROM_FILE_DEFINITION(PlatApiReadFromFile_f);

#define PLAT_API_MOVE_FILE_CURSOR_DEFINITION(functionName) bool functionName(PlatOpenFile_t* openFile, i64 moveAmount)
typedef PLAT_API_MOVE_FILE_CURSOR_DEFINITION(PlatApiMoveFileCursor_f);

#define PLAT_API_SEEK_TO_OFFSET_IN_FILE_DEFINITION(functionName) bool functionName(PlatOpenFile_t* openFile, u64 offset)
typedef PLAT_API_SEEK_TO_OFFSET_IN_FILE_DEFINITION(PlatApiSeekToOffsetInFile_f);

#define PLAT_API_CLOSE_FILE_DEFINITION(functionName) void functionName(PlatOpenFile_t* openFile)
typedef PLAT_API_CLOSE_FILE_DEFINITION(PlatApiCloseFile_f);

#define PLAT_API_TRY_PARSE_IMAGE_FILE_DEF(functionName) bool functionName(const PlatFileContents_t* contents, u8 desiredPixelSize, PlatImageData_t* imageOut)
typedef PLAT_API_TRY_PARSE_IMAGE_FILE_DEF(PlatApiTryParseImageFile_f);

#define PLAT_API_SAVE_IMAGE_DATA_TO_FILE(functionName) bool functionName(MyStr_t filePath, const PlatImageData_t* imageData, PlatImageFormat_t imageFormat, u64* fileSizeOut)
typedef PLAT_API_SAVE_IMAGE_DATA_TO_FILE(PlatApiSaveImageDataToFile_f);

#define PLAT_API_FREE_IMAGE_DATA_DEF(functionName) void functionName(PlatImageData_t* imageData)
typedef PLAT_API_FREE_IMAGE_DATA_DEF(PlatApiFreeImageData_f);

//TODO: Add PLAT_API to this
#define SHOW_FILE_DEFINITION(functionName) bool functionName(MyStr_t filePath)
typedef SHOW_FILE_DEFINITION(PlatApiShowFile_f);

//TODO: Add PLAT_API to this
#define SHOW_SOURCE_FILE_DEFINITION(functionName) bool functionName(MyStr_t filePath, u64 lineNumber)
typedef SHOW_SOURCE_FILE_DEFINITION(PlatApiShowSourceFile_f);

#define PLAT_API_GET_SPECIAL_FOLDER_PATH(functionName) MyStr_t functionName(SpecialFolder_t specialFolder, MyStr_t applicationName, MemArena_t* memArena)
typedef PLAT_API_GET_SPECIAL_FOLDER_PATH(PlatApiGetSpecialFolderPath_f);

#define PLAT_API_IS_FILE_WATCHED_DEF(functionName) bool functionName(MyStr_t filePath, PlatWatchedFile_t** watchedFileOut)
typedef PLAT_API_IS_FILE_WATCHED_DEF(PlatApiIsFileWatched_f);

#define PLAT_API_WATCH_FILE_DEF(functionName) PlatWatchedFile_t* functionName(MyStr_t filePath)
typedef PLAT_API_WATCH_FILE_DEF(PlatApiWatchFile_f);

#define PLAT_API_UNWATCH_FILE_DEF(functionName) bool functionName(const PlatWatchedFile_t* watchedFile)
typedef PLAT_API_UNWATCH_FILE_DEF(PlatApiUnwatchFile_f);

#define PLAT_API_GET_FILE_ICON_ID_DEF(functionName) u64 functionName(MyStr_t filePath)
typedef PLAT_API_GET_FILE_ICON_ID_DEF(PlatApiGetFileIconId_f);

#define PLAT_API_GET_FILE_ICON_IMAGE_DATA_DEF(functionName) bool functionName(MyStr_t filePath, MemArena_t* memArena, PlatImageData_t* imageDataOut)
typedef PLAT_API_GET_FILE_ICON_IMAGE_DATA_DEF(PlatApiGetFileIconImageData_f);

#ifndef WIN32_GFX_TEST
#define PLAT_API_GET_LOAD_PROC_ADDRESS_FUNC(functionName) GlLoadProc_f functionName()
typedef PLAT_API_GET_LOAD_PROC_ADDRESS_FUNC(PlatApiGetLoadProcAddressFunc_f);
#endif

#define PLAT_API_RENDER_LOADING_SCREEN(functionName) bool functionName(r32 completionPercent)
typedef PLAT_API_RENDER_LOADING_SCREEN(PlatApiRenderLoadingScreen_f);

#define PLAT_API_FREE_FONT_DATA_DEF(functionName) void functionName(PlatFontData_t* fontData)
typedef PLAT_API_FREE_FONT_DATA_DEF(PlatApiFreeFontData_f);

#define PLAT_API_READ_PLATFORM_FONT_DEF(functionName) bool functionName(MyStr_t fontName, i32 fontSize, bool bold, bool italic, MemArena_t* memArena, PlatFileContents_t* fileContentsOut)
typedef PLAT_API_READ_PLATFORM_FONT_DEF(PlatApiReadPlatformFont_f);

#define PLAT_API_BAKE_FONT_DEF(functionName) bool functionName(PlatFileContents_t* fontFile, v2i bakeSize, bool expandTo32bit, u64 numRanges, PlatFontRange_t* ranges, PlatFontData_t* fontDataOut, PlatRectPackContext_t* packContextOut)
typedef PLAT_API_BAKE_FONT_DEF(PlatApiBakeFont_f);

#define PLAT_API_DEBUG_READOUT(functionName) void functionName(MyStr_t displayStr, Color_t color, r32 scale)
typedef PLAT_API_DEBUG_READOUT(PlatApiDebugReadout_f);

//TODO: Add PLAT_API to this
#define COPY_TEXT_TO_CLIPBOARD_DEFINITION(functionName) bool functionName(MyStr_t text)
typedef COPY_TEXT_TO_CLIPBOARD_DEFINITION(PlatApiCopyTextToClipboard_f);

//TODO: Add PLAT_API to this
#define PASTE_TEXT_FROM_CLIPBOARD_DEFINITION(functionName) MyStr_t functionName(MemArena_t* memArena)
typedef PASTE_TEXT_FROM_CLIPBOARD_DEFINITION(PlatApiPasteTextFromClipboard_f);

#define PLAT_API_GET_PROGRAM_TIME_DEFINITION(functionName) u64 functionName(r64* programTimeR64Out, bool ignoreFixedTimeScaleEffects)
typedef PLAT_API_GET_PROGRAM_TIME_DEFINITION(PlatApiGetProgramTime_f);

#define PLAT_API_GET_MONITOR_VIDEO_MODE_DEFINITION(functionName) const PlatMonitorVideoMode_t* functionName(const PlatMonitorInfo_t* monitor, v2i resolution, u64* indexOut)
typedef PLAT_API_GET_MONITOR_VIDEO_MODE_DEFINITION(PlatApiGetMonitorVideoMode_f);

#if STEAM_BUILD

#define PLAT_API_GET_STEAM_FRIEND_INFO_BY_ID_DEFINITION(functionName) PlatSteamFriendInfo_t* functionName(u64 id)
typedef PLAT_API_GET_STEAM_FRIEND_INFO_BY_ID_DEFINITION(PlatApiGetSteamFriendInfoById_f);

#define PLAT_API_GET_STEAM_FRIEND_GROUP_BY_ID_DEFINITION(functionName) PlatSteamFriendGroup_t* functionName(u64 id)
typedef PLAT_API_GET_STEAM_FRIEND_GROUP_BY_ID_DEFINITION(PlatApiGetSteamFriendGroupById_f);

#define PLAT_API_GET_STEAM_FRIEND_PRESENCE_STR_DEFINITION(functionName) PlatSteamFriendPresenceStr_t* functionName(u64 friendId, MyStr_t keyStr)
typedef PLAT_API_GET_STEAM_FRIEND_PRESENCE_STR_DEFINITION(PlatApiGetSteamFriendPresenceStr_f);

#define PLAT_API_START_STEAM_FRIENDS_QUERY_DEFINITION(functionName) void functionName()
typedef PLAT_API_START_STEAM_FRIENDS_QUERY_DEFINITION(PlatApiStartSteamFriendsQuery_f);

#define PLAT_API_UPDATE_STEAM_STATUS_DEFINITION(functionName) void functionName()
typedef PLAT_API_UPDATE_STEAM_STATUS_DEFINITION(PlatApiUpdateSteamFriendStatus_f);

#define PLAT_API_REQUEST_STREAM_FRIEND_AVATAR(functionName) bool functionName(u64 friendId, PlatSteamFriendAvatarSize_t size)
typedef PLAT_API_REQUEST_STREAM_FRIEND_AVATAR(PlatApiRequestSteamFriendAvatar_f);

#endif //STEAM_BUILD

#define PLAT_API_START_PROCESS_DEFINITION(functionName) PlatRunningProcess_t* functionName(MyStr_t filePath, MyStr_t argumentsString, MyStr_t workingDirectory)
typedef PLAT_API_START_PROCESS_DEFINITION(PlatApiStartProcess_f);

#define PLAT_API_CHECK_RUNNING_PROCESS_DEFINITION(functionName) void functionName(PlatRunningProcess_t* runningProcess)
typedef PLAT_API_CHECK_RUNNING_PROCESS_DEFINITION(PlatApiCheckRunningProcess_f);

#define PLAT_API_CLOSE_RUNNING_PROCESS_DEFINITION(functionName) void functionName(PlatRunningProcess_t* runningProcess)
typedef PLAT_API_CLOSE_RUNNING_PROCESS_DEFINITION(PlatApiCloseRunningProcess_f);

#if BOX2D_SUPPORTED

#define PLAT_API_INIT_PHYSICS_ENGINE(functionName) void functionName(v2 gravity, r32 scale)
typedef PLAT_API_INIT_PHYSICS_ENGINE(PlatApiInitPhysicsEngine_f);

#define PLAT_API_DESTROY_PHYSICS_ENGINE(functionName) void functionName()
typedef PLAT_API_DESTROY_PHYSICS_ENGINE(PlatApiDestroyPhysicsEngine_f);

#define PLAT_API_PHYSICS_TICK(functionName) void functionName(r64 stepTimeMs, i32 numVelocityIterations, i32 numPositionIterations)
typedef PLAT_API_PHYSICS_TICK(PlatApiPhysicsTick_f);

#define PLAT_API_CREATE_PHYSICS_BODY(functionName) PlatPhysicsBody_t* functionName(const PlatPhysicsBodyDef_t* definition)
typedef PLAT_API_CREATE_PHYSICS_BODY(PlatApiCreatePhysicsBody_f);

#define PLAT_API_DESTROY_PHYSICS_BODY(functionName) void functionName(PlatPhysicsBody_t* body)
typedef PLAT_API_DESTROY_PHYSICS_BODY(PlatApiDestroyPhysicsBody_f);

#define PLAT_API_SET_PHYSICS_BODY_VELOCITY(functionName) void functionName(PlatPhysicsBody_t* body, v2 velocity, r32 angularVelocity)
typedef PLAT_API_SET_PHYSICS_BODY_VELOCITY(PlatApiSetPhysicsBodyVelocity_f);

#define PLAT_API_GET_PHYSICS_BODY_STATE(functionName) bool functionName(const PlatPhysicsBody_t* body, PlatPhysicsBodyState_t* stateOut)
typedef PLAT_API_GET_PHYSICS_BODY_STATE(PlatApiGetPhysicsBodyState_f);

#endif //BOX2D_SUPPORTED

#endif //  _COMMON_API_FUNCS_H
