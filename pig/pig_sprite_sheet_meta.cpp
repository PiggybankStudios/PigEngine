/*
File:   pig_sprite_sheet_meta.cpp
Author: Taylor Robbins
Date:   01\18\2022
Description: 
	** Holds the functions that deserialize and serialize the meta
	** file for SpriteSheet_t's
*/

#define SPRITE_SHEET_META_FILE_PREFIX "# Sprite Meta"

enum SpriteSheetMetaError_t
{
	SpriteSheetMetaError_None = 0,
	SpriteSheetMetaError_TokenBeforeFilePrefix,
	SpriteSheetMetaError_InvalidFilePrefix,
	SpriteSheetMetaError_MultipleFilePrefix,
	SpriteSheetMetaError_MissingFilePrefix,
	SpriteSheetMetaError_KeyValuePairBeforeFrameDefined,
	SpriteSheetMetaError_NumErrors,
};
const char* GetSpriteSheetMetaErrorStr(SpriteSheetMetaError_t error)
{
	switch (error)
	{
		case SpriteSheetMetaError_None:                           return "None";
		case SpriteSheetMetaError_TokenBeforeFilePrefix:          return "TokenBeforeFilePrefix";
		case SpriteSheetMetaError_InvalidFilePrefix:              return "InvalidFilePrefix";
		case SpriteSheetMetaError_KeyValuePairBeforeFrameDefined: return "KeyValuePairBeforeFrameDefined";
		default: return "Unknown";
	}
}

