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

void CreateDefaultProcessLog(ProcessLog_t* logOut)
{
	NotNull2(TempArena, mainHeap);
	CreateProcessLog(logOut, DEFAULT_PROCESS_LOG_FIFO_SIZE, TempArena, mainHeap);
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
	DumpProcessLogContext_t* context = (DumpProcessLogContext_t*)userPntr;
	ProcessLogLine_t* srcMetaStruct = GetFifoLineMetaStruct(srcLine, ProcessLogLine_t);
	SetOptionalOutPntr(metaStructSize, sizeof(DebugConsoleLine_t));
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
	ProcessLogLine_t* srcMetaStruct = GetFifoLineMetaStruct(srcLine, ProcessLogLine_t);
	DebugConsoleLine_t* newMetaStruct = GetFifoLineMetaStruct(newLine, DebugConsoleLine_t);
	ClearPointer(newMetaStruct);
	newMetaStruct->flags = (srcMetaStruct->flags | DbgFlag_New);
	newMetaStruct->programTime = srcMetaStruct->programTime;
	newMetaStruct->preciseProgramTime = srcMetaStruct->preciseProgramTime;
	newMetaStruct->timestamp = srcMetaStruct->timestamp;
	newMetaStruct->thread = (ThreadId_t)srcMetaStruct->threadNumber;
	newMetaStruct->fileLineNumber = srcMetaStruct->fileLineNumber;
	newMetaStruct->dbgLevel = srcMetaStruct->dbgLevel;
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
	ProcessLogLine_t* metaStruct = GetFifoLineMetaStruct(fifoLine, ProcessLogLine_t);
	return metaStruct->preciseProgramTime;
}
void DumpProcessLog(const ProcessLog_t* log, const char* headerAndFooterStr = nullptr, DbgLevel_t minLevel = DbgLevel_Debug, bool dumpToStdOut = true)
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
	
	//NOTE: So the above call properly pushes the lines into the target FIFO but we need to manually push all these lines through to the stdout (aka the platform layer)
	//      In order to do that, we do our own loop over the lines here
	if (dumpToStdOut && plat != nullptr && plat->DebugOutput != nullptr)
	{
		const StringFifoLine_t* srcLine = log->fifo.firstLine;
		while (srcLine != nullptr)
		{
			if (ProcessLogAddLinesBeforeCallback(&pig->debugConsole.fifo, &log->fifo, srcLine, nullptr, &context))
			{
				MyStr_t text = GetFifoLineText(srcLine);
				plat->DebugOutput(text, true);
			}
			srcLine = srcLine->next;
		}
	}
	
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
		true, true,
		nullptr,
		ProcessLogAddLinesAfterCallback,
		&context
	);
	if (headerAndFooterStr != nullptr) { PrintLine_R("^========= %s =========^", headerAndFooterStr); }
}
