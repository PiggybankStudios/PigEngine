/*
File:   pig_settings.cpp
Author: Taylor Robbins
Date:   09\17\2022
Description: 
	** Holds functions that help us serialize and parse an external file that holds
	** settings for the game or application. For example resolution settings and volume settings
*/

#define PIG_SETTINGS_FILE_PREFIX_STR      "# [Pig Settings]"
#define PIG_SETTINGS_FILE_PREFIX_LENGTH   16

enum PigTryDeserSettingsError_t
{
	PigTryDeserSettingsError_None = 0,
	PigTryDeserSettingsError_CantOpenFile,
	PigTryDeserSettingsError_EmptyFile,
	PigTryDeserSettingsError_MissingFilePrefix,
	PigTryDeserSettingsError_TokenBeforeFilePrefix,
	PigTryDeserSettingsError_InvalidFilePrefix,
	PigTryDeserSettingsError_MultipleFilePrefix,
	PigTryDeserSettingsError_NotEnoughMemoryForStrings,
	PigTryDeserSettingsError_NumErrors,
};
const char* GetPigTryDeserSettingsErrorStr(PigTryDeserSettingsError_t error)
{
	switch (error)
	{
		case PigTryDeserSettingsError_None:                      return "None";
		case PigTryDeserSettingsError_CantOpenFile:              return "CantOpenFile";
		case PigTryDeserSettingsError_EmptyFile:                 return "EmptyFile";
		case PigTryDeserSettingsError_MissingFilePrefix:         return "MissingFilePrefix";
		case PigTryDeserSettingsError_TokenBeforeFilePrefix:     return "TokenBeforeFilePrefix";
		case PigTryDeserSettingsError_InvalidFilePrefix:         return "InvalidFilePrefix";
		case PigTryDeserSettingsError_MultipleFilePrefix:        return "MultipleFilePrefix";
		case PigTryDeserSettingsError_NotEnoughMemoryForStrings: return "NotEnoughMemoryForStrings";
		default: return "Unknown";
	}
}

MyStr_t PigGetSettingsFilePath(MemArena_t* tempArena, MemArena_t* memArena, MyStr_t applicationName, MyStr_t fileName, PlatApiGetSpecialFolderPath_f* getSpecialFolderPathFunc = nullptr)
{
	NotNull2(tempArena, memArena);
	PlatApiGetSpecialFolderPath_f* getSpecialFolderPathPntr = getSpecialFolderPathFunc;
	if (getSpecialFolderPathFunc == nullptr) { NotNull(plat); getSpecialFolderPathPntr = plat->GetSpecialFolderPath; }
	MyStr_t settingsFolderPath = getSpecialFolderPathPntr(SpecialFolder_SavesAndSettings, applicationName, tempArena);
	if (settingsFolderPath.length > 0)
	{
		return PrintInArenaStr(memArena, "%.*s/%.*s", settingsFolderPath.length, settingsFolderPath.pntr, fileName.length, fileName.pntr);
	}
	return MyStr_Empty;
}

void PigInitSettings(PigSettings_t* settings, MemArena_t* memArena, u64 numEntriesExpected = 0)
{
	NotNull(settings);
	ClearPointer(settings);
	settings->allocArena = memArena;
	CreateVarArray(&settings->entries, memArena, sizeof(PigSettingsEntry_t), numEntriesExpected);
}

