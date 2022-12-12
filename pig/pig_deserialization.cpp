/*
File:   pig_deserialization.cpp
Author: Taylor Robbins
Date:   01\10\2022
Description: 
	** Holds a bunch of helper functions for use when deserializing files of various formats
*/

// +--------------------------------------------------------------+
// |                Binary Deserialization Helpers                |
// +--------------------------------------------------------------+
const void* BinDeser_ReadStruct_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64 structureSize)
{
	NotNull2(dataPntr, byteIndexPntr);
	if ((*byteIndexPntr) + structureSize > dataSize) { return nullptr; }
	const void* result = ((u8*)dataPntr) + (*byteIndexPntr);
	*byteIndexPntr = (*byteIndexPntr) + structureSize;
	return result;
}
#define BinDeser_ReadStruct(dataPntr, dataSize, byteIndexPntr, type) (type*)BinDeser_ReadStruct_((dataPntr), (dataSize), (byteIndexPntr), sizeof(type))

const void* BinDeser_ReadStructDynamicSize_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64* structSizeOut, u64 sizeMemberOffset, u64 minStructSize, u64 maxStructSize)
{
	NotNull2(dataPntr, byteIndexPntr);
	Assert (sizeMemberOffset + sizeof(u64) <= minStructSize);
	if ((*byteIndexPntr) + minStructSize > dataSize) { return nullptr; }
	const u64* sizePntr = (const u64*)(((u8*)dataPntr) + (*byteIndexPntr) + sizeMemberOffset);
	if (*sizePntr < minStructSize) { return nullptr; }
	if (*sizePntr > maxStructSize) { return nullptr; }
	if (structSizeOut != nullptr) { *structSizeOut = *sizePntr; }
	const void* result = ((u8*)dataPntr) + (*byteIndexPntr);
	*byteIndexPntr = (*byteIndexPntr) + (*sizePntr);
	return result;
}
#define BinDeser_ReadStructDynamicSize(dataPntr, dataSize, byteIndexPntr, structSizeOut, type, sizeMemberName, firstOptionalMemberName) (type*)BinDeser_ReadStructDynamicSize_((dataPntr), (dataSize), (byteIndexPntr), (structSizeOut), STRUCT_VAR_OFFSET(type, sizeMemberName), STRUCT_VAR_OFFSET(type, firstOptionalMemberName), sizeof(type))

#define BinDeser_IsMemberPresent(structSize, type, memberName) (structSize >= (STRUCT_VAR_OFFSET(type, memberName) + sizeof(((type*)0)->memberName)))

bool BinDeser_ReadVariable_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64 valueSize, void* valueOutPntr)
{
	NotNull3(dataPntr, byteIndexPntr, valueOutPntr);
	if ((*byteIndexPntr) + valueSize > dataSize) { return false; }
	const void* valueReadPntr = ((u8*)dataPntr) + (*byteIndexPntr);
	MyMemCopy(valueOutPntr, valueReadPntr, valueSize);
	*byteIndexPntr = (*byteIndexPntr) + valueSize;
	return true;
}
#define BinDeser_ReadU8(dataPntr, dataSize, byteIndexPntr, variableOut)  BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u8),  (void*)(variableOut))
#define BinDeser_ReadU16(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u16), (void*)(variableOut))
#define BinDeser_ReadU32(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u32), (void*)(variableOut))
#define BinDeser_ReadU64(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u64), (void*)(variableOut))
#define BinDeser_ReadI8(dataPntr, dataSize, byteIndexPntr, variableOut)  BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i8),  (void*)(variableOut))
#define BinDeser_ReadI16(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i16), (void*)(variableOut))
#define BinDeser_ReadI32(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i32), (void*)(variableOut))
#define BinDeser_ReadI64(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i64), (void*)(variableOut))

