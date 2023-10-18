/*
File:   web_debug.cpp
Author: Taylor Robbins
Date:   10\16\2023
Description: 
	** Holds functions that help with debug output when compiling for the web
*/

void Web_DebugOutputFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* message)
{
	//TODO: Add support for finding line breaks and adhering to them (requires backing state)
	switch (dbgLevel)
	{
		case DbgLevel_None:    jsConsoleWriteLine(1, message); break;
		case DbgLevel_Debug:   jsConsoleWriteLine(0, message); break;
		case DbgLevel_Regular: jsConsoleWriteLine(1, message); break;
		case DbgLevel_Info:    jsConsoleWriteLine(2, message); break;
		case DbgLevel_Notify:  jsConsoleWriteLine(3, message); break;
		case DbgLevel_Other:   jsConsoleWriteLine(4, message); break;
		case DbgLevel_Warning: jsConsoleWriteLine(5, message); break;
		case DbgLevel_Error:   jsConsoleWriteLine(6, message); break;
		default: Assert_(false); break;
	}
}

void Web_DebugPrintFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* formatString, ...)
{
	MemArena_t* scratch = GetScratchArena();
	PrintInArenaVa(scratch, printResult, printLength, formatString);
	if (printLength >= 0)
	{
		Web_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, printResult);
	}
	else
	{
		Web_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, DbgLevel_Error, true, "[[PRINT_ERROR]]");
		Web_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
	}
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                        GyLib Handlers                        |
// +--------------------------------------------------------------+
// void Web_GyLibDebugOutputHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
GYLIB_DEBUG_OUTPUT_HANDLER_DEF(Web_GyLibDebugOutputHandler)
{
	Web_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, message);
}
// void Web_GyLibDebugPrintHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
GYLIB_DEBUG_PRINT_HANDLER_DEF(Web_GyLibDebugPrintHandler)
{
	MemArena_t* scratch = GetScratchArena();
	PrintInArenaVa(scratch, printResult, printLength, formatString);
	if (printLength >= 0)
	{
		Web_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, printResult);
	}
	else
	{
		Web_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, DbgLevel_Error, true, "[[PRINT_ERROR]]");
		Web_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, level, newLine, formatString);
	}
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Web_DebugInit()
{
	GyLibDebugOutputFunc = Web_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc = Web_GyLibDebugPrintHandler;
}

// +--------------------------------------------------------------+
// |             WriteLine Macros for Platform Layer              |
// +--------------------------------------------------------------+
#define WriteAt(dbgLevel, message)               Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, dbgLevel, false, message)
#define WriteLineAt(dbgLevel, message)           Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  message)
#define PrintAt(dbgLevel, formatString, ...)     Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, dbgLevel, false, formatString, ##__VA_ARGS__)
#define PrintLineAt(dbgLevel, formatString, ...) Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  formatString, ##__VA_ARGS__)

#define Write_D(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
#define WriteLine_D(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
#define Print_D(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
#define PrintLine_D(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)

#define Write_R(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
#define WriteLine_R(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
#define Print_R(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
#define PrintLine_R(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)

#define Write_I(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
#define WriteLine_I(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
#define Print_I(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
#define PrintLine_I(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)

#define Write_N(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Fify, false, message)
#define WriteLine_N(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
#define Print_N(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
#define PrintLine_N(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)

#define Write_O(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
#define WriteLine_O(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
#define Print_O(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
#define PrintLine_O(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)

#define Write_W(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
#define WriteLine_W(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
#define Print_W(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
#define PrintLine_W(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)

#define Write_E(message)                         Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
#define WriteLine_E(message)                     Web_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
#define Print_E(formatString, ...)               Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
#define PrintLine_E(formatString, ...)           Web_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
