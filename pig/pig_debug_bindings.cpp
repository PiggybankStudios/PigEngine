/*
File:   pig_debug_bindings.cpp
Author: Taylor Robbins
Date:   09\20\2022
Description: 
	** None 
*/

#define PIG_DBG_BINDINGS_FILE_PREFIX_STR      "# [Pig Bindings]"
#define PIG_DBG_BINDINGS_FILE_PREFIX_LENGTH   16

#define PIG_DBG_BINDINGS_MOUSE_BINDING_PREFIX      "(M)"
#define PIG_DBG_BINDINGS_CONTROLLER_BINDING_PREFIX "(C)"

enum PigTryDeserDebugBindingsError_t
{
	PigTryDeserDebugBindingsError_None = 0,
	PigTryDeserDebugBindingsError_CantOpenFile,
	PigTryDeserDebugBindingsError_MissingFilePrefix,
	PigTryDeserDebugBindingsError_TokenBeforeFilePrefix,
	PigTryDeserDebugBindingsError_InvalidFilePrefix,
	PigTryDeserDebugBindingsError_MultipleFilePrefix,
	PigTryDeserDebugBindingsError_NumErrors,
};
const char* GetPigTryDeserDebugBindingsErrorStr(PigTryDeserDebugBindingsError_t error)
{
	switch (error)
	{
		case PigTryDeserDebugBindingsError_None:                  return "None";
		case PigTryDeserDebugBindingsError_CantOpenFile:          return "CantOpenFile";
		case PigTryDeserDebugBindingsError_MissingFilePrefix:     return "MissingFilePrefix";
		case PigTryDeserDebugBindingsError_TokenBeforeFilePrefix: return "TokenBeforeFilePrefix";
		case PigTryDeserDebugBindingsError_InvalidFilePrefix:     return "InvalidFilePrefix";
		case PigTryDeserDebugBindingsError_MultipleFilePrefix:    return "MultipleFilePrefix";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void PigFreeDebugBindingsEntry(PigDebugBindingsEntry_t* entry, MemArena_t* memArena)
{
	NotNull2(entry, memArena);
	FreeString(memArena, &entry->commandStr);
	ClearPointer(entry);
}

bool PigDoBindingEntriesMatch(const PigDebugBindingsEntry_t* left, const PigDebugBindingsEntry_t* right)
{
	NotNull2(left, right);
	if (left->type != right->type) { return false; }
	if (left->type == PigDebugBindingType_Keyboard && left->modifiers != right->modifiers) { return false; }
	if (left->type == PigDebugBindingType_Keyboard && left->key != right->key) { return false; }
	if (left->type == PigDebugBindingType_Mouse && left->mouseBtn != right->mouseBtn) { return false; }
	if (left->type == PigDebugBindingType_Controller && left->controllerBtn != right->controllerBtn) { return false; }
	return true;
}

PigDebugBindingsEntry_t* PigFindDebugBinding(PigDebugBindings_t* bindings, const PigDebugBindingsEntry_t* comparisonBinding, u64* indexOut = nullptr)
{
	NotNull2(bindings, comparisonBinding);
	VarArrayLoop(&bindings->entries, bIndex)
	{
		VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &bindings->entries, bIndex);
		if (PigDoBindingEntriesMatch(binding, comparisonBinding))
		{
			if (indexOut != nullptr) { *indexOut = bIndex; }
			return binding;
		}
	}
	return nullptr;
}

PigDebugBindingsEntry_t* PigAddDebugBindingsEntryFromTemplate(PigDebugBindings_t* bindings, const PigDebugBindingsEntry_t* templateBinding, MyStr_t commandStr)
{
	NotNull2(bindings, templateBinding);
	NotNull(bindings->allocArena);
	NotNullStr(&commandStr);
	Assert(commandStr.length > 0);
	AssertSingleThreaded();
	
	PigDebugBindingsEntry_t* existingEntry = PigFindDebugBinding(bindings, templateBinding);
	if (existingEntry != nullptr)
	{
		existingEntry->type = templateBinding->type;
		existingEntry->modifiers = templateBinding->modifiers;
		existingEntry->key = templateBinding->key;
		existingEntry->mouseBtn = templateBinding->mouseBtn;
		existingEntry->controllerBtn = templateBinding->controllerBtn;
		FreeString(bindings->allocArena, &existingEntry->commandStr);
		existingEntry->commandStr = AllocString(bindings->allocArena, &commandStr);
		NotNullStr(&existingEntry->commandStr);
		return existingEntry;
	}
	else
	{
		PigDebugBindingsEntry_t* newEntry = VarArrayAdd(&bindings->entries, PigDebugBindingsEntry_t);
		NotNull(newEntry);
		ClearPointer(newEntry);
		newEntry->id = bindings->nextId;
		bindings->nextId++;
		newEntry->type = templateBinding->type;
		newEntry->modifiers = templateBinding->modifiers;
		newEntry->key = templateBinding->key;
		newEntry->mouseBtn = templateBinding->mouseBtn;
		newEntry->controllerBtn = templateBinding->controllerBtn;
		newEntry->commandStr = AllocString(bindings->allocArena, &commandStr);
		NotNullStr(&newEntry->commandStr);
		return newEntry;
	}
}

PigDebugBindingsEntry_t* PigAddDebugBindingsEntryKey(PigDebugBindings_t* bindings, u8 modifiers, Key_t key, MyStr_t commandStr)
{
	Assert(key > Key_None && key < Key_NumKeys);
	PigDebugBindingsEntry_t templateBinding = {};
	templateBinding.type = PigDebugBindingType_Keyboard;
	templateBinding.modifiers = modifiers;
	templateBinding.key = key;
	return PigAddDebugBindingsEntryFromTemplate(bindings, &templateBinding, commandStr);
}
PigDebugBindingsEntry_t* PigAddDebugBindingsEntryMouse(PigDebugBindings_t* bindings, MouseBtn_t mouseBtn, MyStr_t commandStr)
{
	Assert(mouseBtn > MouseBtn_None && mouseBtn < MouseBtn_NumBtns);
	PigDebugBindingsEntry_t templateBinding = {};
	templateBinding.type = PigDebugBindingType_Mouse;
	templateBinding.mouseBtn = mouseBtn;
	return PigAddDebugBindingsEntryFromTemplate(bindings, &templateBinding, commandStr);
}
PigDebugBindingsEntry_t* PigAddDebugBindingsEntryController(PigDebugBindings_t* bindings, ControllerBtn_t controllerBtn, MyStr_t commandStr)
{
	Assert(controllerBtn > ControllerBtn_None && controllerBtn < ControllerBtn_NumBtns);
	PigDebugBindingsEntry_t templateBinding = {};
	templateBinding.type = PigDebugBindingType_Controller;
	templateBinding.controllerBtn = controllerBtn;
	return PigAddDebugBindingsEntryFromTemplate(bindings, &templateBinding, commandStr);
}

MyStr_t PigGetDebugBindingsFilePath(MemArena_t* memArena, MyStr_t applicationName, MyStr_t fileName)
{
	NotNull(memArena);
	NotNullStr(&applicationName);
	NotNullStr(&fileName);
	MyStr_t settingsFolderPath = plat->GetSpecialFolderPath(SpecialFolder_SavesAndSettings, applicationName, TempArena);
	if (settingsFolderPath.length > 0)
	{
		return PrintInArenaStr(memArena, "%.*s/%.*s", settingsFolderPath.length, settingsFolderPath.pntr, fileName.length, fileName.pntr);
	}
	return MyStr_Empty;
}

// +--------------------------------------------------------------+
// |                       Deserialization                        |
// +--------------------------------------------------------------+
bool PigTryDeserBindingStr(MyStr_t bindingStr, PigDebugBindingsEntry_t* bindingOut, ProcessLog_t* log = nullptr)
{
	NotNull(bindingOut);
	
	if (bindingStr.length == 0)
	{
		if (log != nullptr)
		{
			LogPrintLine_W(log, "Empty string cannot be a valid binding!");
			log->hadWarnings = true;
		}
		return false;
	}
	
	// +==============================+
	// |     Parse Mouse Binding      |
	// +==============================+
	if (StrStartsWith(bindingStr, PIG_DBG_BINDINGS_MOUSE_BINDING_PREFIX, true))
	{
		bindingOut->type = PigDebugBindingType_Mouse;
		bindingStr = StrSubstring(&bindingStr, MyStrLength64(PIG_DBG_BINDINGS_MOUSE_BINDING_PREFIX));
		TrimWhitespace(&bindingStr);
		
		bindingOut->mouseBtn = MouseBtn_None;
		for (u64 bIndex = 0; bIndex < MouseBtn_NumBtns; bIndex++)
		{
			MouseBtn_t mouseBtn = (MouseBtn_t)bIndex;
			const char* buttonName = GetMouseBtnStr(mouseBtn);
			if (StrCompareIgnoreCase(bindingStr, buttonName) == 0)
			{
				bindingOut->mouseBtn = mouseBtn;
				break;
			}
		}
		if (bindingOut->mouseBtn == MouseBtn_None)
		{
			if (log != nullptr)
			{
				LogPrintLine_W(log, "Unknown mouse button in binding: \"%.*s\"", bindingStr.length, bindingStr.pntr);
				log->hadWarnings = true;
			}
			return false;
		}
		
		return true;
	}
	// +==============================+
	// |   Parse Controller Binding   |
	// +==============================+
	else if (StrStartsWith(bindingStr, PIG_DBG_BINDINGS_CONTROLLER_BINDING_PREFIX, true))
	{
		bindingOut->type = PigDebugBindingType_Controller;
		bindingStr = StrSubstring(&bindingStr, MyStrLength64(PIG_DBG_BINDINGS_CONTROLLER_BINDING_PREFIX));
		TrimWhitespace(&bindingStr);
		
		bindingOut->controllerBtn = ControllerBtn_None;
		for (u64 bIndex = 0; bIndex < ControllerBtn_NumBtns; bIndex++)
		{
			ControllerBtn_t controllerBtn = (ControllerBtn_t)bIndex;
			const char* buttonName = GetControllerBtnStr(controllerBtn);
			if (StrCompareIgnoreCase(bindingStr, buttonName) == 0)
			{
				bindingOut->controllerBtn = controllerBtn;
				break;
			}
		}
		if (bindingOut->controllerBtn == ControllerBtn_None)
		{
			if (log != nullptr)
			{
				LogPrintLine_W(log, "Unknown controller button in binding: \"%.*s\"", bindingStr.length, bindingStr.pntr);
				log->hadWarnings = true;
			}
			return false;
		}
		
		return true;
	}
	// +==============================+
	// |      Parse Key Binding       |
	// +==============================+
	else
	{
		bindingOut->type = PigDebugBindingType_Keyboard;
		
		u64 prevBreakIndex = 0;
		for (u64 bIndex = 0; bIndex <= bindingStr.length; )
		{
			u32 codepoint = 0;
			u8 codepointSize = 0;
			if (bIndex < bindingStr.length)
			{
				codepointSize = GetCodepointForUtf8Str(bindingStr, bIndex, &codepoint);
				if (codepointSize == 0)
				{
					if (log != nullptr)
					{
						LogPrintLine_W(log, "Invalid UTF-8 found in binding str: \"%.*s\"", bindingStr.length, bindingStr.pntr);
						log->hadWarnings = true;
					}
					return false;
				}
			}
			else
			{
				codepoint = CharToU32('+');
				codepointSize = 1;
			}
			
			if (codepoint == '+')
			{
				MyStr_t bindingPiece = StrSubstring(&bindingStr, prevBreakIndex, bIndex);
				
				if (bindingPiece.length == 0)
				{
					if (log != nullptr)
					{
						LogPrintLine_W(log, "Binding has an empty piece: \"%.*s\"", bindingStr.length, bindingStr.pntr);
						log->hadWarnings = true;
					}
					return false;
				}
				
				Key_t key = Key_None;
				for (u64 keyIndex = 0; keyIndex < Key_NumKeys; keyIndex++)
				{
					Key_t testKey = (Key_t)keyIndex;
					const char* keyName = GetKeyStr(testKey);
					if (StrCompareIgnoreCase(bindingPiece, keyName) == 0)
					{
						key = testKey;
						break;
					}
					if (GetModifierKeyForKey(testKey) != ModifierKey_None)
					{
						ModifierKey_t testModifierKey = GetModifierKeyForKey(testKey);
						const char* modifierKeyName = GetModifierKeyStr(testModifierKey);
						if (StrCompareIgnoreCase(bindingPiece, modifierKeyName) == 0)
						{
							key = testKey;
							break;
						}
					}
				}
				
				if (key == Key_None)
				{
					if (log != nullptr)
					{
						LogPrintLine_W(log, "Unknown key \"%.*s\" in binding: \"%.*s\"", bindingPiece.length, bindingPiece.pntr, bindingStr.length, bindingStr.pntr);
						log->hadWarnings = true;
					}
					return false;
				}
				
				ModifierKey_t modifierKey = GetModifierKeyForKey(key);
				bool isModifierKey = (modifierKey != ModifierKey_None);
				if (bIndex < bindingStr.length)
				{
					if (!isModifierKey)
					{
						if (log != nullptr)
						{
							LogPrintLine_W(log, "Non modifier key %s used as modifier in binding: \"%.*s\"", GetKeyStr(key), bindingStr.length, bindingStr.pntr);
							log->hadWarnings = true;
						}
						return false;
					}
					
					if (IsFlagSet(bindingOut->modifiers, modifierKey))
					{
						if (log != nullptr)
						{
							LogPrintLine_W(log, "Modifier key %s repeated in binding: \"%.*s\"", GetKeyStr(key), bindingStr.length, bindingStr.pntr);
							log->hadWarnings = true;
						}
						return false;
					}
					
					FlagSet(bindingOut->modifiers, modifierKey);
				}
				else
				{
					if (isModifierKey)
					{
						if (log != nullptr)
						{
							LogPrintLine_W(log, "Modifier key %s used as primary binding key in binding: \"%.*s\"", GetKeyStr(key), bindingStr.length, bindingStr.pntr);
							log->hadWarnings = true;
						}
						return false;
					}
					
					bindingOut->key = key;
					return true;
				}
				
				prevBreakIndex = bIndex + codepointSize;
			}
			
			bIndex += codepointSize;
		}
		
		DebugAssertMsg(false, "We really shouldn't be able to get here");
		return false;
	}
}
bool PigTryDeserDebugBindings(MyStr_t fileContents, ProcessLog_t* log, PigDebugBindings_t* bindingsOut)
{
	NotNullStr(&fileContents);
	NotNull2(log, bindingsOut);
	NotNull(bindingsOut->allocArena);
	
	LogWriteLine_N(log, "Entering PigTryDeserDebugBindings...");
	SetProcessLogName(log, NewStr("Deser Debug Bindings"));
	
	TextParser_t textParser = NewTextParser(fileContents);
	
	bool foundFilePrefix = false;
	ParsingToken_t token = {};
	while (TextParserGetToken(&textParser, &token))
	{
		if (!foundFilePrefix && token.type != ParsingTokenType_FilePrefix)
		{
			LogPrintLine_E(log, "Found %s token before file prefix: \"%.*s\"", GetParsingTokenTypeStr(token.type), token.str.length, token.str.pntr);
			LogExitFailure(log, PigTryDeserDebugBindingsError_TokenBeforeFilePrefix);
			return false;
		}
		
		switch (token.type)
		{
			// +==============================+
			// |   ParsingTokenType_Comment   |
			// +==============================+
			case ParsingTokenType_Comment:
			{
				//Nothing to do here
			} break;
			
			// +==============================+
			// | ParsingTokenType_FilePrefix  |
			// +==============================+
			case ParsingTokenType_FilePrefix:
			{
				if (!foundFilePrefix)
				{
					if (token.str.length != PIG_DBG_BINDINGS_FILE_PREFIX_LENGTH || MyMemCompare(token.str.pntr, PIG_DBG_BINDINGS_FILE_PREFIX_STR, PIG_DBG_BINDINGS_FILE_PREFIX_LENGTH) != 0)
					{
						LogPrintLine_E(log, "Invalid file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
						LogExitFailure(log, PigTryDeserDebugBindingsError_InvalidFilePrefix);
						return false;
					}
					foundFilePrefix = true;
				}
				else
				{
					LogPrintLine_E(log, "Second file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
					LogExitFailure(log, PigTryDeserDebugBindingsError_MultipleFilePrefix);
					return false;
				}
			} break;
			
			// +===============================+
			// | ParsingTokenType_KeyValuePair |
			// +===============================+
			case ParsingTokenType_KeyValuePair:
			{
				PigDebugBindingsEntry_t binding = {};
				bool isValidBinding = PigTryDeserBindingStr(token.key, &binding, log);
				if (!isValidBinding)
				{
					LogPrintLine_W(log, "Invalid binding on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
					log->hadWarnings = true;
					break;
				}
				
				if (binding.type == PigDebugBindingType_Keyboard)
				{
					PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryKey(bindingsOut, binding.modifiers, binding.key, token.value);
					NotNull(newEntry);
				}
				else if (binding.type == PigDebugBindingType_Mouse)
				{
					PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryMouse(bindingsOut, binding.mouseBtn, token.value);
					NotNull(newEntry);
				}
				else if (binding.type == PigDebugBindingType_Controller)
				{
					PigDebugBindingsEntry_t* newEntry = PigAddDebugBindingsEntryController(bindingsOut, binding.controllerBtn, token.value);
					NotNull(newEntry);
				}
				else { DebugAssert(false); }
			} break;
			
			// +==============================+
			// |   ParsingTokenType_Unknown   |
			// +==============================+
			case ParsingTokenType_Unknown:
			{
				LogPrintLine_W(log, "WARNING: Unknown token in file line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
				log->hadWarnings = true;
			} break;
			
			default: DebugAssert(false); break;
		}
	}
	
	if (!foundFilePrefix)
	{
		LogWriteLine_E(log, "Found no file prefix! This is probably an empty file");
		LogExitFailure(log, PigTryDeserDebugBindingsError_MissingFilePrefix);
		return false;
	}
	
	LogExitSuccess(log);
	return true;
}

bool PigTryLoadDebugBindings(MyStr_t filePath, ProcessLog_t* log, PigDebugBindings_t* bindingsOut)
{
	NotNullStr(&filePath);
	bool result = false;
	
	SetProcessLogFilePath(log, filePath);
	
	PlatFileContents_t bindingsFile = {};
	if (plat->ReadFileContents(filePath, &bindingsFile))
	{
		result = PigTryDeserDebugBindings(NewStr(bindingsFile.size, bindingsFile.chars), log, bindingsOut);
		plat->FreeFileContents(&bindingsFile);
	}
	else
	{
		if (plat->DoesFileExist(filePath, nullptr))
		{
			LogPrintLine_E(log, "Failed to open debug bindings file at \"%.*s\"", filePath.length, filePath.pntr);
			LogExitFailure(log, PigTryDeserDebugBindingsError_CantOpenFile);
		}
		else
		{
			LogPrintLine_W(log, "No debug bindings file found at \"%.*s\"", filePath.length, filePath.pntr);
			log->hadWarnings = true;
		}
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void PigClearDebugBindings(PigDebugBindings_t* bindings)
{
	VarArrayLoop(&bindings->entries, eIndex)
	{
		VarArrayLoopGet(PigDebugBindingsEntry_t, entry, &bindings->entries, eIndex);
		PigFreeDebugBindingsEntry(entry, bindings->allocArena);
	}
	VarArrayClear(&bindings->entries);
}

void PigInitDebugBindings(PigDebugBindings_t* bindings, MemArena_t* memArena)
{
	NotNull2(bindings, memArena);
	ClearPointer(bindings);
	bindings->allocArena = memArena;
	bindings->nextId = 1;
	CreateVarArray(&bindings->entries, memArena, sizeof(PigDebugBindingsEntry_t));
}

void PigLoadDebugBindingsFullService(PigDebugBindings_t* bindingsOut, MyStr_t filePath)
{
	NotNullStr(&filePath);
	if (plat->DoesFileExist(filePath, nullptr))
	{
		u64 numBindingsBefore = bindingsOut->entries.length;
		ProcessLog_t deserProcessLog = {};
		CreateProcessLog(&deserProcessLog, Kilobytes(8), mainHeap, mainHeap);
		if (PigTryLoadDebugBindings(filePath, &deserProcessLog, bindingsOut))
		{
			u64 numNewBindings = bindingsOut->entries.length - numBindingsBefore;
			PrintLine_D("Loaded %llu debug binding%s from \"%.*s\"", numNewBindings, (numNewBindings == 1 ? "" : "s"), filePath.length, filePath.pntr);
		}
		else
		{
			PrintLine_D("Failed to load debug bindings from \"%.*s\"", filePath.length, filePath.pntr);
		}
		if (deserProcessLog.hadWarnings || deserProcessLog.hadErrors) { DumpProcessLog(&deserProcessLog, "Debug Bindings Parse Log"); }
		FreeProcessLog(&deserProcessLog);
	}
	else
	{
		PrintLine_D("No debug bindings file found at \"%.*s\"", filePath.length, filePath.pntr);
	}
}