// +--------------------------------------------------------------+
// |                          LineParser                          |
// +--------------------------------------------------------------+
//This is a simple utility that helps a deserialization process break a string into lines
//Handles stuff line /n vs /r/n and keeping track of the lineIndex
struct LineParser_t
{
	u64 byteIndex;
	u64 lineIndex;
	MyStr_t fileContents;
};

LineParser_t NewLineParser(MyStr_t fileContents)
{
	NotNullStr(&fileContents);
	LineParser_t result = {};
	result.byteIndex = 0;
	result.lineIndex = 0;
	result.fileContents = fileContents;
	return result;
}

bool LineParserGetLine(LineParser_t* parser, MyStr_t* lineOut)
{
	NotNull(parser);
	NotNullStr(&parser->fileContents);
	if (parser->byteIndex >= parser->fileContents.length) { return false; }
	parser->lineIndex++;
	
	u64 endOfLineByteSize = 0;
	u64 startIndex = parser->byteIndex;
	while (parser->byteIndex < parser->fileContents.length)
	{
		char nextChar = parser->fileContents.pntr[parser->byteIndex];
		char nextNextChar = parser->fileContents.pntr[parser->byteIndex+1];
		if (nextChar != nextNextChar &&
			(nextChar     == '\n' || nextChar     == '\r') &&
			(nextNextChar == '\n' || nextNextChar == '\r'))
		{
			endOfLineByteSize = 2;
			break;
		}
		else if (nextChar == '\n' || nextChar == '\r')
		{
			endOfLineByteSize = 1;
			break;
		}
		else
		{
			parser->byteIndex++;
		}
	}
	
	MyStr_t line = NewStr(parser->byteIndex - startIndex, &parser->fileContents.pntr[startIndex]);
	parser->byteIndex += endOfLineByteSize;
	if (lineOut != nullptr) { *lineOut = line; }
	return true;
}

// +--------------------------------------------------------------+
// |                          TextParser                          |
// +--------------------------------------------------------------+
enum ParsingTokenType_t
{
	ParsingTokenType_Unknown = 0,
	ParsingTokenType_FilePrefix,
	ParsingTokenType_KeyValuePair,
	ParsingTokenType_Comment,
	ParsingTokenType_NumTypes,
};
struct ParsingToken_t
{
	ParsingTokenType_t type;
	MyStr_t str;
	MyStr_t key;
	MyStr_t value;
};
struct TextParser_t
{
	LineParser_t lineParser;
	MyStr_t currentLine;
	u64 byteIndex;
};

const char* GetParsingTokenTypeStr(ParsingTokenType_t type)
{
	switch (type)
	{
		case ParsingTokenType_Unknown:      return "Unknown";
		case ParsingTokenType_FilePrefix:   return "FilePrefix";
		case ParsingTokenType_KeyValuePair: return "KeyValuePair";
		case ParsingTokenType_Comment:      return "Comment";
		default: return "UNKNOWN";
	}
}

TextParser_t NewTextParser(MyStr_t fileContents)
{
	TextParser_t result = {};
	result.lineParser = NewLineParser(fileContents);
	result.byteIndex = 0;
	result.currentLine = MyStr_Empty;
	return result;
}

