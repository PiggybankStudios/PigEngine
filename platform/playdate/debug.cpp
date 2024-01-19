/*
File:   debug.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG_OUTPUT_ENABLED   1
#define REGULAR_OUTPUT_ENABLED 1
#define INFO_OUTPUT_ENABLED    1
#define NOTIFY_OUTPUT_ENABLED  1
#define OTHER_OUTPUT_ENABLED   1
#define WARNING_OUTPUT_ENABLED 1
#define ERROR_OUTPUT_ENABLED   1

#define DEBUG_OUTPUT_DELAY     0 //ms

enum DbgFlags_t
{
	DbgFlags_None         = 0x00,
	DbgFlag_Inverted      = 0x01,
	DbgFlag_New           = 0x04,
	// DbgFlag_Unused     = 0x08,
	DbgFlag_UserInput     = 0x10,
	DbgFlag_GyLib         = 0x20,
	DbgFlag_TaskThread    = 0x40,
	// DbgFlag_Unused        = 0x80,
};

// +--------------------------------------------------------------+
// |                 Main Debug Output Functions                  |
// +--------------------------------------------------------------+
void AppDebugOutput(u8 flags, const char* filePath, u64 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
{
	if (pd != nullptr)
	{
		if (pig == nullptr || pig->debugOutputDisabledLevels[level] == false)
		{
			pd->system->logToConsole(message);
			#if DEBUG_OUTPUT_DELAY > 0
			MicroDelay(DEBUG_OUTPUT_DELAY);
			#endif
		}
	}
}
void AppDebugPrint(u8 flags, const char* filePath, u64 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
{
	if (pig != nullptr)
	{
		MemArena_t* scratch = GetScratchArena();
		if (scratch != nullptr)
		{
			PrintInArenaVa(scratch, printedStr, printedLength, formatString);
			//TODO: Add error checking!
			AppDebugOutput(flags, filePath, lineNumber, funcName, level, newLine, printedStr);
			FreeScratchArena(scratch);
		}
	}
}

// +--------------------------------------------------------------+
// |                        GyLib Handlers                        |
// +--------------------------------------------------------------+
// +==============================+
// |      GyLibOutputHandler      |
// +==============================+
// void GyLibOutputHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* message)
GYLIB_DEBUG_OUTPUT_HANDLER_DEF(GyLibOutputHandler)
{
	if (pig != nullptr) { AppDebugOutput(DbgFlag_GyLib, filePath, lineNumber, funcName, level, newLine, message); }
}
// +==============================+
// |      GyLibPrintHandler       |
// +==============================+
// void GyLibPrintHandler(const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t level, bool newLine, const char* formatString, ...)
GYLIB_DEBUG_PRINT_HANDLER_DEF(GyLibPrintHandler)
{
	if (pig != nullptr)
	{
		MemArena_t* scratch = GetScratchArena();
		if (scratch != nullptr)
		{
			PrintInArenaVa(scratch, printedStr, printedLength, formatString);
			//TODO: Add error checking!
			GyLibOutputHandler(filePath, lineNumber, funcName, level, newLine, printedStr);
			FreeScratchArena(scratch);
		}
	}
}

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define WriteLineAt(dbgLevel, message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), true,  message)
#define PrintLineAt(dbgLevel, formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), true,  formatString, ##__VA_ARGS__)
#define WriteLineAtx(flags, dbgLevel, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, (dbgLevel), true,  message)
#define PrintLineAtx(flags, dbgLevel, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, (dbgLevel), true,  formatString, ##__VA_ARGS__)

#if DEBUG_OUTPUT_ENABLED
	#define WriteLine_D(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
	#define PrintLine_D(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Dx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
	#define PrintLine_Dx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_D(message)                   //nothing
	#define PrintLine_D(formatString, ...)         //nothing
	#define WriteLine_Dx(flags, message)           //nothing
	#define PrintLine_Dx(flags, formatString, ...) //nothing
#endif

#if REGULAR_OUTPUT_ENABLED
	#define WriteLine_R(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
	#define PrintLine_R(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Rx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
	#define PrintLine_Rx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_R(message)                   //nothing
	#define PrintLine_R(formatString, ...)         //nothing
	#define WriteLine_Rx(flags, message)           //nothing
	#define PrintLine_Rx(flags, formatString, ...) //nothing
#endif

#if INFO_OUTPUT_ENABLED
	#define WriteLine_I(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
	#define PrintLine_I(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Ix(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
	#define PrintLine_Ix(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_I(message)                   //nothing
	#define PrintLine_I(formatString, ...)         //nothing
	#define WriteLine_Ix(flags, message)           //nothing
	#define PrintLine_Ix(flags, formatString, ...) //nothing
#endif

#if NOTIFY_OUTPUT_ENABLED
	#define WriteLine_N(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
	#define PrintLine_N(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Nx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
	#define PrintLine_Nx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_N(message)                   //nothing
	#define PrintLine_N(formatString, ...)         //nothing
	#define WriteLine_Nx(flags, message)           //nothing
	#define PrintLine_Nx(flags, formatString, ...) //nothing
#endif

#if OTHER_OUTPUT_ENABLED
	#define WriteLine_O(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
	#define PrintLine_O(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Ox(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
	#define PrintLine_Ox(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_O(message)                   //nothing
	#define PrintLine_O(formatString, ...)         //nothing
	#define WriteLine_Ox(flags, message)           //nothing
	#define PrintLine_Ox(flags, formatString, ...) //nothing
#endif

#if WARNING_OUTPUT_ENABLED
	#define WriteLine_W(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
	#define PrintLine_W(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Wx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
	#define PrintLine_Wx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_W(message)                   //nothing
	#define PrintLine_W(formatString, ...)         //nothing
	#define WriteLine_Wx(flags, message)           //nothing
	#define PrintLine_Wx(flags, formatString, ...) //nothing
#endif

#if ERROR_OUTPUT_ENABLED
	#define WriteLine_E(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
	#define PrintLine_E(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
	#define WriteLine_Ex(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
	#define PrintLine_Ex(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
#else
	#define WriteLine_E(message)                   //nothing
	#define PrintLine_E(formatString, ...)         //nothing
	#define WriteLine_Ex(flags, message)           //nothing
	#define PrintLine_Ex(flags, formatString, ...) //nothing
#endif

// +--------------------------------------------------------------+
// |                       Dump Process Log                       |
// +--------------------------------------------------------------+
void DumpProcessLog(const ProcessLog_t* log, const char* headerAndFooterStr = nullptr, DbgLevel_t minLevel = DbgLevel_Debug)
{
	NotNull(log);
	Assert(minLevel < DbgLevel_NumLevels);
	MemArena_t* scratch = GetScratchArena();
	
	if (headerAndFooterStr != nullptr) { PrintLine_R("v========= %s =========v", headerAndFooterStr); }
	
	const StringFifoLine_t* srcLine = log->fifo.firstLine;
	while (srcLine != nullptr)
	{
		PushMemMark(scratch);
		const ProcessLogLine_t* srcMetaStruct = GetFifoLineMetaStruct(srcLine, ProcessLogLine_t);
		
		if (srcMetaStruct->dbgLevel >= minLevel)
		{
			MyStr_t text = GetFifoLineText(srcLine);
			text = AllocString(scratch, &text);
			MyStr_t metaString = GetFifoLineMetaString(srcLine);
			u64 sepCharIndex = 0;
			bool foundSepChar = FindNextCharInStr(metaString, 0, "|", &sepCharIndex);
			MyStr_t filePathStr = (foundSepChar ? StrSubstring(&metaString, 0, sepCharIndex) : metaString);
			filePathStr = AllocString(scratch, &filePathStr);
			MyStr_t funcNameStr = (foundSepChar ? StrSubstring(&metaString, sepCharIndex+1) : MyStr_Empty);
			funcNameStr = AllocString(scratch, &funcNameStr);
			AppDebugOutput(srcMetaStruct->flags, filePathStr.chars, srcMetaStruct->fileLineNumber, funcNameStr.chars, srcMetaStruct->dbgLevel, true, text.chars);
		}
		
		PopMemMark(scratch);
		srcLine = srcLine->next;
	}
	
	if (headerAndFooterStr != nullptr) { PrintLine_R("^========= %s =========^", headerAndFooterStr); }
}

#endif //  _DEBUG_H
