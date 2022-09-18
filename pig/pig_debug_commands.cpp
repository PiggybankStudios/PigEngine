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
	"reload", "Reloads a specific resource", "[resource_name]", "{resource_type}", "\n",
	"resources", "List all resources (or all by a specific type)", "{resource_type}", "\n",
	"watches", "List all file watches that are active for resources", "{resource_type}", "\n",
	"mute", "Sets Master Volume to 0 (or disables music or sounds if argument is passed)", "{music/sounds}", "\n",
	"volumes", "Prints out the current audio volume settings", "\n",
	"set_volume", "Changes the specified volume to the specified value", "[master/music/sounds]", "[value]", "\n",
	"cyclic_funcs", "Toggles rendering of the cyclic function debug overlay", "\n",
	"monitors", "Toggles debug overlay for monitor info, resolutions, video modes, window position, etc.", "\n",
	"set_resolution", "Changes the resolution of the window", "[width]", "[height]", "\n",
	"video_modes", "Lists all the video modes for all monitors (or a specific monitor if a monitor number is given)", "{monitor}", "\n",
	"fullscreen", "Enables or disables fullscreen and also sets resolution and framerate", "[on/off/toggle]", "{width}", "{height}", "{framerate}", "{monitor}", "\n",
	// "template", "description", "\n",
};
DebugCommandInfoList_t PigGetDebugCommandInfoList()
{
	return CreateDebugCommandInfoList(PigDebugCommandInfoStrs, ArrayCount(PigDebugCommandInfoStrs));
}