bool TextParserGetToken(TextParser_t* parser, ParsingToken_t* tokenOut)
{
	NotNull(parser);
	NotNull(tokenOut);
	
	while (parser->lineParser.byteIndex < parser->lineParser.fileContents.length)
	{
		if (parser->byteIndex >= parser->currentLine.length)
		{
			bool gotLine = LineParserGetLine(&parser->lineParser, &parser->currentLine);
			if (!gotLine)
			{
				return false;
			}
			parser->byteIndex = 0;
		}
		
		MyStr_t line = StrSubstring(&parser->currentLine, parser->byteIndex);
		u64 numTrimmedWhitespaceChars = TrimLeadingWhitespace(&line);
		if (line.length == 0)
		{
			parser->byteIndex = parser->currentLine.length;
			continue;
		}
		
		//TODO: This doesn't handle if a // shows up inside something like a string where it shouldn't be treated as a comment
		u64 commentStartIndex = 0;
		bool lineContainsComment = FindSubstring(line, "//", &commentStartIndex);
		
		if (lineContainsComment && commentStartIndex == 0)
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_Comment;
			tokenOut->str = line;
			tokenOut->value = StrSubstring(&tokenOut->str, 2, tokenOut->str.length);
			parser->byteIndex = parser->currentLine.length;
			return true;
		}
		else if (lineContainsComment)
		{
			line = StrSubstring(&line, 0, commentStartIndex);
		}
		
		u64 colonIndex = 0;
		bool lineContainsColon = FindSubstring(line, ":", &colonIndex);
		
		if (lineContainsColon)
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_KeyValuePair;
			tokenOut->str = line;
			tokenOut->key = StrSubstring(&line, 0, colonIndex);
			tokenOut->value = StrSubstring(&line, colonIndex+1, line.length);
			TrimWhitespace(&tokenOut->key);
			TrimWhitespace(&tokenOut->value);
			parser->byteIndex += numTrimmedWhitespaceChars + line.length;
			return true;
		}
		
		if (StrStartsWith(line, "#"))
		{
			ClearPointer(tokenOut);
			tokenOut->type = ParsingTokenType_FilePrefix;
			tokenOut->str = line;
			tokenOut->value = StrSubstring(&line, 1);
			parser->byteIndex += numTrimmedWhitespaceChars + line.length;
			return true;
		}
		
		ClearPointer(tokenOut);
		tokenOut->type = ParsingTokenType_Unknown;
		tokenOut->str = line;
		parser->byteIndex += numTrimmedWhitespaceChars + line.length;
		return true;
	}
	
	return false;
}

// +--------------------------------------------------------------+
// |                          XmlParser                           |
// +--------------------------------------------------------------+
struct XmlProperty_t
{
	MyStr_t key;
	MyStr_t value; //often encoded with escape characters
	bool valueWasQuoted;
};
struct XmlToken_t
{
	u64 tokenParseIndex;
	MyStr_t type;
	VarArray_t properties;
};
struct XmlParser_t
{
	MemArena_t* allocArena;
	
	u64 nextTokenParseIndex;
	
	LineParser_t lineParser;
	MyStr_t currentLine;
	u64 byteIndex;
	
	VarArray_t parentTokens; //XmlToken_t
	XmlToken_t newToken;
	bool newTokenIsOpening;
};
enum XmlParseResultType_t
{
	XmlParseResultType_None,
	XmlParseResultType_Token,
	XmlParseResultType_EndToken,
	XmlParseResultType_Comment,
	XmlParseResultType_Contents,
	XmlParseResultType_Directive,
	XmlParseResultType_Error,
	XmlParseResultType_NumTypes,
};
struct XmlParseResult_t
{
	XmlParseResultType_t type;
	XmlToken_t token;
	MyStr_t string;
	XmlParsingError_t error;
};

XmlParser_t NewXmlParser(MemArena_t* arenaForLists, MyStr_t fileContents)
{
	XmlParser_t result = {};
	result.allocArena = arenaForLists;
	result.lineParser = NewLineParser(fileContents);
	result.currentLine = MyStr_Empty;
	result.byteIndex = 0;
	CreateVarArray(&result.parentTokens, result.allocArena, sizeof(XmlToken_t));
	return result;
}

void FreeXmlParser(XmlParser_t* parser)
{
	NotNull(parser);
	VarArrayLoop(&parser->parentTokens, tIndex)
	{
		VarArrayLoopGet(XmlToken_t, token, &parser->parentTokens, tIndex);
		FreeVarArray(&token->properties);
	}
	FreeVarArray(&parser->parentTokens);
	ClearPointer(parser);
}

