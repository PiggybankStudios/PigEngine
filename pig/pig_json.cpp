

#if JSON_SUPPORTED

const char* GetYajlTypeStr(yajl_type type)
{
	switch (type)
	{
		case yajl_t_string: return "string";
		case yajl_t_number: return "number";
		case yajl_t_object: return "object";
		case yajl_t_array:  return "array";
		case yajl_t_true:   return "true";
		case yajl_t_false:  return "false";
		case yajl_t_null:   return "null";
		case yajl_t_any:    return "any";
		default: return "Unknown";
	}
}

struct ParsedJson_t
{
	MemArena_t* allocArena;
	bool isValid;
	MyStr_t jsonStr;
	yajl_val rootNode;
	u64 errorBufferSize;
	char* errorBuffer;
};

bool TryParseJson(MemArena_t* memArena, MyStr_t jsonStr, u64 errorBufferSize, ParsedJson_t* result)
{
	NotNull2(memArena, result);
	ClearPointer(result);
	result->allocArena = memArena;
	result->errorBufferSize = errorBufferSize;
	if (errorBufferSize > 0)
	{
		result->errorBuffer = AllocArray(memArena, char, errorBufferSize);
		result->errorBuffer[0] = '\0';
	}
	result->jsonStr = AllocString(memArena, &jsonStr);
	NotNullStr(&result->jsonStr);
	result->rootNode = yajl_tree_parse(result->jsonStr.chars, result->errorBuffer, result->errorBufferSize);
	result->isValid = (result->rootNode != nullptr);
	AssertIfMsg(!result->isValid, result->errorBuffer[0] != '\0', "yajl_tree_parse did not give us an error!");
	return result->isValid;
}

void FreeParsedJson(ParsedJson_t* parsedJson)
{
	NotNull(parsedJson);
	if (parsedJson->jsonStr.chars != nullptr)
	{
		NotNull(parsedJson->allocArena);
		if (DoesMemArenaSupportFreeing(parsedJson->allocArena)) { FreeString(parsedJson->allocArena, &parsedJson->jsonStr); }
	}
	if (parsedJson->errorBuffer != nullptr)
	{
		NotNull(parsedJson->allocArena);
		if (DoesMemArenaSupportFreeing(parsedJson->allocArena)) { FreeMem(parsedJson->allocArena, parsedJson->errorBuffer, parsedJson->errorBufferSize); }
	}
	if (parsedJson->rootNode != nullptr)
	{
		yajl_tree_free(parsedJson->rootNode);
	}
	ClearPointer(parsedJson);
}

char** ParseJsonPath(MemArena_t* memArena, MyStr_t pathStr, u64* numPiecesOut = nullptr, u64* allocSizeOut = nullptr)
{
	u64 resultAllocSize = 0;
	u64 resultCount = 0;
	char** result = nullptr;
	u64 resultCharCount = 0;
	char* resultCharBuffer = nullptr;
	
	for (u8 pass = 0; pass < 2; pass++)
	{
		u64 pieceIndex = 0;
		u64 charIndex = 0;
		u64 numCharsNeeded = 0;
		
		u64 partStartIndex = 0;
		for (u64 cIndex = 0; cIndex <= pathStr.length; cIndex++)
		{
			char nextChar = (cIndex < pathStr.length) ? pathStr.chars[cIndex] : '\0';
			if (nextChar == '/' || nextChar == '\\' || (nextChar == '\0' && cIndex > partStartIndex))
			{
				u64 partLength = (cIndex - partStartIndex);
				if (pass == 1)
				{
					Assert(pieceIndex < resultCount);
					Assert(charIndex + partLength + 1 <= resultCharCount);
					result[pieceIndex] = &resultCharBuffer[charIndex];
					if (partLength > 0) { MyMemCopy(&resultCharBuffer[charIndex], &pathStr.chars[partStartIndex], partLength); }
					resultCharBuffer[charIndex + partLength] = '\0';
				}
				charIndex += partLength + 1;
				pieceIndex++;
				partStartIndex = cIndex+1;
			}
		}
		Assert(partStartIndex >= pathStr.length);
		
		if (pass == 0)
		{
			resultCount = pieceIndex + 1; //+1 for nullptr str to indicate end
			resultCharCount = charIndex;
			resultAllocSize = (sizeof(char*) * resultCount) + (sizeof(char) * resultCharCount);
			if (numPiecesOut != nullptr) { *numPiecesOut = pieceIndex; }
			if (allocSizeOut != nullptr) { *allocSizeOut = resultAllocSize; }
			if (memArena == nullptr) { return nullptr; }
			
			u8* allocation = (u8*)AllocMem(memArena, resultAllocSize);
			if (allocation == nullptr) { return nullptr; }
			result = (char**)allocation;
			resultCharBuffer = (char*)(allocation + (sizeof(char*) * resultCount));
		}
		else
		{
			Assert(pieceIndex+1 == resultCount);
			result[pieceIndex] = nullptr;
			Assert(charIndex == resultCharCount);
		}
	}
	
	return result;
}

