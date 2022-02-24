/*
File:   win32_debug.cpp
Author: Taylor Robbins
Date:   09\23\2021
Description: 
	** Holds functions that help us reroute and distribute debug output to various destinations
*/

void Win32_DebugOutputInternal(MyStr_t messageStr, bool newLine)
{
	MyStr_t tempMessage;
	tempMessage.length = messageStr.length;
	tempMessage.pntr = (char*)malloc(messageStr.length+1);
	MyMemCopy(tempMessage.pntr, messageStr.pntr, messageStr.length);
	tempMessage.pntr[messageStr.length] = '\0';
	//NOTE: we use these special characters for easy encoding of style info when rendering text using our font flowing stuff.
	//      But windows will interpret them specially so we replace them with normal characters
	StrReplaceInPlace(tempMessage, "\b", "*");
	StrReplaceInPlace(tempMessage, "\a", "/");
	StrReplaceInPlace(tempMessage, "\f", "_");
	
	OutputDebugStringA(tempMessage.pntr);
	if (newLine) { OutputDebugStringA("\n"); }
	
	printf("%s", tempMessage.pntr);
	if (newLine) { printf("\n"); }
	
	free(tempMessage.pntr);
}

//TODO: Take into account that not all debug outputs are line breaks! We need to build the line piece by piece and then push it into the StringFifo_t
void Win32_DebugOutputFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* message)
{
	//Rather than relying on the TempArena which is hard to make thread safe, we are going to join the filePath and funcName
	//using this local buffer on the stack.
	char filePathAndFuncNameBuffer[PLAT_MAX_FILEPATH_AND_FUNC_NAME+1];
	
	if (InitPhase >= Win32InitPhase_DebugOutputInitialized)
	{
		MyStr_t text = NewStr(message);
		MyStr_t filePathAndFuncName = MyStr_Empty;
		if (filePath != nullptr && funcName != nullptr)
		{
			//TODO: Can we convert this into a function? Like strs combine in array allowing crop (with 1 char added in-between)
			u64 filePathLength = MyStrLength64(filePath);
			u64 funcNameLength = MyStrLength64(funcName);
			u64 neededSpace = filePathLength + 1 + funcNameLength + 1;
			if (neededSpace > ArrayCount(filePathAndFuncNameBuffer))
			{
				if (neededSpace - ArrayCount(filePathAndFuncNameBuffer) <= funcNameLength)
				{
					funcNameLength = 0;
				}
				else
				{
					funcName = nullptr;
					funcNameLength = 0;
					filePathLength = ArrayCount(filePathAndFuncNameBuffer)-1;
				}
			}
			if (filePathLength > 0 && filePath != nullptr)
			{
				MyMemCopy(&filePathAndFuncNameBuffer[0], filePath, filePathLength);
			}
			if (funcNameLength > 0 && funcName != nullptr)
			{
				filePathAndFuncNameBuffer[filePathLength] = PLAT_DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR;
				MyMemCopy(&filePathAndFuncNameBuffer[filePathLength+1], funcName, funcNameLength);
				filePathAndFuncNameBuffer[filePathLength+1+funcNameLength] = '\0';
				filePathAndFuncName.length = filePathLength+1+funcNameLength;
				filePathAndFuncName.pntr = &filePathAndFuncNameBuffer[0];
			}
			else
			{
				filePathAndFuncNameBuffer[filePathLength] = '\0';
				filePathAndFuncName.length = filePathLength;
				filePathAndFuncName.pntr = &filePathAndFuncNameBuffer[0];
			}
		}
		PlatDebugLine_t metaInfo = {};
		metaInfo.flags = flags;
		metaInfo.fileLineNumber = lineNumber;
		metaInfo.dbgLevel = dbgLevel;
		metaInfo.programTime = Platform->engineActiveInput.programTime;
		metaInfo.preciseProgramTime = Win32_GetProgramTime(nullptr);
		metaInfo.timestamp = Platform->engineActiveInput.localTime.timestamp;
		metaInfo.thread = Win32_GetThisThreadId();
		
		Win32_LockMutex(&Platform->debugLineMutex, MUTEX_LOCK_INFINITE);
		StringFifoLine_t* newFifoLine = StringFifoPushLineExt(&Platform->debugLineFifo, text, sizeof(metaInfo), &metaInfo, filePathAndFuncName);
		DebugAssert_(newFifoLine != nullptr);
		Win32_UnlockMutex(&Platform->debugLineMutex);
	}
	
	Win32_DebugOutputInternal(NewStr(message), newLine);
}
void Win32_DebugPrintFromPlat(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* formatString, ...)
{
	//TODO: Print into the thread's temporary memory
	char* resultStr = nullptr;
	va_list args;
	va_start(args, formatString);
	int printLength = MyVaListPrintf(resultStr, 0, formatString, args); //Measure first
	if (printLength < 0)
	{
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, false, "[Failed Print]: ");
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
		return;
	}
	va_end(args);
	resultStr = (char*)malloc(printLength+1); //Allocate
	if (resultStr == nullptr)
	{
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, false, "[Failed Print Allocation]: ");
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
		return;
	}
	va_start(args, formatString);
	MyVaListPrintf(resultStr, printLength+1, formatString, args); //Real printf
	va_end(args);
	resultStr[printLength] = '\0';
	Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, resultStr);
	free(resultStr);
}
void Win32_DebugPrintLocalFromPlat(void* bufferPntr, u32 bufferSize, u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool newLine, const char* formatString, ...)
{
	Assert_(bufferPntr != nullptr);
	va_list args;
	va_start(args, formatString);
	int printResult = MyVaListPrintf((char*)bufferPntr, bufferSize, formatString, args);
	va_end(args);
	if (printResult < 0)
	{
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, false, "[Failed Print]: ");
		Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, formatString);
		return;
	}
	Assert_((u32)printResult <= bufferSize);
	((char*)bufferPntr)[printResult] = '\0';
	Win32_DebugOutputFromPlat(flags, filePath, lineNumber, funcName, dbgLevel, newLine, (char*)bufferPntr);
}

