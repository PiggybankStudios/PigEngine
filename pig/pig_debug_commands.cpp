/*
File:   pig_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the logic that parses the text that is entered in the inputTextbox of the debug console.
	** After some basic command checks, we pass info over to the game to handle
*/

#define DEBUG_COMMAND_DOC_TRUNCATE_LIMIT   64 //chars

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
			PrintLine_I("%s: \"%.*s\"", GetKeyStr(binding->key), StrPrint(binding->commandStr));
		} break;
		case PigDebugBindingType_Mouse:
		{
			PrintLine_I("%sMouse_%s: \"%.*s\"", indentation, GetMouseBtnStr(binding->mouseBtn), StrPrint(binding->commandStr));
		} break;
		case PigDebugBindingType_Controller:
		{
			PrintLine_I("%sController_%s: \"%.*s\"", indentation, GetControllerBtnStr(binding->controllerBtn), StrPrint(binding->commandStr));
		} break;
		default: DebugAssert(false); break;
	}
}

void AddDebugCommandDef(ExpContext_t* context, const char* funcDefString, ExpressionFunc_f* functionPntr, const char* documentation)
{
	MemArena_t* scratch = GetScratchArena(context->allocArena);
	MyStr_t errorStr = TryAddExpFuncDefByStrErrorStr(context, scratch, NewStr(funcDefString), functionPntr, NewStr(documentation));
	if (!IsEmptyStr(errorStr))
	{
		PrintLine_E("Invalid funcDef \"%s\": %.*s", funcDefString, StrPrint(errorStr));
		AssertMsg(false, "Invalid funcDef string for debug comand!");
		FreeScratchArena(scratch);
		return;
	}
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                             help                             |
// +--------------------------------------------------------------+
#define Debug_Help_Def "void help(string var_or_func_name)"
#define Debug_Help_Desc "Displays this list of commands"
void Debug_Help(MyStr_t varOrFuncName)
{
	//TODO: Implement using commandStr!
	
	MemArena_t* scratch = GetScratchArena();
	ExpContext_t context = {};
	InitDebugConsoleExpContext(scratch, &context);
	
	if (!IsEmptyStr(varOrFuncName))
	{
		bool foundMatch = false;
		VarArrayLoop(&context.variableDefs, vIndex)
		{
			VarArrayLoopGet(ExpVariableDef_t, varDef, &context.variableDefs, vIndex);
			if (StrEquals(varDef->name, varOrFuncName))
			{
				foundMatch = true;
				PrintLine_I("%s \b%.*s\b:", GetExpValueTypeStrLower(varDef->type), StrPrint(varDef->name));
				PrintLine_D("%.*s", StrPrint(varDef->documentation));
				break;
			}
		}
		VarArrayLoop(&context.functionDefs, fIndex)
		{
			VarArrayLoopGet(ExpFuncDef_t, funcDef, &context.functionDefs, fIndex);
			if (foundMatch) { break; }
			if (StrEquals(funcDef->name, varOrFuncName))
			{
				foundMatch = true;
				Print_I("%s \b%.*s\b(", GetExpValueTypeStrLower(funcDef->returnType), StrPrint(funcDef->name));
				for (u64 aIndex = 0; aIndex < funcDef->numArguments; aIndex++)
				{
					Print_I("%s%s %.*s", (aIndex > 0) ? ", " : "", GetExpValueTypeStrLower(funcDef->arguments[aIndex].type), StrPrint(funcDef->arguments[aIndex].name));
				}
				WriteLine_I(")");
				PrintLine_D("%.*s", StrPrint(funcDef->documentation));
				for (u64 aIndex = 0; aIndex < funcDef->numArguments; aIndex++)
				{
					if (!IsEmptyStr(funcDef->arguments[aIndex].documentation))
					{
						PrintLine_D("%.*s: %.*s", StrPrint(funcDef->arguments[aIndex].name), StrPrint(funcDef->arguments[aIndex].documentation));
					}
				}
				break;
			}
		}
		
		if (!foundMatch)
		{
			PrintLine_E("There is no variable or function named \"%.*s\"!", StrPrint(varOrFuncName));
		}
	}
	else
	{
		PrintLine_N("There %s %llu variable%s:", PluralEx(context.variableDefs.length, "is", "are"), context.variableDefs.length, Plural(context.variableDefs.length, "s"));
		VarArrayLoop(&context.variableDefs, vIndex)
		{
			VarArrayLoopGet(ExpVariableDef_t, varDef, &context.variableDefs, vIndex);
			bool isDocTruncated = false;
			MyStr_t documentationTruncated = varDef->documentation;
			if (documentationTruncated.length > DEBUG_COMMAND_DOC_TRUNCATE_LIMIT) { documentationTruncated.length = DEBUG_COMMAND_DOC_TRUNCATE_LIMIT-3; isDocTruncated = true; }
			PrintLine_D("  %s \b%.*s\b:\t\t%.*s%s", GetExpValueTypeStrLower(varDef->type), StrPrint(varDef->name), StrPrint(documentationTruncated), (isDocTruncated ? "..." : ""));
		}
		PrintLine_N("There %s %llu function%s:", PluralEx(context.functionDefs.length, "is", "are"), context.functionDefs.length, Plural(context.functionDefs.length, "s"));
		VarArrayLoop(&context.functionDefs, fIndex)
		{
			VarArrayLoopGet(ExpFuncDef_t, funcDef, &context.functionDefs, fIndex);
			bool isDocTruncated = false;
			MyStr_t documentationTruncated = funcDef->documentation;
			if (documentationTruncated.length > DEBUG_COMMAND_DOC_TRUNCATE_LIMIT) { documentationTruncated.length = DEBUG_COMMAND_DOC_TRUNCATE_LIMIT-3; isDocTruncated = true; }
			Print_D("  %s \b%.*s\b(", GetExpValueTypeStrLower(funcDef->returnType), StrPrint(funcDef->name));
			for (u64 aIndex = 0; aIndex < funcDef->numArguments; aIndex++)
			{
				Print_D("%s%s %.*s", (aIndex > 0) ? ", " : "", GetExpValueTypeStrLower(funcDef->arguments[aIndex].type), StrPrint(funcDef->arguments[aIndex].name));
			}
			PrintLine_D("):\t\t%.*s%s", StrPrint(documentationTruncated), (isDocTruncated ? "..." : ""));
		}
	}
	
	FreeScratchArena(scratch);
}
EXPRESSION_FUNC_DEFINITION(Debug_Help_Glue) { EXP_GET_ARG_STR(0, filterStr); Debug_Help(filterStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                            break                             |
// +--------------------------------------------------------------+
#define Debug_Break_Def "void break()"
#define Debug_Break_Desc "Runs MyDebugBreak()"
void Debug_Break()
{
	WriteLine_I("Hitting manual breakpoint...");
	MyDebugBreak();
}
EXPRESSION_FUNC_DEFINITION(Debug_Break_Glue) { Debug_Break(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                            assert                            |
// +--------------------------------------------------------------+
#define Debug_Assert_Def "void assert()"
#define Debug_Assert_Desc "Runs Assert()"
void Debug_Assert()
{
	WriteLine_I("Hitting manual assertion...");
	AssertMsg(false, "Manual assertion triggered by debug command");
	#if !GYLIB_ASSERTIONS_ENABLED
	WriteLine_W("Assertions are not enabled!");
	#endif
}
EXPRESSION_FUNC_DEFINITION(Debug_Assert_Glue) { Debug_Assert(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          arena_info                          |
// +--------------------------------------------------------------+
#define Debug_ArenaInfo_Def "void arena_info(string arena_name)"
#define Debug_ArenaInfo_Desc "Displays info about a particular memory arena"
void Debug_ArenaInfo(MyStr_t arenaName)
{
	if (StrEqualsIgnoreCase(arenaName, "Fixed") || StrEqualsIgnoreCase(arenaName, "FixedHeap"))
	{
		DebugPrintArenaInfo(&pig->fixedHeap, "FixedHeap");
	}
	else if (StrEqualsIgnoreCase(arenaName, "Main") || StrEqualsIgnoreCase(arenaName, "MainHeap"))
	{
		DebugPrintArenaInfo(&pig->mainHeap, "MainHeap");
	}
	else if (StrEqualsIgnoreCase(arenaName, "LargeAlloc") || StrEqualsIgnoreCase(arenaName, "LargeAllocHeap"))
	{
		DebugPrintArenaInfo(&pig->largeAllocHeap, "LargeAllocHeap");
	}
	else if (StrEqualsIgnoreCase(arenaName, "Std") || StrEqualsIgnoreCase(arenaName, "StdHeap"))
	{
		DebugPrintArenaInfo(platInfo->stdHeap, "stdHeap");
	}
	else if (StrEqualsIgnoreCase(arenaName, "Temp") || StrEqualsIgnoreCase(arenaName, "TempArena"))
	{
		DebugPrintArenaInfo(&pig->tempArena, "TempArena");
	}
	else if (StrEqualsIgnoreCase(arenaName, "Audio") || StrEqualsIgnoreCase(arenaName, "AudioHeap"))
	{
		DebugPrintArenaInfo(&pig->audioHeap, "AudioHeap");
	}
	else 
	{
		PrintLine_E("Unknown arena name: \"%.*s\"", StrPrint(arenaName));
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_ArenaInfo_Glue) { EXP_GET_ARG_STR(0, arenaName); Debug_ArenaInfo(arenaName); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          walk_arena                          |
// +--------------------------------------------------------------+
#define Debug_WalkArena_Def "void walk_arena(string arena_name, bool assert_on_verify_failure)"
#define Debug_WalkArena_Desc "Fires a debug breakpoint and then runs MemArenaVery on the specified arena"
void Debug_WalkArena(MyStr_t arenaName, bool assertOnVerifyFailure)
{
	if (StrEqualsIgnoreCase(arenaName, "Fixed") || StrEqualsIgnoreCase(arenaName, "FixedHeap"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->fixedHeap, assertOnVerifyFailure);
	}
	else if (StrEqualsIgnoreCase(arenaName, "Main") || StrEqualsIgnoreCase(arenaName, "MainHeap"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->mainHeap, assertOnVerifyFailure);
	}
	else if (StrEqualsIgnoreCase(arenaName, "LargeAlloc") || StrEqualsIgnoreCase(arenaName, "LargeAllocHeap"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->largeAllocHeap, assertOnVerifyFailure);
	}
	else if (StrEqualsIgnoreCase(arenaName, "Std") || StrEqualsIgnoreCase(arenaName, "StdHeap"))
	{
		MyDebugBreak();
		MemArenaVerify(platInfo->stdHeap, assertOnVerifyFailure);
	}
	else if (StrEqualsIgnoreCase(arenaName, "Temp") || StrEqualsIgnoreCase(arenaName, "TempArena"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->tempArena, assertOnVerifyFailure);
	}
	else if (StrEqualsIgnoreCase(arenaName, "Audio") || StrEqualsIgnoreCase(arenaName, "AudioHeap"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->audioHeap, assertOnVerifyFailure);
	}
	#if PIG_MAIN_ARENA_DEBUG
	else if (StrEqualsIgnoreCase(arenaName, "MainDebug") || StrEqualsIgnoreCase(arenaName, "MainHeapDebug"))
	{
		MyDebugBreak();
		MemArenaVerify(&pig->mainHeapDebug, assertOnVerifyFailure);
	}
	#endif
	else 
	{
		PrintLine_E("Unknown arena name: \"%.*s\"", StrPrint(arenaName));
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_WalkArena_Glue) { EXP_GET_ARG_STR(0, arenaName); EXP_GET_ARG_BOOL(1, assertOnVerifyFailure); Debug_WalkArena(arenaName, assertOnVerifyFailure); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                        set_time_scale                        |
// +--------------------------------------------------------------+
#define Debug_SetTimeScale_Def "void set_time_scale(r64 new_value)"
#define Debug_SetTimeScale_Desc "Fixes and scales the elapsed time that is fed into the game logic to a multiple of 16.666ms (60fps). Set to 2 to test 30 fps logic or 0.5 for 120fps logic. Set to 0 to unfix the time scale"
void Debug_SetTimeScale(r64 timeScale)
{
	if (timeScale < 0 || timeScale > 20) { PrintLine_E("Invalid scale value. Must be in range [0, 20] not %f", timeScale); return; }
	if (timeScale == 0)
	{
		if (pigOut->fixedTimeScaleEnabled)
		{
			pigOut->fixedTimeScaleEnabled = false;
			PrintLine_I("Time scale fixing disabled");
		}
		else { WriteLine_W("Time scale fixing is already disabled"); }
	}
	else
	{
		pigOut->fixedTimeScaleEnabled = true;
		pigOut->fixedTimeScale = (r64)timeScale;
		r64 fixedElapsedMs = PIG_DEFAULT_FRAME_TIME * pigOut->fixedTimeScale;
		PrintLine_I("Time scale fixing set to %lgx (%lgfps logic)", pigOut->fixedTimeScale, 1000.0 / fixedElapsedMs);
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_SetTimeScale_Glue) { EXP_GET_ARG_R64(0, timeScale); Debug_SetTimeScale(timeScale); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                       bump_time_scale                        |
// +--------------------------------------------------------------+
#define Debug_BumpTimeScale_Def "r64 bump_time_scale(string up_or_down)"
#define Debug_BumpTimeScale_Desc "Changes the time scale up (faster) or down (slower) using a predetermined set of distinct values: [1/100, 1/20, 1/10, 1/4, 1/2, 3/4, 1, 1.2x, 1.5x, 2x, 5x, 10x, 20x]"
r64 Debug_BumpTimeScale(MyStr_t upOrDownStr)
{
	r32 timeScaleOptions[] = { 0.01f, 0.05f, 0.1f, 0.25f, 0.5f, 0.75f, 1.0f, 1.2f, 1.5f, 2.0f, 5.0f, 10.0f, 20.0f };
	
	r32 requestedScale = 1.0f;
	if (StrEqualsIgnoreCase(upOrDownStr, "up"))
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
		if (requestedScale == 1.0f) { requestedScale = 0; }
	}
	else if (StrEqualsIgnoreCase(upOrDownStr, "down"))
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
		if (requestedScale == 1.0f) { requestedScale = 0; }
	}
	else if (StrEqualsIgnoreCase(upOrDownStr, "reset"))
	{
		requestedScale = 0.0f;
	}
	else
	{
		PrintLine_E("Please pass either \"up\", \"down\" or \"reset\". Not \"%.*s", StrPrint(upOrDownStr));
		return (pigOut->fixedTimeScaleEnabled ? pigOut->fixedTimeScale : 1.0);
	}
	
	if (requestedScale == 0.0f)
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
	
	return (pigOut->fixedTimeScaleEnabled ? pigOut->fixedTimeScale : 1.0);
}
EXPRESSION_FUNC_DEFINITION(Debug_BumpTimeScale_Glue) { EXP_GET_ARG_STR(0, upOrDownStr); return NewExpValueR64(Debug_BumpTimeScale(upOrDownStr)); }

// +--------------------------------------------------------------+
// |                            reload                            |
// +--------------------------------------------------------------+
#define Debug_Reload_Def "void reload(string resource_name, string resource_type)"
#define Debug_Reload_Desc "Reloads a specific resource"
void Debug_Reload(MyStr_t targetName, MyStr_t resourceType)
{
	if (StrEqualsIgnoreCase(targetName, "all"))
	{
		WriteLine_I("Reloading all resources...");
		Pig_LoadAllResources();
		return;
	}
	
	ResourceType_t targetType = ResourceType_None;
	if (!IsEmptyStr(resourceType))
	{
		for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
		{
			if (StrEqualsIgnoreCase(resourceType, GetResourceTypeStr((ResourceType_t)tIndex)))
			{
				targetType = (ResourceType_t)tIndex;
				break;
			}
		}
		if (targetType == ResourceType_None)
		{
			PrintLine_E("Unknown resource type given: \"%.*s\"", StrPrint(resourceType));
			return;
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
				if (StrEqualsIgnoreCase(resourceName, targetName))
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
		PrintLine_E("Unknown resource name \"%.*s\" for %s resource", StrPrint(targetName), (targetType == ResourceType_None ? "Any" : GetResourceTypeStr(targetType)));
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_Reload_Glue) { EXP_GET_ARG_STR(0, targetName); EXP_GET_ARG_STR(1, resourceType); Debug_Reload(targetName, resourceType); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          resources                           |
// +--------------------------------------------------------------+
#define Debug_Resources_Def "void resources(string resource_type)"
#define Debug_Resources_Desc "List all resources (or all by a specific type)"
void Debug_Resources(MyStr_t resourceType)
{
	ResourceType_t targetType = ResourceType_None;
	if (!IsEmptyStr(resourceType))
	{
		for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
		{
			if (StrEqualsIgnoreCase(resourceType, GetResourceTypeStr((ResourceType_t)tIndex)))
			{
				targetType = (ResourceType_t)tIndex;
				break;
			}
		}
		if (targetType == ResourceType_None)
		{
			PrintLine_E("Unknown resource type given for argument 1: \"%.*s\"", StrPrint(resourceType));
			return;
		}
	}
	
	for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
	{
		ResourceType_t type = (ResourceType_t)tIndex;
		if (type != ResourceType_None && (targetType == ResourceType_None || targetType == type))
		{
			u64 numOfType = GetNumResourcesOfType(type);
			PrintLine_N("%llu %s Resource%s:", numOfType, GetResourceTypeStr(type), Plural(numOfType, "s"));
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
				PrintLineAt(dbgLevel, "%s[%llu]: \"%.*s\"", GetResourceTypeStr(type), rIndex, StrPrint(resourceName));
			}
		}
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_Resources_Glue) { EXP_GET_ARG_STR(0, resourceType); Debug_Resources(resourceType); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                           watches                            |
// +--------------------------------------------------------------+
#define Debug_Watches_Def "void watches(string resource_type)"
#define Debug_Watches_Desc "List all file watches that are active for resources"
void Debug_Watches(MyStr_t resourceType)
{
	#if DEVELOPER_BUILD
	ResourceType_t targetType = ResourceType_None;
	if (!IsEmptyStr(resourceType))
	{
		for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
		{
			if (StrEqualsIgnoreCase(resourceType, GetResourceTypeStr((ResourceType_t)tIndex)))
			{
				targetType = (ResourceType_t)tIndex;
				break;
			}
		}
		if (targetType == ResourceType_None)
		{
			PrintLine_E("Unknown resource type given for argument 1: \"%.*s\"", StrPrint(resourceType));
			return;
		}
	}
	
	PrintLine_N("There are %llu watches active", pig->resources.watches.length);
	VarArrayLoop(&pig->resources.watches, wIndex)
	{
		VarArrayLoopGet(ResourceWatch_t, watch, &pig->resources.watches, wIndex);
		if (targetType == watch->type || targetType == ResourceType_None)
		{
			bool doesFileExist = plat->DoesFileExist(watch->watchedFile->path, nullptr);
			PrintLineAt(doesFileExist ? DbgLevel_Info : DbgLevel_Warning, "\t%s[%llu]: \"%.*s\"", GetResourceTypeStr(watch->type), watch->resourceIndex, StrPrint(watch->watchedFile->path));
		}
	}
	#else //DEVELOPER_MODE
	WriteLine_E("This application is not compiled in DEVELOPER_MODE so we aren't watching any files");
	#endif
}
EXPRESSION_FUNC_DEFINITION(Debug_Watches_Glue) { EXP_GET_ARG_STR(0, resourceType); Debug_Watches(resourceType); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                             mute                             |
// +--------------------------------------------------------------+
#define Debug_Mute_Def "void mute(string sound_type)"
#define Debug_Mute_Desc "Sets Master Volume to 0 (or disables music or sounds if argument is passed)"
void Debug_Mute(MyStr_t soundType)
{
	if (StrEqualsIgnoreCase(soundType, "music"))
	{
		if (plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE))
		{
			pig->musicEnabled = !pig->musicEnabled;
			plat->UnlockMutex(&pig->volumeMutex);
			PrintLine_I("Music %s", pig->musicEnabled ? "Enabled" : "Disabled");
		}
	}
	else if (StrEqualsIgnoreCase(soundType, "sounds") || StrEqualsIgnoreCase(soundType, "sound"))
	{
		if (plat->LockMutex(&pig->volumeMutex, MUTEX_LOCK_INFINITE))
		{
			pig->soundsEnabled = !pig->soundsEnabled;
			plat->UnlockMutex(&pig->volumeMutex);
			PrintLine_I("Sounds %s", pig->soundsEnabled ? "Enabled" : "Disabled");
		}
	}
	else if (IsEmptyStr(soundType))
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
		PrintLine_E("Unknown argument \"%.*s\"! Known values: \"music\", \"sounds\"", StrPrint(soundType));
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_Mute_Glue) { EXP_GET_ARG_STR(0, soundType); Debug_Mute(soundType); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                           volumes                            |
// +--------------------------------------------------------------+
#define Debug_Volumes_Def "void volumes()"
#define Debug_Volumes_Desc "Prints out the current audio volume settings"
void Debug_Volumes()
{
	PrintLine_I("Master Volume: %.0f", pig->masterVolume*100);
	PrintLine_I("Music Volume: %.0f%s", pig->musicVolume*100, (pig->musicEnabled ? "" : " (Disabled)"));
	PrintLine_I("Sounds Volume: %.0f%s", pig->soundsVolume*100, (pig->soundsEnabled ? "" : " (Disabled)"));
}
EXPRESSION_FUNC_DEFINITION(Debug_Volumes_Glue) { Debug_Volumes(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          set_volume                          |
// +--------------------------------------------------------------+
#define Debug_SetVolume_Def "void set_volume(string sound_type, r32 volume)"
#define Debug_SetVolume_Desc "Changes the specified volume to the specified value"
void Debug_SetVolume(MyStr_t soundType, r32 volume)
{
	if (volume < 0 || volume > 100)
	{
		PrintLine_E("Volume must be in range [0, 100], not %g", volume);
		return;
	}
	
	if (StrEqualsIgnoreCase(soundType, "master"))
	{
		PrintLine_I("Master volume set to %g (was %.0f)", volume, pig->masterVolume*100);
		pig->masterVolume = volume/100.0f;
	}
	else if (StrEqualsIgnoreCase(soundType, "music"))
	{
		PrintLine_I("Music volume set to %g (was %.0f)", volume, pig->musicVolume*100);
		pig->musicVolume = volume/100.0f;
	}
	else if (StrEqualsIgnoreCase(soundType, "sounds") || StrEqualsIgnoreCase(soundType, "sound"))
	{
		PrintLine_I("Sounds volume set to %g (was %.0f)", volume, pig->soundsVolume*100);
		pig->soundsVolume = volume/100.0f;
	}
	else
	{
		PrintLine_E("Unknown volume type \"%.*s\"", StrPrint(soundType));
		return;
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_SetVolume_Glue) { EXP_GET_ARG_STR(0, soundType); EXP_GET_ARG_R32(1, volume); Debug_SetVolume(soundType, volume); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                        set_resolution                        |
// +--------------------------------------------------------------+
#define Debug_SetResolution_Def "void set_resolution(i32 width, i32 height)"
#define Debug_SetResolution_Desc "Changes the resolution of the window"
void Debug_SetResolution(i32 width, i32 height)
{
	const PlatWindow_t* window = platInfo->mainWindow;
	NotNull(window);
	const PlatMonitorInfo_t* monitor = GetCurrentMonitorInfoForWindow(window);
	NotNull(monitor);
	Assert(monitor->currentVideoModeIndex < monitor->videoModes.length);
	const PlatMonitorVideoMode_t* videoMode = VarArrayGet(&monitor->videoModes, monitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
	
	v2i resolution = NewVec2i(width, height);
	if (resolution.width < PIG_WINDOW_MIN_SIZE.width || resolution.height < PIG_WINDOW_MIN_SIZE.height)
	{
		PrintLine_E("Invalid resolution %dx%d. Our minimum size is %dx%d", resolution.width, resolution.height, PIG_WINDOW_MIN_SIZE.width, PIG_WINDOW_MIN_SIZE.height);
		return;
	}
	if (resolution.width > videoMode->resolution.width) { PrintLine_W("Limiting width to %d", videoMode->resolution.width); resolution.width = videoMode->resolution.width; }
	if (resolution.height > videoMode->resolution.height) { PrintLine_W("Limiting height to %d", videoMode->resolution.height); resolution.height = videoMode->resolution.height; }
	
	pigOut->moveWindow = true;
	pigOut->moveWindowId = window->id;
	pigOut->moveWindowRec = NewReci(window->input.desktopInnerRec.topLeft, resolution);
	PrintLine_I("Moving window to (%d, %d, %d, %d)", pigOut->moveWindowRec.x, pigOut->moveWindowRec.y, pigOut->moveWindowRec.width, pigOut->moveWindowRec.height);
}
EXPRESSION_FUNC_DEFINITION(Debug_SetResolution_Glue) { EXP_GET_ARG_I32(0, width); EXP_GET_ARG_I32(1, height); Debug_SetResolution(width, height); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         video_modes                          |
// +--------------------------------------------------------------+
#define Debug_VideoModes_Def "void video_modes(u64 monitor_number)"
#define Debug_VideoModes_Desc "Lists all the video modes for all monitors (or a specific monitor if a monitor number is given)"
void Debug_VideoModes(u64 monitorNumber)
{
	bool foundMonitor = false;
	const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
	for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
	{
		if (monitorNumber == 0 || monitorInfo->designatedNumber == monitorNumber)
		{
			foundMonitor = true;
			PrintLine_N("Monitor %llu \"%.*s\" supports %llu video mode%s:",
				monitorInfo->designatedNumber,
				StrPrint(monitorInfo->name),
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
		return;
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_VideoModes_Glue) { EXP_GET_ARG_U64(0, monitorNumber); Debug_VideoModes(monitorNumber); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          fullscreen                          |
// +--------------------------------------------------------------+
#define Debug_Fullscreen_Def "void fullscreen(bool enable, i32 width, i32 height, i64 framerate, u64 monitor_number)"
#define Debug_Fullscreen_Desc "Enables or disables fullscreen and also sets resolution and framerate"
void Debug_Fullscreen(bool fullscreenEnabled, i32 width, i32 height, i64 framerate, u64 monitorNumber)
{
	const PlatWindow_t* window = platInfo->mainWindow;
	NotNull(window);
	
	v2i resolution = NewVec2i(width, height);
	if (framerate < 10 || framerate > 300) { PrintLine_E("Invalid framerate requested. Must be in range [10, 300]: %lld", framerate); return; }
	
	if (fullscreenEnabled)
	{
		const PlatMonitorInfo_t* targetMonitor = GetMonitorInfoByNumber(monitorNumber);
		if (targetMonitor == nullptr)
		{
			PrintLine_E("There are no monitors with number %llu (There are %llu monitors available)", monitorNumber, platInfo->monitors->list.count);
			return;
		}
		const PlatMonitorVideoMode_t* targetVideoMode = GetVideoModeWithResolution(targetMonitor, resolution);
		if (targetVideoMode == nullptr)
		{
			PrintLine_E("The target monitor does not support resolution %dx%d", resolution.width, resolution.height); 
			return;
		}
		i64 targetFramerateIndex = FindVideoModeFramerateIndex(targetVideoMode, framerate);
		if (targetFramerateIndex < 0)
		{
			PrintLine_E("The target monitor does not support %lldHz at %dx%d", framerate, resolution.width, resolution.height);
			return;
		}
		
		DoFullscreenOnMonitor(window, targetMonitor, resolution, framerate, true);
	}
	else
	{
		// const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
		// const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
		if (resolution.width < PIG_WINDOW_MIN_SIZE.width || resolution.height < PIG_WINDOW_MIN_SIZE.height)
		{
			PrintLine_E("Invalid resolution %dx%d. Our minimum size is %dx%d", resolution.width, resolution.height, PIG_WINDOW_MIN_SIZE.width, PIG_WINDOW_MIN_SIZE.height);
			return;
		}
		
		StopFullscreen(window, resolution, framerate, true);
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_Fullscreen_Glue) { EXP_GET_ARG_BOOL(0, fullscreenEnabled); EXP_GET_ARG_I32(1, width); EXP_GET_ARG_I32(2, height); EXP_GET_ARG_I64(3, framerate); EXP_GET_ARG_U64(4, monitorNumber); Debug_Fullscreen(fullscreenEnabled, width, height, framerate, monitorNumber); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                      toggle_fullscreen                       |
// +--------------------------------------------------------------+
#define Debug_ToggleFullscreen_Def "bool toggle_fullscreen()"
#define Debug_ToggleFullscreen_Desc "Toggles fullscreen on/off"
bool Debug_ToggleFullscreen()
{
	ToggleFullscreen(true);
	return !pig->currentWindow->input.fullscreen;
}
EXPRESSION_FUNC_DEFINITION(Debug_ToggleFullscreen_Glue) { return NewExpValueBool(Debug_ToggleFullscreen()); }

// +--------------------------------------------------------------+
// |                          thread_ids                          |
// +--------------------------------------------------------------+
#define Debug_ThreadIds_Def "void thread_ids()"
#define Debug_ThreadIds_Desc "Prints out the IDs of all the threads related this application is currently running"
void Debug_ThreadIds()
{
	PrintLine_I("MainThread:         %llu (%lld or 0x%08X)", platInfo->mainThreadId.internalId, (i64)platInfo->mainThreadId.osId, (i64)platInfo->mainThreadId.osId);
	PrintLine_I("FileWatchingThread: %llu (%lld or 0x%08X)", platInfo->fileWatchingThreadId.internalId, (i64)platInfo->fileWatchingThreadId.osId, (i64)platInfo->fileWatchingThreadId.osId);
	PrintLine_I("AudioThread:        %llu (%lld or 0x%08X)", platInfo->audioThreadId.internalId, (i64)platInfo->audioThreadId.osId, (i64)platInfo->audioThreadId.osId);
	for (u64 tIndex = 0; tIndex < platInfo->numThreadPoolThreads; tIndex++)
	{
		PrintLine_I("ThreadPool[%llu]:      %llu (%lld or 0x%08X)", tIndex, platInfo->threadPoolIds[tIndex].internalId, (i64)platInfo->threadPoolIds[tIndex].osId, (i64)platInfo->threadPoolIds[tIndex].osId);
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_ThreadIds_Glue) { Debug_ThreadIds(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                             bind                             |
// +--------------------------------------------------------------+
#define Debug_Bind_Def "void bind(string binding, string command)"
#define Debug_Bind_Desc "Creates a custom binding that will run a debug command when a specific binding is performed"
void Debug_Bind(MyStr_t bindingStr, MyStr_t commandStr)
{
	PigDebugBindingsEntry_t binding = {};
	if (!PigTryDeserBindingStr(bindingStr, &binding))
	{
		PrintLine_E("Couldn't parse \"%.*s\" as binding", StrPrint(bindingStr));
		return;
	}
	
	//TODO: We should make sure the command is parsable as an expression!
	
	switch (binding.type)
	{
		case PigDebugBindingType_Keyboard:
		{
			PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryKey(&pig->sessionDebugBindings, true, binding.modifiers, binding.key, commandStr);
			NotNull(newEntry);
			WriteLine_I("Keyboard binding created!");
		} break;
		case PigDebugBindingType_Mouse:
		{
			PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryMouse(&pig->sessionDebugBindings, true, binding.mouseBtn, commandStr);
			NotNull(newEntry);
			WriteLine_I("Mouse binding created!");
		} break;
		case PigDebugBindingType_Controller:
		{
			PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryController(&pig->sessionDebugBindings, true, binding.controllerBtn, commandStr);
			NotNull(newEntry);
			WriteLine_I("Controller binding created!");
		} break;
		default: DebugAssert(false); break;
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_Bind_Glue) { EXP_GET_ARG_STR(0, bindingStr); EXP_GET_ARG_STR(1, commandStr); Debug_Bind(bindingStr, commandStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                            unbind                            |
// +--------------------------------------------------------------+
#define Debug_Unbind_Def "void unbind(string binding)"
#define Debug_Unbind_Desc "Removes a previously set custom binding"
void Debug_Unbind(MyStr_t bindingStr)
{
	PigDebugBindingsEntry_t binding = {};
	if (!PigTryDeserBindingStr(bindingStr, &binding))
	{
		PrintLine_E("Couldn't parse \"%.*s\" as binding", StrPrint(bindingStr));
		return;
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
EXPRESSION_FUNC_DEFINITION(Debug_Unbind_Glue) { EXP_GET_ARG_STR(0, bindingStr); Debug_Unbind(bindingStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                           bindings                           |
// +--------------------------------------------------------------+
#define Debug_Bindings_Def "void bindings()"
#define Debug_Bindings_Desc "Prints out the list of debug bindings"
void Debug_Bindings()
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
EXPRESSION_FUNC_DEFINITION(Debug_Bindings_Glue) { Debug_Bindings(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                       reload_bindings                        |
// +--------------------------------------------------------------+
#define Debug_ReloadBindings_Def "void reload_bindings()"
#define Debug_ReloadBindings_Desc "Reloads the debug bindings from the serialized locations"
void Debug_ReloadBindings()
{
	PigClearDebugBindings(&pig->debugBindings);
	GameLoadDebugBindings(&pig->debugBindings);
}
EXPRESSION_FUNC_DEFINITION(Debug_ReloadBindings_Glue) { Debug_ReloadBindings(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                          app_states                          |
// +--------------------------------------------------------------+
#define Debug_AppStates_Def "void app_states()"
#define Debug_AppStates_Desc "Prints out the AppState stack and also the names of all appstates that aren't active for reference"
void Debug_AppStates()
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
EXPRESSION_FUNC_DEFINITION(Debug_AppStates_Glue) { Debug_AppStates(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                        pop_app_state                         |
// +--------------------------------------------------------------+
#define Debug_PopAppState_Def "void pop_app_state()"
#define Debug_PopAppState_Desc "Pops the current AppState off the stack (unless it's the last AppState on the stack)"
void Debug_PopAppState()
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
EXPRESSION_FUNC_DEFINITION(Debug_PopAppState_Glue) { Debug_PopAppState(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                        push_app_state                        |
// +--------------------------------------------------------------+
#define Debug_PushAppState_Def "void push_app_state(string app_state)"
#define Debug_PushAppState_Desc "Pushes a specified AppState onto the stack (unless it is already active in the stack)"
void Debug_PushAppState(MyStr_t appStateStr)
{
	AppState_t requestedAppState = AppState_None;
	for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
	{
		AppState_t appState = (AppState_t)aIndex;
		const char* potentialAppStateStr = GetAppStateStr(appState);
		if (StrEqualsIgnoreCase(appStateStr, potentialAppStateStr))
		{
			requestedAppState = appState;
			break;
		}
	}
	if (requestedAppState == AppState_None)
	{
		PrintLine_E("There is no AppState \"%.*s\"", StrPrint(appStateStr));
		return;
	}
	
	if (!IsAppStateActive(requestedAppState))
	{
		PrintLine_I("Pushing AppState_%s", GetAppStateStr(requestedAppState));
		PushAppState(requestedAppState);
	}
	else
	{
		PrintLine_E("AppState_%s is already active! Can't push!", GetAppStateStr(requestedAppState));
		return;
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_PushAppState_Glue) { EXP_GET_ARG_STR(0, appStateStr); Debug_PushAppState(appStateStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                       change_app_state                       |
// +--------------------------------------------------------------+
#define Debug_ChangeAppState_Def "void change_app_state(string app_state)"
#define Debug_ChangeAppState_Desc "Swaps out the top AppState with a specified state (unless it is already active in the stack)"
void Debug_ChangeAppState(MyStr_t appStateStr)
{
	AppState_t requestedAppState = AppState_None;
	for (u64 aIndex = 1; aIndex < AppState_NumStates; aIndex++)
	{
		AppState_t appState = (AppState_t)aIndex;
		const char* potentialAppStateStr = GetAppStateStr(appState);
		if (StrEqualsIgnoreCase(appStateStr, potentialAppStateStr))
		{
			requestedAppState = appState;
			break;
		}
	}
	if (requestedAppState == AppState_None)
	{
		PrintLine_E("There is no AppState \"%.*s\"", StrPrint(appStateStr));
		return;
	}
	
	if (!IsAppStateActive(requestedAppState))
	{
		PrintLine_I("Changing to AppState_%s from AppState_%s", GetAppStateStr(requestedAppState), GetAppStateStr(GetCurrentAppState()));
		ChangeAppState(requestedAppState);
	}
	else
	{
		PrintLine_E("AppState_%s is already active! Can't change to it!", GetAppStateStr(requestedAppState));
		return;
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_ChangeAppState_Glue) { EXP_GET_ARG_STR(0, appStateStr); Debug_ChangeAppState(appStateStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                    regen_basic_resources                     |
// +--------------------------------------------------------------+
#define Debug_RegenBasicResources_Def "void regen_basic_resources()"
#define Debug_RegenBasicResources_Desc "Regenerates the \"basic\" resources used by the render context (like the primitive shape vertex buffers)"
void Debug_RegenBasicResources()
{
	WriteLine_D("Reloading render context basic resources");
	RcLoadBasicResources();
}
EXPRESSION_FUNC_DEFINITION(Debug_RegenBasicResources_Glue) { Debug_RegenBasicResources(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                           sizeofs                            |
// +--------------------------------------------------------------+
#define Debug_Sizeofs_Def "void sizeofs()"
#define Debug_Sizeofs_Desc "Prints out some byte sized for various structures and arrays that we may want to monitor"
void Debug_Sizeofs()
{
	PrintLine_D("sizeof(PigState_t) = %s", FormatBytesNt(sizeof(PigState_t), TempArena));
	PrintLine_D("sizeof(RenderContext_t) = %s", FormatBytesNt(sizeof(RenderContext_t), TempArena));
	PrintLine_D("DBG_CONSOLE_BUFFER_SIZE = %s", FormatBytesNt(DBG_CONSOLE_BUFFER_SIZE, TempArena));
	PrintLine_D("DBG_CONSOLE_BUILD_SPACE_SIZE = %s", FormatBytesNt(DBG_CONSOLE_BUILD_SPACE_SIZE, TempArena));
	PrintLine_D("sizeof(DebugConsole_t) = %s", FormatBytesNt(sizeof(DebugConsole_t), TempArena));
	PrintLine_D("sizeof(PigNotificationQueue_t) = %s", FormatBytesNt(sizeof(PigNotificationQueue_t), TempArena));
	PrintLine_D("sizeof(SoundInstance_t) = %s (x%llu)", FormatBytesNt(sizeof(SoundInstance_t), TempArena), (u64)PIG_MAX_SOUND_INSTANCES);
	PrintLine_D("sizeof(PigDebugOverlay_t) = %s", FormatBytesNt(sizeof(PigDebugOverlay_t), TempArena));
	PrintLine_D("sizeof(PigPerfGraph_t) = %s", FormatBytesNt(sizeof(PigPerfGraph_t), TempArena));
	PrintLine_D("sizeof(PigMemGraph_t) = %s", FormatBytesNt(sizeof(PigMemGraph_t), TempArena));
	PrintLine_D("sizeof(PigAudioOutGraph_t) = %s", FormatBytesNt(sizeof(PigAudioOutGraph_t), TempArena));
	PrintLine_D("sizeof(Vertex2D_t) = %s", FormatBytesNt(sizeof(Vertex2D_t), TempArena));
	PrintLine_D("sizeof(Vertex3D_t) = %s", FormatBytesNt(sizeof(Vertex3D_t), TempArena));
}
EXPRESSION_FUNC_DEFINITION(Debug_Sizeofs_Glue) { Debug_Sizeofs(); return NewExpValueVoid(); }

#if STEAM_BUILD

// +--------------------------------------------------------------+
// |                        steam_friends                         |
// +--------------------------------------------------------------+
#define Debug_SteamFriends_Def "void steam_friends()"
#define Debug_SteamFriends_Desc "Exercises the steam friend list enumeration APIs"
void Debug_SteamFriends()
{
	plat->StartSteamFriendsQuery();
	if (platInfo->steamFriendsList->lastQuerySuccessTime != 0)
	{
		PrintLine_N("You have %llu friend%s (as of %s ago):",
			platInfo->steamFriendsList->friends.length,
			(platInfo->steamFriendsList->friends.length == 1) ? "" : "s",
			FormatMillisecondsNt(TimeSince(platInfo->steamFriendsList->lastQuerySuccessTime), TempArena)
		);
		VarArrayLoop(&platInfo->steamFriendsList->friends, fIndex)
		{
			VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &platInfo->steamFriendsList->friends, fIndex);
			DbgLevel_t dbgLevel = DbgLevel_Debug;
			if (friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Online) { dbgLevel = DbgLevel_Info; }
			if (friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Away || friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Snooze) { dbgLevel = DbgLevel_Warning; }
			PrintLineAt(dbgLevel,
				"  Friend[%llu]: ID: %llu \"%.*s%s%.*s%s\" %s %llu group%s%s",
				fIndex,
				friendInfo->id,
				StrPrint(friendInfo->name),
				(friendInfo->nickname.length > 0) ? " (" : "",
				StrPrint(friendInfo->nickname),
				(friendInfo->nickname.length > 0) ? ")" : "",
				GetPlatSteamFriendOnlineStatusStr(friendInfo->onlineStatus),
				friendInfo->numGroups, (friendInfo->numGroups == 1) ? "" : "s",
				(friendInfo->state == PlatSteamFriendState_Incoming) ? " (Incoming)" : ((friendInfo->state == PlatSteamFriendState_Outgoing) ? " (Outgoing)" : "")
			);
			if (friendInfo->avatarSize != PlatSteamFriendAvatarSize_None)
			{
				PrintLine_N("    Avatar %s is %dx%d (%s)",
					GetPlatSteamFriendAvatarSizeStr(friendInfo->avatarSize),
					friendInfo->avatarImageData.width, friendInfo->avatarImageData.height,
					FormatBytesNt(friendInfo->avatarImageData.dataSize, TempArena)
				);
			}
			if (friendInfo->isInGame)
			{
				// "APICO" ID=1390190
				// "Mx Bikes" ID=655500
				// friendInfo->inGameLobbySteamId
				PrintLine_N("    In-Game: AppId %llu IP 0x%08X Port %u (Query Port %u)",
					friendInfo->inGameSteamAppId,
					friendInfo->inGameIP,
					friendInfo->inGamePort,
					friendInfo->inGameQueryPort
				);
			}
			if (friendInfo->presenceStrs.length > 0)
			{
				Print_D("    Presence %llux:", friendInfo->presenceStrs.length);
				VarArrayLoop(&friendInfo->presenceStrs, sIndex)
				{
					VarArrayLoopGet(PlatSteamFriendPresenceStr_t, presenceStr, &friendInfo->presenceStrs, sIndex);
					Print_D(" \"%.*s\"=\"%.*s\"", StrPrint(presenceStr->key), StrPrint(presenceStr->value));
				}
				WriteLine_D("");
			}
		}
		PrintLine_N("You have %llu friend group%s:",
			platInfo->steamFriendsList->groups.length,
			(platInfo->steamFriendsList->groups.length == 1) ? "" : "s"
		);
		VarArrayLoop(&platInfo->steamFriendsList->groups, gIndex)
		{
			VarArrayLoopGet(PlatSteamFriendGroup_t, group, &platInfo->steamFriendsList->groups, gIndex);
			PrintLine_I("  Group[%llu]: \"%.*s\" (%llu member%s)", gIndex, StrPrint(group->name), group->memberIds.length, (group->memberIds.length == 1) ? "" : "s");
			VarArrayLoop(&group->memberIds, mIndex)
			{
				VarArrayLoopGet(u64, memberIdPntr, &group->memberIds, mIndex);
				PlatSteamFriendInfo_t* friendInfo = plat->GetSteamFriendInfoById(*memberIdPntr);
				NotNull(friendInfo);
				DbgLevel_t dbgLevel = DbgLevel_Debug;
				if (friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Online) { dbgLevel = DbgLevel_Info; }
				if (friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Away || friendInfo->onlineStatus == PlatSteamFriendOnlineStatus_Snooze) { dbgLevel = DbgLevel_Warning; }
				PrintLineAt(dbgLevel,
					"    Member[%llu]: \"%.*s%s%.*s%s\" %s",
					mIndex,
					StrPrint(friendInfo->name),
					(friendInfo->nickname.length > 0) ? " (" : "",
					StrPrint(friendInfo->nickname),
					(friendInfo->nickname.length > 0) ? ")" : "",
					GetPlatSteamFriendOnlineStatusStr(friendInfo->onlineStatus)
				);
			}
		}
}
	else
	{
		WriteLine_D("Run this command again. If the query succeeded, we should have a valid friends list");
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_SteamFriends_Glue) { Debug_SteamFriends(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                     update_friend_status                     |
// +--------------------------------------------------------------+
#define Debug_UpdateFriendStatus_Def "void update_friend_status()"
#define Debug_UpdateFriendStatus_Desc "Manually requests a refresh on the status of all steam friends"
void Debug_UpdateFriendStatus()
{
	if (platInfo->steamFriendsList->lastQuerySuccessTime == 0) { WriteLine_E("We don't have the steam friends list right now!"); return; }
	WriteLine_I("Refreshing steam friend statuses");
	plat->UpdateSteamFriendStatus();
}
EXPRESSION_FUNC_DEFINITION(Debug_UpdateFriendStatus_Glue) { Debug_UpdateFriendStatus(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                      get_friend_avatar                       |
// +--------------------------------------------------------------+
#define Debug_GetFriendAvatar_Def "void get_friend_avatar(string friend_name)"
#define Debug_GetFriendAvatar_Desc "Manually requests the avatar image of a specific steam friend"
void Debug_GetFriendAvatar(MyStr_t friendName)
{
	if (platInfo->steamFriendsList->lastQuerySuccessTime == 0) { WriteLine_E("We don't have the steam friends list right now!"); return; }
	
	PlatSteamFriendInfo_t* friendInfo = nullptr;
	VarArrayLoop(&platInfo->steamFriendsList->friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, iterFriendInfo, &platInfo->steamFriendsList->friends, fIndex);
		if (StrEqualsIgnoreCase(iterFriendInfo->name, friendName))
		{
			friendInfo = iterFriendInfo;
			break;
		}
	}
	if (friendInfo == nullptr)
	{
		PrintLine_E("Unknown friend \"%.*s\"", StrPrint(friendName));
		return;
	}
	
	if (plat->RequestSteamFriendAvatar(friendInfo->id, PlatSteamFriendAvatarSize_Large))
	{
		PrintLine_I("Requesting friend %llu avatar...", friendInfo->id);
	}
	else
	{
		PrintLine_E("Failed to request friend %llu avatar!", friendInfo->id);
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_GetFriendAvatar_Glue) { EXP_GET_ARG_STR(0, friendName); Debug_GetFriendAvatar(friendName); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                     clear_friend_avatars                     |
// +--------------------------------------------------------------+
#define Debug_ClearFriendAvatars_Def "void clear_friend_avatars()"
#define Debug_ClearFriendAvatars_Desc "Clears the steam friend avatar texture cache"
void Debug_ClearFriendAvatars()
{
	u64 numAvatars = pig->steamAvatars.length;
	VarArrayLoop(&pig->steamAvatars, aIndex)
	{
		VarArrayLoopGet(SteamAvatar_t, steamAvatar, &pig->steamAvatars, aIndex);
		DestroyTexture(&steamAvatar->texture);
	}
	VarArrayClear(&pig->steamAvatars);
	PrintLine_I("Cleared %llu avatar%s", numAvatars, (numAvatars == 1) ? "" : "s");
}
EXPRESSION_FUNC_DEFINITION(Debug_ClearFriendAvatars_Glue) { Debug_ClearFriendAvatars(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                      show_friends_list                       |
// +--------------------------------------------------------------+
#define Debug_ShowFriendsList_Def "void show_friends_list()"
#define Debug_ShowFriendsList_Desc "Toggles showing of a debug overlay of all your steam friends"
void Debug_ShowFriendsList()
{
	pig->debugRenderSteamFriendsList = !pig->debugRenderSteamFriendsList;
	if (platInfo->steamFriendsList->lastQueryAttemptTime == 0)
	{
		WriteLine_W("Kicking off first steam friends query...");
		plat->StartSteamFriendsQuery();
	}
	PrintLine_I("Friends List %s", pig->debugRenderSteamFriendsList ? "Shown" : "Hidden");
}
EXPRESSION_FUNC_DEFINITION(Debug_ShowFriendsList_Glue) { Debug_ShowFriendsList(); return NewExpValueVoid(); }

#endif //STEAM_BUILD

// +--------------------------------------------------------------+
// |                      list_resource_pool                      |
// +--------------------------------------------------------------+
#define Debug_ListResourcePool_Def "void list_resource_pool(string resource_type)"
#define Debug_ListResourcePool_Desc "Lists information about all resources in a pool (optionally filtered to a specific type of resource)"
void Debug_ListResourcePool(MyStr_t resourceTypeStr)
{
	ResourcePool_t* resourcePool = GameGetCurrentResourcePool();
	if (resourcePool == nullptr)
	{
		WriteLine_E("There is no currently active resourcePool to list!");
		return;
	}
	
	ResourceType_t targetResourceType = ResourceType_None;
	if (!IsEmptyStr(resourceTypeStr))
	{
		if (!TryParseEnum(resourceTypeStr, &targetResourceType, ResourceType_NumTypes, GetResourceTypeStr)) { PrintLine_E("Unknown resource type \"%.*s\"", StrPrint(resourceTypeStr)); return; }
	}
	
	for (u64 tIndex = 1; tIndex < ResourceType_NumTypes; tIndex++)
	{
		ResourceType_t resourceType = (ResourceType_t)tIndex;
		if (targetResourceType == ResourceType_None || resourceType == targetResourceType)
		{
			BktArray_t* resourceArray = &resourcePool->arrays[tIndex];
			u64 resourceCount = resourcePool->resourceCounts[tIndex];
			PrintLine_N("%llu %s Resource%s", resourceCount, GetResourceTypeStr(resourceType), ((resourceCount == 1) ? "" : "s"));
			u64 loadedResourceIndex = 0;
			for (u64 rIndex = 0; rIndex < resourceArray->length; rIndex++)
			{
				ResourcePoolEntry_t* entry = BktArrayGet(resourceArray, ResourcePoolEntry_t, rIndex);
				if (entry->id != 0)
				{
					PrintLine_I("  %s[%llu]: \"%.*s\" (referenced %llu time%s)", GetResourceTypeStr(resourceType), loadedResourceIndex, StrPrint(entry->filePath), entry->refCount, ((entry->refCount == 1) ? "" : "s"));
					loadedResourceIndex++;
				}
			}
		}
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_ListResourcePool_Glue) { EXP_GET_ARG_STR(0, resourceTypeStr); Debug_ListResourcePool(resourceTypeStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         test_scratch                         |
// +--------------------------------------------------------------+
//TODO: Make this argument a regular u64 and make the expression syntax allow for defining stuff like kilobytes and megabytes in a terse way?
#define Debug_TestScratch_Def "void test_scratch(string num_bytes)"
#define Debug_TestScratch_Desc "Allocates a specified number of bytes from one of the scratch arenas"
void Debug_TestScratch(MyStr_t numBytesStr)
{
	TrimWhitespace(&numBytesStr);
	u64 unitMultiplier = 1;
	if (StrEndsWith(numBytesStr, "kb", true)) { numBytesStr.length -= 2; unitMultiplier = Kilobytes(1); }
	if (StrEndsWith(numBytesStr, "mb", true)) { numBytesStr.length -= 2; unitMultiplier = Megabytes(1); }
	
	TryParseFailureReason_t parseFailureReason;
	u64 numBytesToAlloc = 0;
	if (!TryParseU64(numBytesStr, &numBytesToAlloc, &parseFailureReason))
	{
		PrintLine_E("Couldn't parse \"%.*s\" as u64: %s", StrPrint(numBytesStr), GetTryParseFailureReasonStr(parseFailureReason));
		return;
	}
	numBytesToAlloc *= unitMultiplier;
	
	MemArena_t* scratch = GetScratchArena();
	PrintLine_D("Allocating %s... (%llu/%llu, %llu pages)", FormatBytesNt(numBytesToAlloc, scratch), scratch->used, scratch->size, scratch->numPages);
	void* allocResult = AllocMem(scratch, numBytesToAlloc);
	PrintLine_D("Success %p! (%llu/%llu, %llu pages)", allocResult, scratch->used, scratch->size, scratch->numPages);
	FreeScratchArena(scratch);
}
EXPRESSION_FUNC_DEFINITION(Debug_TestScratch_Glue) { EXP_GET_ARG_STR(0, numBytesStr); Debug_TestScratch(numBytesStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         open_window                          |
// +--------------------------------------------------------------+
#define Debug_OpenWindow_Def "void open_window(string window_name)"
#define Debug_OpenWindow_Desc "Opens a particular imgui window by name. Open the launcher from the F3 menu for a way to discover what windows exist"
void Debug_OpenWindow(MyStr_t windowName)
{
	bool foundWindow = false;
	VarArrayLoop(&pig->imgui.registeredWindows, wIndex)
	{
		VarArrayLoopGet(PigRegisteredImguiWindow_t, registeredWindow, &pig->imgui.registeredWindows, wIndex);
		if (StrEqualsIgnoreCase(registeredWindow->name, windowName))
		{
			PrintLine_I("Opening \"%.*s\"", StrPrint(registeredWindow->name));
			registeredWindow->isOpen = true;
			foundWindow = true;
			break;
		}
	}
	
	if (!foundWindow)
	{
		PrintLine_E("No window registered as \"%.*s\"", StrPrint(windowName));
	}
}
EXPRESSION_FUNC_DEFINITION(Debug_OpenWindow_Glue) { EXP_GET_ARG_STR(0, windowName); Debug_OpenWindow(windowName); return NewExpValueVoid(); }

#if LUA_SUPPORTED
// +--------------------------------------------------------------+
// |                             lua                              |
// +--------------------------------------------------------------+
#define Debug_Lua_Def "void lua(string code)"
#define Debug_Lua_Desc "Runs the Lua interpreter on the supplied code"
void Debug_Lua(MyStr_t codeStr)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t scratchCodeStr = AllocString(scratch, &codeStr);
	luaL_loadstring(pig->lua.handle, scratchCodeStr.chars);
	lua_pcall(pig->lua.handle, 0, LUA_MULTRET, 0);
	FreeScratchArena(scratch);
}
EXPRESSION_FUNC_DEFINITION(Debug_Lua_Glue) { EXP_GET_ARG_STR(0, codeStr); Debug_Lua(codeStr); return NewExpValueVoid(); }
#endif //LUA_SUPPORTED

#if PYTHON_SUPPORTED

// +--------------------------------------------------------------+
// |                            python                            |
// +--------------------------------------------------------------+
#define Debug_Python_Def "void python(string code)"
#define Debug_Python_Desc "Runs the python interpreter on the supplied code and runs the \"main\" function in the script"
void Debug_Python(MyStr_t codeStr)
{
	// PyObject* mainModule = PyImport_GetModule(pig->python.mainModuleName);
	// if (mainModule == nullptr)
	// {
	// 	mainModule = PyImport_AddModule("__main__");
	// 	if (mainModule == nullptr) { WriteLine_E("Failed to add __main__ module to Python!"); return validCommand; }
	// 	Py_INCREF(mainModule);
	// }
	// PyObject* mainModuleDict = PyModule_GetDict(mainModule);
	// PrintPyObject("mainModuleDict: ", mainModuleDict);
	
	PyObject* localDict = PyDict_New();
	{
		MemArena_t* scratch = GetScratchArena();
		MyStr_t scratchCodeStr = AllocString(scratch, &codeStr);
		
		PyCompilerFlags flags = {};
		flags.cf_flags = PyCF_SOURCE_IS_UTF8;
		flags.cf_feature_version = PY_MINOR_VERSION;
		PyObject* parseResult = PyRun_StringFlags(scratchCodeStr.chars, Py_file_input, localDict, localDict, &flags);
		FreeScratchArena(scratch);
		if (parseResult == nullptr)
		{
			PrintPyException("Parse exception: ");
			return;
		}
		Py_DECREF(parseResult);
	}
	// PrintPyObject("localDict: ", localDict);
	
	PyObject* mainFunction = PyDict_GetItem(localDict, pig->python.mainFunctionName);
	if (mainFunction != nullptr)
	{
		Py_INCREF(mainFunction);
		if (PyFunction_Check(mainFunction))
		{
			WriteLine_D("Calling main()...");
			PyObject* mainResult = PyObject_CallNoArgs(mainFunction);
			if (mainResult != nullptr)
			{
				PrintPyObject("main() => ", mainResult);
				if (mainResult != nullptr) { Py_DECREF(mainResult); }
			}
			else
			{
				PrintPyException("Runtime exception: ");
			}
		}
		else
		{
			WriteLine_E("The script is missing a \"main()\" function (found \"main\" but it wasn't a function)");
		}
		Py_DECREF(mainFunction);
	}
	else
	{
		WriteLine_E("The script is missing a \"main()\" function");
	}
	
	i64 localDictSize = PyDict_Size(localDict);
	PyObject* localDictKeys = PyDict_Keys(localDict);
	NotNull(localDictKeys);
	Assert(PyList_Check(localDictKeys));
	Assert(PyList_Size(localDictKeys) == localDictSize);
	for (i64 keyIndex = 0; keyIndex < localDictSize; keyIndex++)
	{
		PyObject* key = PyList_GetItem(localDictKeys, keyIndex);
		NotNull(key);
		PyDict_DelItem(localDict, key);
	}
	// Assert(Py_REFCNT(localDict) == 1);
	Py_DECREF(localDict);
	
	// Py_DECREF(mainModule);
}
EXPRESSION_FUNC_DEFINITION(Debug_Python_Glue) { EXP_GET_ARG_STR(0, codeStr); Debug_Python(codeStr); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         python_file                          |
// +--------------------------------------------------------------+
#define Debug_PythonFile_Def "void python_file(string file_path)"
#define Debug_PythonFile_Desc "Runs the python interpreter on the code stored in a file and runs the \"main\" function in the script"
void Debug_PythonFile(MyStr_t filePath)
{
	StartPerfSections(4, "LoadFile", true);
	MemArena_t* scratch = GetScratchArena();
	PlatFileContents_t scriptFile;
	if (!plat->ReadFileContents(filePath, scratch, true, &scriptFile)) { PrintLine_E("Failed to open file \"%.*s\"", StrPrint(filePath)); FreeScratchArena(scratch); return; }
	MyStr_t scriptStr = NewStr(scriptFile.length, scriptFile.chars);
	
	PerfSection("CreateScript");
	PythonScript_t* script = CreatePythonScript(scratch);
	if (!ParsePythonScriptStr(script, scriptStr))
	{
		PrintPyException("Parse exception: ");
		plat->FreeFileContents(&scriptFile);
		FreeScratchArena(scratch);
		return;
	}
	
	PerfSection("RunScript");
	PyObject* mainResult = nullptr;
	Result_t runResult = RunPythonFunctionInScript(script, NewStr("main"), &mainResult);
	if (runResult == Result_Success)
	{
		PrintPyObject("main() => ", mainResult);
		if (mainResult != nullptr) { Py_DECREF(mainResult); }
		
		runResult = RunPythonFunctionInScript(script, NewStr("main"), &mainResult);
		PrintPyObject("2nd main() => ", mainResult);
		if (mainResult != nullptr) { Py_DECREF(mainResult); }
		
		runResult = RunPythonFunctionInScript(script, NewStr("main"), &mainResult);
		PrintPyObject("2nd main() => ", mainResult);
		if (mainResult != nullptr) { Py_DECREF(mainResult); }
	}
	else if (runResult == Result_Exception)
	{
		PrintPyException("Runtime exception: ");
	}
	else if (runResult == Result_NotAFunction)
	{
		WriteLine_E("The script is missing a \"main()\" function (found \"main\" but it wasn't a function)");
	}
	else
	{
		WriteLine_E("The script is missing a \"main()\" function");
	}
	
	PerfSection("FreeScript");
	FreePythonScript(script);
	
	plat->FreeFileContents(&scriptFile);
	FreeScratchArena(scratch);
	
	EndPerfSections();
	PrintPerfSections(PrintLine_D, "python_file");
}
EXPRESSION_FUNC_DEFINITION(Debug_PythonFile_Glue) { EXP_GET_ARG_STR(0, filePath); Debug_PythonFile(filePath); return NewExpValueVoid(); }

#endif //PYTHON_SUPPORTED

#if PIG_MEM_ARENA_TEST_SET
// +--------------------------------------------------------------+
// |                       run_mem_test_set                       |
// +--------------------------------------------------------------+
#define Debug_RunMemTestSet_Def "void run_mem_test_set()"
#define Debug_RunMemTestSet_Desc "Runs a paged heap memory arena through a series of allocs/frees/reallocs that were captured by some other process (right now, it's the initialization process of python that we capture)"
void Debug_RunMemTestSet()
{
	if (pig->arenaTestSet.actions.length == 0) { WriteLine_E("No test set was captured"); return; }
	MemArena_t testArena;
	InitMemArena_PagedHeapFuncs(&testArena, Megabytes(1), PlatAllocFunc, PlatFreeFunc, 0, AllocAlignment_8Bytes);
	FlagSet(testArena.flags, MemArenaFlag_CacheFreeOffset);
	MemArenaTestSetPrepare(&pig->arenaTestSet);
	PerfTime_t allocStartTime = GetPerfTime();
	MemArenaTestSetPerformAllActions(&testArena, &pig->arenaTestSet);
	PerfTime_t allocEndTime = GetPerfTime();
	FreeMemArena(&testArena);
	MemArena_t* scratch = GetScratchArena();
	PrintLine_I("%llu allocations took %s", pig->arenaTestSet.actions.length, FormatMillisecondsNt((u64)GetPerfTimeDiff(&allocStartTime, &allocEndTime), scratch));
	FreeScratchArena(scratch);
}
EXPRESSION_FUNC_DEFINITION(Debug_RunMemTestSet_Glue) { Debug_RunMemTestSet(); return NewExpValueVoid(); }
#endif //PIG_MEM_ARENA_TEST_SET

// +--------------------------------------------------------------+
// |                      current_app_state                       |
// +--------------------------------------------------------------+
#define Debug_CurrentAppState_Def "string current_app_state()"
#define Debug_CurrentAppState_Desc "Returns the name of the current app state (the AppState which is on the top of the stack)"
const char* Debug_CurrentAppState()
{
	return GetAppStateStr(GetCurrentAppState());
}
EXPRESSION_FUNC_DEFINITION(Debug_CurrentAppState_Glue) { return NewExpValueStr(NewStr(Debug_CurrentAppState())); }

void PigAddDebugVarsToExpContext(ExpContext_t* context)
{
	const bool read = false;
	const bool write = true;
	
	// Read-only variables
	AddExpVariableDefU64(context,  read,  "program_time",        &ProgramTime,                          "Number of milliseconds since the start of the program");
	AddExpVariableDefR64(context,  read,  "time_scale",          &TimeScale,                            "A floating pointer number that changes per frame to indicate the ratio between elapsed_ms and the target framerate elapsed ms");
	AddExpVariableDefR64(context,  read,  "elapsed_ms",          &ElapsedMs,                            "The number of milliseconds elapsed since last frame (this can be manipulated using debug commands, and is capped to a max value so we don't time travel forward)");
	AddExpVariableDefR64(context,  read,  "uncapped_elapsed_ms", &UncappedElapsedMs,                    "The number of milliseconds elapsed since last frame (not capped to a max value, still can be manipulated by debug commands)");
	AddExpVariableDefU64(context,  read,  "local_timestamp",     &LocalTimestamp,                       "Number of seconds from Jan 1st 1980 as a U64 (offset by some number of hours to account for computer's timezone)");
	AddExpVariableDefU64(context,  read,  "unix_timestamp",      &UnixTimestamp,                        "Number of seconds from Jan 1st 1980 as a U64");
	AddExpVariableDefR32(context,  read,  "mouse_x",             &MousePos.x,                           "The x coordinate of the mouse (relative to the game window, 1px unit)");
	AddExpVariableDefR32(context,  read,  "mouse_y",             &MousePos.y,                           "The y coordinate of the mouse (relative to the game window, 1px unit)");
	AddExpVariableDefR32(context,  read,  "screen_width",        &ScreenSize.width,                     "The width in pixels of the window (the renderable portion of it at least)");
	AddExpVariableDefR32(context,  read,  "screen_height",       &ScreenSize.height,                    "The height in pixels of the window (the renderable portion of it at least)");
	AddExpVariableDefBool(context, read,  "fullscreen_enabled",  &pig->currentWindow->input.fullscreen, "Whether the game is currently in fullscreen mode or not"); //TODO: This is reporting the opposite value than I expect!
	
	AddExpVariableDefR64(context,  write, "fixed_time_scale",         &pigOut->fixedTimeScale,        "The current time scale multiplier (only applies if fixed_time_scale_enabled)");
	AddExpVariableDefBool(context, write, "fixed_time_scale_enabled", &pigOut->fixedTimeScaleEnabled, "Whether the time scale value for the game is currently fixed by fixed_time_scale multiplier");
	AddExpVariableDefBool(context, write, "dont_exit_on_assert",      &pig->dontExitOnAssert,         "Toggles whether assertions should force close the application when hit");
	AddExpVariableDefBool(context, write, "show_monitors",            &pig->monitorsDebug,            "Toggles debug overlay for monitor info, resolutions, video modes, window position, etc.");
	AddExpVariableDefBool(context, write, "show_cyclic_funcs",        &pig->cyclicFuncsDebug,         "Toggles rendering of the cyclic function debug overlay");
}

void PigAddDebugCommandsToExpContext(ExpContext_t* context)
{
	AddDebugCommandDef(context, Debug_Help_Def,                Debug_Help_Glue,                Debug_Help_Desc);
	AddDebugCommandDef(context, Debug_Break_Def,               Debug_Break_Glue,               Debug_Break_Desc);
	AddDebugCommandDef(context, Debug_Assert_Def,              Debug_Assert_Glue,              Debug_Assert_Desc);
	AddDebugCommandDef(context, Debug_ArenaInfo_Def,           Debug_ArenaInfo_Glue,           Debug_ArenaInfo_Desc);
	AddDebugCommandDef(context, Debug_WalkArena_Def,           Debug_WalkArena_Glue,           Debug_WalkArena_Desc);
	AddDebugCommandDef(context, Debug_SetTimeScale_Def,        Debug_SetTimeScale_Glue,        Debug_SetTimeScale_Desc);
	AddDebugCommandDef(context, Debug_BumpTimeScale_Def,       Debug_BumpTimeScale_Glue,       Debug_BumpTimeScale_Desc);
	AddDebugCommandDef(context, Debug_Reload_Def,              Debug_Reload_Glue,              Debug_Reload_Desc);
	AddDebugCommandDef(context, Debug_Resources_Def,           Debug_Resources_Glue,           Debug_Resources_Desc);
	AddDebugCommandDef(context, Debug_Watches_Def,             Debug_Watches_Glue,             Debug_Watches_Desc);
	AddDebugCommandDef(context, Debug_Mute_Def,                Debug_Mute_Glue,                Debug_Mute_Desc);
	AddDebugCommandDef(context, Debug_Volumes_Def,             Debug_Volumes_Glue,             Debug_Volumes_Desc);
	AddDebugCommandDef(context, Debug_SetVolume_Def,           Debug_SetVolume_Glue,           Debug_SetVolume_Desc);
	AddDebugCommandDef(context, Debug_SetResolution_Def,       Debug_SetResolution_Glue,       Debug_SetResolution_Desc);
	AddDebugCommandDef(context, Debug_VideoModes_Def,          Debug_VideoModes_Glue,          Debug_VideoModes_Desc);
	AddDebugCommandDef(context, Debug_Fullscreen_Def,          Debug_Fullscreen_Glue,          Debug_Fullscreen_Desc);
	AddDebugCommandDef(context, Debug_ToggleFullscreen_Def,    Debug_ToggleFullscreen_Glue,    Debug_ToggleFullscreen_Desc);
	AddDebugCommandDef(context, Debug_ThreadIds_Def,           Debug_ThreadIds_Glue,           Debug_ThreadIds_Desc);
	AddDebugCommandDef(context, Debug_Bind_Def,                Debug_Bind_Glue,                Debug_Bind_Desc);
	AddDebugCommandDef(context, Debug_Unbind_Def,              Debug_Unbind_Glue,              Debug_Unbind_Desc);
	AddDebugCommandDef(context, Debug_Bindings_Def,            Debug_Bindings_Glue,            Debug_Bindings_Desc);
	AddDebugCommandDef(context, Debug_ReloadBindings_Def,      Debug_ReloadBindings_Glue,      Debug_ReloadBindings_Desc);
	AddDebugCommandDef(context, Debug_AppStates_Def,           Debug_AppStates_Glue,           Debug_AppStates_Desc);
	AddDebugCommandDef(context, Debug_PopAppState_Def,         Debug_PopAppState_Glue,         Debug_PopAppState_Desc);
	AddDebugCommandDef(context, Debug_PushAppState_Def,        Debug_PushAppState_Glue,        Debug_PushAppState_Desc);
	AddDebugCommandDef(context, Debug_ChangeAppState_Def,      Debug_ChangeAppState_Glue,      Debug_ChangeAppState_Desc);
	AddDebugCommandDef(context, Debug_RegenBasicResources_Def, Debug_RegenBasicResources_Glue, Debug_RegenBasicResources_Desc);
	AddDebugCommandDef(context, Debug_Sizeofs_Def,             Debug_Sizeofs_Glue,             Debug_Sizeofs_Desc);
	#if STEAM_BUILD
	AddDebugCommandDef(context, Debug_SteamFriends_Def,        Debug_SteamFriends_Glue,        Debug_SteamFriends_Desc);
	AddDebugCommandDef(context, Debug_UpdateFriendStatus_Def,  Debug_UpdateFriendStatus_Glue,  Debug_UpdateFriendStatus_Desc);
	AddDebugCommandDef(context, Debug_GetFriendAvatar_Def,     Debug_GetFriendAvatar_Glue,     Debug_GetFriendAvatar_Desc);
	AddDebugCommandDef(context, Debug_ClearFriendAvatars_Def,  Debug_ClearFriendAvatars_Glue,  Debug_ClearFriendAvatars_Desc);
	AddDebugCommandDef(context, Debug_ShowFriendsList_Def,     Debug_ShowFriendsList_Glue,     Debug_ShowFriendsList_Desc);
	#endif //STEAM_BUILD
	AddDebugCommandDef(context, Debug_ListResourcePool_Def,    Debug_ListResourcePool_Glue,    Debug_ListResourcePool_Desc);
	AddDebugCommandDef(context, Debug_TestScratch_Def,         Debug_TestScratch_Glue,         Debug_TestScratch_Desc);
	AddDebugCommandDef(context, Debug_OpenWindow_Def,          Debug_OpenWindow_Glue,          Debug_OpenWindow_Desc);
	#if LUA_SUPPORTED
	AddDebugCommandDef(context, Debug_Lua_Def,                 Debug_Lua_Glue,                 Debug_Lua_Desc);
	#endif //LUA_SUPPORTED
	#if PYTHON_SUPPORTED
	AddDebugCommandDef(context, Debug_Python_Def,              Debug_Python_Glue,              Debug_Python_Desc);
	AddDebugCommandDef(context, Debug_PythonFile_Def,          Debug_PythonFile_Glue,          Debug_PythonFile_Desc);
	#endif //PYTHON_SUPPORTED
	#if PIG_MEM_ARENA_TEST_SET
	AddDebugCommandDef(context, Debug_RunMemTestSet_Def,       Debug_RunMemTestSet_Glue,       Debug_RunMemTestSet_Desc);
	#endif //PIG_MEM_ARENA_TEST_SET
	AddDebugCommandDef(context, Debug_CurrentAppState_Def,     Debug_CurrentAppState_Glue,     Debug_CurrentAppState_Desc);
}

void InitDebugConsoleExpContext(MemArena_t* scratchArena, ExpContext_t* contextOut) //pre-declared in pig_func_defs.h
{
	InitExpContext(scratchArena, contextOut, false);
	contextOut->isConsoleInput = true;
	contextOut->scratch = scratchArena;
	PigAddDebugVarsToExpContext(contextOut);
	PigAddDebugCommandsToExpContext(contextOut);
	GameAddDebugVarsToExpContext(contextOut);
	GameAddDebugCommandsToExpContext(contextOut);
}

bool PigParseDebugCommand(MyStr_t commandStr) //pre-declared in pig_func_defs.h
{
	if (commandStr.length == 0) { return false; }
	
	MemArena_t* scratch = GetScratchArena();
	ExpContext_t context = {};
	InitDebugConsoleExpContext(scratch, &context);
	
	ExpValue_t result = {};
	MyStr_t errorStr = TryRunExpressionErrorStr(commandStr, scratch, &result, &context);
	bool isValidCommand = IsEmptyStr(errorStr);
	if (isValidCommand)
	{
		if (result.type != ExpValueType_Void) { PrintLine_I("=> %s", ExpValueToStr(result, scratch).chars); }
		// if (foo != 14.123f) { PrintLine_D("foo is now %g", foo); }
	}
	else
	{
		PrintLine_W("\"%.*s\" => %.*s", StrPrint(commandStr), StrPrint(errorStr));
	}
	
	FreeScratchArena(scratch);
	return isValidCommand;
}
