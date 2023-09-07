/*
File:   pig_model_armature_serialization.cpp
Author: Taylor Robbins
Date:   08\29\2023
Description: 
	** Holds the code that deserializes ModelArmature_t data from our own custom format
*/

#define PIG_PARTS_MODEL_ARMATURE_FILE_PREFIX        "# Pig Engine Armature Data"
#define PIG_PARTS_MODEL_ARMATURE_FILE_PREFIX_LENGTH 26

enum TryDeserModelArmatureError_t
{
	TryDeserModelArmatureError_None = 0,
	TryDeserModelArmatureError_CouldntOpenFile,
	TryDeserModelArmatureError_EmptyFile,
	TryDeserModelArmatureError_MissingFilePrefix,
	TryDeserModelArmatureError_MissingRequiredFields,
	TryDeserModelArmatureError_UnknownBoneName,
	TryDeserModelArmatureError_MissingOrCorruptBones,
	TryDeserModelArmatureError_MissingOrCorruptChildren,
	TryDeserModelArmatureError_UnknownParentBoneName,
	TryDeserModelArmatureError_NumErrors,
};
const char* GetTryDeserModelArmatureErrorStr(TryDeserModelArmatureError_t enumValue)
{
	switch (enumValue)
	{
		case TryDeserModelArmatureError_None:                     return "None";
		case TryDeserModelArmatureError_CouldntOpenFile:          return "CouldntOpenFile";
		case TryDeserModelArmatureError_EmptyFile:                return "EmptyFile";
		case TryDeserModelArmatureError_MissingFilePrefix:        return "MissingFilePrefix";
		case TryDeserModelArmatureError_MissingRequiredFields:    return "MissingRequiredFields";
		case TryDeserModelArmatureError_UnknownBoneName:          return "UnknownBoneName";
		case TryDeserModelArmatureError_MissingOrCorruptBones:    return "MissingOrCorruptBones";
		case TryDeserModelArmatureError_MissingOrCorruptChildren: return "MissingOrCorruptChildren";
		case TryDeserModelArmatureError_UnknownParentBoneName:    return "UnknownParentBoneName";
		default: return "Unknown";
	}
}

