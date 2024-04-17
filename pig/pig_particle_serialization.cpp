/*
File:   pig_particle_serialization.cpp
Author: Taylor Robbins
Date:   07\12\2022
Description: 
	** Holds functions that help us parse ParticleTemplate_t structures from a plain text file format.
*/

#define PARTS_PREFAB_COLLECTION_PREFIX "# Parts Prefab Collection"

struct CollectionNamedColor_t
{
	MyStr_t name;
	Color_t value;
};

bool Deser_WasKeySeen(const VarArray_t* previousKeysArray, MyStr_t key)
{
	NotNull(previousKeysArray);
	Assert(previousKeysArray->itemSize == sizeof(MyStr_t));
	VarArrayLoop(previousKeysArray, pIndex)
	{
		const VarArrayLoopGet(MyStr_t, previousKey, previousKeysArray, pIndex);
		if (StrEqualsIgnoreCase(*previousKey, key))
		{
			return true;
		}
	}
	return false;
}
bool Deser_FindNamedColor(const VarArray_t* namedColorsArray, MyStr_t name, Color_t* colorOut)
{
	VarArrayLoop(namedColorsArray, cIndex)
	{
		VarArrayLoopGet(CollectionNamedColor_t, namedColor, namedColorsArray, cIndex);
		if (StrEqualsIgnoreCase(namedColor->name, name))
		{
			if (colorOut != nullptr) { *colorOut = namedColor->value; }
			return true;
		}
	}
	return false;
}

u64 Deser_FindPartsTypeByName(const VarArray_t* typesArray, MyStr_t name)
{
	NotNull(typesArray);
	NotNullStr(&name);
	VarArrayLoop(typesArray, tIndex)
	{
		VarArrayLoopGet(PartsType_t, type, typesArray, tIndex);
		if (StrEqualsIgnoreCase(type->name, name)) { return type->id; }
	}
	return UINT64_MAX;
}
u64 Deser_FindPartsBurstByName(const VarArray_t* burstsArray, MyStr_t name)
{
	NotNull(burstsArray);
	NotNullStr(&name);
	VarArrayLoop(burstsArray, bIndex)
	{
		VarArrayLoopGet(PartsBurst_t, burst, burstsArray, bIndex);
		if (StrEqualsIgnoreCase(burst->name, name)) { return burst->id; }
	}
	return UINT64_MAX;
}
u64 Deser_FindPartsPrefabByName(const VarArray_t* prefabsArray, MyStr_t name)
{
	NotNull(prefabsArray);
	NotNullStr(&name);
	VarArrayLoop(prefabsArray, pIndex)
	{
		VarArrayLoopGet(PartsPrefab_t, prefab, prefabsArray, pIndex);
		if (StrEqualsIgnoreCase(prefab->name, name)) { return prefab->id; }
	}
	return UINT64_MAX;
}

PartsParamI32_t NewPartsParamI32(PartsDistroFunc_t distroFunc, i32 minValue, i32 maxValue)
{
	PartsParamI32_t result;
	result.distroFunc = distroFunc;
	result.minValue = minValue;
	result.maxValue = maxValue;
	return result;
}
PartsParamR32_t NewPartsParamR32(PartsDistroFunc_t distroFunc, r32 minValue, r32 maxValue)
{
	PartsParamR32_t result;
	result.distroFunc = distroFunc;
	result.minValue = minValue;
	result.maxValue = maxValue;
	return result;
}
PartsParamVec2_t NewPartsParamVec2(bool isPolar, PartsDistroFunc_t distroFunc, v2 minValue, v2 maxValue)
{
	PartsParamVec2_t result;
	result.isPolar = isPolar;
	result.distroFunc = distroFunc;
	result.minValue = minValue;
	result.maxValue = maxValue;
	return result;
}
PartsParamVec2i_t NewPartsParamVec2i(PartsDistroFunc_t distroFunc, v2i minValue, v2i maxValue)
{
	PartsParamVec2i_t result;
	result.distroFunc = distroFunc;
	result.minValue = minValue;
	result.maxValue = maxValue;
	return result;
}
PartsParamColor_t NewPartsParamColor(PartsDistroFunc_t distroFunc, Color_t minColor, Color_t maxColor)
{
	PartsParamColor_t result = {};
	result.pickFromList = false;
	result.distroFunc = distroFunc;
	result.minColor = minColor;
	result.maxColor = maxColor;
	return result;
}
PartsParamColor_t NewPartsParamColor(PartsDistroFunc_t distroFunc, u64 numPossibleColors, const Color_t* possibleColorsPntr)
{
	AssertIf(numPossibleColors > 0, possibleColorsPntr != nullptr);
	Assert(numPossibleColors <= MAX_PARTS_PARAM_COLOR_POSSIBLE_COLORS);
	PartsParamColor_t result = {};
	result.pickFromList = true;
	result.numPossibleColors = numPossibleColors;
	result.distroFunc = distroFunc;
	if (numPossibleColors > 0)
	{
		MyMemCopy(&result.possibleColors[0], possibleColorsPntr, sizeof(Color_t) * numPossibleColors);
	}
	return result;
}

