/*
File:   pig_log.cpp
Author: Taylor Robbins
Date:   01\10\2022
Description: 
	** Holds functions that help us interact with a ProcessLog_t structure
	** These log structures are often used to record successes or failures during
	** deserialization or other complex tasks where we want introspection to
	** the failure after the code has already executed
*/

//TODO: Add support for routing prints through a non TempArena arena. This is mostly useful in GetStartupOptions where we don't have full platform support yet (and thus GetTempArena will crash)

#define DEFAULT_PROCESS_LOG_FIFO_SIZE Kilobytes(8)

// +--------------------------------------------------------------+
// |                       Create and Free                        |
// +--------------------------------------------------------------+
void FreeProcessLog(ProcessLog_t* log)
{
	NotNull_(log);
	if (log->fifo.allocArena == nullptr || log->fifo.allocArena->type != MemArenaType_MarkedStack)
	{
		DestroyStringFifo(&log->fifo);
	}
	if (!IsEmptyStr(log->filePath))
	{
		NotNull(log->allocArena);
		FreeString(log->allocArena, &log->filePath);
	}
	if (!IsEmptyStr(log->processName))
	{
		NotNull(log->allocArena);
		FreeString(log->allocArena, &log->processName);
	}
	ClearPointer(log);
}

void CreateProcessLog(ProcessLog_t* logOut, u64 fifoSize, MemArena_t* fifoArena, MemArena_t* logArena)
{
	NotNull(logOut);
	NotNull(logArena);
	ClearPointer(logOut);
	logOut->allocArena = logArena;
	if (fifoSize > 0)
	{
		NotNull(fifoArena);
		CreateStringFifoInArena(&logOut->fifo, fifoArena, fifoSize);
	}
	logOut->hadErrors = false;
	logOut->hadWarnings = false;
	logOut->debugBreakOnWarningsAndErrors = false;
	logOut->errorCode = 0;
	logOut->filePath = NewStringInArenaNt(logOut->allocArena, "[UnspecifiedPath]");
	logOut->processName = NewStringInArenaNt(logOut->allocArena, "[UnspecifiedName]");
}

void CreateDefaultProcessLog(ProcessLog_t* logOut)
{
	NotNull2(TempArena, mainHeap);
	CreateProcessLog(logOut, DEFAULT_PROCESS_LOG_FIFO_SIZE, TempArena, mainHeap);
}

void CreateProcessLogStub(ProcessLog_t* logOut)
{
	NotNull(logOut);
	ClearPointer(logOut);
	logOut->allocArena = nullptr;
	logOut->hadErrors = false;
	logOut->hadWarnings = false;
	logOut->debugBreakOnWarningsAndErrors = false;
	logOut->errorCode = 0;
	logOut->filePath = MyStr_Empty;
	logOut->processName = MyStr_Empty;
}

//TODO: Create a CopyProcessLog function?

// +--------------------------------------------------------------+
// |                   FilePath and ProcessName                   |
// +--------------------------------------------------------------+
void SetProcessLogFilePath(ProcessLog_t* log, MyStr_t filePath)
{
	NotNull(log);
	NotNullStr(&filePath);
	if (log->allocArena != nullptr)
	{
		if (!IsEmptyStr(log->filePath))
		{
			FreeString(log->allocArena, &log->filePath);
		}
		log->filePath = AllocString(log->allocArena, &filePath);
	}
}
void SetProcessLogName(ProcessLog_t* log, MyStr_t processName)
{
	NotNull(log);
	NotNullStr(&processName);
	if (log->allocArena != nullptr)
	{
		if (!IsEmptyStr(log->processName))
		{
			if (DoesMemArenaSupportFreeing(log->allocArena)) { FreeString(log->allocArena, &log->processName); }
		}
		log->processName = AllocString(log->allocArena, &processName);
	}
}

// +--------------------------------------------------------------+
// |                       Output and Print                       |
// +--------------------------------------------------------------+
void LogOutput_(ProcessLog_t* log, u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* message)
{
	NotNull_(log);
	UNUSED(addNewLine); //TODO: We should probably take this into account
	if (log->debugBreakOnWarningsAndErrors && (dbgLevel == DbgLevel_Warning || dbgLevel == DbgLevel_Error)) { MyDebugBreak(); }
	if (log->fifo.bufferSize > 0)
	{
		TempPushMark();
		
		MyStr_t text = NewStr(message);
		MyStr_t filePathAndFuncName = TempPrintStr("%s%c%s", filePath, DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR, funcName);
		DebugConsoleLine_t metaInfo = {};
		metaInfo.flags = flags;
		metaInfo.fileLineNumber = lineNumber;
		metaInfo.dbgLevel = dbgLevel;
		metaInfo.programTime = ProgramTime;
		metaInfo.preciseProgramTime = plat->GetProgramTime(nullptr, false);
		metaInfo.timestamp = LocalTimestamp;
		metaInfo.thread = plat->GetThisThreadId();
		metaInfo.programTime = ProgramTime;
		
		StringFifoLine_t* newLine = StringFifoPushLineExt(&log->fifo, text, sizeof(metaInfo), &metaInfo, filePathAndFuncName);
		DebugAssertAndUnused_(newLine != nullptr, newLine);
		
		TempPopMark();
	}
}

