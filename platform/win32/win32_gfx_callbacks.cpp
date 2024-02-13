/*
File:   win32_gfx_callbacks.cpp
Author: Taylor Robbins
Date:   02\11\2024
Description: 
	** Holds functions that Pig Gfx calls back to us through (passed in PigGfxContext_t to PigGfx_Init)
*/

// +==============================+
// |   Win32_PigGfxInitFailure    |
// +==============================+
// void Win32_PigGfxInitFailure(const char* message)
PIG_GFX_INIT_FAILURE_DEF(Win32_PigGfxInitFailure)
{
	Win32_InitError(message);
}

// +==============================+
// |   Win32_PigGfxDebugOutput    |
// +==============================+
// void Win32_PigGfxDebugOutput(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
PIG_GFX_DEBUG_OUTPUT_DEF(Win32_PigGfxDebugOutput)
{
	Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, message);
}

// +==============================+
// |    Win32_PigGfxDebugPrint    |
// +==============================+
// void Win32_PigGfxDebugPrint(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
PIG_GFX_DEBUG_PRINT_DEF(Win32_PigGfxDebugPrint)
{
	//TODO: Print into the thread's temporary memory
	char* resultStr = nullptr;
	va_list args;
	va_start(args, formatString);
	int printLength = MyVaListPrintf(resultStr, 0, formatString, args); //Measure first
	if (printLength < 0)
	{
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, false, "[Failed Print]: ");
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, formatString);
		return;
	}
	va_end(args);
	resultStr = (char*)malloc(printLength+1); //Allocate
	if (resultStr == nullptr)
	{
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, false, "[Failed Print Allocation]: ");
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, formatString);
		return;
	}
	va_start(args, formatString);
	MyVaListPrintf(resultStr, printLength+1, formatString, args); //Real printf
	va_end(args);
	resultStr[printLength] = '\0';
	Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, resultStr);
	free(resultStr);
}
