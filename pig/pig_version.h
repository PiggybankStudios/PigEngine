/*
File:   pig_version.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Defines the macros that contain the values for the current platform layer version number
	** for the win32 version of the platform layer.
	** The build number is incremented automatically by a python script that runs before each build of the platform layer
*/

#ifndef _PIG_VERSION_H
#define _PIG_VERSION_H

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 1

//NOTE: Auto-incremented by a python script before each build
#define ENGINE_VERSION_BUILD 7164

#endif //  _PIG_VERSION_H
