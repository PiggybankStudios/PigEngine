/*
File:   win32_file_watching.h
Author: Taylor Robbins
Date:   01\23\2022
*/

#ifndef _WIN_32_FILE_WATCHING_H
#define _WIN_32_FILE_WATCHING_H

struct FileWatchingContext_t
{
	PlatMutex_t watchedFilesMutex;
	LinkedList_t watchedFiles;
};

#endif //  _WIN_32_FILE_WATCHING_H