bool TryFindJsonNodeByPathStr(yajl_val rootNode, MyStr_t pathStr, yajl_val* nodeOut)
{
	NotNull(rootNode);
	char** pathPieces = ParseJsonPath(TempArena, pathStr);
	if (pathPieces == nullptr) { return false; }
	yajl_val foundNode = yajl_tree_get(rootNode, (const char **)pathPieces, yajl_t_any);
	if (foundNode == nullptr) { return false; }
	if (nodeOut != nullptr) { *nodeOut = foundNode; }
	return true;
}
bool TryFindJsonNodeByPathStr(yajl_val rootNode, const char* nullTermPathStr, yajl_val* nodeOut)
{
	return TryFindJsonNodeByPathStr(rootNode, NewStr(nullTermPathStr), nodeOut);
}
bool TryFindJsonNodeByPathStr(ParsedJson_t* parsedJson, MyStr_t pathStr, yajl_val* nodeOut)
{
	NotNull(parsedJson);
	return TryFindJsonNodeByPathStr(parsedJson->rootNode, pathStr, nodeOut);
}
bool TryFindJsonNodeByPathStr(ParsedJson_t* parsedJson, const char* nullTermPathStr, yajl_val* nodeOut)
{
	NotNull(parsedJson);
	return TryFindJsonNodeByPathStr(parsedJson->rootNode, NewStr(nullTermPathStr), nodeOut);
}

u64 GetJsonArrayLength(yajl_val arrayNode)
{
	if (arrayNode == nullptr) { return 0; }
	Assert(YAJL_IS_ARRAY(arrayNode));
	return (u64)arrayNode->u.array.len;
}
yajl_val GetJsonArrayElement(yajl_val arrayNode, u64 index)
{
	if (arrayNode == nullptr) { return nullptr; }
	Assert(YAJL_IS_ARRAY(arrayNode));
	DebugAssert(index < GetJsonArrayLength(arrayNode));
	return arrayNode->u.array.values[index];
}

// +==============================+
// |            Object            |
// +==============================+
yajl_val TryGetJsonObject(yajl_val rootNode, MyStr_t pathStr, yajl_val defaultValue = nullptr)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found) { return defaultValue; }
	if (!YAJL_IS_OBJECT(resultNode)) { return defaultValue; }
	return resultNode;
}
yajl_val TryGetJsonObject(yajl_val rootNode, const char* nullTermPathStr, yajl_val defaultValue = nullptr)
{
	return TryGetJsonObject(rootNode, NewStr(nullTermPathStr), defaultValue);
}
yajl_val TryGetJsonObject(ParsedJson_t* parsedJson, MyStr_t pathStr, yajl_val defaultValue = nullptr)
{
	NotNull(parsedJson);
	return TryGetJsonObject(parsedJson->rootNode, pathStr, defaultValue);
}
yajl_val TryGetJsonObject(ParsedJson_t* parsedJson, const char* nullTermPathStr, yajl_val defaultValue = nullptr)
{
	NotNull(parsedJson);
	return TryGetJsonObject(parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue);
}

