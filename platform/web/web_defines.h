/*
File:   web_defines.h
Author: Taylor Robbins
Date:   10\16\2023
*/

#ifndef _WEB_DEFINES_H
#define _WEB_DEFINES_H

#define WEB_SCRATCH_ARENA_PAGE_SIZE       Kilobytes(512) //512kB * 3 arenas = 1,536kB = 24 pages
#define WEB_SCRATCH_ARENA_MAX_NUM_MARKS   256 //u64 for each mark, 8b * 256 = 2kB total

#endif //  _WEB_DEFINES_H
