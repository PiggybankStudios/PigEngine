/*
File:   win32_version.h
Author: Taylor Robbins
Date:   09\15\2021
Description:
	** Defines the macros that contain the values for the current platform layer version number
	** for the win32 version of the platform layer.
	** The build number is incremented automatically by a python script that runs before each build of the platform layer
*/

#ifndef _WIN_32_VERSION_H
#define _WIN_32_VERSION_H

#define WIN32_VERSION_MAJOR 0
#define WIN32_VERSION_MINOR 1

//NOTE: Auto-incremented by a python script before each build
#define WIN32_VERSION_BUILD 3525

#endif //  _WIN_32_VERSION_H