bool TryDeserSpriteSheetMeta(MyStr_t fileContents, SpriteSheet_t* sheet, ProcessLog_t* log)
{
	NotNullStr(&fileContents);
	NotNull(sheet);
	NotNull(sheet->allocArena);
	NotNull(log);
	
	LogWriteLine_N(log, "Entering TryDeserSpriteSheetMeta...");
	SetProcessLogName(log, NewStr("Deserialize SpriteSheet Meta"));
	
	TextParser_t textParser = NewTextParser(fileContents);
	
	bool foundFilePrefix = false;
	
	bool currentFrameFilled = false;
	bool currentFrameIsValid = false;
	v2i currentFramePos = Vec2i_Zero;
	SpriteSheetFrame_t* currentFramePntr = nullptr;
	SpriteSheetFramePoint_t* currentFramePoint = nullptr;
	
	ParsingToken_t token = {};
	while (TextParserGetToken(&textParser, &token))
	{
		if (!foundFilePrefix && token.type != ParsingTokenType_FilePrefix)
		{
			LogPrintLine_E(log, "Found %s token before file prefix: \"%.*s\"", GetParsingTokenTypeStr(token.type), token.str.length, token.str.pntr);
			LogExitFailure(log, SpriteSheetMetaError_TokenBeforeFilePrefix);
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
					if (!StrEquals(token.str, SPRITE_SHEET_META_FILE_PREFIX))
					{
						LogPrintLine_E(log, "Invalid file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_InvalidFilePrefix);
						return false;
					}
					foundFilePrefix = true;
				}
				else
				{
					LogPrintLine_E(log, "Second file prefix found: \"%.*s\"", token.str.length, token.str.pntr);
					LogExitFailure(log, SpriteSheetMetaError_MultipleFilePrefix);
					return false;
				}
			} break;
			
			// +===============================+
			// | ParsingTokenType_KeyValuePair |
			// +===============================+
			case ParsingTokenType_KeyValuePair:
			{
				// +==============================+
				// |             Name             |
				// +==============================+
				if (StrCompareIgnoreCase(token.key, "Name") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found Name key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						if (currentFramePoint != nullptr)
						{
							FreeString(sheet->allocArena, &currentFramePoint->name);
							currentFramePoint->name = AllocString(sheet->allocArena, &token.value);
						}
						else
						{
							FreeString(sheet->allocArena, &currentFramePntr->name);
							currentFramePntr->name = AllocString(sheet->allocArena, &token.value);
						}
					}
				}
				// +==============================+
				// |            Point             |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "Point") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found Name key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						v2 pointValue = Vec2_Zero;
						if (TryParseV2(token.value, &pointValue, &log->parseFailureReason))
						{
							SpriteSheetFramePoint_t* newPoint = VarArrayAdd(&currentFramePntr->points, SpriteSheetFramePoint_t);
							NotNull(newPoint);
							ClearPointer(newPoint);
							newPoint->point = pointValue;
							newPoint->name = MyStr_Empty;
							currentFramePoint = newPoint;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as v2 for Point on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				// +==============================+
				// |          Codepoint           |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "Codepoint") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found Codepoint key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						u32 codepointValue = 0;
						if (TryParseU32(token.value, &codepointValue, &log->parseFailureReason))
						{
							currentFramePntr->codepoint = codepointValue;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as u32 for Codepoint on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				// +==============================+
				// |           AdvanceX           |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "AdvanceX") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found AdvanceX key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						r32 advanceXValue = 0;
						if (TryParseR32(token.value, &advanceXValue, &log->parseFailureReason))
						{
							currentFramePntr->advanceX = advanceXValue;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as r32 for AdvanceX on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				// +==============================+
				// |          CharBounds          |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "CharBounds") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found CharBounds key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						reci charBoundsValue = Reci_Zero;
						if (TryParseReci(token.value, &charBoundsValue, &log->parseFailureReason))
						{
							currentFramePntr->charBounds = charBoundsValue;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as reci for CharBounds on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				// +==============================+
				// |        LogicalBounds         |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "LogicalBounds") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found LogicalBounds key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						reci logicalBoundsValue = Reci_Zero;
						if (TryParseReci(token.value, &logicalBoundsValue, &log->parseFailureReason))
						{
							currentFramePntr->logicalBounds = logicalBoundsValue;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as reci for LogicalBounds on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				// +==============================+
				// |          CharOffset          |
				// +==============================+
				else if (StrCompareIgnoreCase(token.key, "CharOffset") == 0)
				{
					if (!currentFrameFilled)
					{
						LogPrintLine_E(log, "Found CharOffset key before a frame index was given on line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
						LogExitFailure(log, SpriteSheetMetaError_KeyValuePairBeforeFrameDefined);
						return false;
					}
					
					if (currentFrameIsValid)
					{
						v2i charOffsetValue = Vec2i_Zero;
						if (TryParseV2i(token.value, &charOffsetValue, &log->parseFailureReason))
						{
							currentFramePntr->charOffset = charOffsetValue;
						}
						else
						{
							LogPrintLine_W(log, "WARNING: Couldn't parse value as v2i for CharOffset on line %llu for frame (%d, %d): \"%.*s\"",
								textParser.lineParser.lineIndex+1,
								currentFramePos.x, currentFramePos.y,
								token.value.length, token.value.pntr
							);
							log->hadWarnings = true;
						}
					}
				}
				else
				{
					LogPrintLine_W(log, "WARNING: Unhandled key: \"%.*s\"", token.key.length, token.key.pntr);
					log->hadWarnings = true;
				}
			} break;
			
			// +==============================+
			// |   ParsingTokenType_Unknown   |
			// +==============================+
			case ParsingTokenType_Unknown:
			{
				v2i newFrameValue = Vec2i_Zero;
				if (TryParseV2i(token.str, &newFrameValue, &log->parseFailureReason))
				{
					if (newFrameValue.x < 0 || newFrameValue.y < 0 ||
						newFrameValue.x >= sheet->numFramesX || newFrameValue.y >= sheet->numFramesY)
					{
						LogPrintLine_W(log, "WARNING: Invalid frame index on line %llu (%d, %d) in sheet that is %dx%d: \"%.*s\"",
							textParser.lineParser.lineIndex+1,
							newFrameValue.x, newFrameValue.y,
							sheet->numFramesX, sheet->numFramesY,
							token.str.length, token.str.pntr
						);
						log->hadWarnings = true;
						currentFrameIsValid = false;
						currentFramePntr = nullptr;
					}
					else
					{
						currentFrameIsValid = true;
						currentFramePntr = VarArrayGetHard(&sheet->frames, (sheet->numFramesX * newFrameValue.y) + newFrameValue.x, SpriteSheetFrame_t);
					}
					currentFramePos = newFrameValue;
					currentFrameFilled = true;
					currentFramePoint = nullptr;
					
				}
				else
				{
					LogPrintLine_W(log, "WARNING: Unknown token in file line %llu: \"%.*s\"", textParser.lineParser.lineIndex+1, token.str.length, token.str.pntr);
					log->hadWarnings = true;
				}
			} break;
			
			default:
			{
				DebugAssert(false);
			} break;
		}
	}
	
	if (!foundFilePrefix)
	{
		LogWriteLine_E(log, "Found no file prefix! This is probably an empty file");
		LogExitFailure(log, SpriteSheetMetaError_MissingFilePrefix);
		return false;
	}
	
	LogWriteLine_I(log, "Successfully deserialized sprite sheet meta file");
	LogExitSuccess(log);
	return true;
}

MyStr_t SerializeSpriteSheetMeta(const SpriteSheet_t* sheet, MemArena_t* memArena)
{
	NotNull(sheet);
	MyStr_t result = MyStr_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		u64 byteIndex = 0;
		
		TwoPassPrint(result.pntr, result.length, &byteIndex, "%s\n", SPRITE_SHEET_META_FILE_PREFIX);
		TwoPassPrint(result.pntr, result.length, &byteIndex, "// %s %s %u\n\n", GetMonthStr(pigIn->localTime.month), GetDayOfMonthString(pigIn->localTime.day), pigIn->localTime.year);
		
		VarArrayLoop(&sheet->frames, fIndex)
		{
			VarArrayLoopGet(SpriteSheetFrame_t, frame, &sheet->frames, fIndex);
			if (!IsEmptyStr(frame->name) || frame->points.length > 0 || frame->codepoint != 0 ||
				frame->advanceX != 0 || frame->charBounds != Reci_Zero || frame->logicalBounds != Reci_Zero || frame->charOffset != Vec2i_Zero)
			{
				TwoPassPrint(result.pntr, result.length, &byteIndex, "(%d, %d)\n", frame->gridPos.x, frame->gridPos.y);
				if (!IsEmptyStr(frame->name))
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tName: %.*s\n", frame->name.length, frame->name.pntr);
				}
				if (frame->codepoint != 0)
				{
					if (frame->codepoint >= 0x21 && frame->codepoint <= 0x7E)
					{
						TwoPassPrint(result.pntr, result.length, &byteIndex, "\tCodepoint: 0x%08X //%c\n", frame->codepoint, (char)frame->codepoint);
					}
					else
					{
						TwoPassPrint(result.pntr, result.length, &byteIndex, "\tCodepoint: 0x%08X\n", frame->codepoint);
					}
				}
				if (frame->advanceX != 0)
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tAdvanceX: %g\n", frame->advanceX);
				}
				if (frame->charBounds != Reci_Zero)
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tCharBounds: (%d, %d, %d, %d)\n", frame->charBounds.x, frame->charBounds.y, frame->charBounds.width, frame->charBounds.height);
				}
				if (frame->logicalBounds != Reci_Zero)
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tLogicalBounds: (%d, %d, %d, %d)\n", frame->logicalBounds.x, frame->logicalBounds.y, frame->logicalBounds.width, frame->logicalBounds.height);
				}
				if (frame->charOffset != Vec2i_Zero)
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tCharOffset: (%d, %d)\n", frame->charOffset.x, frame->charOffset.y);
				}
				VarArrayLoop(&frame->points, pIndex)
				{
					VarArrayLoopGet(SpriteSheetFramePoint_t, point, &frame->points, pIndex);
					TwoPassPrint(result.pntr, result.length, &byteIndex, "\tPoint: (%g, %g)\n", point->point.x, point->point.y);
					if (!IsEmptyStr(point->name))
					{
						TwoPassPrint(result.pntr, result.length, &byteIndex, "\t\tName: %.*s\n", point->name.length, point->name.pntr);
					}
				}
			}
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
