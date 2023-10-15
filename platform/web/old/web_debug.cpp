/*
File:   web_debug.cpp
Author: Taylor Robbins
Date:   03\13\2022
Description: 
	** Holds functions that help us reroute and distribute debug output to the javascript print functions
*/

#define WEB_DEBUG_OUTPUT_PRINT_BUFFER_SIZE 1024 //chars

void Web_DebugOutputFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* message)
{
	//TODO: Add support for finding line breaks and adhering to them (requires backing state)
	switch (dbgLevel)
	{
		case DbgLevel_None:    js_ConsoleLog(message);    break;
		case DbgLevel_Debug:   js_ConsoleDebug(message);  break;
		case DbgLevel_Regular: js_ConsoleLog(message);    break;
		case DbgLevel_Info:    js_ConsoleInfo(message);   break;
		case DbgLevel_Notify:  js_ConsoleNotify(message); break;
		case DbgLevel_Other:   js_ConsoleOther(message);  break;
		case DbgLevel_Warning: js_ConsoleWarn(message);   break;
		case DbgLevel_Error:   js_ConsoleError(message);  break;
		default: Assert_(false); break;
	}
}

void Web_DebugPrintFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* formatString, ...)
{
	char printBuffer[WEB_DEBUG_OUTPUT_PRINT_BUFFER_SIZE];
	va_list args;
	va_start(args, formatString);
	int printResult = MyVaListPrintf(
		printBuffer,
		WEB_DEBUG_OUTPUT_PRINT_BUFFER_SIZE,
		formatString,
		args
	);
	va_end(args);
	//TODO: Add error checking!
	printBuffer[WEB_DEBUG_OUTPUT_PRINT_BUFFER_SIZE-1] = '\0';
	
	Web_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, printBuffer);
}

// +--------------------------------------------------------------+
// |                        GyLib Handlers                        |
// +--------------------------------------------------------------+
//TODO: Should we add any flags that indicate this message comes from mylib?
//void Web_GyLibDebugOutputHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
GYLIB_DEBUG_OUTPUT_HANDLER_DEF(Web_GyLibDebugOutputHandler)
{
	// TODO: Unimplemented
}
// void Web_GyLibDebugPrintHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...);
GYLIB_DEBUG_PRINT_HANDLER_DEF(Web_GyLibDebugPrintHandler)
{
	// TODO: Unimplemented
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Web_DebugInit()
{
	
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
