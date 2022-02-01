/*
File:   win32_threading.cpp
Author: Taylor Robbins
Date:   09\25\2021
Description: 
	** Holds a bunch of functions that help us manage and interact with threads in windows
	** NOTE: All thread management should be done on the main thread only.
	**       This includes creation+deletion of Semaphores and Mutexes.
*/

THREAD_FUNCTION_DEF(Win32_ThreadPoolFunc, userPntr);

// +--------------------------------------------------------------+
// |                       Mutex Functions                        |
// +--------------------------------------------------------------+
// +==============================+
// |      Win32_CreateMutex       |
// +==============================+
// void CreateMutex(PlatMutex_t* mutex)
PLAT_API_CREATE_MUTEX_DEF(Win32_CreateMutex) //pre-declared in win32_func_defs.cpp
{
	AssertSingleThreaded_();
	NotNull_(mutex);
	ClearPointer(mutex);
	mutex->handle = CreateMutex(nullptr, false, nullptr); //default security, no ownership, no name
	Assert_(mutex->handle != NULL);
}
// +==============================+
// |      Win32_DestroyMutex      |
// +==============================+
// void DestroyMutex(PlatMutex_t* mutex)
PLAT_API_DESTROY_MUTEX_DEF(Win32_DestroyMutex) //pre-declared in win32_func_defs.cpp
{
	AssertSingleThreaded_();
	NotNull_(mutex);
	if (mutex->handle != NULL)
	{
		BOOL closeResult = CloseHandle(mutex->handle);
		Assert_(closeResult != 0);
	}
	ClearPointer(mutex);
}
// +==============================+
// |       Win32_LockMutex        |
// +==============================+
// bool LockMutex(PlatMutex_t* mutex, u32 timeoutMs)
PLAT_API_LOCK_MUTEX_DEF(Win32_LockMutex) //pre-declared in win32_func_defs.cpp
{
	NotNull_(mutex);
	Assert_(mutex->handle != 0x00000000);
	DWORD realTimeout = ((timeoutMs == MUTEX_LOCK_INFINITE) ? INFINITE : (DWORD)timeoutMs);
	DWORD mutexResult = WaitForSingleObject(
		mutex->handle,
		realTimeout
	);
	if (realTimeout == INFINITE && mutexResult != WAIT_OBJECT_0) { Assert_(false); } //Failed to lock mutex!
	return (mutexResult == WAIT_OBJECT_0);
}
// +==============================+
// |      Win32_UnlockMutex       |
// +==============================+
// void UnlockMutex(PlatMutex_t* mutex)
PLAT_API_UNLOCK_MUTEX_DEF(Win32_UnlockMutex) //pre-declared in win32_func_defs.cpp
{
	BOOL mutexResult = ReleaseMutex(mutex->handle);
	Assert_(mutexResult != 0);
}
// +==============================+
// |  Win32_CreateInterlockedInt  |
// +==============================+
// void CreateInterlockedInt(PlatInterlockedInt_t* interlockedInt, u32 initialValue)
PLAT_API_CREATE_INTERLOCKED_INT(Win32_CreateInterlockedInt)
{
	AssertSingleThreaded();
	NotNull(interlockedInt);
	ClearPointer(interlockedInt);
	interlockedInt->id = Platform->nextInterlockedIntId;
	Platform->nextInterlockedIntId++;
	interlockedInt->value = (LONG)initialValue;
}

// +==============================+
// | Win32_DestroyInterlockedInt  |
// +==============================+
// void DestroyInterlockedInt(PlatInterlockedInt_t* interlockedInt)
PLAT_API_DESTROY_INTERLOCKED_INT(Win32_DestroyInterlockedInt)
{
	AssertSingleThreaded();
	NotNull(interlockedInt);
	ClearPointer(interlockedInt);
} 

// +==============================+
// |  Win32_InterlockedExchange   |
// +==============================+
// u32 InterlockedExchange(PlatInterlockedInt_t* interlockedInt, u32 newValue)
PLAT_API_INTERLOCKED_EXCHANGE(Win32_InterlockedExchange) 
{
	NotNull(interlockedInt);
	LONG result = InterlockedExchange(&interlockedInt->value, (LONG)newValue);
	return (u32)result;
}

