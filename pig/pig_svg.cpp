/*
File:   pig_svg.cpp
Author: Taylor Robbins
Date:   01\23\2022
Description: 
	** Holds the code that parses svg files (an xml file format)
*/

enum SvgPathDataCmd_t
{
	SvgPathDataCmd_None = 0,
	SvgPathDataCmd_MoveTo,
	SvgPathDataCmd_Horizontal,
	SvgPathDataCmd_Vertical,
	SvgPathDataCmd_Line,
	SvgPathDataCmd_Quadratic,
	SvgPathDataCmd_Cubic,
	SvgPathDataCmd_SmoothQuadratic,
	SvgPathDataCmd_SmoothCubic,
	SvgPathDataCmd_Ellipse,
	SvgPathDataCmd_ClosePath,
	SvgPathDataCmd_NumCommands,
};
const char* GetSvgPathDataCmdStr(SvgPathDataCmd_t pathDataCmd)
{
	switch (pathDataCmd)
	{
		case SvgPathDataCmd_None:            return "None";
		case SvgPathDataCmd_MoveTo:          return "MoveTo";
		case SvgPathDataCmd_Horizontal:      return "Horizontal";
		case SvgPathDataCmd_Vertical:        return "Vertical";
		case SvgPathDataCmd_Line:            return "Line";
		case SvgPathDataCmd_Quadratic:       return "Quadratic";
		case SvgPathDataCmd_Cubic:           return "Cubic";
		case SvgPathDataCmd_SmoothQuadratic: return "SmoothQuadratic";
		case SvgPathDataCmd_SmoothCubic:     return "SmoothCubic";
		case SvgPathDataCmd_Ellipse:         return "Ellipse";
		case SvgPathDataCmd_ClosePath:       return "ClosePath";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                             Free                             |
// +--------------------------------------------------------------+
void FreeSvgData(SvgData_t* data)
{
	NotNull(data);
	AssertIf(data->groups.length > 0, data->allocArena != nullptr);
	VarArrayLoop(&data->groups, gIndex)
	{
		VarArrayLoopGet(SvgGroup_t, group, &data->groups, gIndex);
		VarArrayLoop(&group->shapes, sIndex)
		{
			VarArrayLoopGet(SvgShape_t, shape, &group->shapes, sIndex);
			if (shape->type == SvgShapeType_Path)
			{
				FreeBezierPath(&shape->path.value);
				DestroyVertBuffer(&shape->path.vertBuffer);
			}
			FreeString(data->allocArena, &shape->idStr);
			FreeString(data->allocArena, &shape->label);
		}
		FreeString(data->allocArena, &group->idStr);
		FreeString(data->allocArena, &group->label);
		FreeVarArray(&group->shapes);
	}
	FreeVarArray(&data->groups);
	ClearPointer(data);
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
bool TryParseSvgTransformString(ProcessLog_t* log, MyStr_t transformStr, SvgTransform_t* transformOut)
{
	NotNull(log);
	NotNullStr(&transformStr);
	NotNull(transformOut);
	
	MyStr_t parensPart = FindStrParensPart(transformStr);
	if (parensPart.length == 0)
	{
		LogPrintLine_E(log, "Transform string contained no parenthesis: \"%.*s\"", transformStr.length, transformStr.pntr);
		return false;
	}
	u64 parensIndex = (u64)(parensPart.pntr - transformStr.pntr);
	MyStr_t typePart = StrSubstring(&transformStr, 0, parensIndex);
	parensPart = StrSubstring(&parensPart, 1, parensPart.length-1);
	
	if (StrEqualsIgnoreCase(typePart, "scale"))
	{
		r32 parsedValue = 0;
		if (!TryParseR32(parensPart, &parsedValue, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Couldn't parse transform scale as r32: \"%.*s\"", parensPart.length, parensPart.pntr);
			return false;
		}
		
		ClearPointer(transformOut);
		transformOut->type = SvgTransformType_Scale;
		transformOut->scale = parsedValue;
		return true;
	}
	else if (StrEqualsIgnoreCase(typePart, "rotate"))
	{
		r32 parsedValue = 0;
		if (!TryParseR32(parensPart, &parsedValue, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Couldn't parse transform rotation as r32: \"%.*s\"", parensPart.length, parensPart.pntr);
			return false;
		}
		
		ClearPointer(transformOut);
		transformOut->type = SvgTransformType_Rotate;
		transformOut->rotation = ToRadians32(parsedValue);
		return true;
	}
	else if (StrEqualsIgnoreCase(typePart, "translate"))
	{
		v2 parsedValue = Vec2_Zero;
		if (!TryParseV2(parensPart, &parsedValue, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Couldn't parse transform translate as vec2: \"%.*s\"", parensPart.length, parensPart.pntr);
			return false;
		}
		
		ClearPointer(transformOut);
		transformOut->type = SvgTransformType_Translate;
		transformOut->vector = parsedValue;
		return true;
	}
	else if (StrEqualsIgnoreCase(typePart, "matrix"))
	{
		TempPushMark();
		u64 numNumberParts = 0;
		MyStr_t* numberParts = SplitString(TempArena, parensPart, ",", &numNumberParts);
		if (numNumberParts != 6)
		{
			LogPrintLine_E(log, "We don't support %llu parts in a \"matrix\" transformation: \"%.*s\"", numNumberParts, transformStr.length, transformStr.pntr);
			return false;
		}
		
		transformOut->type = SvgTransformType_Matrix;
		transformOut->matrix = Mat4_Identity;
		
		if (!TryParseR32(numberParts[0], &transformOut->matrix.r0c0, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[0] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		if (!TryParseR32(numberParts[1], &transformOut->matrix.r0c1, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[1] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		if (!TryParseR32(numberParts[2], &transformOut->matrix.r1c0, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[2] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		if (!TryParseR32(numberParts[3], &transformOut->matrix.r1c1, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[3] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		//The last 2 numbers are translation, so in a 4x4 matrix, they go in the w column
		if (!TryParseR32(numberParts[4], &transformOut->matrix.r0c3, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[4] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		if (!TryParseR32(numberParts[5], &transformOut->matrix.r1c3, &log->parseFailureReason))
		{
			LogPrintLine_E(log, "Failed to parse number[5] as r32 in matrix transformation: \"%.*s\"", transformStr.length, transformStr.pntr);
			return false;
		}
		
		//NOTE: It seems since inkscape works in a "+y is up" coordinate space, we need to flip the sign
		//      of these two matrix values when translating to our "+y is down" coordinate space
		transformOut->matrix.r0c1 = -transformOut->matrix.r0c1;
		transformOut->matrix.r1c0 = -transformOut->matrix.r1c0;
		
		TempPopMark();
		return true;
	}
	else
	{
		LogPrintLine_E(log, "Unknown transform type before parenthesis: \"%.*s\"", transformStr.length, transformStr.pntr);
		return false;
	}
}

bool TryParseSvgShapeStyleString(ProcessLog_t* log, MyStr_t styleStr, SvgFill_t* fillOut, SvgStroke_t* strokeOut)
{
	NotNull(log);
	NotNullStr(&styleStr);
	if (fillOut != nullptr) { ClearPointer(fillOut); }
	if (strokeOut != nullptr) { ClearPointer(strokeOut); }
	
	r32 opacity = 1.0f;
	r32 strokeOpacity = 1.0f;
	
	TempPushMark();
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(TempArena, styleStr, ";", &numPieces);
	for (u64 pIndex = 0; pIndex < numPieces; pIndex++)
	{
		u64 colonIndex = 0;
		bool pieceHasColon = FindNextCharInStr(pieces[pIndex], 0, ":", &colonIndex);
		if (!pieceHasColon)
		{
			LogPrintLine_E(log, "Style string property %u/%u did not contain a colon: \"%.*s\"", pIndex+1, numPieces, pieces[pIndex].length, pieces[pIndex].pntr);
			TempPopMark();
			return false;
		}
		
		MyStr_t key = NewStr(colonIndex, &pieces[pIndex].pntr[0]);
		MyStr_t value = NewStr(pieces[pIndex].length - (colonIndex+1), &pieces[pIndex].pntr[colonIndex+1]);
		if (key.length == 0 || value.length == 0)
		{
			LogPrintLine_E(log, "Style string property %u/%u had empty key or value: \"%.*s\"", pIndex+1, numPieces, pieces[pIndex].length, pieces[pIndex].pntr);
			TempPopMark();
			return false;
		}
		
		//TODO: Handle some of these options?
		//        fill-opacity
		//        stroke-linecap
		//        stroke-linejoin
		//        stroke-miterlimit
		//        stroke-dasharray
		//        stroke-dashoffset
		
		// +==============================+
		// |             fill             |
		// +==============================+
		if (StrEquals(key, "fill"))
		{
			Color_t colorValue;
			if (StrEquals(value, "none"))
			{
				if (fillOut != nullptr) { fillOut->style = SvgFillStyle_None; }
			}
			else if (TryParseColor(value, &colorValue, &log->parseFailureReason))
			{
				colorValue.a = MultiplyColorChannelR32(colorValue.a, opacity);
				if (fillOut != nullptr)
				{
					fillOut->style = SvgFillStyle_Solid;
					fillOut->color = colorValue;
				}
			}
			else
			{
				LogPrintLine_E(log, "Failed to parse style value \"fill\" as known style or as color: \"%.*s\"", value.length, value.pntr);
				TempPopMark();
				return false;
			}
		}
		// +==============================+
		// |           opacity            |
		// +==============================+
		else if (StrEquals(key, "opacity"))
		{
			if (!TryParseR32(value, &opacity, &log->parseFailureReason))
			{
				LogPrintLine_E(log, "Failed to parse style value \"opacity\" as r32: \"%.*s\"", value.length, value.pntr);
				TempPopMark();
				return false;
			}
			if (fillOut != nullptr) { fillOut->color.a = MultiplyColorChannelR32(fillOut->color.a, opacity); }
		}
		// +==============================+
		// |            stroke            |
		// +==============================+
		else if (StrEquals(key, "stroke"))
		{
			Color_t colorValue;
			if (StrEquals(value, "none"))
			{
				if (strokeOut != nullptr) { strokeOut->style = SvgStrokeStyle_None; }
			}
			else if (TryParseColor(value, &colorValue, &log->parseFailureReason))
			{
				colorValue.a = MultiplyColorChannelR32(colorValue.a, strokeOpacity);
				if (strokeOut != nullptr)
				{
					strokeOut->style = SvgStrokeStyle_Solid;
					strokeOut->color = colorValue;
				}
			}
			else
			{
				LogPrintLine_E(log, "Failed to parse style value \"stroke\" as known style or as color: \"%.*s\"", value.length, value.pntr);
				TempPopMark();
				return false;
			}
		}
		// +==============================+
		// |         stroke-width         |
		// +==============================+
		else if (StrEquals(key, "stroke-width"))
		{
			r32 widthValue;
			if (!TryParseR32(value, &widthValue, &log->parseFailureReason))
			{
				LogPrintLine_E(log, "Failed to parse style value \"stroke-width\" as r32: \"%.*s\"", value.length, value.pntr);
				TempPopMark();
				return false;
			}
			
			if (strokeOut != nullptr) { strokeOut->thickness = widthValue; }
		}
		// +==============================+
		// |        stroke-opacity        |
		// +==============================+
		else if (StrEquals(key, "stroke-opacity"))
		{
			if (!TryParseR32(value, &strokeOpacity, &log->parseFailureReason))
			{
				LogPrintLine_E(log, "Failed to parse style value \"stroke-opacity\" as r32: \"%.*s\"", value.length, value.pntr);
				TempPopMark();
				return false;
			}
			
			if (strokeOut != nullptr) { strokeOut->color.a = MultiplyColorChannelR32(strokeOut->color.a, strokeOpacity); }
		}
	}
	TempPopMark();
	
	return true;
}

bool TryParseSvgPathDataString(ProcessLog_t* log, MyStr_t dataStr, BezierPath_t* pathOut)
{
	NotNull(log);
	NotNullStr(&dataStr);
	NotNull(pathOut);
	//https://www.w3.org/TR/SVG2/paths.html#PathData
	
	v2 currentPos = Vec2_Zero;
	v2 startPos = Vec2_Zero;
	bool isDetached = true;
	
	bool commandIsRelative = false;
	SvgPathDataCmd_t command = SvgPathDataCmd_None;
	u64 numParsedPoints = 0;
	v2 parsedPoints[4];
	BezierPathPart_t* prevPathPart = nullptr;
	
	for (u64 cIndex = 0; cIndex < dataStr.length; )
	{
		u64 startOfLoopIndex = cIndex;
		u32 nextCodepoint = 0;
		u8 codepointByteSize = GetCodepointForUtf8Str(dataStr, cIndex, &nextCodepoint);
		if (codepointByteSize == 0)
		{
			LogPrintLine_E(log, "UTF-8 encoding error in path data string byte %llu", cIndex);
			return false;
		}
		bool nextCodepointIsLower = IsCharLowercaseAlphabet(nextCodepoint);
		u32 nextLowerCodepoint = GetLowercaseCodepoint(nextCodepoint);
		
		BezierPathPart_t newPathPart = {};
		
		if (IsCharWhitespace(nextCodepoint))
		{
			cIndex += codepointByteSize;
			continue;
		}
		else if (nextLowerCodepoint >= 'a' && nextLowerCodepoint <= 'z')
		{
			commandIsRelative = nextCodepointIsLower;
			numParsedPoints = 0;
			if (nextLowerCodepoint == 'm')      { command = SvgPathDataCmd_MoveTo;          }
			else if (nextLowerCodepoint == 'h') { command = SvgPathDataCmd_Horizontal;      }
			else if (nextLowerCodepoint == 'v') { command = SvgPathDataCmd_Vertical;        }
			else if (nextLowerCodepoint == 'l') { command = SvgPathDataCmd_Line;            }
			else if (nextLowerCodepoint == 'q') { command = SvgPathDataCmd_Quadratic;       }
			else if (nextLowerCodepoint == 'c') { command = SvgPathDataCmd_Cubic;           }
			else if (nextLowerCodepoint == 't') { command = SvgPathDataCmd_SmoothQuadratic; }
			else if (nextLowerCodepoint == 's') { command = SvgPathDataCmd_SmoothCubic;     }
			else if (nextLowerCodepoint == 'a') { command = SvgPathDataCmd_Ellipse;         }
			else if (nextLowerCodepoint == 'z')
			{
				//TODO: Does relativity matter for the z command?
				if (!Vec2BasicallyEqual(currentPos, startPos))
				{
					command = SvgPathDataCmd_ClosePath;
					newPathPart.type = BezierPathPartType_Line;
					newPathPart.endPos = startPos;
					if (isDetached)
					{
						newPathPart.detached = true;
						newPathPart.startPos = currentPos;
					}
					currentPos = newPathPart.endPos;
					isDetached = false;
				}
			}
			else
			{
				LogPrintLine_E(log, "Unhandled path data command at byte %llu 0x%08X \'%c\'", cIndex, nextCodepoint, (char)nextCodepoint);
				return false;
			}
			cIndex += codepointByteSize;
		}
		else
		{
			if (command == SvgPathDataCmd_None)
			{
				LogPrintLine_E(log, "Found numbers before first command char in path data byte %llu", cIndex);
				return false;
			}
			else if (command == SvgPathDataCmd_ClosePath)
			{
				LogPrintLine_E(log, "Found numbers after close path command in path data byte %llu: 0x%08X \'%c\'", cIndex, nextCodepoint, (char)nextCodepoint);
				return false;
			}
			
			u64 nextNonNumberIndex = 0;
			bool foundNonNumber = FindNextUnknownCharInStr(dataStr, cIndex, "0123456789.-+,e", &nextNonNumberIndex);
			if (!foundNonNumber)
			{
				nextNonNumberIndex = dataStr.length;
			}
			MyStr_t numbersStr = NewStr(nextNonNumberIndex - cIndex, &dataStr.pntr[cIndex]);
			
			if (command == SvgPathDataCmd_Horizontal || command == SvgPathDataCmd_Vertical || (command == SvgPathDataCmd_Ellipse && numParsedPoints >= 1 && numParsedPoints <= 3))
			{
				r32 parsedValue = 0;
				if (!TryParseR32(numbersStr, &parsedValue, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Couldn't parse value as r32 in path number starting at byte %llu: \"%.*s\"", cIndex, numbersStr.length, numbersStr.pntr);
					return false;
				}
				
				// +==============================+
				// |  SvgPathDataCmd_Horizontal   |
				// +==============================+
				if (command == SvgPathDataCmd_Horizontal)
				{
					newPathPart.type = BezierPathPartType_Line;
					newPathPart.endPos = NewVec2((commandIsRelative ? currentPos.x : 0) + parsedValue, currentPos.y);
					if (isDetached)
					{
						newPathPart.detached = true;
						newPathPart.startPos = currentPos;
					}
					currentPos = newPathPart.endPos;
					isDetached = false;
				}
				// +==============================+
				// |   SvgPathDataCmd_Vertical    |
				// +==============================+
				else if (command == SvgPathDataCmd_Vertical)
				{
					newPathPart.type = BezierPathPartType_Line;
					newPathPart.endPos = NewVec2(currentPos.x, (commandIsRelative ? currentPos.y : 0) + parsedValue);
					if (isDetached)
					{
						newPathPart.detached = true;
						newPathPart.startPos = currentPos;
					}
					currentPos = newPathPart.endPos;
					isDetached = false;
				}
				// +==============================+
				// |    SvgPathDataCmd_Ellipse    |
				// +==============================+
				else if (command == SvgPathDataCmd_Ellipse)
				{
					//TODO: Should we make sure numbers 2 and 3 are either 0 or 1 because they are flags?
					parsedPoints[numParsedPoints] = NewVec2(parsedValue, 0);
					numParsedPoints++;
				}
				else { DebugAssert(false); }
			}
			else
			{
				u64 commaIndex = 0;
				bool foundComma = FindNextCharInStr(numbersStr, 0, ",", &commaIndex);
				if (!foundComma)
				{
					LogPrintLine_E(log, "Expected vector 2 for %s command but couldn't find comma in next number piece at byte %llu: \"%.*s\"", GetSvgPathDataCmdStr(command), cIndex, numbersStr.length, numbersStr.pntr);
					return false;
				}
				MyStr_t xStr = StrSubstring(&numbersStr, 0, commaIndex);
				MyStr_t yStr = StrSubstring(&numbersStr, commaIndex+1, numbersStr.length);
				
				v2 parsedValue = Vec2_Zero;
				if (!TryParseR32(xStr, &parsedValue.x, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Couldn't parse x value as r32 in path vec2 starting at byte %llu: \"%.*s\"", cIndex, numbersStr.length, numbersStr.pntr);
					return false;
				}
				if (!TryParseR32(yStr, &parsedValue.y, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Couldn't parse y value as r32 in path vec2 starting at byte %llu: \"%.*s\"", cIndex, numbersStr.length, numbersStr.pntr);
					return false;
				}
				
				// +==============================+
				// |    SvgPathDataCmd_MoveTo     |
				// +==============================+
				if (command == SvgPathDataCmd_MoveTo)
				{
					currentPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
					startPos = currentPos;
					isDetached = true;
					command = SvgPathDataCmd_Line;
				}
				// +==============================+
				// |     SvgPathDataCmd_Line      |
				// +==============================+
				else if (command == SvgPathDataCmd_Line)
				{
					newPathPart.type = BezierPathPartType_Line;
					newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
					if (isDetached)
					{
						newPathPart.detached = true;
						newPathPart.startPos = currentPos;
					}
					currentPos = newPathPart.endPos;
					isDetached = false;
				}
				// +==============================+
				// |   SvgPathDataCmd_Quadratic   |
				// +==============================+
				else if (command == SvgPathDataCmd_Quadratic)
				{
					if (numParsedPoints == 1)
					{
						newPathPart.type = BezierPathPartType_Curve3;
						newPathPart.control1 = (commandIsRelative ? currentPos : Vec2_Zero) + parsedPoints[0];
						newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
						if (isDetached)
						{
							newPathPart.detached = true;
							newPathPart.startPos = currentPos;
						}
						currentPos = newPathPart.endPos;
						isDetached = false;
						numParsedPoints = 0;
					}
					else
					{
						parsedPoints[numParsedPoints] = parsedValue;
						numParsedPoints++;
					}
				}
				// +================================+
				// | SvgPathDataCmd_SmoothQuadratic |
				// +================================+
				else if (command == SvgPathDataCmd_SmoothQuadratic)
				{
					v2 previousControlPoint = currentPos;
					if (prevPathPart != nullptr && prevPathPart->type == BezierPathPartType_Curve3)
					{
						previousControlPoint = prevPathPart->control1;
					}
					else if (prevPathPart != nullptr && prevPathPart->type == BezierPathPartType_Curve4)
					{
						previousControlPoint = prevPathPart->control2;
					}
					newPathPart.type = BezierPathPartType_Curve3;
					newPathPart.control1 = currentPos + (currentPos - previousControlPoint);
					newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
					if (isDetached)
					{
						newPathPart.detached = true;
						newPathPart.startPos = currentPos;
					}
					currentPos = newPathPart.endPos;
					isDetached = false;
					numParsedPoints = 0;
				}
				// +==============================+
				// |     SvgPathDataCmd_Cubic     |
				// +==============================+
				else if (command == SvgPathDataCmd_Cubic)
				{
					if (numParsedPoints == 2)
					{
						newPathPart.type = BezierPathPartType_Curve4;
						newPathPart.control1 = (commandIsRelative ? currentPos : Vec2_Zero) + parsedPoints[0];
						newPathPart.control2 = (commandIsRelative ? currentPos : Vec2_Zero) + parsedPoints[1];
						newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
						if (isDetached)
						{
							newPathPart.detached = true;
							newPathPart.startPos = currentPos;
						}
						currentPos = newPathPart.endPos;
						isDetached = false;
						numParsedPoints = 0;
					}
					else
					{
						parsedPoints[numParsedPoints] = parsedValue;
						numParsedPoints++;
					}
				}
				// +==============================+
				// |  SvgPathDataCmd_SmoothCubic  |
				// +==============================+
				else if (command == SvgPathDataCmd_SmoothCubic)
				{
					if (numParsedPoints == 1)
					{
						v2 previousControlPoint = currentPos;
						if (prevPathPart != nullptr && prevPathPart->type == BezierPathPartType_Curve3)
						{
							previousControlPoint = prevPathPart->control1;
						}
						else if (prevPathPart != nullptr && prevPathPart->type == BezierPathPartType_Curve4)
						{
							previousControlPoint = prevPathPart->control2;
						}
						newPathPart.type = BezierPathPartType_Curve4;
						newPathPart.control1 = currentPos + (currentPos - previousControlPoint);
						newPathPart.control2 = (commandIsRelative ? currentPos : Vec2_Zero) + parsedPoints[0];
						newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
						if (isDetached)
						{
							newPathPart.detached = true;
							newPathPart.startPos = currentPos;
						}
						currentPos = newPathPart.endPos;
						isDetached = false;
						numParsedPoints = 0;
					}
					else
					{
						parsedPoints[numParsedPoints] = parsedValue;
						numParsedPoints++;
					}
				}
				// +==============================+
				// |    SvgPathDataCmd_Ellipse    |
				// +==============================+
				else if (command == SvgPathDataCmd_Ellipse)
				{
					if (numParsedPoints == 4)
					{
						newPathPart.type = BezierPathPartType_EllipseArc;
						newPathPart.radius = parsedPoints[0];
						newPathPart.axisAngle = parsedPoints[1].x;
						newPathPart.arcFlags = BezierPathPartArcFlag_None;
						FlagSetTo(newPathPart.arcFlags, BezierPathPartArcFlag_Large, (parsedPoints[2].x > 0));
						FlagSetTo(newPathPart.arcFlags, BezierPathPartArcFlag_Sweep, (parsedPoints[3].x > 0));
						newPathPart.endPos = (commandIsRelative ? currentPos : Vec2_Zero) + parsedValue;
						if (isDetached)
						{
							newPathPart.detached = true;
							newPathPart.startPos = currentPos;
						}
						currentPos = newPathPart.endPos;
						isDetached = false;
						numParsedPoints = 0;
					}
					else
					{
						parsedPoints[numParsedPoints] = parsedValue;
						numParsedPoints++;
					}
				}
				else { DebugAssert(false); }
			}
			
			cIndex = nextNonNumberIndex;
		}
		
		if (newPathPart.type != BezierPathPartType_None)
		{
			BezierPathPart_t* newPartPntr = VarArrayAdd(&pathOut->parts, BezierPathPart_t);
			NotNull(newPartPntr);
			ClearPointer(newPartPntr);
			MyMemCopy(newPartPntr, &newPathPart, sizeof(BezierPathPart_t));
			prevPathPart = newPartPntr;
		}
		DebugAssertAndUnused(cIndex > startOfLoopIndex, startOfLoopIndex);
	}
	
	if (Vec2BasicallyEqual(currentPos, startPos))
	{
		pathOut->isClosedLoop = true;
	}
	
	return true;
}

v2 ApplySvgTransformToPosition(const SvgTransform_t* transform, v2 position)
{
	NotNull(transform);
	switch (transform->type)
	{
		case SvgTransformType_None: return position;
		case SvgTransformType_Scale:
		{
			return position * transform->scale;
		} break;
		case SvgTransformType_Rotate:
		{
			return Vec2Rotate(position, transform->rotation);
		} break;
		case SvgTransformType_Translate:
		{
			return position + transform->vector;
		} break;
		case SvgTransformType_Matrix:
		{
			return Mat4MultiplyVec2(transform->matrix, position, true);
		} break;
		default: Unimplemented(); return position;
	}
}
v2 ApplySvgTransformToVector(const SvgTransform_t* transform, v2 vector) //no translation
{
	NotNull(transform);
	switch (transform->type)
	{
		case SvgTransformType_None: return vector;
		case SvgTransformType_Scale:
		{
			return vector * transform->scale;
		} break;
		case SvgTransformType_Rotate:
		{
			return Vec2Rotate(vector, transform->rotation);
		} break;
		case SvgTransformType_Translate:
		{
			return vector;
		} break;
		case SvgTransformType_Matrix:
		{
			return Mat4MultiplyVec2(transform->matrix, vector, false);
		} break;
		default: Unimplemented(); return vector;
	}
}
v2 ApplySvgTransformToScalars(const SvgTransform_t* transform, v2 scalars) //no translation or rotation
{
	NotNull(transform);
	switch (transform->type)
	{
		case SvgTransformType_None: return scalars;
		case SvgTransformType_Scale:
		{
			return scalars * transform->scale;
		} break;
		case SvgTransformType_Rotate:
		{
			return scalars;
		} break;
		case SvgTransformType_Translate:
		{
			return scalars;
		} break;
		case SvgTransformType_Matrix:
		{
			r32 resultX = Vec2Length(Mat4MultiplyVec2(transform->matrix, NewVec2(scalars.x, 0), false));
			r32 resultY = Vec2Length(Mat4MultiplyVec2(transform->matrix, NewVec2(0, scalars.y), false));
			return NewVec2(resultX, resultY);
		} break;
		default: Unimplemented(); return scalars;
	}
}
void ApplyTransformToSvgShape(SvgShape_t* shape, const SvgTransform_t* transform)
{
	NotNull(shape);
	NotNull(transform);
	shape->stroke.thickness = ApplySvgTransformToScalars(transform, NewVec2(shape->stroke.thickness, 0)).x;
	switch (shape->type)
	{
		case SvgShapeType_Circle:
		{
			 shape->circle.center = ApplySvgTransformToPosition(transform, shape->circle.center);
			 shape->circle.radius = ApplySvgTransformToScalars(transform, NewVec2(shape->circle.radius, 0)).x;
		} break;
		case SvgShapeType_Rectangle:
		{
			 v2 newBaseVec1 = ApplySvgTransformToVector(transform, Vec2_Right);
			 v2 newBaseVec2 = ApplySvgTransformToVector(transform, Vec2_Down);
			 shape->rectangle.center = ApplySvgTransformToPosition(transform, shape->rectangle.center);
			 shape->rectangle.size.width = Vec2Length(newBaseVec1 * shape->rectangle.size.width);
			 shape->rectangle.size.height = Vec2Length(newBaseVec2 * shape->rectangle.size.height);
			 shape->rectangle.rotation = AngleFromVec2(ApplySvgTransformToVector(transform, Vec2FromAngle(shape->rectangle.rotation)));
			 shape->rectangle.roundedRadius = ApplySvgTransformToScalars(transform, shape->rectangle.roundedRadius);
		} break;
		case SvgShapeType_Path:
		{
			VarArrayLoop(&shape->path.value.parts, pIndex)
			{
				VarArrayLoopGet(BezierPathPart_t, pathPart, &shape->path.value.parts, pIndex);
				pathPart->startPos  = ApplySvgTransformToPosition(transform, pathPart->startPos);
				pathPart->control1  = ApplySvgTransformToPosition(transform, pathPart->control1);
				pathPart->control2  = ApplySvgTransformToPosition(transform, pathPart->control2);
				pathPart->endPos    = ApplySvgTransformToPosition(transform, pathPart->endPos);
				pathPart->radius    = ApplySvgTransformToScalars(transform, pathPart->radius);
				pathPart->axisAngle = AngleFromVec2(ApplySvgTransformToVector(transform, Vec2FromAngle(pathPart->axisAngle)));
			}
		} break;
		default: Unimplemented(); break;
	}
}
void ApplyTransformToSvgGroup(SvgGroup_t* group, const SvgTransform_t* transform)
{
	NotNull(group);
	NotNull(transform);
	//TODO: Seems we don't need to apply transforms to origins so maybe we should remove the SvgApplyParentGroupTransformsToGroup call altogether
	// if (group->origin != Vec2_Zero)
	// {
	// 	group->origin = ApplySvgTransformToPosition(transform, group->origin);
	// }
}

void SvgApplyParentGroupTransformsToShape(ProcessLog_t* log, SvgData_t* svgData, SvgGroup_t* currentGroup, SvgShape_t* shape)
{
	NotNull(log);
	NotNull(svgData);
	NotNull(shape);
	while (currentGroup != nullptr)
	{
		ApplyTransformToSvgShape(shape, &currentGroup->transform);
		if (currentGroup->parentIndex < svgData->groups.length)
		{
			currentGroup = VarArrayGetHard(&svgData->groups, currentGroup->parentIndex, SvgGroup_t);
		}
		else { currentGroup = nullptr; }
	}
}
void SvgApplyParentGroupTransformsToGroup(ProcessLog_t* log, SvgData_t* svgData, SvgGroup_t* group)
{
	NotNull(log);
	NotNull(svgData);
	SvgGroup_t* currentGroup = group;
	while (currentGroup != nullptr)
	{
		ApplyTransformToSvgGroup(group, &currentGroup->transform);
		if (currentGroup->parentIndex < svgData->groups.length)
		{
			currentGroup = VarArrayGetHard(&svgData->groups, currentGroup->parentIndex, SvgGroup_t);
		}
		else { currentGroup = nullptr; }
	}
}

// +--------------------------------------------------------------+
// |                       Deserialization                        |
// +--------------------------------------------------------------+
bool TryDeserSvgFile(MyStr_t fileContents, ProcessLog_t* log, SvgData_t* dataOut, MemArena_t* memArena)
{
	NotNull(log);
	NotNull(dataOut);
	NotNull(memArena);
	
	LogWriteLine_N(log, "Entering TryDeserSvgFile...");
	SetProcessLogName(log, NewStr("DeserSvgFile"));
	
	if (fileContents.length == 0)
	{
		LogWriteLine_E(log, "Empty file is a not a valid SVG file");
		LogExitFailure(log, TryDeserSvgFileError_EmptyFile);
		return false;
	}
	
	ClearPointer(dataOut);
	dataOut->allocArena = memArena;
	CreateVarArray(&dataOut->groups, dataOut->allocArena, sizeof(SvgGroup_t));
	
	bool foundSvgToken = false;
	SvgGroup_t* currentGroup = nullptr;
	SvgTransform_t currentGroupTransform = {};
	
	XmlParser_t xmlParser = NewXmlParser(mainHeap, fileContents);
	XmlParseResult_t parse = {};
	while (XmlParserGetToken(&xmlParser, &parse, log))
	{
		switch (parse.type)
		{
			// +==============================+
			// |   XmlParseResultType_Token   |
			// +==============================+
			case XmlParseResultType_Token:
			{
				XmlToken_t* token = &parse.token;
				
				// +==============================+
				// |             svg              |
				// +==============================+
				if (StrEquals(token->type, "svg"))
				{
					foundSvgToken = true;
					//TODO: Any properties we'd like the parse?
					//      "width", "height", "viewBox", "id", "version"
				}
				// +==============================+
				// |              g               |
				// +==============================+
				else if (StrEquals(token->type, "g"))
				{
					if (!foundSvgToken)
					{
						LogPrintLine_E(log, "Found group before svg token on line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_GroupOutsideSvg);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					currentGroup = VarArrayAdd(&dataOut->groups, SvgGroup_t);
					NotNull(currentGroup);
					ClearPointer(currentGroup);
					currentGroup->tokenParseIndex = token->tokenParseIndex;
					currentGroup->parentIndex = UINT64_MAX;
					currentGroup->origin = Vec2_Zero;
					CreateVarArray(&currentGroup->shapes, dataOut->allocArena, sizeof(SvgShape_t));
					
					if (xmlParser.parentTokens.length > 0)
					{
						for (u64 parentIndex = xmlParser.parentTokens.length; parentIndex > 0; parentIndex--)
						{
							XmlToken_t* parent = VarArrayGetHard(&xmlParser.parentTokens, parentIndex-1, XmlToken_t);
							if (StrEquals(parent->type, "g"))
							{
								//lets see if we can find the parent group by tokenParseIndex
								bool foundParentGroup = false;
								VarArrayLoop(&dataOut->groups, gIndex)
								{
									VarArrayLoopGet(SvgGroup_t, parentGroup, &dataOut->groups, gIndex);
									if (parentGroup->tokenParseIndex == parent->tokenParseIndex)
									{
										currentGroup->parentIndex = gIndex;
										currentGroup->interleaveIndex = parentGroup->shapes.length;
										foundParentGroup = true;
										break;
									}
								}
								if (!foundParentGroup)
								{
									LogPrintLine_W(log, "Failed to find parent group with tokenParseIndex %llu for group on line %llu", parent->tokenParseIndex, xmlParser.lineParser.lineIndex);
								}
								break;
							}
						}
					}
					
					XmlProperty_t* idProperty = GetXmlProperty(token, NewStr("id"));
					if (idProperty != nullptr && idProperty->value.length > 0)
					{
						currentGroup->idStr = AllocString(dataOut->allocArena, &idProperty->value);
					}
					XmlProperty_t* labelProperty = GetXmlProperty(token, NewStr("inkscape:label"));
					if (labelProperty != nullptr && labelProperty->value.length > 0)
					{
						currentGroup->label = AllocString(dataOut->allocArena, &labelProperty->value);
					}
					XmlProperty_t* centerXProperty = GetXmlProperty(token, NewStr("inkscape:transform-center-x"));
					if (centerXProperty != nullptr && centerXProperty->value.length > 0)
					{
						r32 parsedValue = 0;
						if (TryParseR32(centerXProperty->value, &parsedValue, &log->parseFailureReason))
						{
							currentGroup->origin.x = parsedValue;
						}
						else
						{
							LogPrintLine_W(log, "Failed to parse transform-center-x property as r32 in group \"%.*s\" ID \"%.*s\" on line %llu: \"%.*s\"",
								currentGroup->label.length, currentGroup->label.pntr,
								currentGroup->idStr.length, currentGroup->idStr.pntr,
								xmlParser.lineParser.lineIndex,
								centerXProperty->value.length, centerXProperty->value.pntr
							);
						}
					}
					XmlProperty_t* centerYProperty = GetXmlProperty(token, NewStr("inkscape:transform-center-y"));
					if (centerYProperty != nullptr && centerYProperty->value.length > 0)
					{
						r32 parsedValue = 0;
						if (TryParseR32(centerYProperty->value, &parsedValue, &log->parseFailureReason))
						{
							currentGroup->origin.y = -parsedValue;
						}
						else
						{
							LogPrintLine_W(log, "Failed to parse transform-center-y property as r32 in group \"%.*s\" ID \"%.*s\" on line %llu: \"%.*s\"",
								currentGroup->label.length, currentGroup->label.pntr,
								currentGroup->idStr.length, currentGroup->idStr.pntr,
								xmlParser.lineParser.lineIndex,
								centerYProperty->value.length, centerYProperty->value.pntr
							);
						}
					}
					
					XmlProperty_t* transformProperty = GetXmlProperty(token, NewStr("transform"));
					if (transformProperty != nullptr)
					{
						if (!TryParseSvgTransformString(log, transformProperty->value, &currentGroup->transform))
						{
							LogPrintLine_E(log, "Failed to parse transform for group on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, transformProperty->value.length, transformProperty->value.pntr);
							LogExitFailure(log, TryDeserSvgFileError_GroupTransformIsInvalid);
							FreeXmlParser(&xmlParser);
							FreeSvgData(dataOut);
							return false;
						}
					}
					
					SvgApplyParentGroupTransformsToGroup(log, dataOut, currentGroup);
				}
				// +==============================+
				// |            circle            |
				// +==============================+
				else if (StrEquals(token->type, "circle"))
				{
					if (currentGroup == nullptr)
					{
						LogPrintLine_E(log, "Found circle shape outside of a group on line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_ShapeOutsideGroup);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					XmlProperty_t* styleProperty = GetXmlProperty(token, NewStr("style"));
					if (styleProperty == nullptr)
					{
						LogPrintLine_E(log, "Circle shape is missing \"style\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* cxProperty = GetXmlProperty(token, NewStr("cx"));
					if (cxProperty == nullptr)
					{
						LogPrintLine_E(log, "Circle shape is missing \"cx\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* cyProperty = GetXmlProperty(token, NewStr("cy"));
					if (cyProperty == nullptr)
					{
						LogPrintLine_E(log, "Circle shape is missing \"cy\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* rProperty = GetXmlProperty(token, NewStr("r"));
					if (rProperty == nullptr)
					{
						LogPrintLine_E(log, "Circle shape is missing \"r\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					SvgFill_t circleFill = {};
					SvgStroke_t circleStroke = {};
					if (!TryParseSvgShapeStyleString(log, styleProperty->value, &circleFill, &circleStroke))
					{
						LogExitFailure(log, TryDeserSvgFileError_StyleFormatError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					v2 circleCenter = Vec2_Zero;
					if (!TryParseR32(cxProperty->value, &circleCenter.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse circle center x value in circle on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, cxProperty->value.length, cxProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					if (!TryParseR32(cyProperty->value, &circleCenter.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse circle center y value in circle on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, cyProperty->value.length, cyProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					r32 circleRadius = 0.0f;
					if (!TryParseR32(rProperty->value, &circleRadius, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse circle radius value in circle on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, rProperty->value.length, rProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					SvgShape_t* newShape = VarArrayAdd(&currentGroup->shapes, SvgShape_t);
					NotNull(newShape);
					ClearPointer(newShape);
					newShape->type = SvgShapeType_Circle;
					newShape->circle.center = circleCenter;
					newShape->circle.radius = circleRadius;
					newShape->fill = circleFill;
					newShape->stroke = circleStroke;
					
					XmlProperty_t* idProperty = GetXmlProperty(token, NewStr("id"));
					if (idProperty != nullptr && idProperty->value.length > 0)
					{
						newShape->idStr = AllocString(dataOut->allocArena, &idProperty->value);
					}
					XmlProperty_t* labelProperty = GetXmlProperty(token, NewStr("inkscape:label"));
					if (labelProperty != nullptr && labelProperty->value.length > 0)
					{
						newShape->label = AllocString(dataOut->allocArena, &labelProperty->value);
					}
					
					XmlProperty_t* transformProperty = GetXmlProperty(token, NewStr("transform"));
					if (transformProperty != nullptr && transformProperty->value.length > 0)
					{
						if (!TryParseSvgTransformString(log, transformProperty->value, &newShape->transform))
						{
							LogPrintLine_E(log, "Couldn't parse circle transform on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, transformProperty->value.length, transformProperty->value.pntr);
							LogExitFailure(log, TryDeserSvgFileError_TryParseError);
							FreeXmlParser(&xmlParser);
							FreeSvgData(dataOut);
							return false;
						}
						ApplyTransformToSvgShape(newShape, &newShape->transform);
					}
					
					SvgApplyParentGroupTransformsToShape(log, dataOut, currentGroup, newShape);
				}
				// +==============================+
				// |             rect             |
				// +==============================+
				else if (StrEquals(token->type, "rect"))
				{
					if (currentGroup == nullptr)
					{
						LogPrintLine_E(log, "Found rectangle shape outside of a group on line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_ShapeOutsideGroup);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					XmlProperty_t* styleProperty = GetXmlProperty(token, NewStr("style"));
					if (styleProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"style\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* xProperty = GetXmlProperty(token, NewStr("x"));
					if (xProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"x\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* yProperty = GetXmlProperty(token, NewStr("y"));
					if (yProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"y\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* widthProperty = GetXmlProperty(token, NewStr("width"));
					if (widthProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"width\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* heightProperty = GetXmlProperty(token, NewStr("height"));
					if (heightProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"height\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* rxProperty = GetXmlProperty(token, NewStr("rx"));
					XmlProperty_t* ryProperty = GetXmlProperty(token, NewStr("ry"));
					
					SvgFill_t rectangleFill = {};
					SvgStroke_t rectangleStroke = {};
					if (!TryParseSvgShapeStyleString(log, styleProperty->value, &rectangleFill, &rectangleStroke))
					{
						LogExitFailure(log, TryDeserSvgFileError_StyleFormatError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					rec rectangleValue = Rec_Zero;
					if (!TryParseR32(xProperty->value, &rectangleValue.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle x value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, xProperty->value.length, xProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					if (!TryParseR32(yProperty->value, &rectangleValue.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle y value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, yProperty->value.length, yProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					if (!TryParseR32(widthProperty->value, &rectangleValue.width, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle width value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, widthProperty->value.length, widthProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					if (!TryParseR32(heightProperty->value, &rectangleValue.height, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle height value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, heightProperty->value.length, heightProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					v2 roundedRadius = Vec2_Zero;
					if (rxProperty != nullptr && !TryParseR32(rxProperty->value, &roundedRadius.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle rx value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, rxProperty->value.length, rxProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					if (ryProperty != nullptr && !TryParseR32(ryProperty->value, &roundedRadius.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse rectangle ry value on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, ryProperty->value.length, ryProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_TryParseError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					SvgShape_t* newShape = VarArrayAdd(&currentGroup->shapes, SvgShape_t);
					NotNull(newShape);
					ClearPointer(newShape);
					newShape->type = SvgShapeType_Rectangle;
					newShape->rectangle.value = ToObb2D(rectangleValue);
					newShape->rectangle.roundedRadius = roundedRadius;
					newShape->fill = rectangleFill;
					newShape->stroke = rectangleStroke;
					
					XmlProperty_t* idProperty = GetXmlProperty(token, NewStr("id"));
					if (idProperty != nullptr && idProperty->value.length > 0)
					{
						newShape->idStr = AllocString(dataOut->allocArena, &idProperty->value);
					}
					XmlProperty_t* labelProperty = GetXmlProperty(token, NewStr("inkscape:label"));
					if (labelProperty != nullptr && labelProperty->value.length > 0)
					{
						newShape->label = AllocString(dataOut->allocArena, &labelProperty->value);
					}
					
					XmlProperty_t* transformProperty = GetXmlProperty(token, NewStr("transform"));
					if (transformProperty != nullptr && transformProperty->value.length > 0)
					{
						if (!TryParseSvgTransformString(log, transformProperty->value, &newShape->transform))
						{
							LogPrintLine_E(log, "Couldn't parse rectangle transform on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, transformProperty->value.length, transformProperty->value.pntr);
							LogExitFailure(log, TryDeserSvgFileError_TryParseError);
							FreeXmlParser(&xmlParser);
							FreeSvgData(dataOut);
							return false;
						}
						ApplyTransformToSvgShape(newShape, &newShape->transform);
					}
					
					SvgApplyParentGroupTransformsToShape(log, dataOut, currentGroup, newShape);
				}
				// +==============================+
				// |             path             |
				// +==============================+
				else if (StrEquals(token->type, "path"))
				{
					if (currentGroup == nullptr)
					{
						LogPrintLine_E(log, "Found path shape outside of a group on line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_ShapeOutsideGroup);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					XmlProperty_t* styleProperty = GetXmlProperty(token, NewStr("style"));
					if (styleProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"style\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					XmlProperty_t* dataProperty = GetXmlProperty(token, NewStr("d"));
					if (dataProperty == nullptr)
					{
						LogPrintLine_E(log, "Rectangle shape is missing \"d\" property line %llu", xmlParser.lineParser.lineIndex);
						LogExitFailure(log, TryDeserSvgFileError_MissingProperty);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					SvgFill_t pathFill = {};
					SvgStroke_t pathStroke = {};
					if (!TryParseSvgShapeStyleString(log, styleProperty->value, &pathFill, &pathStroke))
					{
						LogExitFailure(log, TryDeserSvgFileError_StyleFormatError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					
					SvgShape_t* newShape = VarArrayAdd(&currentGroup->shapes, SvgShape_t);
					NotNull(newShape);
					ClearPointer(newShape);
					newShape->type = SvgShapeType_Path;
					newShape->fill = pathFill;
					newShape->stroke = pathStroke;
					CreateBezierPath(&newShape->path.value, dataOut->allocArena);
					
					XmlProperty_t* idProperty = GetXmlProperty(token, NewStr("id"));
					if (idProperty != nullptr && idProperty->value.length > 0)
					{
						newShape->idStr = AllocString(dataOut->allocArena, &idProperty->value);
					}
					XmlProperty_t* labelProperty = GetXmlProperty(token, NewStr("inkscape:label"));
					if (labelProperty != nullptr && labelProperty->value.length > 0)
					{
						newShape->label = AllocString(dataOut->allocArena, &labelProperty->value);
					}
					
					if (!TryParseSvgPathDataString(log, dataProperty->value, &newShape->path.value))
					{
						LogPrintLine_E(log, "Failed to parse path data on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, dataProperty->value.length, dataProperty->value.pntr);
						LogExitFailure(log, TryDeserSvgFileError_PathFormatError);
						FreeXmlParser(&xmlParser);
						FreeSvgData(dataOut);
						return false;
					}
					DebugAssert(newShape->path.value.parts.length > 0);
					
					XmlProperty_t* transformProperty = GetXmlProperty(token, NewStr("transform"));
					if (transformProperty != nullptr && transformProperty->value.length > 0)
					{
						if (!TryParseSvgTransformString(log, transformProperty->value, &newShape->transform))
						{
							LogPrintLine_E(log, "Couldn't parse rectangle transform on line %llu: \"%.*s\"", xmlParser.lineParser.lineIndex, transformProperty->value.length, transformProperty->value.pntr);
							LogExitFailure(log, TryDeserSvgFileError_TryParseError);
							FreeXmlParser(&xmlParser);
							FreeSvgData(dataOut);
							return false;
						}
						ApplyTransformToSvgShape(newShape, &newShape->transform);
					}
					
					SvgApplyParentGroupTransformsToShape(log, dataOut, currentGroup, newShape);
				}
				else
				{
					// LogPrintLine_I(log, "Parsed unknown token \"%.*s\" with %llu propert%s (and %llu parent%s):", token->type.length, token->type.pntr, token->properties.length, (token->properties.length == 1) ? "y" : "ies", xmlParser.parentTokens.length, (xmlParser.parentTokens.length == 1) ? "" : "s");
					// VarArrayLoop(&token->properties, pIndex)
					// {
					// 	VarArrayLoopGet(XmlProperty_t, property, &token->properties, pIndex);
					// 	LogPrintLine_D(log, "\tProperty[%llu]: \"%.*s\"", pIndex, property->key.length, property->key.pntr);
					// }
				}
			} break;
			
			// +==============================+
			// | XmlParseResultType_EndToken  |
			// +==============================+
			case XmlParseResultType_EndToken:
			{
				if (StrEquals(parse.token.type, "g"))
				{
					if (currentGroup->parentIndex < dataOut->groups.length)
					{
						currentGroup = VarArrayGetHard(&dataOut->groups, currentGroup->parentIndex, SvgGroup_t);
					}
					else
					{
						currentGroup = nullptr;
					}
				}
				else
				{
					//TODO: Any other token types we care about when they close?
				}
			} break;
			
			// +==============================+
			// | XmlParseResultType_Contents  |
			// +==============================+
			case XmlParseResultType_Contents:
			{
				// LogPrintLine_O(log, "Got contents \"%.*s\"", parse.string.length, parse.string.pntr);
			} break;
			
			// +==============================+
			// |   XmlParseResultType_Error   |
			// +==============================+
			case XmlParseResultType_Error:
			{
				log->xmlParsingError = parse.error;
				LogExitFailure(log, TryDeserSvgFileError_XmlParsingError);
				FreeXmlParser(&xmlParser);
				FreeSvgData(dataOut);
				return false;
			} break;
			
			// +==============================+
			// | Unhandled XmlParseResultType |
			// +==============================+
			default:
			{
				Unimplemented(); //TODO: Implement me!
			} break;
		}
	}
	
	FreeXmlParser(&xmlParser);
	LogExitSuccess(log);
	return true;
}

bool GenerateVertBufferForSvgPathShape(SvgShape_t* shape, u64 numCurveVertices, v2** debugVerticesOut = nullptr, u64* debugNumVerticesOut = nullptr)
{
	NotNull(shape);
	Assert(shape->type == SvgShapeType_Path);
	Assert(numCurveVertices > 0);
	
	u64 numPositionsNeeded = 0;
	u64 numSubParts = 0;
	VarArrayLoop(&shape->path.value.parts, pIndex)
	{
		VarArrayLoopGet(BezierPathPart_t, pathPart, &shape->path.value.parts, pIndex);
		if (pathPart->detached)
		{
			numPositionsNeeded++;
			numSubParts++;
		}
		if (pathPart->type == BezierPathPartType_Curve3 ||
			pathPart->type == BezierPathPartType_Curve4 ||
			pathPart->type == BezierPathPartType_EllipseArc)
		{
			numPositionsNeeded += numCurveVertices;
		}
		else if (pathPart->type == BezierPathPartType_Line)
		{
			numPositionsNeeded += 1;
		}
		else { Assert(false); }
		if (pIndex == shape->path.value.parts.length-1 && shape->path.value.isClosedLoop) { numPositionsNeeded--; }
	}
	Assert(numSubParts > 0);
	Assert(numPositionsNeeded > 0);
	
	TempPushMark();
	v2* vertPositions = AllocArray(TempArena, v2, numPositionsNeeded);
	NotNull(vertPositions);
	u64* subPartStartIndices = AllocArray(TempArena, u64, numSubParts);
	NotNull(subPartStartIndices);
	u64* subPartVertCounts = AllocArray(TempArena, u64, numSubParts);
	NotNull(subPartVertCounts);
	
	// PrintLine_D("Generating %llu vert%s", numPositionsNeeded, ((numPositionsNeeded == 1) ? "ex" : "icies"));
	
	u64 vpIndex = 0;
	u64 subPartIndex = 0;
	v2 currentPos = Vec2_Zero;
	VarArrayLoop(&shape->path.value.parts, pIndex)
	{
		VarArrayLoopGet(BezierPathPart_t, pathPart, &shape->path.value.parts, pIndex);
		if (pathPart->detached)
		{
		}
		bool addLastVertex = !(pIndex == shape->path.value.parts.length-1 && shape->path.value.isClosedLoop);
		if (pathPart->detached)
		{
			Assert(subPartIndex < numSubParts);
			subPartStartIndices[subPartIndex] = vpIndex;
			subPartVertCounts[subPartIndex] = 0;
			subPartIndex++;
			currentPos = pathPart->startPos;
			
			Assert(subPartIndex > 0);
			Assert(vpIndex < numPositionsNeeded);
			vertPositions[vpIndex] = (pathPart->detached ? pathPart->startPos : currentPos);
			vpIndex++;
			subPartVertCounts[subPartIndex-1]++;
		}
		if (pathPart->type == BezierPathPartType_Curve3)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = BezierCurve3(currentPos, pathPart->control1, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_Curve4)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = BezierCurve4(currentPos, pathPart->control1, pathPart->control2, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_EllipseArc)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = EllipseArcCurveStartEnd(currentPos, pathPart->radius, pathPart->axisAngle, pathPart->arcFlags, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_Line)
		{
			if (addLastVertex)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				vertPositions[vpIndex] = pathPart->endPos;
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else { Assert(false); }
		currentPos = pathPart->endPos;
	}
	Assert(vpIndex == numPositionsNeeded);
	Assert(subPartIndex == numSubParts);
	
	if (debugVerticesOut != nullptr)
	{
		*debugVerticesOut = AllocArray(mainHeap, v2, numPositionsNeeded);
		MyMemCopy(*debugVerticesOut, vertPositions, sizeof(v2) * numPositionsNeeded);
		*debugNumVerticesOut = numPositionsNeeded;
	}
	
	u64 numTotalIndicesNeeded = 0;
	for (u64 subIndex = 0; subIndex < numSubParts; subIndex++)
	{
		u64 subPartStartIndex = subPartStartIndices[subIndex];
		u64 subPartVertCount = subPartVertCounts[subIndex];
		Assert(subPartStartIndex+subPartVertCount <= numPositionsNeeded);
		v2* subPartVertices = &vertPositions[subPartStartIndex];
		u64 triangulationNumIndices = 0;
		Triangulate2DEarClip(nullptr, nullptr, subPartVertCount, subPartVertices, &triangulationNumIndices);
		if (triangulationNumIndices == 0)
		{
			WriteLine_E("Triangulate2DEarClip failed! (early)");
			TempPopMark();
			return false;
		}
		numTotalIndicesNeeded += triangulationNumIndices;
	}
	Assert(numTotalIndicesNeeded > 0);
	
	u64* allIndices = AllocArray(TempArena, u64, numTotalIndicesNeeded);
	NotNull(allIndices);
	
	u64 allIndicesIndex = 0;
	for (u64 subIndex = 0; subIndex < numSubParts; subIndex++)
	{
		TempPushMark();
		u64 subPartStartIndex = subPartStartIndices[subIndex];
		u64 subPartVertCount = subPartVertCounts[subIndex];
		Assert(subPartStartIndex+subPartVertCount <= numPositionsNeeded);
		v2* subPartVertices = &vertPositions[subPartStartIndex];
		u64 triangulationNumIndices = 0;
		u64* triangulationIndices = Triangulate2DEarClip(TempArena, TempArena, subPartVertCount, subPartVertices, &triangulationNumIndices);
		if (triangulationIndices == nullptr)
		{
			//could be that the path is defined in counter-clockwise order. Let's try reversing the vertPositions and retrying
			WriteLine_D("Triangulation failed. Trying reversed vertex array...");
			for (u64 reverseIndex = 0; reverseIndex < subPartVertCount/2; reverseIndex++)
			{
				v2* vertLeft = &subPartVertices[reverseIndex];
				v2* vertRight = &subPartVertices[subPartVertCount-1 - reverseIndex];
				SWAP_POINTERS(v2, vertLeft, vertRight);
			}
			triangulationIndices = Triangulate2DEarClip(TempArena, TempArena, subPartVertCount, subPartVertices, &triangulationNumIndices);
		}
		if (triangulationIndices == nullptr)
		{
			WriteLine_E("Triangulate2DEarClip failed!");
			TempPopMark();
			TempPopMark();
			return false;
		}
		Assert(allIndicesIndex + triangulationNumIndices <= numTotalIndicesNeeded);
		for (u64 triIndicesIndex = 0; triIndicesIndex < triangulationNumIndices; triIndicesIndex++) { triangulationIndices[triIndicesIndex] += subPartStartIndex; }
		MyMemCopy(&allIndices[allIndicesIndex], triangulationIndices, sizeof(u64) * triangulationNumIndices);
		allIndicesIndex += triangulationNumIndices;
		TempPopMark();
	}
	Assert(allIndicesIndex == numTotalIndicesNeeded);
	
	Assert((numTotalIndicesNeeded % 3) == 0);
	Vertex2D_t* vertices = AllocArray(TempArena, Vertex2D_t, numTotalIndicesNeeded);
	NotNull(vertices);
	for (u64 iIndex = 0; iIndex < numTotalIndicesNeeded; iIndex++)
	{
		u64 vertPosIndex = allIndices[iIndex + 0];
		Assert(vertPosIndex < numPositionsNeeded);
		vertices[iIndex].position = NewVec3(vertPositions[vertPosIndex].x, vertPositions[vertPosIndex].y, 0);
		vertices[iIndex].color = Vec4_One;
		vertices[iIndex].texCoord = Vec2_Zero; //TODO: Implement me!
	}
	
	DestroyVertBuffer(&shape->path.vertBuffer);
	shape->path.vertBufferUpToDate = false;
	
	if (!CreateVertBuffer2D(mainHeap, &shape->path.vertBuffer, false, numTotalIndicesNeeded, vertices, false))
	{
		WriteLine_E("CreateVertBuffer2D failed!");
		TempPopMark();
		return false;
	}
	shape->path.vertBufferUpToDate = true;
	
	TempPopMark();
	
	return true;
}
