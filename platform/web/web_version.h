/*
File:   web_version.h
Author: Taylor Robbins
Date:   03\08\2022
Description:
	** Defines the macros that contain the values for the current platform layer version number
	** for the web assembly version of the platform layer.
	** The build number is incremented automatically by a python script that runs before each build of the platform layer
*/

#ifndef _WEB_VERSION_H
#define _WEB_VERSION_H

#define WEB_VERSION_MAJOR 0
#define WEB_VERSION_MINOR 1

//NOTE: Auto-incremented by a python script before each build
#define WEB_VERSION_BUILD 3129

#endif //  _WEB_VERSION_H
