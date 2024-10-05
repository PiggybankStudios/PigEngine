/*
File:   build_config.h
Author: Taylor Robbins
Date:   04\07\2024
Description:
	** This file is included by pig_main.cpp and win32_main.cpp BEFORE any other files are included.
	** It's also scraped by a python script in build.bat to inform which libraries to link and compiler options.
	** This allows us to configure Pig Engine features\options on a per-game basis in a single
	** file and without having to change the the build.bat as often
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

//NOTE: These must be 1/0 (not true/false) so the batch file can parse them properly

// +--------------------------------------------------------------+
// |                      Compilation Units                       |
// +--------------------------------------------------------------+
#define COMPILE_PLATFORM       1
#define COMPILE_ENGINE         1
#define COMPILE_INSTALLER      0

//Determines whether we should copy the exe/dll to the data directory so it can be double-clicked
// in the file explorer and run from the proper working directory. Normally our
// debugger will run the exe/dll from the build folder but with working directory
// set as the data folder. This way dll copies during hot-reload don't clog up
// the data folder
#define COPY_TO_DATA_DIRECTORY 1

//Determines if we should run piggen.exe on the source files to update generated code
// Generally it only needs to run once after syncing the project, or if the source
// inside #if PIGGEN regions changes, so you can save a bit of time by turning this off
#define RUN_PIG_GEN              1
//Rather than compiling, this will run the code through the preprocessor ONLY and
// output the result to a file in the build folder
#define DUMP_PREPROCESSOR        0
//You have to run the game from a terminal that has run VsDevCmd.bat for this to work
#define ENABLE_ADDRESS_SANITIZER 0

// +--------------------------------------------------------------+
// |                       Boolean Options                        |
// +--------------------------------------------------------------+
// This enables various things that we only want for debug/internal builds
//   1. Enables Debug assertions
//   2. Enables some debug commands that are unsafe for users to use
//   3. Enables debug symbol generation
//   4. Disables optimization
//   5. Disables some strict compiler warnings (like unused variables)
#define DEBUG_BUILD         1
#define DEVELOPER_BUILD     1
#define DEMO_BUILD          0
#define STEAM_BUILD         0

#define ASSERTIONS_ENABLED  1

#define OPENGL_SUPPORTED    1
#define WEBGL_SUPPORTED     0
#define VULKAN_SUPPORTED    0
#define DIRECTX_SUPPORTED   0

#define PROCMON_SUPPORTED   0
#define SOCKETS_SUPPORTED   1
#define BOX2D_SUPPORTED     0
#define SLUG_SUPPORTED      0
#define JSON_SUPPORTED      0
#define LUA_SUPPORTED       0
#define BULLET_SUPPORTED    0
#define PYTHON_SUPPORTED    1

// +--------------------------------------------------------------+
// |                         Project Name                         |
// +--------------------------------------------------------------+
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#define STRINGIFY(text)          #text

//NOTE: We need to change these in the build_installer.nsi as well

// This is the human readable project name, it can contain special characters, spaces, etc.
#define PROJECT_NAME           New Game
#define PROJECT_NAME_STR       STRINGIFY_DEFINE(PROJECT_NAME)

//This determines the name of the exe or other paths that depend on our app name.
//It cannot contain special characters or spaces
#define PROJECT_NAME_SAFE      NewGame
#define PROJECT_NAME_SAFE_STR  STRINGIFY_DEFINE(PROJECT_NAME_SAFE)

#endif //  _BUILD_CONFIG_H
