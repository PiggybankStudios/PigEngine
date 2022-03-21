/*
File:   common_defines.h
Author: Taylor Robbins
Date:   01\14\2022
*/

#ifndef _COMMON_DEFINES_H
#define _COMMON_DEFINES_H

#define PLAT_MAX_NUM_THREADS         32 //threads
#define PLAT_MAX_THREADPOOL_SIZE     16 //threads
#define PLAT_MAX_NUM_TASKS           128 //tasks

#define PLAT_MAIN_HEAP_SIZE                     Megabytes(1) //TODO: Measure our usage and size accordingly
#define PLAT_MAIN_HEAP_PAGE_SIZE                Kilobytes(64) //TODO: Measure our usage and size accordingly
#define PLAT_THREAD_SAFE_HEAP_SIZE              Kilobytes(64) //TODO: Measure our usage and size accordingly
#define PLAT_TEMP_ARENA_SIZE                    Kilobytes(512) //TODO: Measure our usage and size accordingly
#define PLAT_TEMP_ARENA_MAX_MARKS               256
#define PLAT_DEBUG_OUTPUT_FIFO_SIZE             Kilobytes(32)
#define PLAT_MAX_FILEPATH_AND_FUNC_NAME         2048 //chars
#define PLAT_DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR '|'

//Min 5000 milliseconds for pull mode, max 2000 ms for push mode
#define AUDIO_OUTPUT_BUFFER_DURATION    2000 //ms NOTE: This has to be a multiple of 1000ms
#define AUDIO_FILL_TIME                 100 //ms
#define AUDIO_SLEEP_TIME                50 //ms

#define MUTEX_LOCK_INFINITE     0xFFFFFFFFUL
#define SEMAPHORE_WAIT_INFINITE 0xFFFFFFFFUL

#define SDL_CONTROLLER_DB_PATH "Resources/Text/gamecontrollerdb.txt"

#define FILE_WATCHING_SLEEP_TIME   50 //ms
#define FILE_WATCHING_NOTIFY_DELAY 300 //ms


#endif //  _COMMON_DEFINES_H
