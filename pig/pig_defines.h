/*
File:   pig_defines.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Holds a bunch of engine wide defines and macros
*/

#ifndef _PIG_DEFINES_H
#define _PIG_DEFINES_H

#define PIG_PERMANANT_MEM_SIZE     Kilobytes(1024)
#define PIG_TEMP_ARENA_SIZE        Megabytes(16)
#define PIG_MAIN_ARENA_PAGE_SIZE   Megabytes(1)

#define PIG_WINDOW_ALWAYS_ON_TOP        false
#define SHOW_PERF_GRAPH_ON_STARTUP      false
#define SHOW_MEM_GRAPH_ON_STARTUP       false
#define SHOW_PIE_GRAPHS_ON_STARTUP      false
#define SHOW_AUDIO_OUT_GRAPH_ON_STARTUP false
#define PAUSE_PERF_GRAPH_ONCE_FULL      false
#define LOCK_PERF_GRAPH_SCALE           true

#define DEBUG_OUTPUT_ENABLED   1
#define REGULAR_OUTPUT_ENABLED 1
#define INFO_OUTPUT_ENABLED    1
#define NOTIFY_OUTPUT_ENABLED  1
#define OTHER_OUTPUT_ENABLED   1
#define WARNING_OUTPUT_ENABLED 1
#define ERROR_OUTPUT_ENABLED   1

#define PIG_TEMP_MAX_MARKS      256

#define PIG_TARGET_FRAMERATE   60 //frames per second //TODO: Make this dependent on the hardware (monitor) we are rendering on
#define PIG_TARGET_FRAME_TIME  (1000.0 / PIG_TARGET_FRAMERATE) //ms

#define PIG_PHYS_FRAMERATE       120 //frames per second
#define PIG_PHYS_FRAME_TIME_SEC  (1.0 / PIG_PHYS_FRAMERATE) //secs

#define PIG_MAX_SOUND_INSTANCES               64 //instances
#define PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH   44100 //samples

#define PIG_MAX_NUM_NOTIFICATIONS     10 //notifications at one time

#endif //  _PIG_DEFINES_H
