/*
File:   debug.cpp
Author: Taylor Robbins
Date:   10\07\2024
Description:
	** Holds the primary debug logging functions we use in our Orca applications
	** there are a bunch of aliases in debug.h that help us call these functions with various
	** logging levels and options, and with source line information being passed implicitly
*/

void AppDebugOutput(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* message)
{
	if (dbgLevel == DbgLevel_Warning) { OC_Log_W(message); }
	else if (dbgLevel == DbgLevel_Error) { OC_Log_E(message); }
	else { OC_Log_I(message); }
	
	//TODO: Route this to a buffer somewhere so we can display it in a in-app console
}
void AppDebugPrint(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* formatString, ...)
{
	MemArena_t* scratch = GetScratchArena();
	NotNull_(scratch);
	
	PrintInArenaVa(scratch, outputStr, printResult, formatString);
	
	if (printResult >= 0)
	{
		AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, outputStr);
	}
	else
	{
		AppDebugOutput(flags, filePath, lineNumber, funcName, dbgLevel, addNewLine, "[[PRINT ERROR!]]");
	}
	
	FreeScratchArena(scratch);
}
