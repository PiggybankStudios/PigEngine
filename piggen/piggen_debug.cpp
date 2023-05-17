/*
File:   piggen_debug.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds our debug output functions and all the wrapper macros that make them easy to call
*/

// +--------------------------------------------------------------+
// |                        Real Functions                        |
// +--------------------------------------------------------------+
void DebugOutputFromPigGen(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* message)
{
	//NOTE: we use these special characters for easy encoding of style info when rendering text using our font flowing stuff.
	//      But windows will interpret them specially so we replace them with normal characters
	MyStr_t tempMessage = NewStringInArenaNt(&pig->stdHeap, message);
	StrReplaceInPlace(tempMessage, "\b", "*");
	StrReplaceInPlace(tempMessage, "\a", "/");
	StrReplaceInPlace(tempMessage, "\f", "_");
	
	OutputDebugStringA(tempMessage.pntr);
	if (newLine) { OutputDebugStringA("\n"); }
	
	printf("%s%s", tempMessage.pntr, (newLine ? "\n" : ""));
	
	FreeString(&pig->stdHeap, &tempMessage);
}
void DebugPrintFromPigGen(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* formatString, ...)
{
	char* resultStr = nullptr;
	va_list args;
	va_start(args, formatString);
	int printLength = MyVaListPrintf(resultStr, 0, formatString, args); //Measure first
	if (printLength < 0)
	{
		DebugOutputFromPigGen(flags, filePath, lineNumber, funcName, dbgLevel, false, "[Failed Print]: ");
		DebugOutputFromPigGen(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
		return;
	}
	va_end(args);
	resultStr = AllocArray(&pig->stdHeap, char, printLength+1); //Allocate
	if (resultStr == nullptr)
	{
		DebugOutputFromPigGen(flags, filePath, lineNumber, funcName, dbgLevel, false, "[Failed Print Allocation]: ");
		DebugOutputFromPigGen(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
		return;
	}
	va_start(args, formatString);
	MyVaListPrintf(resultStr, printLength+1, formatString, args); //Real printf
	va_end(args);
	resultStr[printLength] = '\0';
	DebugOutputFromPigGen(flags, filePath, lineNumber, funcName, dbgLevel, newLine, resultStr);
	FreeMem(&pig->stdHeap, resultStr);
}

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define WriteAt(dbgLevel, message)               DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, dbgLevel, false, message)
#define WriteLineAt(dbgLevel, message)           DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  message)
#define PrintAt(dbgLevel, formatString, ...)     DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, dbgLevel, false, formatString, ##__VA_ARGS__)
#define PrintLineAt(dbgLevel, formatString, ...) DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  formatString, ##__VA_ARGS__)

#define Write_D(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug,   false, message)
#define WriteLine_D(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug,   true,  message)
#define Print_D(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug,   false, formatString, ##__VA_ARGS__)
#define PrintLine_D(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug,   true,  formatString, ##__VA_ARGS__)

#define Write_R(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
#define WriteLine_R(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
#define Print_R(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
#define PrintLine_R(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)

#define Write_I(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info,    false, message)
#define WriteLine_I(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info,    true,  message)
#define Print_I(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info,    false, formatString, ##__VA_ARGS__)
#define PrintLine_I(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info,    true,  formatString, ##__VA_ARGS__)

#define Write_N(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify,  false, message)
#define WriteLine_N(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify,  true,  message)
#define Print_N(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify,  false, formatString, ##__VA_ARGS__)
#define PrintLine_N(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify,  true,  formatString, ##__VA_ARGS__)

#define Write_O(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other,   false, message)
#define WriteLine_O(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other,   true,  message)
#define Print_O(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other,   false, formatString, ##__VA_ARGS__)
#define PrintLine_O(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other,   true,  formatString, ##__VA_ARGS__)

#define Write_W(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
#define WriteLine_W(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
#define Print_W(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
#define PrintLine_W(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)

#define Write_E(message)                         DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error,   false, message)
#define WriteLine_E(message)                     DebugOutputFromPigGen(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error,   true,  message)
#define Print_E(formatString, ...)               DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error,   false, formatString, ##__VA_ARGS__)
#define PrintLine_E(formatString, ...)           DebugPrintFromPigGen (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error,   true,  formatString, ##__VA_ARGS__)