bool XmlParserCheckIdentifierHasValidChars(XmlParser_t* parser, MyStr_t identifierStr, XmlParseResult_t* result, ProcessLog_t* log)
{
	NotNull(parser);
	NotNullStr(&identifierStr);
	NotNull(result);
	NotNull(log);
	for (u64 cIndex = 0; cIndex < identifierStr.length; )
	{
		u32 codepoint = 0;
		u8 codepointByteSize = GetCodepointForUtf8Str(identifierStr, cIndex, &codepoint);
		if (codepointByteSize == 0)
		{
			LogPrintLine_E(log, "Invalid UTF-8 encoding in identifierStr on line %llu", parser->lineParser.lineIndex);
			result->error = XmlParsingError_InvalidUtf8;
			return false;
		}
		if (!(codepoint >= '0' && codepoint <= '9') &&
			!(codepoint >= 'A' && codepoint <= 'Z') &&
			!(codepoint >= 'a' && codepoint <= 'z') &&
			codepoint != '_' && codepoint != ':' && codepoint != '-')
		{
			LogPrintLine_E(log, "Invalid character in identifierStr on line %llu: 0x%08X \'%c\'", parser->lineParser.lineIndex, codepoint, (char)codepoint);
			result->error = XmlParsingError_InvalidCharInIdentifier;
			return false;
		}
		cIndex += codepointByteSize;
	}
	return true;
}

