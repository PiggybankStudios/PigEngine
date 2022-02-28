/*
File:   pig_debug.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds the implementation for debug AppDebugOutput and AppDebugPrint (pre-declared in pig_debug.h)
*/

#define DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR '|'

void AppDebugOutput_Internal(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, MyStr_t messageStr)
{
	if (plat != nullptr && plat->DebugOutput != nullptr)
	{
		plat->DebugOutput(messageStr, addNewLine);
	}
	
	if (pig->debugConsole.hasFifo && GetTempArena() != nullptr)
	{
		TempPushMark();
		
		MyStr_t filePathAndFuncName = TempPrintStr("%s%c%s", filePath, DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR, funcName);
		DebugConsoleLine_t metaInfo = {};
		metaInfo.flags = flags;
		metaInfo.fileLineNumber = lineNumber;
		metaInfo.dbgLevel = dbgLevel;
		metaInfo.programTime = ProgramTime;
		metaInfo.preciseProgramTime = plat->GetProgramTime(nullptr);
		metaInfo.timestamp = LocalTimestamp;
		metaInfo.thread = plat->GetThisThreadId();
		
		plat->LockMutex(&pig->debugConsole.fifoMutex, MUTEX_LOCK_INFINITE);
		if (addNewLine)
		{
			StringFifoLine_t* newLine = StringFifoPushLineExt(&pig->debugConsole.fifo, messageStr, sizeof(metaInfo), &metaInfo, filePathAndFuncName);
			DebugAssert_(newLine != nullptr);
			if (newLine != nullptr) { DebugConsoleLineAdded(&pig->debugConsole, newLine); }
		}
		else
		{
			StringFifoBuildEx(&pig->debugConsole.fifo, messageStr, sizeof(metaInfo), &metaInfo, filePathAndFuncName);
		}
		plat->UnlockMutex(&pig->debugConsole.fifoMutex);
		
		TempPopMark();
	}
}

void AppDebugOutput(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* message)
{
	NotNull(message);
	u64 messageLength = MyStrLength64(message);
	u64 lineStart = 0;
	for (u64 bIndex = 0; bIndex <= messageLength; )
	{
		u32 codepoint = 0;
		u8 charByteSize = 0;
		if (bIndex < messageLength)
		{
			charByteSize = GetCodepointForUtf8(messageLength - bIndex, &message[bIndex], &codepoint);
			if (codepoint == '\n')
			{
				AppDebugOutput_Internal(flags, filePath, lineNumber, funcName, dbgLevel, true, NewStr(bIndex - lineStart, &message[lineStart]));
				lineStart = bIndex+1;
			}
		}
		else
		{
			charByteSize = 1;
			AppDebugOutput_Internal(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, NewStr(bIndex - lineStart, &message[lineStart]));
		}
		bIndex += charByteSize;
	}
}

void AppDebugPrint(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* formatString, ...)
{
	if (GetTempArena() != nullptr)
	{
		TempPushMark();
		va_list args;
		va_start(args, formatString);
		int formattedStrLength = PrintVa_Measure((formatString), args);
		va_end(args);
		if (formattedStrLength >= 0)
		{
			char* formattedStr = TempArray(char, formattedStrLength+1); //Allocate
			if (formattedStr != nullptr)
			{
				va_start(args, formatString);
				PrintVa_Print(formatString, args, formattedStr, formattedStrLength);
				va_end(args);
				formattedStr[formattedStrLength] = '\0';
				
				AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formattedStr);
			}
			else
			{
				//failed print, just send out the formatString
				AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formatString);
			}
		}
		else
		{
			//failed print, just send out the formatString
			AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formatString);
		}
		TempPopMark();
	}
	else
	{
		AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, "[No TempMem For Print]");
		//TODO: Can we do some sort of backup print
	}
}

