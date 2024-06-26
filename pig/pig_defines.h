/*
File:   pig_defines.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Holds a bunch of engine wide defines and macros
*/

#ifndef _PIG_DEFINES_H
#define _PIG_DEFINES_H

#define PIG_MAIN_ARENA_PAGE_SIZE   Megabytes(1)
#define PIG_THREAD_SAFE_ARENA_PAGE_SIZE   Megabytes(1)
#define PIG_AUDIO_ARENA_PAGE_SIZE  Kilobytes(128) //the pages will probably NOT be this size. Since audio takes LARGE continuous chunks.
#define PIG_LARGE_ALLOC_ARENA_PAGE_SIZE  Kilobytes(1) //we set it small, so most allocations will allocate an entire page, and therefore pages get freed with each allocation free
#define PIG_LUA_ARENA_PAGE_SIZE     Megabytes(1)
#define PIG_PYTHON_ARENA_PAGE_SIZE  Megabytes(1)

#define PIG_MAIN_ARENA_DEBUG             false //Make sure to uncomment GYLIB_MEM_ARENA_DEBUG_ENABLED in common_includes.h
#define PIG_MAIN_ARENA_DEBUG_PAGE_SIZE   Megabytes(1)
#define PIG_MEM_ARENA_TEST_SET           false //make sure to uncomment includes for gy_test_memory.h in common_includes.h

#define SHOW_PERF_GRAPH_ON_STARTUP      false
#define SHOW_MEM_GRAPH_ON_STARTUP       false
#define SHOW_PIE_GRAPHS_ON_STARTUP      false
#define SHOW_AUDIO_OUT_GRAPH_ON_STARTUP false
#define SHOW_AUDIO_INSTANCES_ON_STARTUP false
#define PAUSE_PERF_GRAPH_ONCE_FULL      false
#define LOCK_PERF_GRAPH_SCALE           true

#define LOAD_ALL_RESOURCES_ON_STARTUP     false

#define DEBUG_OUTPUT_ENABLED   1
#define REGULAR_OUTPUT_ENABLED 1
#define INFO_OUTPUT_ENABLED    1
#define NOTIFY_OUTPUT_ENABLED  1
#define OTHER_OUTPUT_ENABLED   1
#define WARNING_OUTPUT_ENABLED 1
#define ERROR_OUTPUT_ENABLED   1

#define PIG_TEMP_MAX_MARKS      256

#define PIG_DEFAULT_FRAMERATE   60 //frames per second //TODO: Make this dependent on the hardware (monitor) we are rendering on
#define PIG_DEFAULT_FRAME_TIME  (1000.0 / PIG_DEFAULT_FRAMERATE) //ms
#define PIG_MAX_ELAPSED_MS      100 //ms

#define PIG_PHYS_FRAMERATE       120 //frames per second
#define PIG_PHYS_FRAME_TIME_SEC  (1.0 / PIG_PHYS_FRAMERATE) //secs

#define PIG_MAX_AUDIO_CHANNELS                2 //channels
#define PIG_MAX_SOUND_INSTANCES               64 //instances
#define PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH   44100 //samples

#define PIG_MAX_NUM_NOTIFICATIONS     10 //notifications at one time

#define PIG_DOUBLE_CLICK_TIME         400 //ms TODO: Does this need any tuning? Is there a common accepted value?
#define PIG_DOUBLE_CLICK_MAX_DIST     5 //px TODO: Does this need any tuning? Is there a common accepted value?

#define GIF_FRAMERATE  50   //fps
#define MAX_GIF_SIZE   Gigabytes(1) //frames (1920x1080@50fps that's about 2.5 seconds, but can be like 40 seconds for small-ish sub-rects)

#define PIG_WINDOW_MIN_SIZE   NewVec2i(400, 100)

#define CONVEX_POLYGON2D_SHADER_SUPPORT false

#endif //  _PIG_DEFINES_H