// +--------------------------------------------------------------+
// |                     Command Registration                     |
// +--------------------------------------------------------------+
void PigRegisterDebugCommands()
{
	DebugCommandInfoList_t engineCommands = PigGetDebugCommandInfoList();
	DebugCommandInfoList_t gameCommands = GameGetDebugCommandInfoList();
	
	PrintLine_D("There are %llu engine and %llu game commands", engineCommands.numCommands, gameCommands.numCommands);
	u64 totalCommandCount = engineCommands.numCommands + gameCommands.numCommands;
	for (u64 cIndex = 0; cIndex < totalCommandCount; cIndex++)
	{
		DebugCommandInfo_t* commandInfo = (cIndex >= engineCommands.numCommands) ? &gameCommands.commands[cIndex - engineCommands.numCommands] : &engineCommands.commands[cIndex];
		DebugConsoleRegisterCommand(&pig->debugConsole, commandInfo->name, commandInfo->description, commandInfo->numArguments, commandInfo->arguments);
	}
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
	// |            reload            |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "reload") == 0)
	{
		if (numArguments < 1 || numArguments > 2) { PrintLine_E("reload takes 1 or 2 arguments, not %llu", numArguments); return validCommand; }
		
		MyStr_t targetName = arguments[0];
		
		if (StrCompareIgnoreCase(targetName, "all") == 0)
		{
			WriteLine_I("Reloading all resources...");
			Pig_LoadAllResources();
			return validCommand;
		}
		
		ResourceType_t targetType = ResourceType_None;
		if (numArguments >= 2)
		{
			for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
			{
				if (StrCompareIgnoreCase(arguments[1], GetResourceTypeStr((ResourceType_t)tIndex)) == 0)
				{
					targetType = (ResourceType_t)tIndex;
					break;
				}
			}
			if (targetType == ResourceType_None)
			{
				PrintLine_E("Unknown resource type given for argument 2: \"%.*s\"", arguments[1].length, arguments[1].pntr);
				return validCommand;
			}
		}
		
		bool foundResource = false;
		for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
		{
			ResourceType_t type = (ResourceType_t)tIndex;
			if (type != ResourceType_None && (targetType == ResourceType_None || targetType == type))
			{
				u64 numOfType = GetNumResourcesOfType(type);
				for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
				{
					const char* resourcePath = GetPathOrNameForResource(type, rIndex);
					MyStr_t resourceName = GetFileNamePart(NewStr(resourcePath));
					if (StrCompareIgnoreCase(resourceName, targetName) == 0)
					{
						PrintLine_I("Reloading %s[%llu]...", GetResourceTypeStr(type), rIndex);
						Pig_LoadResource(type, rIndex);
						foundResource = true;
						break;
					}
				}
				if (foundResource) { break; }
			}
		}
		
		if (!foundResource)
		{
			PrintLine_E("Unknown resource name \"%.*s\" for %s resource", targetName.length, targetName.pntr, (targetType == ResourceType_None ? "Any" : GetResourceTypeStr(targetType)));
		}
	}
	
	// +==============================+
	// |          resources           |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "resources") == 0)
	{
		if (numArguments > 1) { PrintLine_E("resources takes 1 argument, not %llu", numArguments); return validCommand; }
		
		ResourceType_t targetType = ResourceType_None;
		if (numArguments >= 1)
		{
			for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
			{
				if (StrCompareIgnoreCase(arguments[0], GetResourceTypeStr((ResourceType_t)tIndex)) == 0)
				{
					targetType = (ResourceType_t)tIndex;
					break;
				}
			}
			if (targetType == ResourceType_None)
			{
				PrintLine_E("Unknown resource type given for argument 1: \"%.*s\"", arguments[0].length, arguments[0].pntr);
				return validCommand;
			}
		}
		
		for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
		{
			ResourceType_t type = (ResourceType_t)tIndex;
			if (type != ResourceType_None && (targetType == ResourceType_None || targetType == type))
			{
				u64 numOfType = GetNumResourcesOfType(type);
				PrintLine_N("%llu %s Resources:", numOfType, GetResourceTypeStr(type));
				for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
				{
					const char* resourcePath = GetPathOrNameForResource(type, rIndex);
					MyStr_t resourceName = GetFileNamePart(NewStr(resourcePath));
					ResourceStatus_t* status = GetResourceStatus(type, rIndex);
					NotNull(status);
					DbgLevel_t dbgLevel = DbgLevel_Debug;
					if (status->state == ResourceState_Error) { dbgLevel = DbgLevel_Error; }
					else if (status->state == ResourceState_Warning) { dbgLevel = DbgLevel_Warning; }
					else if (status->state == ResourceState_Loaded) { dbgLevel = DbgLevel_Info; }
					PrintLineAt(dbgLevel, "%s[%llu]: \"%.*s\"", GetResourceTypeStr(type), rIndex, resourceName.length, resourceName.pntr);
				}
			}
		}
	}
	
	// +==============================+
	// |           watches            |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "watches") == 0)
	{
		#if DEVELOPER_BUILD
		if (numArguments > 1) { PrintLine_E("watches takes 1 argument, not %llu", numArguments); return validCommand; }
		
		ResourceType_t targetType = ResourceType_None;
		if (numArguments >= 1)
		{
			for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
			{
				if (StrCompareIgnoreCase(arguments[0], GetResourceTypeStr((ResourceType_t)tIndex)) == 0)
				{
					targetType = (ResourceType_t)tIndex;
					break;
				}
			}
			if (targetType == ResourceType_None)
			{
				PrintLine_E("Unknown resource type given for argument 1: \"%.*s\"", arguments[0].length, arguments[0].pntr);
				return validCommand;
			}
		}
		
		PrintLine_N("There are %llu watches active", pig->resources.watches.length);
		VarArrayLoop(&pig->resources.watches, wIndex)
		{
			VarArrayLoopGet(ResourceWatch_t, watch, &pig->resources.watches, wIndex);
			if (targetType == watch->type || targetType == ResourceType_None)
			{
				bool doesFileExist = plat->DoesFileExist(watch->watchedFile->path, nullptr);
				PrintLineAt(doesFileExist ? DbgLevel_Info : DbgLevel_Warning, "\t%s[%llu]: \"%.*s\"", GetResourceTypeStr(watch->type), watch->resourceIndex, watch->watchedFile->path.length, watch->watchedFile->path.pntr);
			}
		}
		#else //DEVELOPER_MODE
		WriteLine_E("This application is not compiled in DEVELOPER_MODE so we aren't watching any files");
		#endif
	}
	
	// +==============================+
	// |             mute             |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "mute") == 0)
	{
		if (numArguments > 1) { PrintLine_E("mute takes 0 or 1 argument, not %llu", numArguments); return validCommand; }
		
		if (numArguments >= 1 && StrCompareIgnoreCase(arguments[0], "music") == 0)
		{
			if (plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE))
			{
				pig->musicEnabled = !pig->musicEnabled;
				plat->UnlockMutex(&pig->volumeMutex);
				PrintLine_I("Music %s", pig->musicEnabled ? "Enabled" : "Disabled");
			}
		}
		else if (numArguments >= 1 && (StrCompareIgnoreCase(arguments[0], "sounds") == 0 || StrCompareIgnoreCase(arguments[0], "sound") == 0))
		{
			if (plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE))
			{
				pig->soundsEnabled = !pig->soundsEnabled;
				plat->UnlockMutex(&pig->volumeMutex);
				PrintLine_I("Sounds %s", pig->soundsEnabled ? "Enabled" : "Disabled");
			}
		}
		else if (numArguments == 0)
		{
			if (plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE))
			{
				//TODO: We should save the old volume before we muted it to 0 and restore it when mute is used while 0 volume
				if (pig->masterVolume != 0) { pig->masterVolumeRestoreAfterMute = pig->masterVolume; }
				else if (pig->masterVolumeRestoreAfterMute == 0) { pig->masterVolumeRestoreAfterMute = 0.8f; }
				pig->masterVolume = (pig->masterVolume == 0) ? pig->masterVolumeRestoreAfterMute : 0.0f;
				plat->UnlockMutex(&pig->volumeMutex);
				PrintLine_I("Master Volume = %.0f", pig->masterVolume*100);
			}
		}
		else
		{
			PrintLine_E("Unknown argument \"%.*s\"! Known values: \"music\", \"sounds\"", arguments[0].length, arguments[0].pntr);
		}
	}
	
	// +==============================+
	// |           volumes            |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "volumes") == 0)
	{
		PrintLine_I("Master Volume: %.0f", pig->masterVolume*100);
		PrintLine_I("Music Volume: %.0f%s", pig->musicVolume*100, (pig->musicEnabled ? "" : " (Disabled)"));
		PrintLine_I("Sounds Volume: %.0f%s", pig->soundsVolume*100, (pig->soundsEnabled ? "" : " (Disabled)"));
	}
	
	// +==============================+
	// |          set_volume          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "set_volume") == 0)
	{
		if (numArguments != 2) { PrintLine_E("set_volume takes 2 arguments, not %llu", numArguments); return validCommand; }
		
		r32 volumeValue = 100;
		TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
		if (!TryParseR32(arguments[1], &volumeValue, &parseFailureReason))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as r32: %s", arguments[1].length, arguments[1].pntr, GetTryParseFailureReasonStr(parseFailureReason));
			return validCommand;
		}
		if (volumeValue < 0 || volumeValue > 100)
		{
			PrintLine_E("Volume must be in range [0, 100], not %g", volumeValue);
			return validCommand;
		}
		
		if (StrCompareIgnoreCase(arguments[0], "master") == 0)
		{
			PrintLine_I("Master volume set to %g (was %.0f)", volumeValue, pig->masterVolume*100);
			pig->masterVolume = volumeValue/100.0f;
		}
		else if (StrCompareIgnoreCase(arguments[0], "music") == 0)
		{
			PrintLine_I("Music volume set to %g (was %.0f)", volumeValue, pig->musicVolume*100);
			pig->musicVolume = volumeValue/100.0f;
		}
		else if (StrCompareIgnoreCase(arguments[0], "sounds") == 0 || StrCompareIgnoreCase(arguments[0], "sound") == 0)
		{
			PrintLine_I("Sounds volume set to %g (was %.0f)", volumeValue, pig->soundsVolume*100);
			pig->soundsVolume = volumeValue/100.0f;
		}
		else
		{
			PrintLine_E("Unknown volume type \"%.*s\"", arguments[0].length, arguments[0].pntr);
			return validCommand;
		}
	}
	
	// +==============================+
	// |         cyclic_funcs         |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "cyclic_funcs") == 0)
	{
		pig->cyclicFuncsDebug = !pig->cyclicFuncsDebug;
		pig->cyclicFunc = CyclicFunc_Default;
		PrintLine_I("Cyclic Functions Overlay %s", pig->cyclicFuncsDebug ? "Enabled" : "Disabled");
	}
	
	// +==============================+
	// |           monitors           |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "monitors") == 0)
	{
		pig->monitorsDebug = !pig->monitorsDebug;
		PrintLine_I("Monitors Debug Overlay %s", pig->monitorsDebug ? "Enabled" : "Disabled");
	}
	
	// +==============================+
	// |         video_modes          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "video_modes") == 0)
	{
		if (numArguments > 1) { PrintLine_E("video_modes only takes 1 optional argument. Not %llu", numArguments); return validCommand; }
		TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
		
		u64 monitorNumber = 0;
		if (numArguments >= 1)
		{
			if (!TryParseU64(arguments[0], &monitorNumber, &parseFailureReason))
			{
				PrintLine_E("Couldn't parse \"%.*s\" as u64: %s", arguments[0].length, arguments[0].pntr, GetTryParseFailureReasonStr(parseFailureReason));
				return validCommand;
			}
			if (monitorNumber == 0)
			{
				WriteLine_E("Invalid monitor number given. Can't be 0");
				return validCommand;
			}
		}
		
		bool foundMonitor = false;
		const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
		for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
		{
			if (monitorNumber == 0 || monitorInfo->designatedNumber == monitorNumber)
			{
				foundMonitor = true;
				PrintLine_N("Monitor %llu \"%.*s\" supports %llu video mode%s:",
					monitorInfo->designatedNumber,
					monitorInfo->name.length,
					monitorInfo->name.pntr,
					monitorInfo->videoModes.length,
					(monitorInfo->videoModes.length == 1 ? "" : "s")
				);
				VarArrayLoop(&monitorInfo->videoModes, vIndex)
				{
					VarArrayLoopGet(PlatMonitorVideoMode_t, videoMode, &monitorInfo->videoModes, vIndex);
					Print_D("  Mode[%llu]: %dx%d (%llu framerate%s:",
						videoMode->index,
						videoMode->resolution.width, videoMode->resolution.height,
						videoMode->numFramerates, (videoMode->numFramerates == 1 ? "" : "s")
					);
					for (u64 fIndex = 0; fIndex < videoMode->numFramerates; fIndex++)
					{
						Print_D(" %lldHz", videoMode->framerates[fIndex]);
					}
					WriteLine_D(")");
				}
			}
			monitorInfo = LinkedListNext(&platInfo->monitors->list, PlatMonitorInfo_t, monitorInfo);
		}
		
		if (!foundMonitor)
		{
			PrintLine_E("There is not monitor with number %llu", monitorNumber);
			return validCommand;
		}
	}
	
	// +==============================+
	// |        set_resolution        |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "set_resolution") == 0)
	{
		if (numArguments != 2) { PrintLine_E("set_resolution takes 2 arguments, not %llu", numArguments); return validCommand; }
		
		const PlatWindow_t* window = platInfo->mainWindow;
		NotNull(window);
		const PlatMonitorInfo_t* monitor = GetCurrentMonitorInfoForWindow(window);
		NotNull(monitor);
		Assert(monitor->currentVideoModeIndex < monitor->videoModes.length);
		const PlatMonitorVideoMode_t* videoMode = VarArrayGet(&monitor->videoModes, monitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
		
		v2i resolution = Vec2i_Zero;
		TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
		if (!TryParseI32(arguments[0], &resolution.width, &parseFailureReason))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as i32: %s", arguments[0].length, arguments[0].pntr, GetTryParseFailureReasonStr(parseFailureReason));
			return validCommand;
		}
		if (!TryParseI32(arguments[1], &resolution.height, &parseFailureReason))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as i32: %s", arguments[1].length, arguments[1].pntr, GetTryParseFailureReasonStr(parseFailureReason));
			return validCommand;
		}
		if (resolution.width < PIG_WINDOW_MIN_SIZE.width || resolution.height < PIG_WINDOW_MIN_SIZE.height)
		{
			PrintLine_E("Invalid resolution %dx%d. Our minimum size is %dx%d", resolution.width, resolution.height, PIG_WINDOW_MIN_SIZE.width, PIG_WINDOW_MIN_SIZE.height);
			return validCommand;
		}
		if (resolution.width > videoMode->resolution.width) { PrintLine_W("Limiting width to %d", videoMode->resolution.width); resolution.width = videoMode->resolution.width; }
		if (resolution.height > videoMode->resolution.height) { PrintLine_W("Limiting height to %d", videoMode->resolution.height); resolution.height = videoMode->resolution.height; }
		
		pigOut->moveWindow = true;
		pigOut->moveWindowId = window->id;
		pigOut->moveWindowRec = NewReci(window->input.desktopInnerRec.topLeft, resolution);
		PrintLine_I("Moving window to (%d, %d, %d, %d)", pigOut->moveWindowRec.x, pigOut->moveWindowRec.y, pigOut->moveWindowRec.width, pigOut->moveWindowRec.height);
	}
	
	// +==============================+
	// |          fullscreen          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "fullscreen") == 0)
	{
		if (numArguments != 1 && numArguments != 4 && numArguments != 5) { PrintLine_E("fullscreen takes 1, 4, or 5 arguments, not %llu", numArguments); return validCommand; }
		TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
		
		const PlatWindow_t* window = platInfo->mainWindow;
		NotNull(window);
		
		if (StrCompareIgnoreCase(arguments[0], "toggle") == 0)
		{
			if (window->input.fullscreen)
			{
				pigOut->changeFullscreen = true;
				pigOut->changeFullscreenWindowId = window->id;
				pigOut->fullscreenEnabled = false;
				pigOut->windowedResolution = NewVec2i(
					window->input.fullscreenVideoMode->resolution.width/2,
					window->input.fullscreenVideoMode->resolution.height/2
				);
				Assert(window->input.fullscreenFramerateIndex < window->input.fullscreenVideoMode->numFramerates);
				pigOut->windowedFramerate = window->input.fullscreenVideoMode->framerates[window->input.fullscreenFramerateIndex];
				PrintLine_I("Coming out of fullscreen on monitor %llu \"%.*s\". Going to windowed at %dx%d %lldHz",
					window->input.fullscreenMonitor->designatedNumber,
					window->input.fullscreenMonitor->name.length, window->input.fullscreenMonitor->name.pntr,
					pigOut->windowedResolution.width, pigOut->windowedResolution.height,
					pigOut->windowedFramerate
				);
			}
			else
			{
				const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
				NotNull(currentMonitor);
				const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
				
				pigOut->changeFullscreen = true;
				pigOut->changeFullscreenWindowId = window->id;
				pigOut->fullscreenEnabled = true;
				pigOut->fullscreenMonitorId = currentMonitor->id;
				pigOut->fullscreenVideoModeIndex = currentVideoMode->index;
				Assert(currentVideoMode->currentFramerateIndex < currentVideoMode->numFramerates);
				pigOut->fullscreenFramerateIndex = currentVideoMode->currentFramerateIndex;
				PrintLine_I("Enabling fullscreen on monitor %llu \"%.*s\" at %dx%d %lldHz",
					currentMonitor->designatedNumber,
					currentMonitor->name.length, currentMonitor->name.pntr,
					currentVideoMode->resolution.width, currentVideoMode->resolution.height,
					currentVideoMode->framerates[currentVideoMode->currentFramerateIndex]
				);
			}
		}
		else
		{
			bool fullscreenEnabled = false;
			if (!TryParseBool(arguments[0], &fullscreenEnabled, &parseFailureReason))
			{
				PrintLine_E("Couldn't parse \"%.*s\" as bool: %s", arguments[0].length, arguments[0].pntr, GetTryParseFailureReasonStr(parseFailureReason));
				return validCommand;
			}
			if (!fullscreenEnabled && numArguments != 4) { PrintLine_E("turning on fullscreen takes 4 arguments, not %llu", numArguments); return validCommand; }
			if (fullscreenEnabled && numArguments != 5) { PrintLine_E("turning on fullscreen takes 5 arguments, not %llu", numArguments); return validCommand; }
			
			v2i resolution = Vec2i_Zero;
			if (!TryParseI32(arguments[1], &resolution.width, &parseFailureReason))
			{
				PrintLine_E("Couldn't parse \"%.*s\" as i32: %s", arguments[1].length, arguments[1].pntr, GetTryParseFailureReasonStr(parseFailureReason));
				return validCommand;
			}
			if (!TryParseI32(arguments[2], &resolution.height, &parseFailureReason))
			{
				PrintLine_E("Couldn't parse \"%.*s\" as i32: %s", arguments[2].length, arguments[2].pntr, GetTryParseFailureReasonStr(parseFailureReason));
				return validCommand;
			}
			
			i64 framerate = 0;
			if (!TryParseI64(arguments[3], &framerate, &parseFailureReason))
			{
				PrintLine_E("Couldn't parse \"%.*s\" as i64: %s", arguments[3].length, arguments[3].pntr, GetTryParseFailureReasonStr(parseFailureReason));
				return validCommand;
			}
			if (framerate <= 0)
			{
				PrintLine_E("Invalid framerate requested. Can't be <= 0: %lld", framerate);
				return validCommand;
			}
			
			if (fullscreenEnabled)
			{
				u64 monitorNumber = 0;
				if (!TryParseU64(arguments[4], &monitorNumber, &parseFailureReason))
				{
					PrintLine_E("Couldn't parse \"%.*s\" as u64: %s", arguments[4].length, arguments[4].pntr, GetTryParseFailureReasonStr(parseFailureReason));
					return validCommand;
				}
				
				const PlatMonitorInfo_t* targetMonitor = GetMonitorInfoByNumber(monitorNumber);
				if (targetMonitor == nullptr)
				{
					PrintLine_E("There are no monitors with number %llu (There are %llu monitors available)", monitorNumber, platInfo->monitors->list.count);
					return validCommand;
				}
				const PlatMonitorVideoMode_t* targetVideoMode = GetVideoModeWithResolution(targetMonitor, resolution);
				if (targetVideoMode == nullptr)
				{
					PrintLine_E("The target monitor does not support resolution %dx%d", resolution.width, resolution.height); 
					return validCommand;
				}
				i64 targetFramerateIndex = FindVideoModeFramerateIndex(targetVideoMode, framerate);
				if (targetFramerateIndex < 0)
				{
					PrintLine_E("The target monitor does not support %lldHz at %dx%d", framerate, resolution.width, resolution.height);
					return validCommand;
				}
				
				PrintLine_I("Changing to fullscreen mode %dx%d at %lldHz on monitor %d \"%.*s\"", resolution.width, resolution.height, framerate, monitorNumber, targetMonitor->name.length, targetMonitor->name.pntr);
				pigOut->changeFullscreen = true;
				pigOut->changeFullscreenWindowId = window->id;
				pigOut->fullscreenEnabled = true;
				pigOut->fullscreenMonitorId = targetMonitor->id;
				pigOut->fullscreenVideoModeIndex = targetVideoMode->index;
				pigOut->fullscreenFramerateIndex = (u64)targetFramerateIndex;
			}
			else
			{
				if (framerate < 10 || framerate > 300)
				{
					PrintLine_E("Invalid framerate requested. Must be in range [10, 300]: %lld", framerate);
					return validCommand;
				}
				const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
				const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
				if (resolution.width < PIG_WINDOW_MIN_SIZE.width || resolution.height < PIG_WINDOW_MIN_SIZE.height)
				{
					PrintLine_E("Invalid resolution %dx%d. Our minimum size is %dx%d", resolution.width, resolution.height, PIG_WINDOW_MIN_SIZE.width, PIG_WINDOW_MIN_SIZE.height);
					return validCommand;
				}
				if (resolution.width > currentVideoMode->resolution.width) { PrintLine_W("Limiting width to %d", currentVideoMode->resolution.width); resolution.width = currentVideoMode->resolution.width; }
				if (resolution.height > currentVideoMode->resolution.height) { PrintLine_W("Limiting height to %d", currentVideoMode->resolution.height); resolution.height = currentVideoMode->resolution.height; }
				
				PrintLine_I("Changing to windowed mode %dx%d at %lldHz", resolution.width, resolution.height, framerate);
				pigOut->changeFullscreen = true;
				pigOut->changeFullscreenWindowId = window->id;
				pigOut->fullscreenEnabled = false;
				pigOut->windowedResolution = resolution;
				pigOut->windowedFramerate = framerate;
			}
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