//NOTE: This XML parser is making a lot of assumptions, it probably doesn't parse the full XML format
//      <!-- ... --> and <? ... ?> tokens get thrown out without the calling code getting a chance to look at them
//      Token property key-value pairs have to be on a single line (the key can't be on a separate line from the value)
bool XmlParserGetToken(XmlParser_t* parser, XmlParseResult_t* result, ProcessLog_t* log)
{
	NotNull(parser);
	NotNull(parser->allocArena);
	NotNull(result);
	
	if (parser->newTokenIsOpening)
	{
		XmlToken_t* newTokenSpace = VarArrayAdd(&parser->parentTokens, XmlToken_t);
		NotNull(newTokenSpace);
		MyMemCopy(newTokenSpace, &parser->newToken, sizeof(XmlToken_t));
	}
	else
	{
		FreeVarArray(&parser->newToken.properties);
	}
	ClearStruct(parser->newToken);
	parser->newTokenIsOpening = false;
	
	bool inComment = false;
	bool inToken = false;
	bool isEndingToken = false;
	bool foundTokenType = false;
	bool inDirectiveToken = false;
	while (true)
	{
		if (parser->byteIndex >= parser->currentLine.length)
		{
			bool gotNextLine = LineParserGetLine(&parser->lineParser, &parser->currentLine);
			if (!gotNextLine) { break; }
			parser->byteIndex = 0;
		}
		Assert(parser->byteIndex <= parser->currentLine.length);
		
		u64 workingStartIndex = parser->byteIndex;
		MyStr_t workingLine = StrSubstring(&parser->currentLine, parser->byteIndex);
		workingStartIndex += TrimLeadingWhitespace(&workingLine);
		TrimTrailingWhitespace(&workingLine);
		
		if (workingLine.length == 0)
		{
			parser->byteIndex = parser->currentLine.length;
			continue;
		}
		
		if (inComment)
		{
			u64 endCommentIndex = 0;
			if (FindSubstring(workingLine, "-->", &endCommentIndex))
			{
				parser->byteIndex = workingStartIndex + endCommentIndex + 3;
				inComment = false;
				continue;
			}
			else
			{
				parser->byteIndex = parser->currentLine.length;
				continue;
			}
		}
		else if (inToken)
		{
			if (!foundTokenType)
			{
				//find the first whitespace or other name ending character (like " or >)
				u64 nameEndingCharIndex = 0;
				bool nameGoesTillEndOfLine = !FindNextCharInStr(workingLine, 0, " \t\"<>", &nameEndingCharIndex);
				if (nameGoesTillEndOfLine) { nameEndingCharIndex = workingLine.length; }
				if (nameEndingCharIndex == 0)
				{
					if (log != nullptr) { LogPrintLine_E(log, "No name given for token on line %llu: \"%.*s\"", parser->lineParser.lineIndex, workingLine.length, workingLine.pntr); }
					result->type = XmlParseResultType_Error;
					result->error = XmlParsingError_NoTypeFoundForToken;
					return true;
				}
				
				
				if (isEndingToken)
				{
					if (nameGoesTillEndOfLine)
					{
						if (log != nullptr ) { LogPrintLine_E(log, "Unexpected end of line when parsing closing token on line %llu byte %llu: \"%.*s\"", parser->lineParser.lineIndex, workingStartIndex, workingLine.length, workingLine.pntr); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_UnexpectedEol;
						return true;
					}
					if (workingLine.pntr[nameEndingCharIndex] != '>')
					{
						if (log != nullptr ) { LogPrintLine_E(log, "Expected > instead of 0x%08X \'%c\' when parsing closing token on line %llu byte %llu: \"%.*s\"", CharToU32(workingLine.pntr[nameEndingCharIndex]), workingLine.pntr[nameEndingCharIndex], parser->lineParser.lineIndex, workingStartIndex, workingLine.length, workingLine.pntr); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_ExpectedClosingAngleBracket;
						return true;
					}
					
					MyStr_t endingTokenType = NewStr(nameEndingCharIndex, workingLine.pntr);
					
					if (parser->parentTokens.length == 0)
					{
						if (log != nullptr ) { LogPrintLine_E(log, "Unexpected closing token. No tokens have been started on line %llu: \"%.*s\"", parser->lineParser.lineIndex, workingLine.length, workingLine.pntr); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_UnexpectedClosingToken;
						return true;
					}
					XmlToken_t* parentToken = VarArrayGetHard(&parser->parentTokens, parser->parentTokens.length-1, XmlToken_t);
					if (!StrEquals(parentToken->type, endingTokenType))
					{
						if (log != nullptr ) { LogPrintLine_E(log, "Ending token mismatch. Expect token \"%.*s\" to end, not \"%.*s\" on line %llu: \"%.*s\"", parentToken->type.length, parentToken->type.pntr, endingTokenType.length, endingTokenType.pntr, parser->lineParser.lineIndex, workingLine.length, workingLine.pntr); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_ClosingTokenMismatch;
						return true;
					}
					
					MyMemCopy(&parser->newToken, parentToken, sizeof(XmlToken_t));
					parser->newTokenIsOpening = false;
					VarArrayPop(&parser->parentTokens, XmlToken_t);
					
					isEndingToken = false;
					inToken = false;
					parser->byteIndex = workingStartIndex + nameEndingCharIndex + 1;
					
					result->type = XmlParseResultType_EndToken;
					MyMemCopy(&result->token, &parser->newToken, sizeof(XmlToken_t));
					return true;
				}
				else
				{
					ClearStruct(parser->newToken);
					parser->newToken.type = NewStr(nameEndingCharIndex, workingLine.pntr);
					
					if (!XmlParserCheckIdentifierHasValidChars(parser, parser->newToken.type, result, log))
					{
						//error and log were already done by XmlParserCheckIdentifierHasValidChars
						return true;
					}
					
					CreateVarArray(&parser->newToken.properties, parser->allocArena, sizeof(XmlProperty_t));
					
					foundTokenType = true;
					if (nameGoesTillEndOfLine) { parser->byteIndex = parser->currentLine.length; }
					else { parser->byteIndex = workingStartIndex + nameEndingCharIndex; }
					continue;
				}
			}
			else
			{
				if (StrStartsWith(workingLine, "/>"))
				{
					parser->newToken.tokenParseIndex = parser->nextTokenParseIndex;
					parser->nextTokenParseIndex++;
					parser->newTokenIsOpening = false;
					parser->byteIndex = workingStartIndex + 2;
					result->type = XmlParseResultType_Token;
					MyMemCopy(&result->token, &parser->newToken, sizeof(XmlToken_t));
					return true;
				}
				else if (StrStartsWith(workingLine, ">"))
				{
					parser->newToken.tokenParseIndex = parser->nextTokenParseIndex;
					parser->nextTokenParseIndex++;
					parser->newTokenIsOpening = true;
					parser->byteIndex = workingStartIndex + 1;
					result->type = XmlParseResultType_Token;
					MyMemCopy(&result->token, &parser->newToken, sizeof(XmlToken_t));
					return true;
				}
				else
				{
					u32 nextCodepoint = 0;
					u8 nextCodepointByteSize = GetCodepointForUtf8Str(workingLine, 0, &nextCodepoint);
					UNUSED(nextCodepointByteSize);
					if (!(nextCodepoint >= 'A' && nextCodepoint <= 'Z') &&
						!(nextCodepoint >= 'a' && nextCodepoint <= 'z') &&
						nextCodepoint != '_')
					{
						if (log != nullptr) { LogPrintLine_E(log, "Invalid first character for property of token on line %llu: 0x%08X \'%c\'", parser->lineParser.lineIndex, nextCodepoint, (char)nextCodepoint); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_InvalidPropertyFirstChar;
						return true;
					}
					
					u64 equalsIndex = 0;
					bool foundEquals = FindNextCharInStr(workingLine, 0, "=", &equalsIndex);
					if (!foundEquals)
					{
						if (log != nullptr) { LogPrintLine_E(log, "No equals found for property of token on line %llu: \"%.*s\"", parser->lineParser.lineIndex, workingLine.length, workingLine.pntr); }
						result->type = XmlParseResultType_Error;
						result->error = XmlParsingError_NoEqualsForProperty;
						return true;
					}
					
					MyStr_t propertyKey = StrSubstring(&workingLine, 0, equalsIndex);
					TrimTrailingWhitespace(&propertyKey);
					if (!XmlParserCheckIdentifierHasValidChars(parser, propertyKey, result, log))
					{
						//error and log were already done by XmlParserCheckIdentifierHasValidChars
						return false;
					}
					
					//TODO: This is not super safe, per-se. If we see a quoted string for the value
					u64 valueEndIndex = 0;
					bool foundValueEndChar = FindNextCharInStr(workingLine, equalsIndex+1, " \t<>", &valueEndIndex, true);
					if (!foundValueEndChar)
					{
						valueEndIndex = workingLine.length;
					}
					
					bool propertyValueWasQuoted = false;
					MyStr_t propertyValue = StrSubstring(&workingLine, equalsIndex+1, valueEndIndex);
					TrimLeadingWhitespace(&propertyValue);
					if (StrStartsWith(propertyValue, "\"") && StrEndsWith(propertyValue, "\""))
					{
						propertyValueWasQuoted = true;
						propertyValue = StrSubstring(&propertyValue, 1, propertyValue.length-1);
					}
					else
					{
						if (propertyValue.length == 0)
						{
							if (log != nullptr) { LogPrintLine_E(log, "Missing property value on line %llu: \"%.*s\"", parser->lineParser.lineIndex, workingLine.length, workingLine.pntr); }
							result->type = XmlParseResultType_Error;
							result->error = XmlParsingError_MissingPropertyValue;
							return true;
						}
						//TODO: We should somehow check that the value is "valid" for implicit types
						//      Stuff like: true, false, 100, -1.01, etc.
					}
					
					XmlProperty_t* newProperty = VarArrayAdd(&parser->newToken.properties, XmlProperty_t);
					NotNull(newProperty);
					ClearPointer(newProperty);
					newProperty->key = propertyKey;
					newProperty->value = propertyValue;
					newProperty->valueWasQuoted = propertyValueWasQuoted;
					
					parser->byteIndex = workingStartIndex + valueEndIndex;
				}
			}
		}
		else if (inDirectiveToken)
		{
			//TODO: Technically this doesn't handle if the directive token has a string with ?> in it,
			//      but that's not a use case we have right now so I'm not going to solve it
			u64 endDirectiveIndex = 0;
			if (FindSubstring(workingLine, "?>", &endDirectiveIndex))
			{
				parser->byteIndex = workingStartIndex + endDirectiveIndex + 2;
				inDirectiveToken = false;
				continue;
			}
			else
			{
				parser->byteIndex = parser->currentLine.length;
				continue;
			}
		}
		else
		{
			if (StrStartsWith(workingLine, "<!--"))
			{
				inComment = true;
				parser->byteIndex = workingStartIndex + 3;
				continue;
			}
			else if (StrStartsWith(workingLine, "<?"))
			{
				inDirectiveToken = true;
				parser->byteIndex = workingStartIndex + 2;
				continue;
			}
			else if (StrStartsWith(workingLine, "</"))
			{
				inToken = true;
				isEndingToken = true;
				foundTokenType = false;
				parser->byteIndex = workingStartIndex + 2;
				continue;
			}
			else if (StrStartsWith(workingLine, "<"))
			{
				inToken = true;
				isEndingToken = false;
				foundTokenType = false;
				parser->byteIndex = workingStartIndex + 1;
				continue;
			}
			else
			{
				u64 nextOpenBracketIndex = 0;
				bool foundOpenBracket = FindNextCharInStr(workingLine, 0, "<", &nextOpenBracketIndex, true);
				if (!foundOpenBracket) { nextOpenBracketIndex = workingLine.length; }
				Assert(nextOpenBracketIndex > 0);
				
				MyStr_t contentStr = NewStr(nextOpenBracketIndex, workingLine.pntr);
				if (parser->parentTokens.length == 0)
				{
					if (log != nullptr ) { LogPrintLine_E(log, "Invalid character(s) found outside token on line %llu byte %llu: \"%.*s\"", parser->lineParser.lineIndex, workingStartIndex, workingLine.length, workingLine.pntr); }
					result->type = XmlParseResultType_Error;
					result->error = XmlParsingError_InvalidCharacterOutsideToken;
					return true;
				}
				
				parser->byteIndex = (foundOpenBracket ? (workingStartIndex + nextOpenBracketIndex) : parser->currentLine.length);
				result->type = XmlParseResultType_Contents;
				result->string = contentStr;
				return true;
			}
		}
	}
	
	if (parser->parentTokens.length > 0)
	{
		if (log != nullptr )
		{
			LogPrintLine_E(log, "Missing closing token for %llu token%s:", parser->parentTokens.length, (parser->parentTokens.length == 1) ? "" : "s");
			VarArrayLoop(&parser->parentTokens, tIndex)
			{
				VarArrayLoopGet(XmlToken_t, token, &parser->parentTokens, tIndex);
				LogPrintLine_E(log, "  [%llu]: %llu \"%.*s\"", tIndex, token->tokenParseIndex, token->type.length, token->type.pntr);
			}
		}
		result->type = XmlParseResultType_Error;
		result->error = XmlParsingError_MissingClosingTokens;
		return true;
	}
	
	return false;
}

XmlProperty_t* GetXmlProperty(XmlToken_t* token, MyStr_t propertyKey, bool ignoreCase = false)
{
	NotNull(token);
	VarArrayLoop(&token->properties, pIndex)
	{
		VarArrayLoopGet(XmlProperty_t, property, &token->properties, pIndex);
		if ((!ignoreCase && StrEquals(property->key, propertyKey)) ||
			(ignoreCase && StrEqualsIgnoreCase(property->key, propertyKey)))
		{
			return property;
		}
	}
	return nullptr;
}