//This can succeed while leaving valueOut untouched
bool Deser_ParseDistroFuncPart(MyStr_t* strPntr, PartsDistroFunc_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	if (IsNullStr(strPntr))
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		return false;
	}
	
	u64 openParensIndex = 0;
	if (FindNextCharInStr(*strPntr, 0, "[", &openParensIndex, true))
	{
		u64 closeParensIndex = 0;
		if (FindNextCharInStr(*strPntr, openParensIndex, "]", &closeParensIndex, true))
		{
			if (closeParensIndex != strPntr->length-1)
			{
				if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_StringOverflow; }
				return false;
			}
			
			MyStr_t givenFuncStr = NewStr(closeParensIndex - (openParensIndex+1), &strPntr->chars[openParensIndex+1]);
			for (u64 fIndex = 0; fIndex < PartsDistroFunc_NumFuncs; fIndex++)
			{
				PartsDistroFunc_t func = (PartsDistroFunc_t)fIndex;
				if (StrEqualsIgnoreCase(givenFuncStr, GetPartsDistroFuncStr(func)))
				{
					strPntr->length = openParensIndex;
					*valueOut = func;
					return true;
				}
			}
			
			if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_UnknownString; }
			return false;
		}
	}
	
	return true; //no parens found, don't touch valueOut
}

bool TryParsePartsEmissionShape(MyStr_t str, PartsEmissionShape_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	for (u64 sIndex = 0; sIndex < PartsEmissionShape_NumShapes; sIndex++)
	{
		PartsEmissionShape_t shape = (PartsEmissionShape_t)sIndex;
		const char* shapeName = GetPartsEmissionShapeStr(shape);
		if (StrEqualsIgnoreCase(str, shapeName))
		{
			if (valueOut != nullptr) { *valueOut = shape; }
			return true;
		}
	}
	if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_UnknownString; }
	return false;
}
bool TryParsePartsParamI32(MyStr_t str, PartsParamI32_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	TempPushMark();
	
	ClearPointer(valueOut);
	valueOut->distroFunc = PartsDistroFunc_Random;
	if (!Deser_ParseDistroFuncPart(&str, &valueOut->distroFunc, reasonOut)) { TempPopMark(); return false; }
	
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, str, "|", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
	
	if (numPieces == 0)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		TempPopMark(); return false;
	}
	if (numPieces > 2)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_Overflow; }
		TempPopMark(); return false;
	}
	
	if (!TryParseI32(pieces[0], &valueOut->minValue, reasonOut)) { TempPopMark(); return false; }
	if (numPieces == 2)
	{
		if (!TryParseI32(pieces[1], &valueOut->maxValue, reasonOut)) { TempPopMark(); return false; }
	}
	else
	{
		valueOut->maxValue = valueOut->minValue;
	}
	
	TempPopMark();
	return true;
}
bool TryParsePartsParamR32(MyStr_t str, PartsParamR32_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	TempPushMark();
	
	ClearPointer(valueOut);
	valueOut->distroFunc = PartsDistroFunc_Random;
	if (!Deser_ParseDistroFuncPart(&str, &valueOut->distroFunc, reasonOut)) { TempPopMark(); return false; }
	
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, str, "|", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
	
	if (numPieces == 0)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		TempPopMark(); return false;
	}
	if (numPieces > 2)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_Overflow; }
		TempPopMark(); return false;
	}
	
	if (!TryParseR32(pieces[0], &valueOut->minValue, reasonOut)) { TempPopMark(); return false; }
	if (numPieces == 2)
	{
		if (!TryParseR32(pieces[1], &valueOut->maxValue, reasonOut)) { TempPopMark(); return false; }
	}
	else
	{
		valueOut->maxValue = valueOut->minValue;
	}
	
	TempPopMark();
	return true;
}
//TODO: Do we want to support multiple choice v2i values? Like we do for colors?
bool TryParsePartsParamVec2i(MyStr_t str, PartsParamVec2i_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	TempPushMark();
	
	ClearPointer(valueOut);
	valueOut->distroFunc = PartsDistroFunc_Random;
	if (!Deser_ParseDistroFuncPart(&str, &valueOut->distroFunc, reasonOut)) { TempPopMark(); return false; }
	
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, str, "|", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
	
	if (numPieces == 0)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		TempPopMark();
		return false;
	}
	if (numPieces > 2)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_Overflow; }
		TempPopMark();
		return false;
	}
	
	if (!TryParseV2i(pieces[0], &valueOut->minValue, reasonOut)) { TempPopMark(); return false; }
	if (numPieces == 2)
	{
		if (!TryParseV2i(pieces[1], &valueOut->maxValue, reasonOut)) { TempPopMark(); return false; }
	}
	else
	{
		valueOut->maxValue = valueOut->minValue;
	}
	
	TempPopMark();
	return true;
}
bool TryParsePartsParamVec2(MyStr_t str, PartsParamVec2_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	TempPushMark();
	
	ClearPointer(valueOut);
	valueOut->distroFunc = PartsDistroFunc_Random;
	if (!Deser_ParseDistroFuncPart(&str, &valueOut->distroFunc, reasonOut)) { TempPopMark(); return false; }
	
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, str, "|", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
	
	if (numPieces == 0)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		TempPopMark();
		return false;
	}
	if (numPieces > 2)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_Overflow; }
		TempPopMark();
		return false;
	}
	
	if (!TryParseV2(pieces[0], &valueOut->minValue, reasonOut)) { TempPopMark(); return false; }
	if (numPieces == 2)
	{
		if (!TryParseV2(pieces[1], &valueOut->maxValue, reasonOut)) { TempPopMark(); return false; }
	}
	else
	{
		valueOut->maxValue = valueOut->minValue;
	}
	
	TempPopMark();
	return true;
}
bool TryParseNamedColor(const VarArray_t* namedColorsArray, MyStr_t str, Color_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr)
{
	VarArrayLoop(namedColorsArray, cIndex)
	{
		VarArrayLoopGet(CollectionNamedColor_t, namedColor, namedColorsArray, cIndex);
		if (StrEqualsIgnoreCase(namedColor->name, str))
		{
			if (valueOut != nullptr) { *valueOut = namedColor->value; }
			return true;
		}
	}
	
	if (!TryParseColor(str, valueOut, reasonOut))
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_UnknownString; }
		return false;
	}
	
	return true;
}
//NOTE: failedColorStrOut is only set in some failure cases
bool TryParsePartsParamColor(const VarArray_t* namedColorsArray, MyStr_t str, PartsParamColor_t* valueOut, TryParseFailureReason_t* reasonOut = nullptr, MyStr_t* failedColorStrOut = nullptr)
{
	NotNull(namedColorsArray);
	TempPushMark();
	
	ClearPointer(valueOut);
	valueOut->pickFromList = false;
	valueOut->distroFunc = PartsDistroFunc_Random;
	if (!Deser_ParseDistroFuncPart(&str, &valueOut->distroFunc, reasonOut)) { TempPopMark(); return false; }
	
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, str, "|", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
	
	if (numPieces == 0)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_NoNumbers; }
		TempPopMark();
		return false;
	}
	if (numPieces > MAX_PARTS_PARAM_COLOR_POSSIBLE_COLORS)
	{
		if (reasonOut != nullptr) { *reasonOut = TryParseFailureReason_Overflow; }
		TempPopMark();
		return false;
	}
	if (numPieces > 2)
	{
		valueOut->pickFromList = true;
		valueOut->numPossibleColors = numPieces;
		for (u64 cIndex = 0; cIndex < numPieces; cIndex++)
		{
			if (!TryParseNamedColor(namedColorsArray, pieces[cIndex], &valueOut->possibleColors[cIndex], reasonOut))
			{
				if (failedColorStrOut != nullptr) { *failedColorStrOut = pieces[cIndex]; }
				TempPopMark();
				return false;
			}
		}
	}
	else
	{
		valueOut->pickFromList = false;
		if (!TryParseNamedColor(namedColorsArray, pieces[0], &valueOut->minColor, reasonOut))
		{
			if (failedColorStrOut != nullptr) { *failedColorStrOut = pieces[0]; }
			TempPopMark();
			return false;
		}
		if (numPieces == 2)
		{
			if (!TryParseNamedColor(namedColorsArray, pieces[1], &valueOut->maxColor, reasonOut))
			{
				if (failedColorStrOut != nullptr) { *failedColorStrOut = pieces[1]; }
				TempPopMark();
				return false;
			}
		}
		else
		{
			valueOut->maxColor = valueOut->minColor;
		}
	}
	
	
	
	TempPopMark();
	return true;
}

