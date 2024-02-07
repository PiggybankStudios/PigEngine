/*
File:   win32_func_defs.h
Author: Taylor Robbins
Date:   09\25\2021
Description:
	** Occasionally the order of win32_.cpp includes in win32_main.cpp causes problems.
	** Rather than creating header files for every file we have one single header file that
	** serves as a place to pre-declare functions and macros before any .cpp files are included
*/

#ifndef _WIN_32_FUNC_DEFS_H
#define _WIN_32_FUNC_DEFS_H

// +==============================+
// |        win32_core.cpp        |
// +==============================+
void Win32_InitError(const char* errorMessage);

// +==============================+
// |     win32_threading.cpp      |
// +==============================+
PLAT_API_CREATE_MUTEX_DEF(Win32_CreateMutex);
PLAT_API_DESTROY_MUTEX_DEF(Win32_DestroyMutex);
PLAT_API_LOCK_MUTEX_DEF(Win32_LockMutex);
PLAT_API_UNLOCK_MUTEX_DEF(Win32_UnlockMutex);
PLAT_API_GET_THIS_THREAD_ID_DEF(Win32_GetThisThreadId);
#define IsMainThread()          ((InitPhase < Win32InitPhase_ThreadingInitialized) ? true : (Win32_GetThisThreadId() == MainThreadId))
#define AssertSingleThreaded()  Assert(IsMainThread())
#define AssertSingleThreaded_() Assert_(IsMainThread())

// +==============================+
// |    win32_performance.cpp     |
// +==============================+
PLAT_API_GET_PROGRAM_TIME_DEFINITION(Win32_GetProgramTime);
u64 Win32_TimeSince(u64 programTimeSnapshot, bool ignoreFixedTimeScaleEffects);

#ifndef WIN32_GFX_TEST
// +==============================+
// |      win32_overlays.cpp      |
// +==============================+
void Win32_DrawOverlays(PlatWindow_t* window);
#endif

#endif //  _WIN_32_FUNC_DEFS_H
