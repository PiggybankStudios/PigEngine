/*
File:   piggen_parse.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds the ParsePigGenRegionContents function
*/

#define ALL_SERIALIZABLE_STRUCTS_PLACEHOLDER_STRING "// {{{{AllSerializableStructs}}}}"

// +--------------------------------------------------------------+
// |                            Parse                             |
// +--------------------------------------------------------------+
//Returns MyStr_Empty on failure
bool TryPigGenerate(MyStr_t regionContents, OpenFile_t* outputFile, VarArray_t* allSerializableStructs, bool* hasPlaceholdersOut, ProcessLog_t* log, u64 baseLineIndex)
{
	NotNullStr(&regionContents);
	NotNull(outputFile);
	NotNull(allSerializableStructs);
	NotNull(hasPlaceholdersOut);
	NotNull(log);
	
	bool insideStruct = false;
	u64 structStartLine = 0;
	bool expectingOpenCurly = false;
	SerializableStruct_t currentStruct = {};
	
	MyStr_t fileComment = PigGenGenerateFileComment(TempArena, log->filePath);
	WriteToFile(outputFile, fileComment.length, fileComment.chars);
	
	*hasPlaceholdersOut = false;
	
	TextParser_t textParser = NewTextParser(regionContents);
	ParsingToken_t token;
	while (TextParserGetToken(&textParser, &token))
	{
		Assert(!IsEmptyStr(token.str)); //assumption was made that we don't get empty tokens
		Assert(textParser.lineParser.lineIndex > 0);
		u64 lineNum = baseLineIndex + textParser.lineParser.lineIndex-1;
		
		if (insideStruct && expectingOpenCurly && token.type != ParsingTokenType_Comment && !StrEquals(token.str, "{"))
		{
			LogPrintLine_E(log, "Expected curly bracket after serializable struct \"%.*s\" on line %llu. Instead we found \"%.*s\"",
				currentStruct.name.length, currentStruct.name.chars,
				baseLineIndex + structStartLine,
				token.str.length, token.str.chars
			);
			LogExitFailure(log, PigGenParseError_ExpectedOpenCurlyBracket);
			FreeSerializableStruct(&currentStruct);
			return false;
		}
		
		if (token.type == ParsingTokenType_KeyValuePair)
		{
			if (!insideStruct && StrEqualsIgnoreCase(token.key, "Serializable"))
			{
				if (!IsStringValidIdentifier(token.value))
				{
					LogPrintLine_E(log, "Serializable struct \"%.*s\" on line %llu has an invalid identifier for the name. Struct names can only contain letters, numbers, and underscores (no starting with numbers)", token.value.length, token.value.chars, lineNum);
					LogExitFailure(log, PigGenParseError_InvalidIdentifierForStructName);
					TempPopMark();
					return false;
				}
				
				NewSerializableStruct(&currentStruct, mainHeap, token.value);
				insideStruct = true;
				structStartLine = textParser.lineParser.lineIndex;
				expectingOpenCurly = true;
			}
			else if (insideStruct)
			{
				bool isOptionalMember = StrEndsWith(token.value, "?");
				if (isOptionalMember) { token.value = StrSubstring(&token.value, 0, token.value.length-1); }
				
				TempPushMark();
				u64 numTypeStrParts = 0;
				MyStr_t* typeStrParts = SplitString(TempArena, token.value, "/", &numTypeStrParts);
				Assert(numTypeStrParts > 0);
				if (numTypeStrParts > 2)
				{
					LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu can only have 2 types, separated by forward slash, not %llu types", token.key.length, token.key.chars, lineNum, numTypeStrParts);
					LogExitFailure(log, PigGenParseError_TooManyTypes);
					FreeSerializableStruct(&currentStruct);
					TempPopMark();
					return false;
				}
				TrimWhitespace(&typeStrParts[0]);
				if (numTypeStrParts >= 2) { TrimWhitespace(&typeStrParts[1]); }
				if (IsEmptyStr(typeStrParts[0]) || numTypeStrParts >= 2 && IsEmptyStr(typeStrParts[1]))
				{
					if (numTypeStrParts == 1)
					{
						LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu has an empty type. Please include a type for the member after the colon", token.key.length, token.key.chars, lineNum);
					}
					else
					{
						LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu has an empty type part. Please remove a slash or make sure both sides of the slash have valid types", token.key.length, token.key.chars, lineNum);
					}
					LogExitFailure(log, PigGenParseError_MissingTypePart);
					TempPopMark();
					FreeSerializableStruct(&currentStruct);
					return false;
				}
				if (!IsStringValidIdentifier(token.key))
				{
					LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu has an invalid name. Member names can only contain letters, numbers, and underscores (no starting with numbers)", token.key.length, token.key.chars, lineNum);
					LogExitFailure(log, PigGenParseError_InvalidIdentifierForMemberName);
					TempPopMark();
					FreeSerializableStruct(&currentStruct);
					return false;
				}
				if (!IsStringValidIdentifier(typeStrParts[0]))
				{
					LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu has an invalid type \"%.*s\". Member types can only contain letters, numbers, and underscores (no starting with numbers)", token.key.length, token.key.chars, lineNum, typeStrParts[0].length, typeStrParts[0].chars);
					LogExitFailure(log, PigGenParseError_InvalidIdentifierForMemberType);
					TempPopMark();
					FreeSerializableStruct(&currentStruct);
					return false;
				}
				if (numTypeStrParts >= 2 && !IsStringValidIdentifier(typeStrParts[1]))
				{
					LogPrintLine_E(log, "Serializable struct member \"%.*s\" on line %llu has an invalid alternate type \"%.*s\". Member types can only contain letters, numbers, and underscores (no starting with numbers)", token.key.length, token.key.chars, lineNum, typeStrParts[1].length, typeStrParts[1].chars);
					LogExitFailure(log, PigGenParseError_InvalidIdentifierForMemberType);
					TempPopMark();
					FreeSerializableStruct(&currentStruct);
					return false;
				}
				
				SerializableStructMember_t* newMember = AddSerializableStructMember(&currentStruct, token.key, typeStrParts[0], (numTypeStrParts >= 2 ? typeStrParts[1] : MyStr_Empty));
				NotNull(newMember);
				newMember->isOptional = isOptionalMember;
				
				TempPopMark();
			}
			else
			{
				//TODO: What other kinds of key-value pairs can we find outside structs
				LogPrintLine_W(log, "Unknown key, outside serializable struct: \"%.*s\" on line %llu", token.key.length, token.key.chars, lineNum);
			}
		}
		else if (token.type == ParsingTokenType_Unknown && insideStruct && expectingOpenCurly && StrEquals(token.str, "{"))
		{
			expectingOpenCurly = false;
		}
		else if (token.type == ParsingTokenType_Unknown && insideStruct && StrEquals(token.str, "}"))
		{
			if (currentStruct.members.length == 0)
			{
				LogPrintLine_E(log, "Serializable struct \"%.*s\" on line %llu contained no members!", currentStruct.name.length, currentStruct.name.chars, baseLineIndex + structStartLine);
				LogExitFailure(log, PigGenParseError_NoMembersInStruct);
				FreeSerializableStruct(&currentStruct);
				return false;
			}
			
			TempPushMark();
			
			MyStr_t structCode = PigGenGenerateSerializableStructCode(TempArena, &currentStruct);
			if (IsEmptyStr(structCode))
			{
				LogPrintLine_E(log, "Failed to generate code for serializable struct \"%.*s\" on line %llu", currentStruct.name.length, currentStruct.name.chars, baseLineIndex + structStartLine);
				LogExitFailure(log, PigGenParseError_FailedToGenerateSerializableStruct);
				FreeSerializableStruct(&currentStruct);
				return false;
			}
			// LogPrintLine_D(log, "Serialized Struct Code:\n%.*s", structCode.length, structCode.chars);
			WriteToFile(outputFile, structCode.length, structCode.chars);
			TempPopMark();
			
			SerializableStruct_t* addedStruct = VarArrayAdd(allSerializableStructs, SerializableStruct_t);
			NotNull(addedStruct);
			MyMemCopy(addedStruct, &currentStruct, sizeof(SerializableStruct_t));
			ClearStruct(currentStruct);
			
			insideStruct = false;
		}
		else if (token.type == ParsingTokenType_Directive)
		{
			if (StrEqualsIgnoreCase(token.value, "AllSerializableStructs"))
			{
				MyStr_t placeholderString = TempPrintStr(PIGGEN_NEW_LINE PIGGEN_NEW_LINE "%s" PIGGEN_NEW_LINE PIGGEN_NEW_LINE, ALL_SERIALIZABLE_STRUCTS_PLACEHOLDER_STRING);
				WriteToFile(outputFile, placeholderString.length, placeholderString.chars);
				*hasPlaceholdersOut = true;
			}
		}
		else if (token.type == ParsingTokenType_Comment)
		{
			//Nothing we need to do (TODO: For now?)
		}
		else
		{
			LogPrintLine_W(log, "Unhandled token type \"%s\" (0x%02X)", GetParsingTokenTypeStr(token.type), token.type);
		}
	}
	
	if (insideStruct)
	{
		LogPrintLine_E(log, "Serializable struct \"%.*s\" on line %llu had no ending curly brace!", currentStruct.name.length, currentStruct.name.chars, baseLineIndex + structStartLine);
		LogExitFailure(log, PigGenParseError_NoMembersInStruct);
		FreeSerializableStruct(&currentStruct);
		return false;
	}
	
	LogExitSuccess(log);
	return true;
}