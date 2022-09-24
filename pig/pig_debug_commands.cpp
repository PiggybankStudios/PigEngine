/*
File:   pig_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the logic that parses the text that is entered in the inputTextbox of the debug console.
	** After some basic command checks, we pass info over to the game to handle
*/

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
	"thread_ids", "Prints out the IDs of all the threads related this application is currently running", "\n",
	"time_scale", "Fixes and scales the elapsed time that is fed into the game logic to a multiple of 16.666ms (60fps). Set to 2 to test 30 fps logic or 0.5 for 120fps logic. Set to 0 to unfix the time scale", "[scale/up/down]", "\n",
	"bind", "Creates a custom binding that will run a debug command when a specific binding is performed", "[binding]", "[command]", "\n",
	"unbind", "Removes a previously set custom binding", "[binding]", "\n",
	"bindings", "Prints out the list of debug bindings", "\n",
	"reload_bindings", "Reloads the debug bindings from the serialized locations", "\n",
	"app_states", "Prints out the AppState stack and also the names of all appstates that aren't active for reference", "\n",
	"pop_app_state", "Pops the current AppState off the stack (unless it's the last AppState on the stack)", "\n",
	"push_app_state", "Pushes a specified AppState onto the stack (unless it is already active in the stack)", "[app_state]", "\n",
	"change_app_state", "Swaps out the top AppState with a specified state (unless it is already active in the stack)", "[app_state]", "\n",
};

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
DebugCommandInfoList_t PigGetDebugCommandInfoList()
{
	return CreateDebugCommandInfoList(PigDebugCommandInfoStrs, ArrayCount(PigDebugCommandInfoStrs));
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

void DebugPrintDebugBindingEntry(const char* indentation, const PigDebugBindingsEntry_t* binding)
{
	NotNull2(indentation, binding);
	switch (binding->type)
	{
		case PigDebugBindingType_Keyboard:
		{
			Print_I("%s", indentation);
			for (u64 mIndex = 0; mIndex < ModifierKey_NumKeys; mIndex++)
			{
				ModifierKey_t modifierKey = (ModifierKey_t)((u8)1 << mIndex);
				if (IsFlagSet(binding->modifiers, modifierKey))
				{
					Print_I("%s+", GetModifierKeyStr(modifierKey));
				}
			}
			PrintLine_I("%s: \"%.*s\"", GetKeyStr(binding->key), binding->commandStr.length, binding->commandStr.pntr);
		} break;
		case PigDebugBindingType_Mouse:
		{
			PrintLine_I("%sMouse_%s: \"%.*s\"", indentation, GetMouseBtnStr(binding->mouseBtn), binding->commandStr.length, binding->commandStr.pntr);
		} break;
		case PigDebugBindingType_Controller:
		{
			PrintLine_I("%sController_%s: \"%.*s\"", indentation, GetControllerBtnStr(binding->controllerBtn), binding->commandStr.length, binding->commandStr.pntr);
		} break;
		default: DebugAssert(false); break;
	}
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
			ToggleFullscreen(true);
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
				
				DoFullscreenOnMonitor(window, targetMonitor, resolution, framerate, true);
			}
			else
			{
				if (framerate < 10 || framerate > 300)
				{
					PrintLine_E("Invalid framerate requested. Must be in range [10, 300]: %lld", framerate);
					return validCommand;
				}
				// const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
				// const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
				if (resolution.width < PIG_WINDOW_MIN_SIZE.width || resolution.height < PIG_WINDOW_MIN_SIZE.height)
				{
					PrintLine_E("Invalid resolution %dx%d. Our minimum size is %dx%d", resolution.width, resolution.height, PIG_WINDOW_MIN_SIZE.width, PIG_WINDOW_MIN_SIZE.height);
					return validCommand;
				}
				
				StopFullscreen(window, resolution, framerate, true);
			}
		}
	}
	
	// +==============================+
	// |          thread_ids          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "thread_ids") == 0)
	{
		PrintLine_I("MainThread:         %llu (%lld or 0x%08X)", platInfo->mainThreadId.internalId, (i64)platInfo->mainThreadId.osId, (i64)platInfo->mainThreadId.osId);
		PrintLine_I("FileWatchingThread: %llu (%lld or 0x%08X)", platInfo->fileWatchingThreadId.internalId, (i64)platInfo->fileWatchingThreadId.osId, (i64)platInfo->fileWatchingThreadId.osId);
		PrintLine_I("AudioThread:        %llu (%lld or 0x%08X)", platInfo->audioThreadId.internalId, (i64)platInfo->audioThreadId.osId, (i64)platInfo->audioThreadId.osId);
		for (u64 tIndex = 0; tIndex < platInfo->numThreadPoolThreads; tIndex++)
		{
			PrintLine_I("ThreadPool[%llu]:      %llu (%lld or 0x%08X)", tIndex, platInfo->threadPoolIds[tIndex].internalId, (i64)platInfo->threadPoolIds[tIndex].osId, (i64)platInfo->threadPoolIds[tIndex].osId);
		}
	}
	
	// +==============================+
	// |          time_scale          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "time_scale") == 0)
	{
		if (numArguments != 1) { PrintLine_E("time_scale takes 1 argument, not %llu", numArguments); return validCommand; }
		TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
		r32 timeScaleOptions[] = { 0.01f, 0.05f, 0.1f, 0.25f, 0.5f, 0.75f, 1.0f, 1.2f, 1.5f, 2.0f, 5.0f, 10.0f, 20.0f };
		
		r32 requestedScale = 1.0;
		if (StrCompareIgnoreCase(arguments[0], "up") == 0)
		{
			if (pigOut->fixedTimeScaleEnabled) { requestedScale = (r32)pigOut->fixedTimeScale; }
			for (u64 oIndex = 0; oIndex < ArrayCount(timeScaleOptions); oIndex++)
			{
				if (timeScaleOptions[oIndex] > requestedScale)
				{
					requestedScale = timeScaleOptions[oIndex];
					break;
				}
			}
			if (requestedScale == 1.0) { requestedScale = 0; }
		}
		else if (StrCompareIgnoreCase(arguments[0], "down") == 0)
		{
			if (pigOut->fixedTimeScaleEnabled) { requestedScale = (r32)pigOut->fixedTimeScale; }
			for (u64 oIndex = ArrayCount(timeScaleOptions); oIndex > 0; oIndex--)
			{
				if (timeScaleOptions[oIndex-1] < requestedScale)
				{
					requestedScale = timeScaleOptions[oIndex-1];
					break;
				}
			}
			if (requestedScale == 1.0) { requestedScale = 0; }
		}
		else if (!TryParseR32(arguments[0], &requestedScale, &parseFailureReason))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as r32: %s", arguments[0].length, arguments[0].pntr, GetTryParseFailureReasonStr(parseFailureReason));
			return validCommand;
		}
		
		if (requestedScale < 0 || requestedScale > 20)
		{
			PrintLine_E("Invalid scale value. Must be in range [0, 20] not %f", requestedScale);
		}
		
		if (requestedScale == 0)
		{
			if (pigOut->fixedTimeScaleEnabled)
			{
				pigOut->fixedTimeScaleEnabled = false;
				PrintLine_I("Time scale fixing disabled");
			}
			else
			{
				WriteLine_W("Time scale fixing is already disabled");
			}
		}
		else
		{
			pigOut->fixedTimeScaleEnabled = true;
			pigOut->fixedTimeScale = (r64)requestedScale;
			r64 fixedElapsedMs = PIG_DEFAULT_FRAME_TIME * pigOut->fixedTimeScale;
			PrintLine_I("Time scale fixing set to %lgx (%lgfps logic)", pigOut->fixedTimeScale, 1000.0 / fixedElapsedMs);
		}
	}
	
	// +==============================+
	// |             bind             |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "bind") == 0)
	{
		if (numArguments < 2) { PrintLine_E("bind takes at least 2 arguments, not %llu", numArguments); return validCommand; }
		
		PigDebugBindingsEntry_t binding = {};
		if (!PigTryDeserBindingStr(arguments[0], &binding))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as binding", arguments[0].length, arguments[0].pntr);
			return validCommand;
		}
		
		switch (binding.type)
		{
			case PigDebugBindingType_Keyboard:
			{
				PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryKey(&pig->sessionDebugBindings, binding.modifiers, binding.key, arguments[1]);
				NotNull(newEntry);
				WriteLine_I("Keyboard binding created!");
			} break;
			case PigDebugBindingType_Mouse:
			{
				PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryMouse(&pig->sessionDebugBindings, binding.mouseBtn, arguments[1]);
				NotNull(newEntry);
				WriteLine_I("Mouse binding created!");
			} break;
			case PigDebugBindingType_Controller:
			{
				PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryController(&pig->sessionDebugBindings, binding.controllerBtn, arguments[1]);
				NotNull(newEntry);
				WriteLine_I("Controller binding created!");
			} break;
			default: DebugAssert(false); break;
		}
	}
	
	// +==============================+
	// |            unbind            |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "unbind") == 0)
	{
		if (numArguments != 1) { PrintLine_E("unbind takes at 1 argument, not %llu", numArguments); return validCommand; }
		
		PigDebugBindingsEntry_t binding = {};
		if (!PigTryDeserBindingStr(arguments[0], &binding))
		{
			PrintLine_E("Couldn't parse \"%.*s\" as binding", arguments[0].length, arguments[0].pntr);
			return validCommand;
		}
		
		u64 existingBindingIndex = 0;
		PigDebugBindingsEntry_t* existingBinding = PigFindDebugBinding(&pig->sessionDebugBindings, &binding, &existingBindingIndex);
		if (existingBinding != nullptr)
		{
			PigFreeDebugBindingsEntry(existingBinding, pig->sessionDebugBindings.allocArena);
			VarArrayRemove(&pig->sessionDebugBindings.entries, existingBindingIndex, PigDebugBindingsEntry_t);
			WriteLine_I("Removed binding!");
		}
		else
		{
			WriteLine_E("That combination is not bound!");
		}
	}
	
	// +==============================+
	// |           bindings           |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "bindings") == 0)
	{
		PrintLine_N("%llu debug binding%s:", pig->debugBindings.entries.length, (pig->debugBindings.entries.length == 1 ? "" : "s"));
		VarArrayLoop(&pig->debugBindings.entries, bIndex)
		{
			VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &pig->debugBindings.entries, bIndex);
			DebugPrintDebugBindingEntry("  ", binding);
		}
		
		if (pig->sessionDebugBindings.entries.length > 0)
		{
			PrintLine_N("%llu user debug binding%s:", pig->sessionDebugBindings.entries.length, (pig->sessionDebugBindings.entries.length == 1 ? "" : "s"));
			VarArrayLoop(&pig->sessionDebugBindings.entries, bIndex)
			{
				VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &pig->sessionDebugBindings.entries, bIndex);
				DebugPrintDebugBindingEntry("  ", binding);
			}
		}
	}
	
	// +==============================+
	// |       reload_bindings        |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "reload_bindings") == 0)
	{
		PigClearDebugBindings(&pig->debugBindings);
		GameLoadDebugBindings(&pig->debugBindings);
	}
	
	// +==============================+
	// |          app_states          |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "app_states") == 0)
	{
		PrintLine_N("%llu active AppState%s:", pig->appStateStackSize, ((pig->appStateStackSize == 1) ? "" : "s"));
		for (u64 sIndex = 0; sIndex < pig->appStateStackSize; sIndex++)
		{
			AppState_t activeAppState = pig->appStateStack[sIndex];
			PrintLine_I("[%llu]: %s", sIndex, GetAppStateStr(activeAppState));
		}
		
		WriteLine_N("Initialized AppStates:");
		u64 numInitializedAppStates = 0;
		for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
		{
			AppState_t appState = (AppState_t)aIndex;
			if (IsAppStateInitialized(appState) && !IsAppStateActive(appState))
			{
				if (numInitializedAppStates > 0)
				{
					Write_D(", ");
				}
				Print_D("%s", GetAppStateStr(appState));
				numInitializedAppStates++;
			}
		}
		if (numInitializedAppStates > 0) { WriteLine_D(""); }
		
		WriteLine_N("Uninitialized AppStates:");
		u64 numUninitializedAppStates = 0;
		for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
		{
			AppState_t appState = (AppState_t)aIndex;
			if (!IsAppStateInitialized(appState))
			{
				if (numUninitializedAppStates > 0)
				{
					Write_D(", ");
				}
				Print_D("%s", GetAppStateStr(appState));
				numUninitializedAppStates++;
			}
		}
		if (numUninitializedAppStates > 0) { WriteLine_D(""); }
	}
	
	// +==============================+
	// |        pop_app_state         |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "pop_app_state") == 0)
	{
		if (pig->appStateStackSize > 1)
		{
			PrintLine_I("Popping AppState_%s", GetAppStateStr(GetCurrentAppState()));
			PopAppState();
		}
		else
		{
			PrintLine_W("AppState_%s is the only AppState left. Can't pop!", GetAppStateStr(GetCurrentAppState()));
		}
	}
	
	// +==============================+
	// |        push_app_state        |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "push_app_state") == 0)
	{
		if (numArguments != 1) { PrintLine_E("push_app_state takes 1 argument, not %llu", numArguments); return validCommand; }
		
		AppState_t requestedAppState = AppState_None;
		for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
		{
			AppState_t appState = (AppState_t)aIndex;
			const char* appStateStr = GetAppStateStr(appState);
			if (StrCompareIgnoreCase(arguments[0], appStateStr) == 0)
			{
				requestedAppState = appState;
				break;
			}
		}
		if (requestedAppState == AppState_None)
		{
			PrintLine_E("There is no AppState \"%.*s\"", arguments[0].length, arguments[0].pntr);
			return validCommand;
		}
		
		if (!IsAppStateActive(requestedAppState))
		{
			PrintLine_I("Pushing AppState_%s", GetAppStateStr(requestedAppState));
			PushAppState(requestedAppState);
		}
		else
		{
			PrintLine_E("AppState_%s is already active! Can't push!", GetAppStateStr(requestedAppState));
			return validCommand;
		}
	}
	
	// +==============================+
	// |       change_app_state       |
	// +==============================+
	else if (StrCompareIgnoreCase(command, "change_app_state") == 0)
	{
		if (numArguments != 1) { PrintLine_E("change_app_state takes 1 argument, not %llu", numArguments); return validCommand; }
		
		AppState_t requestedAppState = AppState_None;
		for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
		{
			AppState_t appState = (AppState_t)aIndex;
			const char* appStateStr = GetAppStateStr(appState);
			if (StrCompareIgnoreCase(arguments[0], appStateStr) == 0)
			{
				requestedAppState = appState;
				break;
			}
		}
		if (requestedAppState == AppState_None)
		{
			PrintLine_E("There is no AppState \"%.*s\"", arguments[0].length, arguments[0].pntr);
			return validCommand;
		}
		
		if (!IsAppStateActive(requestedAppState))
		{
			PrintLine_I("Changing to AppState_%s from AppState_%s", GetAppStateStr(requestedAppState), GetAppStateStr(GetCurrentAppState()));
			ChangeAppState(requestedAppState);
		}
		else
		{
			PrintLine_E("AppState_%s is already active! Can't change to it!", GetAppStateStr(requestedAppState));
			return validCommand;
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

bool PigParseDebugCommand(MyStr_t commandStr) //pre-declared in pig_func_defs.h
{
	if (commandStr.length == 0) { return false; }
	
	TempPushMark();
	u64 numPieces = 0;
	MyStr_t* pieces = SplitStringBySpacesWithQuotesAndUnescape(TempArena, commandStr, &numPieces);
	Assert(numPieces > 0);
	Assert(pieces[0].pntr != nullptr);
	
	MyStr_t command = pieces[0];
	u64 numArguments = numPieces-1;
	MyStr_t* arguments = &pieces[1];
	
	if (command.length == 0) { TempPopMark(); return false; }
	
	bool validCommand = false;
	if (PigHandleDebugCommand(command, numArguments, arguments, commandStr))
	{
		validCommand = true;
	}
	else if (GameHandleDebugCommand(command, numArguments, arguments, commandStr))
	{
		validCommand = true;
	}
	else
	{
		PrintLine_E("Unknown command \"%.*s\"", command.length, command.pntr);
	}
	
	TempPopMark();
	return validCommand;
}
