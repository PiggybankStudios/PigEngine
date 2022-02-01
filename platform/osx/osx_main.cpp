/*
File:   osx_main.cpp
Author: Taylor Robbins
Date:   10\01\2021
Description: 
	** Holds the main entry point for the OSX (Macintosh) Platform Layer executable
*/

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define PLATFORM_LAYER
// #define GYLIB_USE_ASSERT_FAILURE_FUNC //TODO: Re-Enable me!
#include "common_includes.h"

#include "/Users/robbitay/MyStuff/projects/PigEngine/code/platform/osx/osx_version.h"

// +--------------------------------------------------------------+
// |                     OSX Main Entry Point                     |
// +--------------------------------------------------------------+
int main(int argc, char* argv[])
{
	printf("Hello OSX!\n");
	
	if (!glfwInit(nullptr))
	{
		exit(EXIT_FAILURE);
	}
}