// +--------------------------------------------------------------+
// |                     Semaphore Functions                      |
// +--------------------------------------------------------------+
// +==============================+
// |    Win32_CreateSemaphore     |
// +==============================+
// void CreateSemaphore(PlatSemaphore_t* semaphore, u64 startCount, u64 maxCount)
PLAT_API_CREATE_SEMAPHORE_DEF(Win32_CreateSemaphore)
{
	AssertSingleThreaded_();
	NotNull_(semaphore);
	ClearPointer(semaphore);
	//default attributes, no name, (TODO: Make this description better)
	semaphore->handle = CreateSemaphoreExA(
		nullptr,
		(LONG)startCount,
		(LONG)maxCount,
		NULL,
		0,
		SEMAPHORE_ALL_ACCESS
	);
	Assert_(semaphore->handle != NULL);
}
// +==============================+
// |    Win32_DestroySemaphore    |
// +==============================+
// void DestroySemaphore(PlatSemaphore_t* semaphore)
PLAT_API_DESTROY_SEMAPHORE_DEF(Win32_DestroySemaphore)
{
	AssertSingleThreaded_();
	NotNull_(semaphore);
	if (semaphore->handle != NULL)
	{
		BOOL closeResult = CloseHandle(semaphore->handle);
		Assert_(closeResult != 0);
	}
	ClearPointer(semaphore);
}
// +==============================+
// |    Win32_WaitOnSemaphore     |
// +==============================+
// bool WaitOnSemaphore(PlatSemaphore_t* semaphore, u32 timeout)
PLAT_API_WAIT_ON_SEMAPHORE_DEF(Win32_WaitOnSemaphore)
{
	Assert_(semaphore != nullptr && semaphore->handle != NULL);
	DWORD timeoutDword = ((timeout == SEMAPHORE_WAIT_INFINITE) ? INFINITE : (DWORD)timeout);
	DWORD waitResult = WaitForSingleObjectEx(
		semaphore->handle,
		timeoutDword,
		false
	);
	if (waitResult == WAIT_OBJECT_0) { return true; }
	else { return false; }
}
// +==============================+
// |    Win32_TriggerSemaphore    |
// +==============================+
// bool TriggerSemaphore(PlatSemaphore_t* semaphore, u64 count, u64* previousCountOut)
PLAT_API_TRIGGER_SEMAPHORE_DEF(Win32_TriggerSemaphore)
{
	Assert_(semaphore != nullptr && semaphore->handle != NULL);
	Assert_(count > 0);
	LONG previousCount = 0;
	BOOL releaseResult = ReleaseSemaphore(
		semaphore->handle,
		(LONG)count,
		&previousCount
	);
	if (previousCountOut != nullptr) { *previousCountOut = (u64)previousCount; }
	return (releaseResult != 0);
}

// +--------------------------------------------------------------+
// |                    Create/Destroy Threads                    |
// +--------------------------------------------------------------+
PlatThread_t* Win32_CreateThread(PlatThreadFunction_f* function, void* userPntr = nullptr, PlatThread_t** writeThreadPntrBeforeStarting = nullptr)
{
	AssertSingleThreaded_();
	NotNull_(function);
	
	PlatThread_t* result = nullptr;
	for (u64 tIndex = 0; tIndex < PLAT_MAX_NUM_THREADS; tIndex++)
	{
		PlatThread_t* thread = &Platform->threads[tIndex];
		if (!thread->active && !thread->waitingForCleanup)
		{
			result = thread;
			break;
		}
	}
	if (result == nullptr)
	{
		return nullptr; //Cannot create any more threads. All the thread slots are being used
	}
	
	ClearPointer(result);
	result->active = true;
	result->waitingForCleanup = true;
	result->id = Platform->nextThreadId;
	Platform->nextThreadId++;
	result->function = function;
	result->userPntr = userPntr;
	if (writeThreadPntrBeforeStarting != nullptr)
	{
		*writeThreadPntrBeforeStarting = result;
	}
	
	result->handle = CreateThread(
		NULL, //default security attributes
		0, //default stack size
		function,
		userPntr,
		0, //default creation flags
		&result->win32_id
	);
	if (result->handle == NULL) //Failed to create thread for unknown reason
	{
		ClearPointer(result);
		return nullptr;
	}
	
	return result;
}