// +==============================+
// |           MyStr_t            |
// +==============================+
MyStr_t TryGetJsonValueStr(yajl_val rootNode, MyStr_t pathStr, MyStr_t defaultValue)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found) { return defaultValue; }
	if (!YAJL_IS_STRING(resultNode)) { return defaultValue; }
	return NewStr(YAJL_GET_STRING(resultNode));
}
MyStr_t TryGetJsonValueStr(yajl_val rootNode, const char* nullTermPathStr, MyStr_t defaultValue)
{
	return TryGetJsonValueStr(rootNode, NewStr(nullTermPathStr), defaultValue);
}
MyStr_t TryGetJsonValueStr(ParsedJson_t* parsedJson, MyStr_t pathStr, MyStr_t defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueStr(parsedJson->rootNode, pathStr, defaultValue);
}
MyStr_t TryGetJsonValueStr(ParsedJson_t* parsedJson, const char* nullTermPathStr, MyStr_t defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueStr(parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue);
}

MyStr_t GetJsonValueStr(yajl_val rootNode, MyStr_t pathStr)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	AssertMsg(found, "Failed to find Json node by path");
	//TODO: Should we handle YAJL_IS_NULL? Should we turn a number into a string?
	AssertMsg(YAJL_IS_STRING(resultNode), "Expected a string json node!");
	return NewStr(YAJL_GET_STRING(resultNode));
}
MyStr_t GetJsonValueStr(yajl_val rootNode, const char* nullTermPathStr)
{
	return GetJsonValueStr(rootNode, NewStr(nullTermPathStr));
}
MyStr_t GetJsonValueStr(ParsedJson_t* parsedJson, MyStr_t pathStr)
{
	NotNull(parsedJson);
	return GetJsonValueStr(parsedJson->rootNode, pathStr);
}
MyStr_t GetJsonValueStr(ParsedJson_t* parsedJson, const char* nullTermPathStr)
{
	NotNull(parsedJson);
	return GetJsonValueStr(parsedJson->rootNode, NewStr(nullTermPathStr));
}

// +==============================+
// |             I64              |
// +==============================+
i64 TryGetJsonValueI64(yajl_val rootNode, MyStr_t pathStr, i64 defaultValue)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found) { return defaultValue; }
	if (!YAJL_IS_INTEGER(resultNode)) { return defaultValue; }
	return YAJL_GET_INTEGER(resultNode);
}
i64 TryGetJsonValueI64(yajl_val rootNode, const char* nullTermPathStr, i64 defaultValue)
{
	return TryGetJsonValueI64(rootNode, NewStr(nullTermPathStr), defaultValue);
}
i64 TryGetJsonValueI64(ParsedJson_t* parsedJson, MyStr_t pathStr, i64 defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueI64(parsedJson->rootNode, pathStr, defaultValue);
}
i64 TryGetJsonValueI64(ParsedJson_t* parsedJson, const char* nullTermPathStr, i64 defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueI64(parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue);
}

i64 GetJsonValueI64(yajl_val rootNode, MyStr_t pathStr)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	AssertMsg(found, "Failed to find Json node by path");
	AssertMsg(YAJL_IS_INTEGER(resultNode), "Expected an integer json node!");
	return YAJL_GET_INTEGER(resultNode);
}
i64 GetJsonValueI64(yajl_val rootNode, const char* nullTermPathStr)
{
	return GetJsonValueI64(rootNode, NewStr(nullTermPathStr));
}
i64 GetJsonValueI64(ParsedJson_t* parsedJson, MyStr_t pathStr)
{
	NotNull(parsedJson);
	return GetJsonValueI64(parsedJson->rootNode, pathStr);
}
i64 GetJsonValueI64(ParsedJson_t* parsedJson, const char* nullTermPathStr)
{
	NotNull(parsedJson);
	return GetJsonValueI64(parsedJson->rootNode, NewStr(nullTermPathStr));
}