//TODO: Should we add any flags that indicate this message comes from mylib?
// +==============================+
// | Pig_GyLibDebugOutputHandler  |
// +==============================+
//void Pig_GyLibDebugOutputHandler(const char* filePath, u32 lineNumber, const char* funcName, GyDbgLevel_t level, bool newLine, const char* message)
GYLIB_DEBUG_OUTPUT_HANDLER_DEF(Pig_GyLibDebugOutputHandler)
{
	AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, message);
}
// +==============================+
// |  Pig_GyLibDebugPrintHandler  |
// +==============================+
// void Pig_GyLibDebugPrintHandler(const char* filePath, u32 lineNumber, const char* funcName, GyDbgLevel_t level, bool newLine, const char* formatString, ...);
GYLIB_DEBUG_PRINT_HANDLER_DEF(Pig_GyLibDebugPrintHandler)
{
	AssertSingleThreaded();
	//TODO: Make this thread safe!
	//TODO: Print into the thread's temporary memory
	char* resultStr = nullptr;
	va_list args;
	va_start(args, formatString);
	int printLength = MyVaListPrintf(resultStr, 0, formatString, args); //Measure first
	if (printLength < 0)
	{
		AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), false, "[Failed Print]: ");
		AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, formatString);
		return;
	}
	va_end(args);
	resultStr = AllocArray(mainHeap, char, printLength+1); //Allocate
	if (resultStr == nullptr)
	{
		AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), false, "[Failed Print Allocation]: ");
		AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, formatString);
		return;
	}
	va_start(args, formatString);
	MyVaListPrintf(resultStr, printLength+1, formatString, args); //Real printf
	va_end(args);
	resultStr[printLength] = '\0';
	AppDebugOutput(0x00, filePath, lineNumber, funcName, GetDbgLevelForGyDbgLevel(level), newLine, resultStr);
	FreeMem(mainHeap, resultStr, sizeof(char) * printLength+1);
}

// +--------------------------------------------------------------+
// |                 Handle Platform Debug Lines                  |
// +--------------------------------------------------------------+
struct HandlePlatformLinesContext_t
{
	DebugConsole_t* console;
};
// +===============================+
// | PigPlatDebugLinesSortCallback |
// +===============================+
// u64 PigPlatDebugLinesSortCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* fifoLine, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_SORT_CALLBACK_DEF(PigPlatDebugLinesSortCallback)
{
	NotNull(userPntr);
	HandlePlatformLinesContext_t* context = (HandlePlatformLinesContext_t*)userPntr;
	if (fifoLine->metaStructSize == sizeof(DebugConsoleLine_t))
	{
		DebugConsoleLine_t* metaStruct = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
		return metaStruct->preciseProgramTime;
	}
	else if (fifoLine->metaStructSize == sizeof(PlatDebugLine_t))
	{
		PlatDebugLine_t* metaStruct = GetFifoLineMetaStruct(fifoLine, PlatDebugLine_t);
		return metaStruct->preciseProgramTime;
	}
	else { Assert(false); return 0; }
}
// +==================================+
// | PigPlatDebugLinesBeforeCallback  |
// +==================================+
// bool PigPlatDebugLinesBeforeCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* srcLine, u64* metaStructSize, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_BEFORE_CALLBACK_DEF(PigPlatDebugLinesBeforeCallback)
{
	if (metaStructSize != nullptr)
	{
		*metaStructSize = sizeof(DebugConsoleLine_t);
	}
	return true;
}
// +================================+
// | PigPlatDebugLinesAfterCallback |
// +================================+
// void PigPlatDebugLinesAfterCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* srcLine, StringFifoLine_t* newLine, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_AFTER_CALLBACK_DEF(PigPlatDebugLinesAfterCallback)
{
	NotNull(userPntr);
	HandlePlatformLinesContext_t* context = (HandlePlatformLinesContext_t*)userPntr;
	DebugConsoleLine_t* newMetaStruct = GetFifoLineMetaStruct(newLine, DebugConsoleLine_t);
	PlatDebugLine_t* platMetaStruct = GetFifoLineMetaStruct(srcLine, PlatDebugLine_t);
	ClearPointer(newMetaStruct);
	newMetaStruct->flags              = platMetaStruct->flags;
	newMetaStruct->programTime        = platMetaStruct->programTime;
	newMetaStruct->preciseProgramTime = platMetaStruct->preciseProgramTime;
	newMetaStruct->timestamp          = platMetaStruct->timestamp;
	newMetaStruct->thread             = platMetaStruct->thread;
	newMetaStruct->fileLineNumber     = platMetaStruct->fileLineNumber;
	newMetaStruct->dbgLevel           = platMetaStruct->dbgLevel;
	FlagSet(newMetaStruct->flags, DbgFlag_PlatformLayer);
	FlagSet(newMetaStruct->flags, DbgFlag_New);
}
void PigHandlePlatformDebugLines(const StringFifo_t* platDebugLines)
{
	NotNull(platDebugLines);
	if (platDebugLines->numLines > 0)
	{
		HandlePlatformLinesContext_t context = {};
		context.console = &pig->debugConsole;
		
		StringFifoInsertLinesFromFifo(
			&pig->debugConsole.fifo,
			platDebugLines,
			PigPlatDebugLinesSortCallback,
			TempArena,
			true, true,
			PigPlatDebugLinesBeforeCallback,
			PigPlatDebugLinesAfterCallback,
			&context
		);
	}
}