bool Deser_ParseTypeParamR32(ProcessLog_t* log, TextParser_t* parser, VarArray_t* previousKeys, PartsType_t* currentType, PartsParamR32_t* paramOut, ParsingToken_t* token)
{
	if (Deser_WasKeySeen(previousKeys, token->key))
	{
		LogPrintLine_W(log, "Warning: Found \"%.*s\" a second time in one Type \"%.*s\" (on line %llu)",
			StrPrint(token->key),
			StrPrint(currentType->name),
			parser->lineParser.lineIndex+1
		);
		return false;
	}
	if (!TryParsePartsParamR32(token->value, paramOut, &log->parseFailureReason))
	{
		LogPrintLine_E(log, "Failed to parse \"%.*s\" as r32 parameter (line %llu): %s",
			StrPrint(token->key),
			parser->lineParser.lineIndex+1,
			GetTryParseFailureReasonStr(log->parseFailureReason)
		);
		return false;
	}
	return true;
}

// +--------------------------------------------------------------+
// |                             Free                             |
// +--------------------------------------------------------------+
void FreePartsPrefabCollection(PartsPrefabCollection_t* collection)
{
	NotNull(collection);
	AssertIf(collection->prefabs.items != nullptr, collection->allocArena != nullptr);
	VarArrayLoop(&collection->prefabs, pIndex)
	{
		VarArrayLoopGet(PartsPrefab_t, prefab, &collection->prefabs, pIndex);
		VarArrayLoop(&prefab->bursts, bIndex)
		{
			VarArrayLoopGet(PartsBurst_t, burst, &prefab->bursts, bIndex);
			FreeString(collection->allocArena, &burst->name);
			FreeString(collection->allocArena, &burst->type.name);
		}
		FreeVarArray(&prefab->bursts);
		FreeString(collection->allocArena, &prefab->name);
	}
	FreeVarArray(&collection->prefabs);
	ClearPointer(collection);
}