// +==============================+
// |             R64              |
// +==============================+
r64 TryGetJsonValueR64(yajl_val rootNode, MyStr_t pathStr, r64 defaultValue)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found) { return defaultValue; }
	if (!YAJL_IS_DOUBLE(resultNode)) { return defaultValue; }
	return YAJL_GET_DOUBLE(resultNode);
}
r64 TryGetJsonValueR64(yajl_val rootNode, const char* nullTermPathStr, r64 defaultValue)
{
	return TryGetJsonValueR64(rootNode, NewStr(nullTermPathStr), defaultValue);
}
r64 TryGetJsonValueR64(ParsedJson_t* parsedJson, MyStr_t pathStr, r64 defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueR64(parsedJson->rootNode, pathStr, defaultValue);
}
r64 TryGetJsonValueR64(ParsedJson_t* parsedJson, const char* nullTermPathStr, r64 defaultValue)
{
	NotNull(parsedJson);
	return TryGetJsonValueR64(parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue);
}

r64 GetJsonValueR64(yajl_val rootNode, MyStr_t pathStr)
{
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	AssertMsg(found, "Failed to find Json node by path");
	AssertMsg(YAJL_IS_DOUBLE(resultNode), "Expected a r64 json node!");
	return YAJL_GET_DOUBLE(resultNode);
}
r64 GetJsonValueR64(yajl_val rootNode, const char* nullTermPathStr)
{
	return GetJsonValueR64(rootNode, NewStr(nullTermPathStr));
}
r64 GetJsonValueR64(ParsedJson_t* parsedJson, MyStr_t pathStr)
{
	NotNull(parsedJson);
	return GetJsonValueR64(parsedJson->rootNode, pathStr);
}
r64 GetJsonValueR64(ParsedJson_t* parsedJson, const char* nullTermPathStr)
{
	NotNull(parsedJson);
	return GetJsonValueR64(parsedJson->rootNode, NewStr(nullTermPathStr));
}

// +--------------------------------------------------------------+
// |                     ProcessLog Versions                      |
// +--------------------------------------------------------------+
yajl_val LogJsonArray(ProcessLog_t* log, yajl_val rootNode, MyStr_t pathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	if (rootNode == nullptr) { return defaultValue; }
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found)
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Failed to find Json node by path: \"%.*s\"", pathStr.length, pathStr.chars);
		return defaultValue;
	}
	if (!YAJL_IS_ARRAY(resultNode))
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Found %s not array at \"%.*s\"", GetYajlTypeStr(resultNode->type), pathStr.length, pathStr.chars);
		return defaultValue;
	}
	return resultNode;
}
yajl_val LogJsonObject(ProcessLog_t* log, yajl_val rootNode, MyStr_t pathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	if (rootNode == nullptr) { return defaultValue; }
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found)
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Failed to find Json node by path: \"%.*s\"", pathStr.length, pathStr.chars);
		return defaultValue;
	}
	if (!YAJL_IS_OBJECT(resultNode))
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Found %s not object at \"%.*s\"", GetYajlTypeStr(resultNode->type), pathStr.length, pathStr.chars);
		return defaultValue;
	}
	return resultNode;
}
MyStr_t LogJsonValueStr(ProcessLog_t* log, yajl_val rootNode, MyStr_t pathStr, MyStr_t defaultValue, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	if (rootNode == nullptr) { return defaultValue; }
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found)
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Failed to find Json node by path: \"%.*s\"", pathStr.length, pathStr.chars);
		return defaultValue;
	}
	if (!YAJL_IS_STRING(resultNode))
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Found %s not string at \"%.*s\"", GetYajlTypeStr(resultNode->type), pathStr.length, pathStr.chars);
		return defaultValue;
	}
	return NewStr(YAJL_GET_STRING(resultNode));
}
i64 LogJsonValueI64(ProcessLog_t* log, yajl_val rootNode, MyStr_t pathStr, i64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	if (rootNode == nullptr) { return defaultValue; }
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found)
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Failed to find Json node by path: \"%.*s\"", pathStr.length, pathStr.chars);
		return defaultValue;
	}
	if (!YAJL_IS_INTEGER(resultNode))
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Found %s not i64 at \"%.*s\"", GetYajlTypeStr(resultNode->type), pathStr.length, pathStr.chars);
		return defaultValue;
	}
	return YAJL_GET_INTEGER(resultNode);
}
r64 LogJsonValueR64(ProcessLog_t* log, yajl_val rootNode, MyStr_t pathStr, r64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	if (rootNode == nullptr) { return defaultValue; }
	yajl_val resultNode;
	bool found = TryFindJsonNodeByPathStr(rootNode, pathStr, &resultNode);
	if (!found)
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Failed to find Json node by path: \"%.*s\"", pathStr.length, pathStr.chars);
		return defaultValue;
	}
	if (!YAJL_IS_DOUBLE(resultNode))
	{
		if (isError) { log->hadErrors = true; }
		else { log->hadWarnings = true; }
		LogPrintLineAt(log, (isError ? DbgLevel_Error : DbgLevel_Warning), "Found %s not r64 at \"%.*s\"", GetYajlTypeStr(resultNode->type), pathStr.length, pathStr.chars);
		return defaultValue;
	}
	return YAJL_GET_DOUBLE(resultNode);
}