//NOTE: You should call DestroyModelArmature on armatureOut even if this function fails!
bool TryDeserModelArmature(MyStr_t armatureFileContents, ProcessLog_t* log, ModelArmature_t* armatureOut, MemArena_t* memArena)
{
	NotNull3(log, armatureOut, memArena);
	NotNullStr(&armatureFileContents);
	MemArena_t* scratch = GetScratchArena(memArena);
	TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
	
	SetProcessLogName(log, NewStr("TryDeserModelArmature"));
	
	MyStr_t fileContents = armatureFileContents;
	if (fileContents.length == 0)
	{
		LogWriteLine_E(log, "The file is empty!");
		LogExitFailure(log, TryDeserModelArmatureError_EmptyFile);
		FreeScratchArena(scratch);
		return false;
	}
	if (!StrStartsWith(fileContents, PIG_PARTS_MODEL_ARMATURE_FILE_PREFIX))
	{
		LogPrintLine_E(log, "The file is missing the required header: %s", PIG_PARTS_MODEL_ARMATURE_FILE_PREFIX);
		LogExitFailure(log, TryDeserModelArmatureError_MissingFilePrefix);
		FreeScratchArena(scratch);
		return false;
	}
	fileContents = StrSubstring(&fileContents, PIG_PARTS_MODEL_ARMATURE_FILE_PREFIX_LENGTH);
	
	bool foundNumBones = false;
	u64 numBones = 0;
	bool foundNumChildren = false;
	u64 numChildren = 0;
	bool foundArmatureName = false;
	MyStr_t armatureName = MyStr_Empty;
	bool createdArmature = false;
	
	u64 numBonesFound = 0;
	u64 numChildrenFound = 0;
	bool foundRootBone = false;
	
	TextParser_t parser = NewTextParser(fileContents);
	ParsingToken_t token = {};
	while (TextParserGetToken(&parser, &token))
	{
		if (!createdArmature)
		{
			if ((token.type != ParsingTokenType_KeyValuePair && token.type != ParsingTokenType_Comment) ||
				(token.type == ParsingTokenType_KeyValuePair && (!StrEqualsIgnoreCase(token.key, "NumBones") && !StrEqualsIgnoreCase(token.key, "NumChildren") && !StrEqualsIgnoreCase(token.key, "ArmatureName"))))
			{
				LogPrintLine_E(log, "Found %s on line %llu, before NumBones/NumChildren/ArmatureName were specified!", GetParsingTokenTypeStr(token.type), parser.lineParser.lineIndex);
				LogExitFailure(log, TryDeserModelArmatureError_MissingRequiredFields);
				FreeScratchArena(scratch);
				return false;
			}
		}
		
		switch (token.type)
		{
			case ParsingTokenType_FilePrefix:
			{
				TrimWhitespace(&token.value);
				if (StrEqualsIgnoreCase(token.value, "Bones"))
				{
					//TODO: Do we really need these headers? They make the file look nice, but we don't actually care
				}
				else if (StrEqualsIgnoreCase(token.value, "Children"))
				{
					//TODO: Do we really need these headers? They make the file look nice, but we don't actually care
				}
				else
				{
					LogPrintLine_W(log, "Unknown header \"%.*s\" on line %llu", token.value.length, token.value.chars, parser.lineParser.lineIndex);
					log->hadWarnings = true;
				}
			} break;
			
			case ParsingTokenType_KeyValuePair:
			{
				// +==============================+
				// |         ArmatureName         |
				// +==============================+
				if (StrEqualsIgnoreCase(token.key, "ArmatureName"))
				{
					if (foundArmatureName) { LogPrintLine_W(log, "Found another \"ArmatureName\" line on line %llu", parser.lineParser.lineIndex); log->hadWarnings = true; continue; }
					armatureName = token.value;
					foundArmatureName = true;
				}
				// +==============================+
				// |           NumBones           |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "NumBones"))
				{
					if (foundNumBones) { LogPrintLine_W(log, "Found another \"NumBones\" line on line %llu", parser.lineParser.lineIndex); log->hadWarnings = true; continue; }
					
					if (TryParseU64(token.value, &numBones, &parseFailureReason))
					{
						foundNumBones = true;
					}
					else
					{
						LogPrintLine_W(log, "Couldn't parse NumBones \"%.*s\" as u64 %s on line %llu", token.value.length, token.value.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
						log->hadWarnings = true;
					}
				}
				// +==============================+
				// |         NumChildren          |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "NumChildren"))
				{
					if (foundNumChildren) { LogPrintLine_W(log, "Found another \"NumChildren\" line on line %llu", parser.lineParser.lineIndex); log->hadWarnings = true; continue; }
					
					if (TryParseU64(token.value, &numChildren, &parseFailureReason))
					{
						foundNumChildren = true;
					}
					else
					{
						LogPrintLine_W(log, "Couldn't parse NumChildren \"%.*s\" as u64 %s on line %llu", token.value.length, token.value.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
						log->hadWarnings = true;
					}
				}
				// +==============================+
				// |             Bone             |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "Bone"))
				{
					PushMemMark(scratch);
					u64 numParts = 0;
					MyStr_t* parts = SplitString(scratch, token.value, "|", &numParts);
					if (numParts == 5)
					{
						MyStr_t namePart = parts[0];
						MyStr_t parentNamePart = parts[1];
						MyStr_t headPart = parts[2];
						MyStr_t tailPart = parts[3];
						MyStr_t transformPart = parts[4];
						TrimWhitespace(&namePart);
						TrimWhitespace(&parentNamePart);
						TrimWhitespace(&headPart);
						TrimWhitespace(&tailPart);
						TrimWhitespace(&transformPart);
						
						bool isRootBone = true;
						ModelArmatureBone_t* parentBone = nullptr;
						if (!StrEqualsIgnoreCase(parentNamePart, "None"))
						{
							parentBone = FindArmatureBoneByName(armatureOut, parentNamePart);
							isRootBone = false;
						}
						
						if (isRootBone && foundRootBone)
						{
							LogPrintLine_W(log, "Found more than one root bone in armature! \"%.*s\" is second root bone (on line %llu)", namePart.length, namePart.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
							log->hadWarnings = true;
							isRootBone = false;
							parentBone = FindArmatureBoneById(armatureOut, armatureOut->rootBoneId);
							NotNull(parentBone);
						}
						
						u64 parentBoneId = (parentBone != nullptr) ? parentBone->id : 0;
						
						v3 head = Vec3_Zero;
						v3 tail = Vec3_Zero;
						mat4 transform = Mat4_Identity;
						if (!TryParseV3(headPart, &head, &parseFailureReason))
						{
							LogPrintLine_W(log, "Couldn't parse HeadPos \"%.*s\" as v3 %s (on line %llu)", headPart.length, headPart.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
							log->hadWarnings = true;
						}
						else if (!TryParseV3(tailPart, &tail, &parseFailureReason))
						{
							LogPrintLine_W(log, "Couldn't parse TailPos \"%.*s\" as v3 %s (on line %llu)", tailPart.length, tailPart.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
							log->hadWarnings = true;
						}
						else if (!TryParseMat4(transformPart, &transform, &parseFailureReason))
						{
							LogPrintLine_W(log, "Couldn't parse Transform \"%.*s\" as mat4 %s (on line %llu)", transformPart.length, transformPart.chars, GetTryParseFailureReasonStr(parseFailureReason), parser.lineParser.lineIndex);
							log->hadWarnings = true;
						}
						else if (!isRootBone && parentBone == nullptr)
						{
							LogPrintLine_E(log, "Unknown bone name \"%.*s\" for parent on bone \"%.*s\" (on line %llu). This could mean the bones are not ordered properly, or it could mean a bone is missing", parentNamePart.length, parentNamePart.chars, namePart.length, namePart.chars, parser.lineParser.lineIndex);
							LogExitFailure(log, TryDeserModelArmatureError_UnknownParentBoneName);
							PopMemMark(scratch);
							FreeScratchArena(scratch);
							return false;
						}
						else
						{
							ModelArmatureBone_t* newBone = AddModelArmatureBone(armatureOut, namePart);
							NotNull(newBone);
							newBone->head = head;
							newBone->tail = tail;
							newBone->transform = transform;
							newBone->parentBoneId = parentBoneId;
							
							LogPrintLine_D(log, "\"%.*s\" transform\n%g, %g, %g, %g\n%g, %g, %g, %g\n%g, %g, %g, %g\n%g, %g, %g, %g",
								namePart.length, namePart.chars,
								newBone->transform.values[0][0], newBone->transform.values[1][0], newBone->transform.values[2][0], newBone->transform.values[3][0],
								newBone->transform.values[0][1], newBone->transform.values[1][1], newBone->transform.values[2][1], newBone->transform.values[3][1],
								newBone->transform.values[0][2], newBone->transform.values[1][2], newBone->transform.values[2][2], newBone->transform.values[3][2],
								newBone->transform.values[0][3], newBone->transform.values[1][3], newBone->transform.values[2][3], newBone->transform.values[3][3]
							);
							log->hadWarnings = true; //TODO: Remove me!
							
							if (isRootBone)
							{
								foundRootBone = true;
								armatureOut->rootBoneId = newBone->id;
							}
							
							numBonesFound++;
						}
					}
					else
					{
						LogPrintLine_W(log, "Found %llu pieces separated by | on line %llu. We expect 4 pieces: Name|HeadPos|TailPos|Transform. \"%.*s\"", numParts, parser.lineParser.lineIndex, token.value.length, token.value.chars);
						log->hadWarnings = true;
					}
					PopMemMark(scratch);
				}
				// +==============================+
				// |            Child             |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "Child"))
				{
					PushMemMark(scratch);
					u64 numParts = 0;
					MyStr_t* parts = SplitString(scratch, token.value, "|", &numParts);
					if (numParts == 2)
					{
						MyStr_t boneNamePart = parts[0];
						MyStr_t childNamePart = parts[1];
						TrimWhitespace(&boneNamePart);
						TrimWhitespace(&childNamePart);
						
						ModelArmatureBone_t* bone = FindArmatureBoneByName(armatureOut, boneNamePart);
						if (bone != nullptr)
						{
							MyStr_t* partNamePntr = VarArrayAdd(&bone->partNames, MyStr_t);
							NotNull(partNamePntr);
							*partNamePntr = AllocString(armatureOut->allocArena, &childNamePart);
							numChildrenFound++;
						}
						else
						{
							LogPrintLine_W(log, "Unknown bone name \"%.*s\" for child \"%.*s\" on line %llu", boneNamePart.length, boneNamePart.chars, childNamePart.length, childNamePart.chars, parser.lineParser.lineIndex);
							LogExitFailure(log, TryDeserModelArmatureError_UnknownBoneName);
							PopMemMark(scratch);
							FreeScratchArena(scratch);
							return false;
						}
					}
					else
					{
						LogPrintLine_W(log, "Found %llu pieces separated by | on line %llu. We expect 2 pieces: BoneName|ChildName. \"%.*s\"", numParts, parser.lineParser.lineIndex, token.value.length, token.value.chars);
						log->hadWarnings = true;
					}
					PopMemMark(scratch);
				}
				else
				{
					LogPrintLine_W(log, "Unknown Key %.*s on line %llu", token.key.length, token.key.chars, parser.lineParser.lineIndex);
					log->hadWarnings = true;
				}
			} break;
			
			case ParsingTokenType_Comment: /* Do nothing */ break;
			
			default:
			{
				LogPrintLine_W(log, "Unhandled token type in TryDeserModelArmature: %s", GetParsingTokenTypeStr(token.type));
				log->hadWarnings = true;
			} break;
		}
		
		if (foundNumBones && foundNumChildren && foundArmatureName && !createdArmature)
		{
			CreateModelArmature(armatureOut, memArena, numBones);
			armatureOut->name = AllocString(armatureOut->allocArena, &armatureName);
			createdArmature = true;
		}
	}
	
	if (!createdArmature)
	{
		LogWriteLine_E(log, "The file is missing some required fields!");
		if (!foundArmatureName) { LogWriteLine_E(log, "Missing ArmatureName!"); }
		if (!foundNumBones) { LogWriteLine_E(log, "Missing NumBones!"); }
		if (!foundNumChildren) { LogWriteLine_E(log, "Missing NumChildren!"); }
		LogExitFailure(log, TryDeserModelArmatureError_MissingRequiredFields);
		FreeScratchArena(scratch);
		return false;
	}
	if (!foundRootBone)
	{
		LogPrintLine_E(log, "There was no root bone for the model");
	}
	if (numBonesFound != numBones)
	{
		if (numBonesFound > numBones)
		{
			LogPrintLine_W(log, "Found %llu bones instead of %llu that was declared at the top of the file!", numBonesFound, numBones);
			log->hadWarnings = true;
		}
		else
		{
			LogPrintLine_E(log, "Found %llu bones instead of %llu that was declared at the top of the file!", numBonesFound, numBones);
			LogExitFailure(log, TryDeserModelArmatureError_MissingOrCorruptBones);
			FreeScratchArena(scratch);
			return false;
		}
	}
	if (numChildrenFound != numChildren)
	{
		if (numChildrenFound > numChildren)
		{
			LogPrintLine_W(log, "Found %llu children instead of %llu that was declared at the top of the file!", numChildrenFound, numChildren);
			log->hadWarnings = true;
		}
		else
		{
			LogPrintLine_E(log, "Found %llu children instead of %llu that was declared at the top of the file!", numChildrenFound, numChildren);
			LogExitFailure(log, TryDeserModelArmatureError_MissingOrCorruptChildren);
			FreeScratchArena(scratch);
			return false;
		}
	}
	
	ModelArmatureUpdateBonePntrs(armatureOut);
	LogExitSuccess(log);
	FreeScratchArena(scratch);
	return true;
}

//NOTE: You should call DestroyModelArmature on armatureOut even if this function fails!
bool TryLoadModelArmatureFrom(MyStr_t armatureFilePath, ProcessLog_t* log, ModelArmature_t* armatureOut, MemArena_t* memArena)
{
	NotNull(log);
	NotNullStr(&armatureFilePath);
	
	SetProcessLogName(log, NewStr("TryLoadModelArmatureFrom"));
	SetProcessLogFilePath(log, armatureFilePath);
	
	PlatFileContents_t armatureFile;
	if (plat->ReadFileContents(armatureFilePath, &armatureFile))
	{
		MyStr_t armatureFileContents = NewStr(armatureFile.length, armatureFile.chars);
		bool success = TryDeserModelArmature(armatureFileContents, log, armatureOut, memArena);
		plat->FreeFileContents(&armatureFile);
		return success;
	}
	else
	{
		LogPrintLine_E(log, "Failed to open model armature file at \"%.*s\"", armatureFilePath.length, armatureFilePath.chars);
		LogExitFailure(log, TryDeserModelArmatureError_CouldntOpenFile);
		return false;
	}
}