// +--------------------------------------------------------------+
// |                    Main TryDeser Function                    |
// +--------------------------------------------------------------+
bool TryDeserPartsPrefabCollection(MyStr_t fileContents, MemArena_t* memArena, ProcessLog_t* log, PartsPrefabCollection_t* collectionOut)
{
	NotNullStr(&fileContents);
	NotNull3(memArena, log, collectionOut);
	Assert(DoesMemArenaSupportFreeing(memArena));
	
	VarArray_t types;
	VarArray_t bursts;
	VarArray_t previousKeys;
	VarArray_t namedColors;
	CreateVarArray(&types, mainHeap, sizeof(PartsType_t));
	CreateVarArray(&bursts, mainHeap, sizeof(PartsBurst_t));
	CreateVarArray(&previousKeys, mainHeap, sizeof(MyStr_t));
	CreateVarArray(&namedColors, mainHeap, sizeof(CollectionNamedColor_t));
	
	bool foundFilePrefix = false;
	
	PartsType_t*   currentType   = nullptr;
	PartsBurst_t*  currentBurst  = nullptr;
	PartsPrefab_t* currentPrefab = nullptr;
	PartsParamR32_t tempSpeedParam = {};
	PartsParamR32_t tempDirectionParam = {};
	
	ClearPointer(collectionOut);
	collectionOut->allocArena = memArena;
	collectionOut->nextPrefabId = 1;
	collectionOut->nextBurstId = 1;
	collectionOut->nextTypeId = 1;
	CreateVarArray(&collectionOut->prefabs, memArena, sizeof(PartsPrefab_t));
	
	TextParser_t parser = NewTextParser(fileContents);
	ParsingToken_t token = {};
	while (TextParserGetToken(&parser, &token))
	{
		if (!foundFilePrefix && token.type != ParsingTokenType_FilePrefix)
		{
			LogPrintLine_E(log, "Found %s token before file prefix: \"%.*s\"", GetParsingTokenTypeStr(token.type), StrPrint(token.str));
			LogExitFailure(log, Result_TokenBeforeFilePrefix);
			FreeVarArray(&types);
			FreeVarArray(&bursts);
			FreeVarArray(&previousKeys);
			FreeVarArray(&namedColors);
			return false;
		}
		
		switch (token.type)
		{
			case ParsingTokenType_FilePrefix:
			{
				if (!foundFilePrefix)
				{
					if (!StrEquals(token.str, PARTS_PREFAB_COLLECTION_PREFIX))
					{
						LogPrintLine_E(log, "Invalid file prefix found: \"%.*s\"", StrPrint(token.str));
						LogExitFailure(log, Result_InvalidFilePrefix);
						FreeVarArray(&types);
						FreeVarArray(&bursts);
						FreeVarArray(&previousKeys);
						FreeVarArray(&namedColors);
						return false;
					}
					foundFilePrefix = true;
				}
				else
				{
					LogPrintLine_E(log, "Second file prefix found: \"%.*s\"", StrPrint(token.str));
					LogExitFailure(log, Result_MultipleFilePrefix);
					FreeVarArray(&types);
					FreeVarArray(&bursts);
					FreeVarArray(&previousKeys);
					FreeVarArray(&namedColors);
					return false;
				}
			} break;
			case ParsingTokenType_Comment:
			{
				//Nothing we need to do with comments right now
			} break;
			case ParsingTokenType_KeyValuePair:
			{
				bool isKnownKey = true;
				
				// +==============================+
				// |             Type             |
				// +==============================+
				if (StrEqualsIgnoreCase(token.key, "Type"))
				{
					currentType = nullptr;
					currentBurst = nullptr;
					currentPrefab = nullptr;
					VarArrayClear(&previousKeys);
					
					if (IsEmptyStr(token.value))
					{
						LogPrintLine_E(log, "Found Type with empty name on line %llu", parser.lineParser.lineIndex+1);
						log->hadWarnings = true;
					}
					else if (Deser_FindPartsTypeByName(&types, token.value) != UINT64_MAX)
					{
						LogPrintLine_E(log, "Type on line %llu has same name as previous type: \"%.*s\"", parser.lineParser.lineIndex+1, StrPrint(token.value));
						log->hadWarnings = true;
					}
					else
					{
						currentType = VarArrayAdd(&types, PartsType_t);
						currentBurst = nullptr;
						currentPrefab = nullptr;
						NotNull(currentType);
						ClearPointer(currentType);
						currentType->name = token.value;
						currentType->id = types.length;
						
						currentType->flags                = PartsTypeFlag_None;
						currentType->texture              = {};
						currentType->sheet                = {};
						currentType->frame                = NewPartsParamVec2i(PartsDistroFunc_Random, Vec2i_Zero, Vec2i_Zero);
						currentType->frameTime            = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->frameCount           = NewPartsParamI32(PartsDistroFunc_Random, 0, 0);
						currentType->frameOffset          = NewPartsParamI32(PartsDistroFunc_Random, 0, 0);
						currentType->lifeSpan             = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->depth                = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->scaleStart           = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->scaleEnd             = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->alphaStart           = NewPartsParamR32(PartsDistroFunc_Random, 1, 1);
						currentType->alphaEnd             = NewPartsParamR32(PartsDistroFunc_Random, 1, 1);
						currentType->rotation             = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->rotationVelocity     = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->rotationAcceleration = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->velocityDamping      = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->rotVelocityDamping   = NewPartsParamR32(PartsDistroFunc_Random, 0, 0);
						currentType->velocity             = NewPartsParamVec2(false, PartsDistroFunc_Random, Vec2_Zero, Vec2_Zero);
						currentType->acceleration         = NewPartsParamVec2(false, PartsDistroFunc_Random, Vec2_Zero, Vec2_Zero);
						currentType->colorStart           = NewPartsParamColor(PartsDistroFunc_Random, White, White);
						currentType->colorEnd             = NewPartsParamColor(PartsDistroFunc_Random, White, White);
					}
				}
				// +==============================+
				// |            Burst             |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "Burst"))
				{
					currentType = nullptr;
					currentBurst = nullptr;
					currentPrefab = nullptr;
					VarArrayClear(&previousKeys);
					
					if (IsEmptyStr(token.value))
					{
						LogPrintLine_E(log, "Found Burst with empty name on line %llu", parser.lineParser.lineIndex+1);
						log->hadWarnings = true;
					}
					else if (Deser_FindPartsBurstByName(&bursts, token.value) != UINT64_MAX)
					{
						LogPrintLine_E(log, "Burst on line %llu has same name as previous burst: \"%.*s\"", parser.lineParser.lineIndex+1, StrPrint(token.value));
						log->hadWarnings = true;
					}
					else
					{
						currentBurst = VarArrayAdd(&bursts, PartsBurst_t);
						NotNull(currentBurst);
						ClearPointer(currentBurst);
						currentBurst->name = token.value;
						currentBurst->id = bursts.length;
						
						currentBurst->count = NewPartsParamI32(PartsDistroFunc_Random, 0, 0);
						currentBurst->shape = PartsEmissionShape_Point;
						currentBurst->type.id = UINT64_MAX;
					}
				}
				// +==============================+
				// |            Prefab            |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "Prefab"))
				{
					currentType = nullptr;
					currentBurst = nullptr;
					currentPrefab = nullptr;
					VarArrayClear(&previousKeys);
					
					if (IsEmptyStr(token.value))
					{
						LogPrintLine_E(log, "Found Prefab with empty name on line %llu", parser.lineParser.lineIndex+1);
						log->hadWarnings = true;
					}
					else if (Deser_FindPartsPrefabByName(&collectionOut->prefabs, token.value) != UINT64_MAX)
					{
						LogPrintLine_E(log, "Prefab on line %llu has same name as previous prefab: \"%.*s\"", parser.lineParser.lineIndex+1, StrPrint(token.value));
						log->hadWarnings = true;
					}
					else
					{
						currentPrefab = VarArrayAdd(&collectionOut->prefabs, PartsPrefab_t);
						NotNull(currentPrefab);
						ClearPointer(currentPrefab);
						currentPrefab->name = AllocString(collectionOut->allocArena, &token.value);
						NotNullStr(&currentPrefab->name);
						currentPrefab->id = collectionOut->nextPrefabId;
						collectionOut->nextPrefabId++;
						CreateVarArray(&currentPrefab->bursts, collectionOut->allocArena, sizeof(PartsBurst_t));
					}
				}
				// +==============================+
				// |          ColorName           |
				// +==============================+
				else if (StrEqualsIgnoreCase(token.key, "ColorName"))
				{
					TempPushMark();
					
					Color_t colorValue;
					u64 numPieces = 0;
					MyStr_t* pieces = SplitString(TempArena, token.value, "=", &numPieces);
					for (u64 pIndex = 0; pIndex < numPieces; pIndex++) { TrimWhitespace(&pieces[pIndex]); }
					
					if (numPieces < 2)
					{
						LogPrintLine_E(log, "No equals character found in ColorName on line %llu: \"%.*s\"",
							parser.lineParser.lineIndex+1,
							StrPrint(token.value)
						);
						log->hadWarnings = true;
					}
					else if (numPieces > 2)
					{
						LogPrintLine_E(log, "Too many equals in ColorName on line %llu: \"%.*s\"",
							parser.lineParser.lineIndex+1,
							StrPrint(token.value)
						);
						log->hadWarnings = true;
					}
					else if (Deser_FindNamedColor(&namedColors, pieces[0], nullptr))
					{
						LogPrintLine_E(log, "Found a second ColorName named \"%.*s\", please use a different name (on line %llu)",
							StrPrint(pieces[0]),
							parser.lineParser.lineIndex+1
						);
						log->hadWarnings = true;
					}
					else if (!TryParseColor(pieces[1], &colorValue, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Failed to parse value \"%.*s\" for ColorName \"%.*s\" (on line %llu)",
							StrPrint(pieces[1]),
							StrPrint(pieces[0]),
							parser.lineParser.lineIndex+1
						);
						log->hadWarnings = true;
					}
					else
					{
						CollectionNamedColor_t* newColor = VarArrayAdd(&namedColors, CollectionNamedColor_t);
						ClearPointer(newColor);
						newColor->name = pieces[0];
						newColor->value = colorValue;
					}
					
					TempPopMark();
				}
				// +--------------------------------------------------------------+
				// |                         Type Options                         |
				// +--------------------------------------------------------------+
				else if (currentType != nullptr)
				{
					// +==============================+
					// |            Sheet             |
					// +==============================+
					if (StrEqualsIgnoreCase(token.key, "Sheet"))
					{
						u64 resourceIndex = 0;
						ResourceType_t resourceType = FindResourceByPathOrName(token.value, &resourceIndex, true);
						if (Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_W(log, "Warning: Found \"Sheet\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("Texture")))
						{
							LogPrintLine_W(log, "Warning: Found \"Sheet\" on a Type \"%.*s\" when \"Texture\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (resourceType == ResourceType_None)
						{
							LogPrintLine_E(log, "Warning: Unknown path given for \"Sheet\" in Type \"%.*s\": \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (resourceType != ResourceType_Sheet)
						{
							LogPrintLine_E(log, "Warning: A path for a %s Resource was given for \"Sheet\" in Type \"%.*s\": \"%.*s\" (on line %llu)",
								GetResourceTypeStr(resourceType),
								StrPrint(currentType->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else
						{
							currentType->sheet = GetSpriteSheetHandle(&pig->resources.sheets->items[resourceIndex]);
						}
					}
					// +==============================+
					// |           Texture            |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Texture"))
					{
						u64 resourceIndex = 0;
						ResourceType_t resourceType = FindResourceByPathOrName(token.value, &resourceIndex, true);
						if (Deser_WasKeySeen(&previousKeys, NewStr("Texture")))
						{
							LogPrintLine_W(log, "Warning: Found \"Texture\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_W(log, "Warning: Found \"Texture\" on a Type \"%.*s\" when \"Sheet\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (resourceType == ResourceType_None)
						{
							LogPrintLine_E(log, "Warning: Unknown path given for \"Texture\" in Type \"%.*s\": \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (resourceType != ResourceType_Texture)
						{
							LogPrintLine_E(log, "Warning: A path for a %s Resource was given for \"Texture\" in Type \"%.*s\": \"%.*s\" (on line %llu)",
								GetResourceTypeStr(resourceType),
								StrPrint(currentType->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else
						{
							currentType->texture = GetTextureHandle(&pig->resources.textures->items[resourceIndex]);
						}
					}
					// +==============================+
					// |          SourceRec           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "SourceRec"))
					{
						//TODO: Implement me!
					}
					// +==============================+
					// |            Frame             |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Frame"))
					{
						if (!Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_E(log, "The \"Frame\" option was given before \"Sheet\" was defined for Type \"%.*s\". Please set \"Sheet\" before using \"Frame\". (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("Frame")))
						{
							LogPrintLine_W(log, "Warning: Found \"Frame\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamVec2i(token.value, &currentType->frame, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as vec2i parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->frame should be filled out
						}
					}
					// +==============================+
					// |          FrameTime           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "FrameTime"))
					{
						if (!Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_E(log, "The \"FrameTime\" option was given before \"Sheet\" was defined for Type \"%.*s\". Please set \"Sheet\" before using \"FrameTime\". (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("FrameTime")))
						{
							LogPrintLine_W(log, "Warning: Found \"FrameTime\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamR32(token.value, &currentType->frameTime, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as r32 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->frameTime should be filled out
						}
					}
					// +==============================+
					// |          FrameCount          |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "FrameCount"))
					{
						if (!Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_E(log, "The \"FrameCount\" option was given before \"Sheet\" was defined for Type \"%.*s\". Please set \"Sheet\" before using \"FrameCount\". (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("FrameCount")))
						{
							LogPrintLine_W(log, "Warning: Found \"FrameCount\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamI32(token.value, &currentType->frameCount, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as i32 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->frameCount should be filled out
						}
					}
					// +==============================+
					// |         FrameOffset          |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "FrameOffset"))
					{
						if (!Deser_WasKeySeen(&previousKeys, NewStr("Sheet")))
						{
							LogPrintLine_E(log, "The \"FrameOffset\" option was given before \"Sheet\" was defined for Type \"%.*s\". Please set \"Sheet\" before using \"FrameOffset\". (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("FrameOffset")))
						{
							LogPrintLine_W(log, "Warning: Found \"FrameOffset\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamI32(token.value, &currentType->frameOffset, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as i32 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->frameOffset should be filled out
						}
					}
					// +==============================+
					// |           LifeSpan           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "LifeSpan"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->lifeSpan, &token);
					}
					// +==============================+
					// |            Depth             |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Depth"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->depth, &token);
					}
					// +==============================+
					// |          ColorStart          |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "ColorStart"))
					{
						MyStr_t failedColorStr = token.value;
						if (Deser_WasKeySeen(&previousKeys, NewStr("ColorStart")))
						{
							LogPrintLine_W(log, "Warning: Found \"ColorStart\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamColor(&namedColors, token.value, &currentType->colorStart, &log->parseFailureReason, &failedColorStr))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as color parameter parameter \"%.*s\" (line %llu). Make sure all the color names are typed correctly and pre-declared: %s (%llu named colors)",
								StrPrint(token.key),
								StrPrint(failedColorStr),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason),
								namedColors.length
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->colorStart should be filled out
						}
					}
					// +==============================+
					// |           ColorEnd           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "ColorEnd"))
					{
						MyStr_t failedColorStr = token.value;
						if (Deser_WasKeySeen(&previousKeys, NewStr("ColorEnd")))
						{
							LogPrintLine_W(log, "Warning: Found \"ColorEnd\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamColor(&namedColors, token.value, &currentType->colorEnd, &log->parseFailureReason, &failedColorStr))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as color parameter \"%.*s\" (line %llu). Make sure all the color names are typed correctly and pre-declared: %s (%llu named colors)",
								StrPrint(token.key),
								StrPrint(failedColorStr),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason),
								namedColors.length
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->colorEnd should be filled out
						}
					}
					// +==============================+
					// |          AlphaStart          |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "AlphaStart"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->alphaStart, &token);
					}
					// +==============================+
					// |           AlphaEnd           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "AlphaEnd"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->alphaEnd, &token);
					}
					// +==============================+
					// |           Velocity           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Velocity"))
					{
						if (Deser_WasKeySeen(&previousKeys, NewStr("Speed")))
						{
							LogPrintLine_W(log, "Warning: Found \"Velocity\" on a Type \"%.*s\" when \"Speed\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("Direction")))
						{
							LogPrintLine_W(log, "Warning: Found \"Velocity\" on a Type \"%.*s\" when \"Direction\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_WasKeySeen(&previousKeys, NewStr("Velocity")))
						{
							LogPrintLine_W(log, "Warning: Found \"Velocity\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamVec2(token.value, &currentType->velocity, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as vec2 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->velocity should be filled out
						}
					}
					// +==============================+
					// |          Direction           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Direction"))
					{
						tempDirectionParam = {};
						if (Deser_WasKeySeen(&previousKeys, NewStr("Velocity")))
						{
							LogPrintLine_W(log, "Warning: Found \"Direction\" on a Type \"%.*s\" when \"Velocity\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &tempDirectionParam, &token))
						{
							if (Deser_WasKeySeen(&previousKeys, NewStr("Speed")))
							{
								currentType->velocity = NewPartsParamVec2(
									true,
									tempDirectionParam.distroFunc,
									NewVec2(ToRadians32(tempDirectionParam.minValue), tempSpeedParam.minValue),
									NewVec2(ToRadians32(tempDirectionParam.maxValue), tempSpeedParam.maxValue)
								);
							}
						}
					}
					// +==============================+
					// |            Speed             |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Speed"))
					{
						tempSpeedParam = {};
						if (Deser_WasKeySeen(&previousKeys, NewStr("Velocity")))
						{
							LogPrintLine_W(log, "Warning: Found \"Speed\" on a Type \"%.*s\" when \"Velocity\" was already given, Only one or the other is allowed (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &tempSpeedParam, &token))
						{
							if (Deser_WasKeySeen(&previousKeys, NewStr("Direction")))
							{
								currentType->velocity = NewPartsParamVec2(
									true,
									tempDirectionParam.distroFunc,
									NewVec2(ToRadians32(tempDirectionParam.minValue), tempSpeedParam.minValue),
									NewVec2(ToRadians32(tempDirectionParam.maxValue), tempSpeedParam.maxValue)
								);
							}
						}
					}
					// +==============================+
					// |         Acceleration         |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Acceleration"))
					{
						if (Deser_WasKeySeen(&previousKeys, NewStr("Acceleration")))
						{
							LogPrintLine_W(log, "Warning: Found \"Acceleration\" a second time in one Type \"%.*s\" (on line %llu)",
								StrPrint(currentType->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamVec2(token.value, &currentType->acceleration, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as vec2 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentType->acceleration should be filled out
						}
					}
					// +==============================+
					// |          ScaleStart          |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "ScaleStart"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->scaleStart, &token);
					}
					// +==============================+
					// |           ScaleEnd           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "ScaleEnd"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->scaleEnd, &token);
					}
					// +==============================+
					// |           Rotation           |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Rotation"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->rotation, &token);
					}
					// +==============================+
					// |       RotationVelocity       |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "RotationVelocity"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->rotationVelocity, &token);
					}
					// +==============================+
					// |     RotationAcceleration     |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "RotationAcceleration"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->rotationAcceleration, &token);
					}
					// +==============================+
					// |       VelocityDamping        |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "VelocityDamping"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->velocityDamping, &token);
					}
					// +==============================+
					// |      RotVelocityDamping      |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "RotVelocityDamping"))
					{
						Deser_ParseTypeParamR32(log, &parser, &previousKeys, currentType, &currentType->rotVelocityDamping, &token);
					}
					else { isKnownKey = false; }
				}
				// +--------------------------------------------------------------+
				// |                        Burst Options                         |
				// +--------------------------------------------------------------+
				else if (currentBurst != nullptr)
				{
					// +==============================+
					// |            Using             |
					// +==============================+
					if (StrEqualsIgnoreCase(token.key, "Using"))
					{
						u64 typeId = Deser_FindPartsTypeByName(&types, token.value);
						if (Deser_WasKeySeen(&previousKeys, NewStr("Using")))
						{
							LogPrintLine_W(log, "Warning: Found \"Using\" a second time in one Burst \"%.*s\" (on line %llu)",
								StrPrint(currentBurst->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (typeId == UINT64_MAX)
						{
							LogPrintLine_E(log, "Burst \"%.*s\" requests type \"%.*s\" which doesn't exist. Make sure you used to correct name. (line %llu)",
								StrPrint(currentBurst->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else
						{
							Assert(typeId <= types.length);
							const PartsType_t* typePntr = VarArrayGet(&types, typeId-1, PartsType_t);
							MyMemCopy(&currentBurst->type, typePntr, sizeof(PartsType_t));
						}
					}
					// +==============================+
					// |            Count             |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Count"))
					{
						if (Deser_WasKeySeen(&previousKeys, NewStr("Count")))
						{
							LogPrintLine_W(log, "Warning: Found \"Count\" a second time in one Burst \"%.*s\" (on line %llu)",
								StrPrint(currentBurst->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsParamI32(token.value, &currentBurst->count, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as i32 parameter (line %llu): %s",
								StrPrint(token.key),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentBurst->count should be filled out
						}
					}
					// +==============================+
					// |            Shape             |
					// +==============================+
					else if (StrEqualsIgnoreCase(token.key, "Shape"))
					{
						if (Deser_WasKeySeen(&previousKeys, NewStr("Shape")))
						{
							LogPrintLine_W(log, "Warning: Found \"Shape\" a second time in one Burst \"%.*s\" (on line %llu)",
								StrPrint(currentBurst->name),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else if (!TryParsePartsEmissionShape(token.value, &currentBurst->shape, &log->parseFailureReason))
						{
							LogPrintLine_E(log, "Failed to parse \"%.*s\" as emission shape parameter (line %llu): %s",
								StrPrint(token.value),
								parser.lineParser.lineIndex+1,
								GetTryParseFailureReasonStr(log->parseFailureReason)
							);
							log->hadWarnings = true;
						}
						else
						{
							// Cool. currentBurst->shape should be filled out
						}
					}
					//TODO: Parse Position
					//TODO: Parse Size
					//TODO: Parse Radius
					//TODO: Parse StartPos/EndPos
					else { isKnownKey = false; }
				}
				// +--------------------------------------------------------------+
				// |                        Prefab Options                        |
				// +--------------------------------------------------------------+
				else if (currentPrefab != nullptr)
				{
					// +==============================+
					// |              Do              |
					// +==============================+
					if (StrEqualsIgnoreCase(token.key, "Do"))
					{
						u64 burstId = Deser_FindPartsBurstByName(&bursts, token.value);
						if (burstId == UINT64_MAX)
						{
							LogPrintLine_E(log, "Prefab \"%.*s\" requests burst \"%.*s\" which doesn't exist. Make sure you used to correct name. (line %llu)",
								StrPrint(currentPrefab->name),
								StrPrint(token.value),
								parser.lineParser.lineIndex+1
							);
							log->hadWarnings = true;
						}
						else
						{
							Assert(burstId <= bursts.length);
							const PartsBurst_t* burstPntr = VarArrayGet(&bursts, burstId-1, PartsBurst_t);
							PartsBurst_t* newBurst = VarArrayAdd(&currentPrefab->bursts, PartsBurst_t);
							NotNull(newBurst);
							MyMemCopy(newBurst, burstPntr, sizeof(PartsBurst_t));
							newBurst->id = collectionOut->nextBurstId;
							collectionOut->nextBurstId++;
							newBurst->name = AllocString(collectionOut->allocArena, &newBurst->name);
							NotNullStr(&newBurst->name);
							newBurst->type.name = AllocString(collectionOut->allocArena, &newBurst->type.name);
							NotNullStr(&newBurst->type.name);
						}
					}
					else { isKnownKey = false; }
				}
				else { isKnownKey = false; }
				
				if (!isKnownKey)
				{
					LogPrintLine_W(log, "Unknown key \"%.*s\" on line %llu. Are you sure you typed the name correctly? Is this key in the right place in the file?",
						StrPrint(token.key), parser.lineParser.lineIndex+1
					);
					log->hadWarnings = true;
				}
				
				if (!Deser_WasKeySeen(&previousKeys, token.value))
				{
					MyStr_t* newKeySeenSpace = VarArrayAdd(&previousKeys, MyStr_t);
					NotNull(newKeySeenSpace);
					*newKeySeenSpace = token.key;
				}
			} break;
			default:
			{
				LogPrintLine_W(log, "Warning: Unhandled ParsingTokenType! Type: %u Token: \"%.*s\"", token.type, StrPrint(token.str));
				log->hadWarnings = true;
			} break;
		}
	}
	
	FreeVarArray(&types);
	FreeVarArray(&bursts);
	FreeVarArray(&previousKeys);
	FreeVarArray(&namedColors);
	return true;
}