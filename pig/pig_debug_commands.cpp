/*
File:   pig_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the logic that parses the text that is entered in the inputTextbox of the debug console.
	** After some basic command checks, we pass info over to the game to handle
*/

#define DEBUG_COMMAND_DESCRIPTION_TRUNCATE_LIMIT   32 //chars

DebugCommandInfoList_t CreateDebugCommandInfoList(const char** infoStrsArray, u64 numInfoStrs)
{
	NotNull(infoStrsArray);
	DebugCommandInfoList_t result = {};
	u64 commandIndex = 0;
	u64 commandStrIndex = 0;
	for (u64 sIndex = 0; sIndex < numInfoStrs; sIndex++)
	{
		const char* infoStr = infoStrsArray[sIndex];
		if (infoStr[0] == '\n' && infoStr[1] == '\0')
		{
			commandIndex++;
			result.numCommands++;
			AssertMsg(commandStrIndex >= 2, "One of the commands is missing a description string. Or there are 2 new-line strings in a row");
			commandStrIndex = 0;
		}
		else
		{
			if (commandStrIndex >= 2) { result.totalNumArguments++; }
			commandStrIndex++;
		}
	}
	AssertMsg(commandStrIndex == 0, "The last debug command did not have a new line string ending it");
	
	Assert(result.numCommands > 0);
	result.commands = TempArray(DebugCommandInfo_t, result.numCommands);
	if (result.totalNumArguments > 0)
	{
		result.allArguments = TempArray(MyStr_t, result.totalNumArguments);
	}
	
	commandIndex = 0;
	u64 argumentIndex = 0;
	commandStrIndex = 0;
	for (u64 sIndex = 0; sIndex < numInfoStrs; sIndex++)
	{
		const char* infoStr = infoStrsArray[sIndex];
		if (infoStr[0] == '\n' && infoStr[1] == '\0')
		{
			commandIndex++;
			AssertMsg(commandStrIndex >= 2, "One of the commands is missing a description string. Or there are 2 new-line strings in a row");
			commandStrIndex = 0;
		}
		else
		{
			if (commandStrIndex == 0)
			{
				Assert(commandIndex < result.numCommands);
				result.commands[commandIndex] = {};
				result.commands[commandIndex].name = NewStr(infoStr);
				result.commands[commandIndex].numArguments = 0;
				result.commands[commandIndex].arguments = &result.allArguments[argumentIndex];
			}
			else if (commandStrIndex == 1)
			{
				result.commands[commandIndex].description = NewStr(infoStr);
			}
			else if (commandStrIndex >= 2)
			{
				Assert(argumentIndex < result.totalNumArguments);
				result.commands[commandIndex].numArguments++;
				result.allArguments[argumentIndex] = NewStr(infoStr);
				argumentIndex++;
			}
			commandStrIndex++;
		}
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                       Command Helpers                        |
// +--------------------------------------------------------------+
void DebugPrintArenaInfo(MemArena_t* memArena, const char* arenaName)
{
	NotNull(memArena);
	
	//In case we are reading info about the TempArena, which this function affects, let's make a copy of the values
	MemArena_t arenaCopy = {};
	MyMemCopy(&arenaCopy, memArena, sizeof(MemArena_t));
	
	//TODO: We could probably print out much more info about the arena. Especially if we do more type specializations
	PrintLine_N("Arena info for %s:", arenaName);
	PrintLine_I("  %s / %s used (%llu / %llu) (high %s or %llu)", FormatBytesNt(arenaCopy.used, TempArena), FormatBytesNt(arenaCopy.size, TempArena), arenaCopy.used, arenaCopy.size, FormatBytesNt(arenaCopy.highUsedMark, TempArena), arenaCopy.highUsedMark);
	PrintLine_I("  %llu allocations (high %llu)", arenaCopy.numAllocations, arenaCopy.highAllocMark);
	if (arenaCopy.type == MemArenaType_FixedHeap || arenaCopy.type == MemArenaType_MarkedStack || arenaCopy.type == MemArenaType_Buffer)
	{
		PrintLine_I("  Address Range: 0x%08X - 0x%08X", arenaCopy.mainPntr, ((u8*)arenaCopy.mainPntr) + arenaCopy.size);
	}
	else if (arenaCopy.type == MemArenaType_PagedHeap)
	{
		PrintLine_I("  %llu pages (default %s):", arenaCopy.numPages, FormatBytesNt(arenaCopy.pageSize, TempArena));
		HeapPageHeader_t* pageHeader = (HeapPageHeader_t*)arenaCopy.headerPntr;
		u64 pageIndex = 0;
		while (pageHeader != nullptr)
		{
			u8* pageBasePntr = (u8*)(pageHeader + 1);
			PrintLine_I("    Page[%llu] %s / %s used (%llu / %llu)", pageIndex, FormatBytesNt(pageHeader->used, TempArena), FormatBytesNt(pageHeader->size, TempArena), pageHeader->used, pageHeader->size);
			PrintLine_I("    Page[%llu] Address Range: 0x%08X - 0x%08X", pageIndex, pageBasePntr, pageBasePntr + pageHeader->size);
			pageHeader = pageHeader->next;
			pageIndex++;
		}
	}
}

// +--------------------------------------------------------------+
// |                         Command Info                         |
// +--------------------------------------------------------------+
const char* PigDebugCommandInfoStrs[] = {
	//"command", "description", "arg1", "arg2", ... "\n",
	"help", "Displays this list of commands", "{command}", "\n",
	"break", "Runs MyDebugBreak", "\n",
	"arena_info", "Displays info about a particular memory arena", "{arena_name}", "\n",
	// "template", "description", "\n",
};
DebugCommandInfoList_t PigGetDebugCommandInfoList()
{
	return CreateDebugCommandInfoList(PigDebugCommandInfoStrs, ArrayCount(PigDebugCommandInfoStrs));
}

// +--------------------------------------------------------------+
// |                   Command Implementations                    |
// +--------------------------------------------------------------+
bool PigHandleDebugCommand(MyStr_t command, u64 numArguments, MyStr_t* arguments, MyStr_t fullInputStr)
{
	UNUSED(fullInputStr);
	bool validCommand = true;
	
	// +==============================+
	// |             help             |
	// +==============================+
	if (StrCompareIgnoreCase(command, "help") == 0)
	{
		//TODO: Add support for an optional argument that specifies which command you want detailed info on
		
		DebugCommandInfoList_t engineCommands = PigGetDebugCommandInfoList();
		DebugCommandInfoList_t gameCommands = GameGetDebugCommandInfoList();
		
		PrintLine_N("There are %llu engine and %llu game commands:", engineCommands.numCommands, gameCommands.numCommands);
		u64 totalCommandCount = engineCommands.numCommands + gameCommands.numCommands;
		for (u64 cIndex = 0; cIndex < totalCommandCount; cIndex++)
		{
			DebugCommandInfo_t* commandInfo = (cIndex >= engineCommands.numCommands) ? &gameCommands.commands[cIndex - engineCommands.numCommands] : &engineCommands.commands[cIndex];
			bool isDescTruncated = false;
			MyStr_t descriptionTruncated = commandInfo->description;
			if (descriptionTruncated.length > DEBUG_COMMAND_DESCRIPTION_TRUNCATE_LIMIT) { descriptionTruncated.length = DEBUG_COMMAND_DESCRIPTION_TRUNCATE_LIMIT-3; isDescTruncated = true; }
			if (commandInfo->numArguments > 0)
			{
				PrintLine_D("  %.*s (%llu arg%s): %.*s%s", commandInfo->name.length, commandInfo->name.pntr, commandInfo->numArguments, (commandInfo->numArguments == 1) ? "" : "s", descriptionTruncated.length, descriptionTruncated.pntr, (isDescTruncated ? "..." : ""));
			}
			else
			{
				PrintLine_D("  %.*s: %.*s%s", commandInfo->name.length, commandInfo->name.pntr, descriptionTruncated.length, descriptionTruncated.pntr, (isDescTruncated ? "..." : ""));
			}
		}
	}
	
	// +==============================+
	// |            break             |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "break") == 0)
	{
		WriteLine_I("Hitting manual breakpoint...");
		MyDebugBreak();
	}
	
	// +==============================+
	// |          arena_info          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "arena_info") == 0)
	{
		if (numArguments != 1) { PrintLine_E("arena_info takes 1 argument, not %llu", numArguments); return validCommand; }
		
		if (StrCompareIgnoreCase(arguments[0], "Plat") == 0 || StrCompareIgnoreCase(arguments[0], "PlatHeap") == 0)
		{
			DebugPrintArenaInfo(&pig->platHeap, "PlatHeap");
		}
		else if (StrCompareIgnoreCase(arguments[0], "Fixed") == 0 || StrCompareIgnoreCase(arguments[0], "FixedHeap") == 0)
		{
			DebugPrintArenaInfo(&pig->fixedHeap, "FixedHeap");
		}
		else if (StrCompareIgnoreCase(arguments[0], "Main") == 0 || StrCompareIgnoreCase(arguments[0], "MainHeap") == 0)
		{
			DebugPrintArenaInfo(&pig->mainHeap, "MainHeap");
		}
		else if (StrCompareIgnoreCase(arguments[0], "Std") == 0 || StrCompareIgnoreCase(arguments[0], "StdHeap") == 0)
		{
			DebugPrintArenaInfo(&pig->stdHeap, "stdHeap");
		}
		else if (StrCompareIgnoreCase(arguments[0], "Temp") == 0 || StrCompareIgnoreCase(arguments[0], "TempArena") == 0)
		{
			DebugPrintArenaInfo(&pig->tempArena, "TempArena");
		}
		else if (StrCompareIgnoreCase(arguments[0], "Audio") == 0 || StrCompareIgnoreCase(arguments[0], "AudioHeap") == 0)
		{
			DebugPrintArenaInfo(&pig->audioHeap, "AudioHeap");
		}
		else 
		{
			PrintLine_E("Unknown arena name: \"%.*s\"", arguments[0].length, arguments[0].pntr);
		}
	}
	
	// +==============================+
	// |       Unknown Command        |
	// +==============================+
	else
	{
		validCommand = false;
	}
	
	return validCommand;
}

bool PigParseDebugCommand(MyStr_t commandStr)
{
	TempPushMark();
	u64 numPieces = 0;
	MyStr_t* pieces = SplitStringBySpacesFastTemp(TempArena, commandStr, &numPieces);
	if (numPieces == 0) { TempPopMark(); return false; }
	NotNull(pieces);
	MyStr_t command = pieces[0];
	u64 numArguments = numPieces-1;
	MyStr_t* arguments = &pieces[1];
	
	bool validCommand = true;
	
	// +==============================+
	// |             help             |
	// +==============================+
	if (!PigHandleDebugCommand(command, numArguments, arguments, commandStr) &&
		!GameHandleDebugCommand(command, numArguments, arguments, commandStr))
	{
		PrintLine_E("Unknown command \"%.*s\"", command.length, command.pntr);
		validCommand = false;
	}
	
	TempPopMark();
	return validCommand;
}