bool Win32_DestroyThread(PlatThread_t* thread)
{
	AssertSingleThreaded_();
	NotNull_(thread);
	AssertIf_(thread->active || thread->waitingForCleanup, thread->win32_id != NULL);
	
	bool result = true;
	if (thread->win32_id != NULL) //Already stopped
	{
		if (TerminateThread(thread->handle, 0x00) != 0)
		{
			ClearPointer(thread);
		}
		else
		{
			Assert_(false); //Failed to terminate thread, not sure why
			result = false;
		}
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_InitThreading()
{
	MainThreadId = Win32_GetThisThreadId();
	AssertMsg(IsMainThread(), "Our IsMainThread macro must be broken");
	
	Platform->nextThreadId = 1;
	Platform->nextMutexId = 1;
	Platform->nextSemaphoreId = 1;
	Platform->nextInterlockedIntId = 1;
	
	for (u64 tIndex = 0; tIndex < PLAT_MAX_NUM_THREADS; tIndex++)
	{
		PlatThread_t* thread = &Platform->threads[tIndex];
		ClearPointer(thread);
	}
}

void Win32_InitThreadPool(u64 numThreads)
{
	Win32_CreateSemaphore(&Platform->threadPoolSemaphore, 0, PLAT_MAX_NUM_TASKS);
	
	PrintLine_I("Spinning up %llu thread(s) for the thread pool", numThreads);
	Platform->threadPoolSize = numThreads;
	for (u64 tIndex = 0; tIndex < numThreads; tIndex++)
	{
		PlatThreadPoolThread_t* newPoolEntry = &Platform->threadPool[tIndex];
		ClearPointer(newPoolEntry);
		newPoolEntry->id = tIndex;
		newPoolEntry->shouldClose = false;
		newPoolEntry->isClosed = false;
		Win32_CreateThread(Win32_ThreadPoolFunc, newPoolEntry, &newPoolEntry->threadPntr);
		NotNull(newPoolEntry->threadPntr);
	}
}

// +--------------------------------------------------------------+
// |                    Platform API Functions                    |
// +--------------------------------------------------------------+
// +==============================+
// |    Win32_GetThisThreadId     |
// +==============================+
// ThreadId_t GetThisThreadId()
PLAT_API_GET_THIS_THREAD_ID_DEF(Win32_GetThisThreadId)
{
	DWORD result = GetCurrentThreadId();
	return (ThreadId_t)result;
}

// +==============================+
// |       Win32_SleepForMs       |
// +==============================+
// void SleepForMs(u64 numMs)
PLAT_API_SLEEP_FOR_MS_DEF(Win32_SleepForMs)
{
	Sleep((DWORD)numMs);
}

// +--------------------------------------------------------------+
// |                     Thread Pool Function                     |
// +--------------------------------------------------------------+
// +==============================+
// |     Win32_ThreadPoolFunc     |
// +==============================+
THREAD_FUNCTION_DEF(Win32_ThreadPoolFunc, userPntr) //pre-declared at top of file
{
	NotNull_(userPntr);
	PlatThreadPoolThread_t* context = (PlatThreadPoolThread_t*)userPntr;
	NotNull_(context->threadPntr);
	PrintLine_I("Thread Pool Thread[%llu] has started! (Thread %llu 0x%08X)", context->id, context->threadPntr->id, context->threadPntr->win32_id);
	while (!context->shouldClose)
	{
		bool anyWorkToDo = false;
		
		//TODO: Implement the search through the tasks waiting for work
		
		if (!anyWorkToDo && !context->shouldClose)
		{
			Win32_WaitOnSemaphore(&Platform->threadPoolSemaphore, SEMAPHORE_WAIT_INFINITE);
		}
	}
	PrintLine_E("Thread Pool Thread[%llu] is closing! (Thread %llu 0x%08X)", context->id, context->threadPntr->id, context->threadPntr->win32_id);
	context->isClosed = true;
	return 0;
}