// +--------------------------------------------------------------+
// |                       Deserialization                        |
// +--------------------------------------------------------------+
bool PigTryDeserSettings(MyStr_t fileContents, ProcessLog_t* log, PigSettings_t* settingsOut, MemArena_t* memArena)
{
	NotNull3(log, settingsOut, memArena);
	NotNullStr(&fileContents);
	
	LogWriteLine_N(log, "Entering PigTryDeserSettings...");
	SetProcessLogName(log, NewStr("PigTryDeserSettings"));
	
	if (fileContents.length < PIG_SETTINGS_FILE_PREFIX_LENGTH)
	{
		LogWriteLine_E(log, "The file is empty. It can't be a valid settings file");
		LogExitFailure(log, PigTryDeserSettingsError_EmptyFile);
		return false;
	}
	
	bool needMeasurePass = !DoesMemArenaSupportFreeing(memArena);
	u64 numSettings = 0;
	u64 stringBufferSize = 0;
	MemArena_t stringBufferArena = {};
	if (!needMeasurePass)
	{
		PigInitSettings(settingsOut, memArena);
		InitMemArena_Alias(&stringBufferArena, memArena);
	}
	
	for (int pass = (needMeasurePass ? 0 : 1); pass < 2; pass++)
	{
		bool isSecondPass = (needMeasurePass && pass == 1);
		TextParser_t textParser = NewTextParser(fileContents);
		
		bool foundFilePrefix = false;
		ParsingToken_t token = {};
		while (TextParserGetToken(&textParser, &token))
		{
			if (!foundFilePrefix && token.type != ParsingTokenType_FilePrefix)
			{
				LogPrintLine_E(log, "Found %s token before file prefix: \"%.*s\"", GetParsingTokenTypeStr(token.type), token.str.length, token.str.pntr);
				LogExitFailure(log, PigTryDeserSettingsError_TokenBeforeFilePrefix);
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
						if (token.str.length != PIG_SETTINGS_FILE_PREFIX_LENGTH || MyMemCompare(token.str.pntr, PIG_SETTINGS_FILE_PREFIX_STR, PIG_SETTINGS_FILE_PREFIX_LENGTH) != 0)
						{
							LogPrintLine_E(log, "Invalid file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
							LogExitFailure(log, PigTryDeserSettingsError_InvalidFilePrefix);
							return false;
						}
						foundFilePrefix = true;
					}
					else
					{
						LogPrintLine_E(log, "Second file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
						LogExitFailure(log, PigTryDeserSettingsError_MultipleFilePrefix);
						return false;
					}
				} break;
				
				// +===============================+
				// | ParsingTokenType_KeyValuePair |
				// +===============================+
				case ParsingTokenType_KeyValuePair:
				{
					if (token.key.length == 0)
					{
						if (!isSecondPass) { LogPrintLine_W(log, "WARNING: Empty name for setting on line %llu", textParser.lineParser.lineIndex+1); log->hadWarnings = true; }
					}
					else if (token.value.length == 0)
					{
						if (!isSecondPass) { LogPrintLine_W(log, "WARNING: Empty value for setting \"%.*s\" on line %llu", token.key.length, token.key.pntr, textParser.lineParser.lineIndex+1); log->hadWarnings = true; }
					}
					else
					{
						if (pass == 0)
						{
							numSettings++;
							stringBufferSize += token.key.length+1;
							stringBufferSize += token.value.length+1;
						}
						else
						{
							PigSettingsEntry_t* newEntry = VarArrayAdd(&settingsOut->entries, PigSettingsEntry_t);
							NotNull(newEntry);
							ClearPointer(newEntry);
							newEntry->key = AllocString(&stringBufferArena, &token.key);
							newEntry->value = AllocString(&stringBufferArena, &token.value);
							NotNullStr(&newEntry->key);
							NotNullStr(&newEntry->value);
						}
					}
				} break;
				
				// +==============================+
				// |   ParsingTokenType_Unknown   |
				// +==============================+
				case ParsingTokenType_Unknown:
				{
					if (!isSecondPass) { LogPrintLine_W(log, "WARNING: Unknown token in file line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr); log->hadWarnings = true; }
				} break;
				
				default: DebugAssert(false); break;
			}
		}
		
		if (!foundFilePrefix)
		{
			LogWriteLine_E(log, "Found no file prefix! This is probably an empty file");
			LogExitFailure(log, PigTryDeserSettingsError_MissingFilePrefix);
			return false;
		}
		
		if (pass == 0)
		{
			PigInitSettings(settingsOut, memArena, numSettings);
			if (stringBufferSize > 0)
			{
				char* stringsBuffer = AllocArray(memArena, char, stringBufferSize);
				if (stringsBuffer == nullptr)
				{
					LogPrintLine_E(log, "Failed to allocation %llu bytes for strings from provided memory arena!", stringBufferSize);
					LogExitFailure(log, PigTryDeserSettingsError_NotEnoughMemoryForStrings);
					return false;
				}
				InitMemArena_Buffer(&stringBufferArena, stringBufferSize, stringsBuffer);
			}
		}
	}
	
	LogPrintLine_I(log, "Successfully deserialized pig settings file! It had %llu setting%s", settingsOut->entries.length, (settingsOut->entries.length == 1 ? "" : "s"));
	LogExitSuccess(log);
	return true;
}

//NOTE: Settings get loaded in GetStartupOptions which is before things like plat are filled so we have support for passing in to the file reading/freeing functions manually
bool PigTryLoadSettings(MyStr_t filePath, ProcessLog_t* log, PigSettings_t* settingsOut, MemArena_t* memArena, PlatApiReadFileContents_f* readFileFunc = nullptr, PlatApiFreeFileContents_f* freeFileFunc = nullptr)
{
	Assert(plat != nullptr || readFileFunc != nullptr);
	Assert((readFileFunc == nullptr) == (freeFileFunc == nullptr));
	bool result = false;
	PlatFileContents_t settingsFile;
	if ((readFileFunc != nullptr && readFileFunc(filePath, &settingsFile)) || (plat != nullptr && plat->ReadFileContents(filePath, &settingsFile)))
	{
		result = PigTryDeserSettings(NewStr(settingsFile.size, settingsFile.chars), log, settingsOut, memArena);
		if (freeFileFunc != nullptr) { freeFileFunc(&settingsFile); }
		else { plat->FreeFileContents(&settingsFile); }
	}
	else
	{
		LogPrintLine_E(log, "Failed to open settings file at \"%.*s\"", filePath.length, filePath.pntr);
		LogExitFailure(log, PigTryDeserSettingsError_CantOpenFile);
	}
	return result;
}

// +--------------------------------------------------------------+
// |                        Serialization                         |
// +--------------------------------------------------------------+
MyStr_t PigSerializeSettings(const PigSettings_t* settings, MemArena_t* memArena)
{
	NotNull(settings);
	RealTime_t localTime;
	ConvertTimestampToRealTime(LocalTimestamp, &localTime);
	MyStr_t result = MyStr_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		u64 byteIndex = 0;
		
		TwoPassPrint(result.pntr, result.length, &byteIndex, "%s\n", PIG_SETTINGS_FILE_PREFIX_STR);
		TwoPassPrint(result.pntr, result.length, &byteIndex, "// %s %s %u\n\n", GetMonthStr(localTime.month), GetDayOfMonthString(localTime.day), localTime.year);
		
		VarArrayLoop(&settings->entries, sIndex)
		{
			VarArrayLoopGet(PigSettingsEntry_t, setting, &settings->entries, sIndex);
			TwoPassPrint(result.pntr, result.length, &byteIndex, "%.*s: %.*s\n", setting->key.length, setting->key.pntr, setting->value.length, setting->value.pntr);
		}
		
		if (pass == 0)
		{
			result.length = byteIndex;
			if (memArena == nullptr) { return result; }
			result.pntr = AllocArray(memArena, char, result.length+1);
			if (result.pntr == nullptr) { return result; }
		}
		else
		{
			Assert(byteIndex == result.length);
			result.pntr[byteIndex] = '\0';
		}
	}
	return result;
}

bool PigTrySaveSettings(MyStr_t filePath, const PigSettings_t* settings, MemArena_t* serializeArena)
{
	NotNull2(settings, serializeArena);
	NotNullStr(&filePath);
	
	if (!CreateFoldersForPath(filePath))
	{
		PrintLine_E("Failed to create parent folders for settings file at \"%.*s\"", filePath.length, filePath.pntr);
		return false;
	}
	
	MyStr_t settingsSerialization = PigSerializeSettings(settings, serializeArena);
	if (settingsSerialization.pntr == nullptr)
	{
		WriteLine_E("PigSettings serialized to an empty string! The allocation must have failed!");
		return false;
	}
	
	bool saveSuccess = plat->WriteEntireFile(filePath, settingsSerialization.pntr, settingsSerialization.length);
	if (DoesMemArenaSupportFreeing(serializeArena)) { FreeString(serializeArena, &settingsSerialization); }
	
	return saveSuccess;
}

// +--------------------------------------------------------------+
// |                         Basic Checks                         |
// +--------------------------------------------------------------+
PigSettingsEntry_t* PigGetSettingEntry(PigSettings_t* settings, MyStr_t key, bool ignoreCase = true, u64* indexOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	VarArrayLoop(&settings->entries, sIndex)
	{
		VarArrayLoopGet(PigSettingsEntry_t, setting, &settings->entries, sIndex);
		if ((ignoreCase && StrCompareIgnoreCase(setting->key, key) == 0) || (!ignoreCase && StrEquals(setting->key, key)))
		{
			if (indexOut != nullptr) { *indexOut = sIndex; }
			return setting;
		}
	}
	return nullptr;
}
bool PigIsSettingSet(const PigSettings_t* settings, MyStr_t key, bool ignoreCase = true)
{
	PigSettingsEntry_t* entry = PigGetSettingEntry((PigSettings_t*)settings, key, ignoreCase);
	return (entry != nullptr);
}
bool PigIsSettingSet(const PigSettings_t* settings, const char* keyNullTerm, bool ignoreCase = true)
{
	return PigIsSettingSet(settings, NewStr(keyNullTerm), ignoreCase);
}

// +--------------------------------------------------------------+
// |                        Set Functions                         |
// +--------------------------------------------------------------+
bool PigUnsetSetting(PigSettings_t* settings, MyStr_t key, bool ignoreCase = true)
{
	u64 entryIndex = 0;
	PigSettingsEntry_t* entry = PigGetSettingEntry(settings, key, ignoreCase, &entryIndex);
	if (entry != nullptr)
	{
		VarArrayRemove(&settings->entries, entryIndex, PigSettingsEntry_t);
		return true;
	}
	return false;
}
bool PigUnsetSetting(PigSettings_t* settings, const char* keyNullTerm, bool ignoreCase = true)
{
	return PigUnsetSetting(settings, NewStr(keyNullTerm), ignoreCase);
}

PigSettingsEntry_t* PigSetSettingStr(PigSettings_t* settings, MyStr_t key, MyStr_t value, bool ignoreCase = true)
{
	NotNull(settings);
	NotNull(settings->allocArena);
	NotNullStr(&value);
	PigSettingsEntry_t* existingEntry = PigGetSettingEntry(settings, key, ignoreCase);
	if (existingEntry != nullptr)
	{
		if (existingEntry->value.pntr != nullptr)
		{
			FreeString(settings->allocArena, &existingEntry->value);
		}
		existingEntry->value = AllocString(settings->allocArena, &value);
		return existingEntry;
	}
	else
	{
		PigSettingsEntry_t* newEntry = VarArrayAdd(&settings->entries, PigSettingsEntry_t);
		NotNull(newEntry);
		ClearPointer(newEntry);
		newEntry->key = AllocString(settings->allocArena, &key);
		newEntry->value = AllocString(settings->allocArena, &value);
		NotNullStr(&newEntry->key);
		NotNullStr(&newEntry->value);
		return newEntry;
	}
}
PigSettingsEntry_t* PigSetSettingStr(PigSettings_t* settings, const char* keyNullTerm, MyStr_t value, bool ignoreCase = true)
{
	return PigSetSettingStr(settings, NewStr(keyNullTerm), value, ignoreCase);
}

PigSettingsEntry_t* PigSetSettingBool(PigSettings_t* settings, MyStr_t key, bool value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	MemArena_t* printArena = (useArenaForPrint != nullptr) ? useArenaForPrint : GetTempArena();
	NotNull(printArena);
	MyStr_t valueStr = PrintInArenaStr(printArena, "%s", value ? "True" : "False");
	return PigSetSettingStr(settings, key, valueStr, ignoreCase);
}
PigSettingsEntry_t* PigSetSettingBool(PigSettings_t* settings, const char* keyNullTerm, bool value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	return PigSetSettingBool(settings, NewStr(keyNullTerm), value, ignoreCase, useArenaForPrint);
}

PigSettingsEntry_t* PigSetSettingR32(PigSettings_t* settings, MyStr_t key, r32 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	MemArena_t* printArena = (useArenaForPrint != nullptr) ? useArenaForPrint : GetTempArena();
	NotNull(printArena);
	MyStr_t valueStr = PrintInArenaStr(printArena, "%g", value);
	return PigSetSettingStr(settings, key, valueStr, ignoreCase);
}
PigSettingsEntry_t* PigSetSettingR32(PigSettings_t* settings, const char* keyNullTerm, r32 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	return PigSetSettingR32(settings, NewStr(keyNullTerm), value, ignoreCase, useArenaForPrint);
}

PigSettingsEntry_t* PigSetSettingU64(PigSettings_t* settings, MyStr_t key, u64 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	MemArena_t* printArena = (useArenaForPrint != nullptr) ? useArenaForPrint : GetTempArena();
	NotNull(printArena);
	MyStr_t valueStr = PrintInArenaStr(printArena, "%llu", value);
	return PigSetSettingStr(settings, key, valueStr, ignoreCase);
}
PigSettingsEntry_t* PigSetSettingU64(PigSettings_t* settings, const char* keyNullTerm, u64 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	return PigSetSettingU64(settings, NewStr(keyNullTerm), value, ignoreCase, useArenaForPrint);
}

PigSettingsEntry_t* PigSetSettingV2(PigSettings_t* settings, MyStr_t key, v2 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	MemArena_t* printArena = (useArenaForPrint != nullptr) ? useArenaForPrint : GetTempArena();
	NotNull(printArena);
	MyStr_t valueStr = PrintInArenaStr(printArena, "(%g, %g)", value.x, value.y);
	return PigSetSettingStr(settings, key, valueStr, ignoreCase);
}
PigSettingsEntry_t* PigSetSettingV2(PigSettings_t* settings, const char* keyNullTerm, v2 value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	return PigSetSettingV2(settings, NewStr(keyNullTerm), value, ignoreCase, useArenaForPrint);
}

PigSettingsEntry_t* PigSetSettingV2i(PigSettings_t* settings, MyStr_t key, v2i value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	MemArena_t* printArena = (useArenaForPrint != nullptr) ? useArenaForPrint : GetTempArena();
	NotNull(printArena);
	MyStr_t valueStr = PrintInArenaStr(printArena, "(%d, %d)", value.x, value.y);
	return PigSetSettingStr(settings, key, valueStr, ignoreCase);
}
PigSettingsEntry_t* PigSetSettingV2i(PigSettings_t* settings, const char* keyNullTerm, v2i value, bool ignoreCase = true, MemArena_t* useArenaForPrint = nullptr)
{
	return PigSetSettingV2i(settings, NewStr(keyNullTerm), value, ignoreCase, useArenaForPrint);
}

// +--------------------------------------------------------------+
// |                        Get Functions                         |
// +--------------------------------------------------------------+
enum TryGetSettingResult_t
{
	TryGetSettingResult_None = 0,
	TryGetSettingResult_ParseError,
	TryGetSettingResult_Missing,
	TryGetSettingResult_Success,
	TryGetSettingResult_NumTypes,
};
const char* GetTryGetSettingResultStr(TryGetSettingResult_t result)
{
	switch (result)
	{
		case TryGetSettingResult_None:       return "None";
		case TryGetSettingResult_ParseError: return "ParseError";
		case TryGetSettingResult_Missing:    return "Missing";
		case TryGetSettingResult_Success:    return "Success";
		default: return "Unknown";
	}
}

//NOTE: TryGetSetting functions will return 3 different values: Success, Missing, and ParseFailure
//      Normally Try functions should be used when you want to gracefully, or specially, handle the ParseFailure case.
//      Or when you want to detect the missing case and do something more than assign a defaultValue
//      GetSetting functions don't report ParseFailures or Missings. On ParseFailure they throw a DebugAssert. On Missing they return defaultValue

bool PigTryGetSettingStr(const PigSettings_t* settings, MyStr_t key, MyStr_t* valueOut, bool ignoreCase = true)
{
	NotNull(settings);
	NotNullStr(&key);
	VarArrayLoop(&settings->entries, sIndex)
	{
		VarArrayLoopGet(PigSettingsEntry_t, setting, &settings->entries, sIndex);
		if ((ignoreCase && StrCompareIgnoreCase(setting->key, key) == 0) || (!ignoreCase && StrEquals(setting->key, key)))
		{
			if (valueOut != nullptr) { *valueOut = setting->value; }
			return true;
		}
	}
	return false;
}
bool PigTryGetSettingStr(const PigSettings_t* settings, const char* keyNullTerm, MyStr_t* valueOut, bool ignoreCase = true)
{
	return PigTryGetSettingStr(settings, NewStr(keyNullTerm), valueOut, ignoreCase);
}
MyStr_t PigGetSettingStr(const PigSettings_t* settings, MyStr_t key, MyStr_t defaultValue, bool ignoreCase = true)
{
	MyStr_t result = defaultValue;
	bool foundSetting = PigTryGetSettingStr(settings, key, &result, ignoreCase);
	UNUSED(foundSetting);
	return result;
}
MyStr_t PigGetSettingStr(const PigSettings_t* settings, const char* keyNullTerm, MyStr_t defaultValue, bool ignoreCase = true)
{
	return PigGetSettingStr(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}

TryGetSettingResult_t PigTryGetSettingBool(const PigSettings_t* settings, MyStr_t key, bool* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	MyStr_t settingValue;
	if (PigTryGetSettingStr(settings, key, &settingValue, ignoreCase))
	{
		if (TryParseBool(settingValue, valueOut, parseFailureReasonOut)) { return TryGetSettingResult_Success; }
		else { return TryGetSettingResult_ParseError; }
	}
	return TryGetSettingResult_Missing;
}
TryGetSettingResult_t PigTryGetSettingBool(const PigSettings_t* settings, const char* keyNullTerm, bool* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	return PigTryGetSettingBool(settings, NewStr(keyNullTerm), valueOut, ignoreCase, parseFailureReasonOut);
}
bool PigGetSettingBool(const PigSettings_t* settings, MyStr_t key, bool defaultValue, bool ignoreCase = true)
{
	bool result = defaultValue;
	TryGetSettingResult_t foundSetting = PigTryGetSettingBool(settings, key, &result, ignoreCase);
	//TODO: If the setting is unparsable, we should do a debug output here with info maybe?
	DebugAssert(foundSetting == TryGetSettingResult_Success || foundSetting == TryGetSettingResult_Missing);
	return result;
}
bool PigGetSettingBool(const PigSettings_t* settings, const char* keyNullTerm, bool defaultValue, bool ignoreCase = true)
{
	return PigGetSettingBool(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}

TryGetSettingResult_t PigTryGetSettingR32(const PigSettings_t* settings, MyStr_t key, r32* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	MyStr_t settingValue;
	if (PigTryGetSettingStr(settings, key, &settingValue, ignoreCase))
	{
		if (TryParseR32(settingValue, valueOut, parseFailureReasonOut)) { return TryGetSettingResult_Success; }
		else { return TryGetSettingResult_ParseError; }
	}
	return TryGetSettingResult_Missing;
}
TryGetSettingResult_t PigTryGetSettingR32(const PigSettings_t* settings, const char* keyNullTerm, r32* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	return PigTryGetSettingR32(settings, NewStr(keyNullTerm), valueOut, ignoreCase, parseFailureReasonOut);
}
r32 PigGetSettingR32(const PigSettings_t* settings, MyStr_t key, r32 defaultValue, bool ignoreCase = true)
{
	r32 result = defaultValue;
	TryGetSettingResult_t foundSetting = PigTryGetSettingR32(settings, key, &result, ignoreCase);
	//TODO: If the setting is unparsable, we should do a debug output here with info maybe?
	DebugAssert(foundSetting == TryGetSettingResult_Success || foundSetting == TryGetSettingResult_Missing);
	return result;
}
r32 PigGetSettingR32(const PigSettings_t* settings, const char* keyNullTerm, r32 defaultValue, bool ignoreCase = true)
{
	return PigGetSettingR32(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}

TryGetSettingResult_t PigTryGetSettingU64(const PigSettings_t* settings, MyStr_t key, u64* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	MyStr_t settingValue;
	if (PigTryGetSettingStr(settings, key, &settingValue, ignoreCase))
	{
		if (TryParseU64(settingValue, valueOut, parseFailureReasonOut)) { return TryGetSettingResult_Success; }
		else { return TryGetSettingResult_ParseError; }
	}
	return TryGetSettingResult_Missing;
}
TryGetSettingResult_t PigTryGetSettingU64(const PigSettings_t* settings, const char* keyNullTerm, u64* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	return PigTryGetSettingU64(settings, NewStr(keyNullTerm), valueOut, ignoreCase, parseFailureReasonOut);
}
u64 PigGetSettingU64(const PigSettings_t* settings, MyStr_t key, u64 defaultValue, bool ignoreCase = true)
{
	u64 result = defaultValue;
	TryGetSettingResult_t foundSetting = PigTryGetSettingU64(settings, key, &result, ignoreCase);
	//TODO: If the setting is unparsable, we should do a debug output here with info maybe?
	DebugAssert(foundSetting == TryGetSettingResult_Success || foundSetting == TryGetSettingResult_Missing);
	return result;
}
u64 PigGetSettingU64(const PigSettings_t* settings, const char* keyNullTerm, u64 defaultValue, bool ignoreCase = true)
{
	return PigGetSettingU64(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}

TryGetSettingResult_t PigTryGetSettingV2(const PigSettings_t* settings, MyStr_t key, v2* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	MyStr_t settingValue;
	if (PigTryGetSettingStr(settings, key, &settingValue, ignoreCase))
	{
		if (TryParseV2(settingValue, valueOut, parseFailureReasonOut)) { return TryGetSettingResult_Success; }
		else { return TryGetSettingResult_ParseError; }
	}
	return TryGetSettingResult_Missing;
}
TryGetSettingResult_t PigTryGetSettingV2(const PigSettings_t* settings, const char* keyNullTerm, v2* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	return PigTryGetSettingV2(settings, NewStr(keyNullTerm), valueOut, ignoreCase, parseFailureReasonOut);
}
v2 PigGetSettingV2(const PigSettings_t* settings, MyStr_t key, v2 defaultValue, bool ignoreCase = true)
{
	v2 result = defaultValue;
	TryGetSettingResult_t foundSetting = PigTryGetSettingV2(settings, key, &result, ignoreCase);
	//TODO: If the setting is unparsable, we should do a debug output here with info maybe?
	DebugAssert(foundSetting == TryGetSettingResult_Success || foundSetting == TryGetSettingResult_Missing);
	return result;
}
v2 PigGetSettingV2(const PigSettings_t* settings, const char* keyNullTerm, v2 defaultValue, bool ignoreCase = true)
{
	return PigGetSettingV2(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}

TryGetSettingResult_t PigTryGetSettingV2i(const PigSettings_t* settings, MyStr_t key, v2i* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	NotNull(settings);
	NotNullStr(&key);
	MyStr_t settingValue;
	if (PigTryGetSettingStr(settings, key, &settingValue, ignoreCase))
	{
		if (TryParseV2i(settingValue, valueOut, parseFailureReasonOut)) { return TryGetSettingResult_Success; }
		else { return TryGetSettingResult_ParseError; }
	}
	return TryGetSettingResult_Missing;
}
TryGetSettingResult_t PigTryGetSettingV2i(const PigSettings_t* settings, const char* keyNullTerm, v2i* valueOut = nullptr, bool ignoreCase = true, TryParseFailureReason_t* parseFailureReasonOut = nullptr)
{
	return PigTryGetSettingV2i(settings, NewStr(keyNullTerm), valueOut, ignoreCase, parseFailureReasonOut);
}
v2i PigGetSettingV2i(const PigSettings_t* settings, MyStr_t key, v2i defaultValue, bool ignoreCase = true)
{
	v2i result = defaultValue;
	TryGetSettingResult_t foundSetting = PigTryGetSettingV2i(settings, key, &result, ignoreCase);
	//TODO: If the setting is unparsable, we should do a debug output here with info maybe?
	DebugAssert(foundSetting == TryGetSettingResult_Success || foundSetting == TryGetSettingResult_Missing);
	return result;
}
v2i PigGetSettingV2i(const PigSettings_t* settings, const char* keyNullTerm, v2i defaultValue, bool ignoreCase = true)
{
	return PigGetSettingV2i(settings, NewStr(keyNullTerm), defaultValue, ignoreCase);
}