//yajl_val and const char* variants
yajl_val LogJsonArray(ProcessLog_t* log, yajl_val rootNode, const char* nullTermPathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	return LogJsonArray(log, rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
yajl_val LogJsonObject(ProcessLog_t* log, yajl_val rootNode, const char* nullTermPathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	return LogJsonObject(log, rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
MyStr_t LogJsonValueStr(ProcessLog_t* log, yajl_val rootNode, const char* nullTermPathStr, MyStr_t defaultValue, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	return LogJsonValueStr(log, rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
i64 LogJsonValueI64(ProcessLog_t* log, yajl_val rootNode, const char* nullTermPathStr, i64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	return LogJsonValueI64(log, rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
r64 LogJsonValueR64(ProcessLog_t* log, yajl_val rootNode, const char* nullTermPathStr, r64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	return LogJsonValueR64(log, rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}

//ParsedJson_t* and MyStr_t variants
yajl_val LogJsonArray(ProcessLog_t* log, ParsedJson_t* parsedJson, MyStr_t pathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonArray(log, parsedJson->rootNode, pathStr, defaultValue, allowMissing, allowInvalidType, isError);
}
yajl_val LogJsonObject(ProcessLog_t* log, ParsedJson_t* parsedJson, MyStr_t pathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonObject(log, parsedJson->rootNode, pathStr, defaultValue, allowMissing, allowInvalidType, isError);
}
MyStr_t LogJsonValueStr(ProcessLog_t* log, ParsedJson_t* parsedJson, MyStr_t pathStr, MyStr_t defaultValue, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueStr(log, parsedJson->rootNode, pathStr, defaultValue, allowMissing, allowInvalidType, isError);
}
i64 LogJsonValueI64(ProcessLog_t* log, ParsedJson_t* parsedJson, MyStr_t pathStr, i64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueI64(log, parsedJson->rootNode, pathStr, defaultValue, allowMissing, allowInvalidType, isError);
}
r64 LogJsonValueR64(ProcessLog_t* log, ParsedJson_t* parsedJson, MyStr_t pathStr, r64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueR64(log, parsedJson->rootNode, pathStr, defaultValue, allowMissing, allowInvalidType, isError);
}

//ParsedJson_t* and const char* variants
yajl_val LogJsonArray(ProcessLog_t* log, ParsedJson_t* parsedJson, const char* nullTermPathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonArray(log, parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
yajl_val LogJsonObject(ProcessLog_t* log, ParsedJson_t* parsedJson, const char* nullTermPathStr, yajl_val defaultValue = nullptr, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonObject(log, parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
MyStr_t LogJsonValueStr(ProcessLog_t* log, ParsedJson_t* parsedJson, const char* nullTermPathStr, MyStr_t defaultValue, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueStr(log, parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
i64 LogJsonValueI64(ProcessLog_t* log, ParsedJson_t* parsedJson, const char* nullTermPathStr, i64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueI64(log, parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}
r64 LogJsonValueR64(ProcessLog_t* log, ParsedJson_t* parsedJson, const char* nullTermPathStr, r64 defaultValue = 0, bool allowMissing = false, bool allowInvalidType = false, bool isError = true)
{
	Assert(parsedJson != nullptr);
	return LogJsonValueR64(log, parsedJson->rootNode, NewStr(nullTermPathStr), defaultValue, allowMissing, allowInvalidType, isError);
}

#endif // JSON_SUPPORTED