// +--------------------------------------------------------------+
// |                        GyLib Handlers                        |
// +--------------------------------------------------------------+
//TODO: Should we add any flags that indicate this message comes from mylib?
//void Win32_GyLibDebugOutputHandler(const char* filePath, u32 lineNumber, const char* funcName, GyDbgLevel_t level, bool newLine, const char* message)
GYLIB_DEBUG_OUTPUT_HANDLER_DEF(Win32_GyLibDebugOutputHandler)
{
	Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, message);
}
// void Win32_GyLibDebugPrintHandler(const char* filePath, u32 lineNumber, const char* funcName, GyDbgLevel_t level, bool newLine, const char* formatString, ...);
GYLIB_DEBUG_PRINT_HANDLER_DEF(Win32_GyLibDebugPrintHandler)
{
	//TODO: Print into the thread's temporary memory
	char* resultStr = nullptr;
	va_list args;
	va_start(args, formatString);
	int printLength = MyVaListPrintf(resultStr, 0, formatString, args); //Measure first
	if (printLength < 0)
	{
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), false, "[Failed Print]: ");
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, formatString);
		return;
	}
	va_end(args);
	resultStr = (char*)malloc(printLength+1); //Allocate
	if (resultStr == nullptr)
	{
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), false, "[Failed Print Allocation]: ");
		Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, formatString);
		return;
	}
	va_start(args, formatString);
	MyVaListPrintf(resultStr, printLength+1, formatString, args); //Real printf
	va_end(args);
	resultStr[printLength] = '\0';
	Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, resultStr);
	free(resultStr);
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_DebugInit()
{
	GyLibDebugOutputFunc = Win32_GyLibDebugOutputHandler;
	GyLibDebugPrintFunc  = Win32_GyLibDebugPrintHandler;
	
	Win32_CreateMutex(&Platform->debugLineMutex);
	
	u8* debugOutputFifoSpace = (u8*)malloc(PLAT_DEBUG_OUTPUT_FIFO_SIZE);
	if (debugOutputFifoSpace == nullptr)
	{
		Win32_InitError("Failed to allocate space for the platform layer debug output fifo");
	}
	CreateStringFifo(&Platform->debugLineFifo, PLAT_DEBUG_OUTPUT_FIFO_SIZE, debugOutputFifoSpace);
}

void Win32_PassDebugLinesToEngineInput(EngineInput_t* input)
{
	Win32_LockMutex(&Platform->debugLineMutex, MUTEX_LOCK_INFINITE);
	CopyStringFifo(&input->platDebugLines, &Platform->debugLineFifo, &Platform->mainHeap, true);
	ClearStringFifo(&Platform->debugLineFifo);
	Win32_UnlockMutex(&Platform->debugLineMutex);
}

// +==============================+
// |      Win32_DebugOutput       |
// +==============================+
// void DebugOutput(MyStr_t messageStr, bool newLine)
PLAT_API_DEBUG_OUTPUT_DEF(Win32_DebugOutput)
{
	NotNullStr(&messageStr);
	Win32_DebugOutputInternal(messageStr, newLine);
}

// +--------------------------------------------------------------+
// |             WriteLine Macros for Platform Layer              |
// +--------------------------------------------------------------+
#define LocalPrintAt(buffer, dbgLevel, formatString, ...)     Win32_DebugPrintLocalFromPlat(buffer, sizeof(buffer), 0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
#define LocalPrintLineAt(buffer, dbgLevel, formatString, ...) Win32_DebugPrintLocalFromPlat(buffer, sizeof(buffer), 0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, true, formatString, ##__VA_ARGS__)

#define WriteAt(dbgLevel, message)               Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, dbgLevel, false, message)
#define WriteLineAt(dbgLevel, message)           Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  message)
#define PrintAt(dbgLevel, formatString, ...)     Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, dbgLevel, false, formatString, ##__VA_ARGS__)
#define PrintLineAt(dbgLevel, formatString, ...) Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, dbgLevel, true,  formatString, ##__VA_ARGS__)

#define Write_D(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
#define WriteLine_D(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
#define Print_D(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
#define PrintLine_D(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)

#define Write_R(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
#define WriteLine_R(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
#define Print_R(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
#define PrintLine_R(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)

#define Write_I(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
#define WriteLine_I(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
#define Print_I(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
#define PrintLine_I(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)

#define Write_N(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Fify, false, message)
#define WriteLine_N(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
#define Print_N(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
#define PrintLine_N(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)

#define Write_O(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
#define WriteLine_O(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
#define Print_O(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
#define PrintLine_O(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)

#define Write_W(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
#define WriteLine_W(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
#define Print_W(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
#define PrintLine_W(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)

#define Write_E(message)                         Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
#define WriteLine_E(message)                     Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
#define Print_E(formatString, ...)               Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
#define PrintLine_E(formatString, ...)           Win32_DebugPrintFromPlat (0x00, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
