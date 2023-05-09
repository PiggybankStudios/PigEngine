/*
File:   code_gen_main.cpp
Author: Taylor Robbins
Date:   02\12\2023
Description: 
	** Contains the main entry point for the code generator executable
	** This code generator takes metadesk files and produces .h or .cpp files
	** This file also #includes all other files that are needed to compile the code generator 
*/

// +--------------------------------------------------------------+
// |            Standard Library and Platform Includes            |
// +--------------------------------------------------------------+
#define GYLIB_ASSERTIONS_ENABLED ASSERTIONS_ENABLED
#include "gylib/gy_defines_check.h"
#include "gylib/gy_basic_macros.h"

#if WINDOWS_COMPILATION
	#include <windows.h>
#elif OSX_COMPILATION
	#error OSX_COMPILIATION for code generator is not tested yet //TODO: Implement me!
#elif LINUX_COMPILATION
	#error LINUX_COMPILIATION for code generator is not tested yet //TODO: Implement me!
#elif WASM_COMPILATION
	#error Code generator does not support WASM_COMPILIATION
#else
	#error Code generator does not support [unknown_platform]
#endif

// +--------------------------------------------------------------+
// |                  External Library Includes                   |
// +--------------------------------------------------------------+
#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy.h"

// +--------------------------------------------------------------+
// |                      Code-Gen Includes                       |
// +--------------------------------------------------------------+
#include "code_gen_version.h"
#include "code_gen_main.h"

int main()
{
	printf("The code generator is running!\n");
	return 0;
}