void LogPrint_(ProcessLog_t* log, u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* formatString, ...)
{
	NotNull_(log);
	if (log->debugBreakOnWarningsAndErrors && (dbgLevel == DbgLevel_Warning || dbgLevel == DbgLevel_Error)) { MyDebugBreak(); }
	if (log->fifo.bufferSize > 0)
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
					
					LogOutput_(log, flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formattedStr);
				}
				else
				{
					//failed print, just send out the formatString
					LogOutput_(log, flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formatString);
				}
			}
			else
			{
				//failed print, just send out the formatString
				LogOutput_(log, flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, formatString);
			}
			TempPopMark();
		}
		else
		{
			LogOutput_(log, flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, "[No TempMem For Print]");
		}
	}
}

void LogExit_(ProcessLog_t* log, bool success, u32 errorCode, const char* filePath, u32 lineNumber, const char* funcName)
{
	NotNull_(log);
	if (success)
	{
		Assert_(errorCode == 0);
		LogOutput_(log, 0x00, filePath, lineNumber, funcName, DbgLevel_Info, true, "Process succeeded");
	}
	else
	{
		Assert_(errorCode != 0);
		if (log->debugBreakOnWarningsAndErrors) { MyDebugBreak(); }
		log->hadErrors = true;
		log->errorCode = errorCode;
		LogPrint_(log, 0x00, filePath, lineNumber, funcName, DbgLevel_Error, true, "Exiting with error code %u(0x%08X)", errorCode, errorCode);
		
	}
}

// +--------------------------------------------------------------+
// |                        Dump Functions                        |
// +--------------------------------------------------------------+
struct DumpProcessLogContext_t
{
	bool isInsertion;
	const ProcessLog_t* log;
	DebugConsole_t* console;
	DbgLevel_t minLevel;
};
// +==================================+
// | ProcessLogAddLinesBeforeCallback |
// +==================================+
// bool ProcessLogAddLinesBeforeCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* srcLine, u64* metaStructSize, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_BEFORE_CALLBACK_DEF(ProcessLogAddLinesBeforeCallback)
{
	NotNull(userPntr);
	UNUSED(fifo);
	UNUSED(srcFifo);
	UNUSED(metaStructSize);
	DumpProcessLogContext_t* context = (DumpProcessLogContext_t*)userPntr;
	DebugConsoleLine_t* srcMetaStruct = GetFifoLineMetaStruct(srcLine, DebugConsoleLine_t);
	return (srcMetaStruct->dbgLevel >= context->minLevel);
}
// +==================================+
// | ProcessLogAddLinesAfterCallback  |
// +==================================+
// void ProcessLogAddLinesAfterCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* srcLine, StringFifoLine_t* newLine, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_AFTER_CALLBACK_DEF(ProcessLogAddLinesAfterCallback)
{
	NotNull(userPntr);
	UNUSED(fifo);
	UNUSED(srcFifo);
	UNUSED(srcLine);
	DumpProcessLogContext_t* context = (DumpProcessLogContext_t*)userPntr;
	UNUSED(context);
	DebugConsoleLine_t* newMetaStruct = GetFifoLineMetaStruct(newLine, DebugConsoleLine_t);
	FlagSet(newMetaStruct->flags, DbgFlag_New);
}
// +====================================+
// | ProcessLogInsertLinesSortCallback  |
// +====================================+
// u64 ProcessLogInsertLinesSortCallback(StringFifo_t* fifo, const StringFifo_t* srcFifo, const StringFifoLine_t* fifoLine, void* userPntr)
GY_STRING_FIFO_PUSH_LINES_SORT_CALLBACK_DEF(ProcessLogInsertLinesSortCallback)
{
	NotNull(fifoLine);
	UNUSED(fifo);
	UNUSED(srcFifo);
	UNUSED(userPntr);
	DebugConsoleLine_t* metaStruct = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
	return metaStruct->preciseProgramTime;
}
void DumpProcessLog(const ProcessLog_t* log, const char* headerAndFooterStr = nullptr, DbgLevel_t minLevel = DbgLevel_Debug)
{
	NotNull(log);
	Assert(minLevel < DbgLevel_NumLevels);
	DumpProcessLogContext_t context = {};
	context.isInsertion = false;
	context.log = log;
	context.minLevel = minLevel;
	context.console = &pig->debugConsole;
	if (headerAndFooterStr != nullptr) { PrintLine_R("v========= %s =========v", headerAndFooterStr); }
	StringFifoPushLinesFromFifo(
		&pig->debugConsole.fifo,
		&log->fifo,
		true, true,
		ProcessLogAddLinesBeforeCallback,
		ProcessLogAddLinesAfterCallback,
		&context
	);
	if (headerAndFooterStr != nullptr) { PrintLine_R("^========= %s =========^", headerAndFooterStr); }
}

