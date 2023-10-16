/*
File:   web_main.cpp
Author: Taylor Robbins
Date:   10\15\2023
Description: 
	** This is the main file compiled for the web platform layer of Pig Engine.
	** This file includes other .h and .cpp that constitute the platform.
	** This file will be linked with pig_main.cpp to create game.wasm.
*/

#if 1
#define PLATFORM_LAYER
#include "common_includes.h"
#else
#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy_defines_check.h"
#include "gylib/gy.h"
#endif

WASM_IMPORTED_FUNC void jsPrintInteger(const char* labelStrPntr, int number);
WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	jsPrintString("Pig Engine", "in WebAssembly!");
}