//NOTE: This function is sort of just for testing purposes
void InsertProcessLogInOrder(const ProcessLog_t* log, const char* headerAndFooterStr = nullptr, DbgLevel_t minLevel = DbgLevel_Debug)
{
	NotNull(log);
	Assert(minLevel < DbgLevel_NumLevels);
	DumpProcessLogContext_t context = {};
	context.isInsertion = true;
	context.log = log;
	context.minLevel = minLevel;
	context.console = &pig->debugConsole;
	if (headerAndFooterStr != nullptr) { PrintLine_R("v========= %s =========v", headerAndFooterStr); }
	StringFifoInsertLinesFromFifo(
		&pig->debugConsole.fifo,
		&log->fifo,
		ProcessLogInsertLinesSortCallback,
		TempArena,
		true, true,
		nullptr,
		ProcessLogAddLinesAfterCallback,
		&context
	);
	if (headerAndFooterStr != nullptr) { PrintLine_R("^========= %s =========^", headerAndFooterStr); }
}

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+

#define LogWriteAt(log, level, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, (level), false, message)
#define LogWriteLineAt(log, level, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, (level), true,  message)
#define LogPrintAt(log, level, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, (level), false, formatString, ##__VA_ARGS__)
#define LogPrintLineAt(log, level, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, (level), true,  formatString, ##__VA_ARGS__)
#define LogWriteAtx(log, level, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, (level), false, message)
#define LogWriteLineAtx(log, level, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, (level), true,  message)
#define LogPrintAtx(log, level, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, (level), false, formatString, ##__VA_ARGS__)
#define LogPrintLineAtx(log, level, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, (level), true,  formatString, ##__VA_ARGS__)

#define LogWrite_D(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
#define LogWriteLine_D(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
#define LogPrint_D(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_D(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Dx(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
#define LogWriteLine_Dx(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
#define LogPrint_Dx(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Dx(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)

#define LogWrite_R(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
#define LogWriteLine_R(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
#define LogPrint_R(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_R(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Rx(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
#define LogWriteLine_Rx(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
#define LogPrint_Rx(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Rx(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)

#define LogWrite_I(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
#define LogWriteLine_I(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
#define LogPrint_I(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_I(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Ix(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
#define LogWriteLine_Ix(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
#define LogPrint_Ix(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Ix(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)

#define LogWrite_N(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, message)
#define LogWriteLine_N(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
#define LogPrint_N(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_N(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Nx(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, false, message)
#define LogWriteLine_Nx(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
#define LogPrint_Nx(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Nx(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)

#define LogWrite_O(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
#define LogWriteLine_O(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
#define LogPrint_O(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_O(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Ox(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
#define LogWriteLine_Ox(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
#define LogPrint_Ox(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Ox(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)

#define LogWrite_W(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
#define LogWriteLine_W(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
#define LogPrint_W(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_W(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Wx(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
#define LogWriteLine_Wx(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
#define LogPrint_Wx(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Wx(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)

#define LogWrite_E(log, message)                       LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
#define LogWriteLine_E(log, message)                   LogOutput_((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
#define LogPrint_E(log, formatString, ...)             LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_E(log, formatString, ...)         LogPrint_ ((log), DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
#define LogWrite_Ex(log, flags, message)               LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
#define LogWriteLine_Ex(log, flags, message)           LogOutput_((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
#define LogPrint_Ex(log, flags, formatString, ...)     LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
#define LogPrintLine_Ex(log, flags, formatString, ...) LogPrint_ ((log), (flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)

#define LogExitSuccess(log)            LogExit_((log), true,  0,           __FILE__, __LINE__, __func__)
#define LogExitFailure(log, errorCode) LogExit_((log), false, (errorCode), __FILE__, __LINE__, __func__)
