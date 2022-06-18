/*
File:   pig_font_flow.cpp
Author: Taylor Robbins
Date:   12\22\2021
Description: 
	** Holds the main functions that help us place characters on screen in a legible pattern.
	** For loading functions for Font_t structures, look in pig_font.cpp
*/

#define BOLD_TOGGLE_CHAR      '\b'
#define ITALIC_TOGGLE_CHAR    '\a'
#define UNDERLINE_TOGGLE_CHAR '\f'

// +--------------------------------------------------------------+
// |                      FontFlow Functions                      |
// +--------------------------------------------------------------+
const FontCharInfo_t* FontFlow_FindCharInfo(FontFlowState_t* flowState, u32 codepoint)
{
	const FontCharInfo_t* result = nullptr;
	if (flowState->selectedFaceCache != nullptr)
	{
		if (flowState->selectedBakeCache != nullptr)
		{
			result = GetFontCharInfoInBake(flowState->font, flowState->selectedFaceCache, flowState->selectedBakeCache, codepoint);
		}
		else
		{
			flowState->selectedBakeCache = GetFontBakeForChar(flowState->font, flowState->selectedFaceCache, codepoint, &result);
		}
	}
	if (result == nullptr)
	{
		//either the cached face/bake didn't have the char we want or we didn't have a cached face/bake
		flowState->selectedFaceCache = GetFontFace(flowState->font, flowState->faceSelector);
		if (flowState->selectedFaceCache != nullptr)
		{
			flowState->selectedBakeCache = GetFontBakeForChar(flowState->font, flowState->selectedFaceCache, codepoint, &result);
		}
	}
	if (result == nullptr && (flowState->faceSelector.bold || flowState->faceSelector.italic))
	{
		//Alright, still having trouble finding a character. Let's try asking for the face at the correct size but without bold and italic
		FontFaceSelector_t baseSelector = {};
		baseSelector.selectDefault = flowState->faceSelector.selectDefault;
		baseSelector.fontSize = flowState->faceSelector.fontSize;
		baseSelector.bold = false;
		baseSelector.italic = false;
		flowState->selectedFaceCache = GetFontFace(flowState->font, baseSelector);
		if (flowState->selectedFaceCache != nullptr)
		{
			flowState->selectedBakeCache = GetFontBakeForChar(flowState->font, flowState->selectedFaceCache, codepoint, &result);
		}
	}
	return result;
}

//TODO: Something is weird with the way the words wrap when the maxWidth is less than a single character width
//Returns num bytes to where we should either artifically add a line-break because of maxWidth constraints or where there is an explicit \n character.
//If it's the latter scenario or the line break is put before a whitespace character then consumeNextCharOut will be set to true to inidicate that
//one extra character should be consumed in the line break.
u64 FontFlow_FindNextLineBreak(const FontFlowState_t* flowState, r32* widthOut = nullptr, bool* consumeNextCharOut = nullptr)
{
	NotNull(flowState);
	NotNull(flowState->font);
	NotNullStr(&flowState->text);
	if (consumeNextCharOut != nullptr) { *consumeNextCharOut = false; }
	if (widthOut != nullptr) { *widthOut = 0; }
	
	FontFlowState_t lookFlowState = {};
	MyMemCopy(&lookFlowState, flowState, sizeof(FontFlowState_t));
	
	r32 currentLineWidth = 0;
	u64 lastFitByteIndex = lookFlowState.byteIndex;
	r32 lastFitLineWidth = 0;
	u64 lastGoodLineBreakIndex = lookFlowState.byteIndex;
	r32 lastGoodLineBreakLineWidth = 0;
	bool lastGoodLineBreakIsSpace = false;
	while (lookFlowState.byteIndex < lookFlowState.text.length)
	{
		u32 codepoint = 0;
		u8 numBytesInCodepoint = GetCodepointForUtf8(lookFlowState.text.length - lookFlowState.byteIndex, &lookFlowState.text.pntr[lookFlowState.byteIndex], &codepoint);
		
		bool isInvalidEncoding = false;
		if (numBytesInCodepoint == 0)
		{
			isInvalidEncoding = true;
			numBytesInCodepoint = 1;
			codepoint = CharToU32(lookFlowState.text.pntr[lookFlowState.byteIndex]);
		}
		
		bool isTab = false;
		u8 charColumnWidth = 1;
		if (codepoint == '\t')
		{
			codepoint = ' ';
			isTab = true;
			charColumnWidth = 4 - (lookFlowState.columnIndex % 4);
		}
		
		if (IsCharPunctuationStart(codepoint) || codepoint == ' ')
		{
			lastGoodLineBreakIndex = lookFlowState.byteIndex;
			lastGoodLineBreakLineWidth = currentLineWidth;
			lastGoodLineBreakIsSpace = (codepoint == ' ');
		}
		
		r32 thisCharAdvance = 0;
		rec thisCharLogicalRec = NewRec(lookFlowState.position, Vec2_Zero);
		if (codepoint == '\n')
		{
			if (widthOut != nullptr) { *widthOut = currentLineWidth; }
			if (consumeNextCharOut != nullptr) { *consumeNextCharOut = true; }
			return lookFlowState.byteIndex - flowState->byteIndex;
		}
		else if (codepoint == BOLD_TOGGLE_CHAR)
		{
			lookFlowState.faceSelector.bold = !lookFlowState.faceSelector.bold;
			ClearFlowStateCache(&lookFlowState);
		}
		else if (codepoint == ITALIC_TOGGLE_CHAR)
		{
			lookFlowState.faceSelector.italic = !lookFlowState.faceSelector.italic;
			ClearFlowStateCache(&lookFlowState);
		}
		else if (codepoint == UNDERLINE_TOGGLE_CHAR)
		{
			lookFlowState.underlined = !lookFlowState.underlined;
		}
		else
		{
			const FontCharInfo_t* charInfo = FontFlow_FindCharInfo(&lookFlowState, codepoint);
			if (charInfo != nullptr)
			{
				thisCharAdvance = charInfo->advanceX * charColumnWidth * lookFlowState.scale;
				thisCharLogicalRec = NewRec(lookFlowState.position + charInfo->logicalOffset * lookFlowState.scale, charInfo->logicalSize * lookFlowState.scale);
			}
			else
			{
				//We don't have the info to render this character. Draw a rectangle to represent "invalid char"
				const FontFace_t* faceForInvalidInfo = lookFlowState.selectedFaceCache;
				if (faceForInvalidInfo == nullptr) { faceForInvalidInfo = GetFontFace(lookFlowState.font, SelectDefaultFontFace()); }
				
				v2 invalidCharRecSize = NewVec2(5, 10);
				if (faceForInvalidInfo != nullptr)
				{
					invalidCharRecSize = NewVec2(0, (r32)faceForInvalidInfo->fontSize);
					invalidCharRecSize.width = RoundR32(invalidCharRecSize.height * 0.75f);
				}
				
				thisCharLogicalRec = NewRec(lookFlowState.position + NewVec2(0, -invalidCharRecSize.height * 0.9f * lookFlowState.scale), invalidCharRecSize * lookFlowState.scale);
				thisCharAdvance = thisCharLogicalRec.width + (2 * lookFlowState.scale);
			}
		}
		
		bool canFitOnThisLine = true;
		if (lookFlowState.maxWidth > 0)
		{
			if (lookFlowState.position.x + thisCharAdvance - lookFlowState.lineStartPos.x > lookFlowState.maxWidth) { canFitOnThisLine = false; }
			if (thisCharLogicalRec.x + thisCharLogicalRec.width - lookFlowState.lineStartPos.x > lookFlowState.maxWidth) { canFitOnThisLine = false; }
		}
		
		if (!canFitOnThisLine)
		{
			if (lastFitByteIndex == flowState->byteIndex)
			{
				//literally none of the characters could fit before we hit maxWidth so
				//let's make sure we at least include this one character before line breaking
				if (widthOut != nullptr) { *widthOut = currentLineWidth + thisCharAdvance; }
				if (consumeNextCharOut != nullptr) { *consumeNextCharOut = false; }
				Assert(lastFitByteIndex == lookFlowState.byteIndex);
				return lookFlowState.byteIndex + numBytesInCodepoint;
			}
			else
			{
				if (lastGoodLineBreakIndex > flowState->byteIndex)
				{
					if (widthOut != nullptr) { *widthOut = lastGoodLineBreakLineWidth; }
					if (consumeNextCharOut != nullptr) { *consumeNextCharOut = lastGoodLineBreakIsSpace; }
					return lastGoodLineBreakIndex - flowState->byteIndex;
				}
				else
				{
					if (widthOut != nullptr) { *widthOut = lastFitLineWidth; }
					if (consumeNextCharOut != nullptr) { *consumeNextCharOut = false; }
					return lastFitByteIndex - flowState->byteIndex;
				}
			}
		}
		else
		{
			lastFitByteIndex = lookFlowState.byteIndex + numBytesInCodepoint;
			lastFitLineWidth = currentLineWidth + thisCharAdvance;
			if (IsCharPunctuationEnd(codepoint))
			{
				lastGoodLineBreakIndex = lookFlowState.byteIndex + numBytesInCodepoint;
				lastGoodLineBreakLineWidth = currentLineWidth + thisCharAdvance;
				lastGoodLineBreakIsSpace = false;
			}
		}
		
		lookFlowState.position.x += thisCharAdvance;
		currentLineWidth += thisCharAdvance;
		lookFlowState.byteIndex += numBytesInCodepoint;
	}
	
	if (widthOut != nullptr) { *widthOut = currentLineWidth; }
	if (consumeNextCharOut != nullptr) { *consumeNextCharOut = false; }
	return lookFlowState.byteIndex - flowState->byteIndex;
}

void FontFlow_DoLineBreak(FontFlowState_t* flowState, FontFlowCallbacks_t* callbacks = nullptr)
{
	NotNull(flowState);
	NotNull(flowState->font);
	
	if (flowState->selectedFaceCache == nullptr)
	{
		flowState->selectedFaceCache = GetFontFace(flowState->font, flowState->faceSelector);
	}
	if (flowState->thisLineHeight == 0)
	{
		flowState->thisLineHeight = (flowState->selectedFaceCache != nullptr) ? flowState->selectedFaceCache->lineHeight : (r32)flowState->faceSelector.fontSize;
	}
	
	v2 underlineOffset = NewVec2(0, (flowState->selectedFaceCache != nullptr) ? flowState->selectedFaceCache->underlineOffset : DEFAULT_UNDERLINE_OFFSET);
	r32 underlineThickness = (flowState->selectedFaceCache != nullptr) ? flowState->selectedFaceCache->underlineThickness : DEFAULT_UNDERLINE_THICKNESS;
	if (flowState->underlined)
	{
		v2 underlinePos = flowState->position + underlineOffset * flowState->scale;
		if (!Vec2BasicallyEqual(flowState->underlineStartPos, underlinePos))
		{
			rec underlineRec = NewRecBetween(flowState->underlineStartPos, underlinePos);
			if (underlineRec.height < underlineThickness * flowState->scale) { underlineRec.height = underlineThickness * flowState->scale; }
			RcDrawRectangle(underlineRec, flowState->color);
		}
	}
	
	flowState->position.x = flowState->startPos.x;
	flowState->position.y += flowState->thisLineHeight * flowState->scale;
	flowState->lineRec = Rec_Zero;
	flowState->thisLineHeight = 0;
	flowState->lineStartPos = flowState->position;
	if (flowState->underlined)
	{
		v2 underlinePos = flowState->position + underlineOffset * flowState->scale;
		flowState->underlineStartPos = underlinePos;
	}
	flowState->lineIndex++;
	flowState->columnIndex = 0;
	flowState->calledBeforeLineOnThisLine = false;
	flowState->calledAfterLineOnThisLine = false;
}

void FontFlow_Main(FontFlowState_t* flowState, FontFlowCallbacks_t* callbacks = nullptr, FontFlowInfo_t* infoOut = nullptr)
{
	NotNull(flowState);
	NotNull(flowState->font);
	NotNullStr(&flowState->text);
	
	if (infoOut != nullptr)
	{
		ClearPointer(infoOut);
		infoOut->startPos = flowState->position;
	}
	
	if ((flowState->maxWidth > 0 || flowState->alignment != TextAlignment_Left) && flowState->nextLineBreakIndex <= flowState->byteIndex)
	{
		flowState->nextLineBreakIndex = flowState->byteIndex + FontFlow_FindNextLineBreak(flowState, &flowState->widthToLineBreak, &flowState->consumeCharAtLineBreak);
		Assert(flowState->nextLineBreakIndex > flowState->byteIndex || flowState->byteIndex >= flowState->text.length);
		if (flowState->alignment == TextAlignment_Right)
		{
			flowState->position.x -= flowState->widthToLineBreak;
			flowState->lineStartPos.x -= flowState->widthToLineBreak;
			flowState->underlineStartPos.x -= flowState->widthToLineBreak;
		}
		else if (flowState->alignment == TextAlignment_Center)
		{
			flowState->position.x -= RoundR32(flowState->widthToLineBreak/2);
			flowState->lineStartPos.x -= RoundR32(flowState->widthToLineBreak/2);
			flowState->underlineStartPos.x -= RoundR32(flowState->widthToLineBreak/2);
		}
	}
	if (!flowState->calledBeforeLineOnThisLine && callbacks != nullptr && callbacks->beforeLine != nullptr)
	{
		callbacks->beforeLine(flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
		flowState->calledBeforeLineOnThisLine = true;
	}
	
	while (flowState->byteIndex < flowState->text.length)
	{
		u32 codepoint = 0;
		u8 numBytesInCodepoint = GetCodepointForUtf8(flowState->text.length - flowState->byteIndex, &flowState->text.pntr[flowState->byteIndex], &codepoint);
		
		bool isInvalidEncoding = false;
		if (numBytesInCodepoint == 0)
		{
			isInvalidEncoding = true;
			numBytesInCodepoint = 1;
			codepoint = CharToU32(flowState->text.pntr[flowState->byteIndex]);
		}
		
		bool isTab = false;
		u8 charColumnWidth = 1;
		if (codepoint == '\t')
		{
			codepoint = ' ';
			isTab = true;
			charColumnWidth = 4 - (flowState->columnIndex % 4);
		}
		
		if (callbacks != nullptr && callbacks->betweenChar != nullptr)
		{
			callbacks->betweenChar(flowState->byteIndex, flowState->charIndex, flowState->position, flowState, callbacks->context);
		}
		
		bool renderChar = false;
		bool wasWhitespace = false;
		if (codepoint == '\n')
		{
			if (infoOut != nullptr) { infoOut->numLines++; infoOut->numNewLineCharacters++; }
			
			if (callbacks != nullptr && callbacks->afterLine != nullptr)
			{
				callbacks->afterLine(false, flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
				flowState->calledAfterLineOnThisLine = true;
			}
			FontFlow_DoLineBreak(flowState, callbacks);
			if (callbacks != nullptr && callbacks->beforeLine != nullptr)
			{
				callbacks->beforeLine(flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
				flowState->calledBeforeLineOnThisLine = true;
			}
		}
		else if (codepoint == BOLD_TOGGLE_CHAR)
		{
			if (infoOut != nullptr) { infoOut->numControlCharacters++; }
			flowState->faceSelector.bold = !flowState->faceSelector.bold;
			ClearFlowStateCache(flowState);
		}
		else if (codepoint == ITALIC_TOGGLE_CHAR)
		{
			if (infoOut != nullptr) { infoOut->numControlCharacters++; }
			flowState->faceSelector.italic = !flowState->faceSelector.italic;
			ClearFlowStateCache(flowState);
		}
		else if (codepoint == UNDERLINE_TOGGLE_CHAR)
		{
			if (infoOut != nullptr) { infoOut->numControlCharacters++; }
			flowState->underlined = !flowState->underlined;
			
			v2 underlineOffset = NewVec2(0, (flowState->selectedFaceCache != nullptr) ? flowState->selectedFaceCache->underlineOffset : DEFAULT_UNDERLINE_OFFSET);
			r32 underlineThickness = (flowState->selectedFaceCache != nullptr) ? flowState->selectedFaceCache->underlineThickness : DEFAULT_UNDERLINE_THICKNESS;
			v2 underlinePos = flowState->position + underlineOffset * flowState->scale;
			if (flowState->underlined)
			{
				flowState->underlineStartPos = underlinePos;
			}
			else
			{
				if (!Vec2BasicallyEqual(flowState->underlineStartPos, underlinePos))
				{
					rec underlineRec = NewRecBetween(flowState->underlineStartPos, underlinePos);
					if (underlineRec.height < underlineThickness * flowState->scale) { underlineRec.height = underlineThickness * flowState->scale; }
					RcDrawRectangle(underlineRec, flowState->color);
				}
			}
		}
		else
		{
			const FontCharInfo_t* charInfo = FontFlow_FindCharInfo(flowState, codepoint);
			
			if (charInfo != nullptr)
			{
				NotNull(flowState->selectedFaceCache);
				NotNull(flowState->selectedBakeCache);
				
				if (flowState->selectedFaceCache->lineHeight > flowState->thisLineHeight)
				{
					flowState->thisLineHeight = flowState->selectedFaceCache->lineHeight;
				}
				
				r32 scalableEffect = 1.0f;
				if (IsFlagSet(flowState->selectedBakeCache->flags, FontBakeFlag_IsScalable))
				{
					scalableEffect = flowState->selectedFaceCache->renderLineHeight / charInfo->renderSize.height;
				}
				rec renderRec = NewRec(flowState->position + charInfo->renderOffset * flowState->scale * scalableEffect, ToVec2(charInfo->renderSize) * flowState->scale * scalableEffect);
				rec logicalRec = NewRec(flowState->position + charInfo->logicalOffset * flowState->scale * scalableEffect, charInfo->logicalSize * flowState->scale * scalableEffect);
				
				//TODO: Should this check be a proper "isRenderable" flag on the char?
				if (charInfo->renderSize.width > 0 && charInfo->renderSize.height > 0)
				{
					//TODO: Should we do some amount of rounding for each character to make sure it's pixel aligned?
					renderChar = true;
					if (callbacks != nullptr && callbacks->beforeChar != nullptr)
					{
						renderChar = callbacks->beforeChar(codepoint, charInfo, logicalRec, renderRec, flowState, callbacks->context);
					}
					if (renderChar)
					{
						if (!flowState->justMeasuring)
						{
							Color_t drawColor = (IsFlagSet(flowState->selectedBakeCache->flags, FontBakeFlag_IsColored) ? White : flowState->color);
							// RecAlign(&renderRec);//TODO: Implement this properly
							//TODO: The cast to Texture_t* here is dumb mismatch between me wanting to do const pointers here and
							//      not doing them over in RenderContext stuff. Which one should change?
							RcBindTexture1((Texture_t*)&flowState->selectedBakeCache->bitmap);
							RcDrawTexturedPartRectangle(renderRec, drawColor, ToRec(charInfo->sourceRec));
						}
						
						if (infoOut != nullptr)
						{
							if (infoOut->renderRec.width != 0) { infoOut->renderRec = RecBoth(infoOut->renderRec, renderRec); }
							else { infoOut->renderRec = renderRec; }
							if (infoOut->logicalRec.width != 0) { infoOut->logicalRec = RecBoth(infoOut->logicalRec, logicalRec); }
							else { infoOut->logicalRec = logicalRec; }
						}
						if (flowState->lineRec.width == 0)
						{
							//TODO: This isn't perfectly aligned (maxAscend doesn't do what we want). Can we somehow determine what the default height lineRec placement should be?
							flowState->lineRec = NewRec(flowState->position.x, flowState->position.y - flowState->selectedFaceCache->maxAscend, 0, flowState->selectedFaceCache->lineHeight);
						}
						flowState->lineRec = RecBoth(flowState->lineRec, logicalRec);
						
						if (callbacks != nullptr && callbacks->afterChar != nullptr)
						{
							callbacks->afterChar(codepoint, charInfo, logicalRec, renderRec, flowState, callbacks->context);
						}
					}
				}
				
				if (infoOut != nullptr)
				{
					if (renderChar) { infoOut->numCharactersRendered++; }
					else { infoOut->numWhitespaceCharacters++; }
				}
				if (!renderChar && charInfo->advanceX > 0) { wasWhitespace = true; }
				flowState->position.x += charInfo->advanceX * charColumnWidth * flowState->scale * scalableEffect;
			}
			else
			{
				//We don't have the info to render this character. Draw a rectangle to represent "invalid char"
				const FontFace_t* faceForInvalidInfo = flowState->selectedFaceCache;
				if (faceForInvalidInfo == nullptr) { faceForInvalidInfo = GetFontFace(flowState->font, SelectDefaultFontFace()); }
				
				v2 invalidCharRecSize = NewVec2(5, 10);
				if (faceForInvalidInfo != nullptr)
				{
					invalidCharRecSize = NewVec2(0, (r32)faceForInvalidInfo->fontSize);
					invalidCharRecSize.width = RoundR32(invalidCharRecSize.height * 0.75f);
				}
				
				rec renderRec = NewRec(flowState->position + NewVec2(0, -invalidCharRecSize.height * 0.9f * flowState->scale), invalidCharRecSize * flowState->scale);
				rec logicalRec = renderRec;
				
				if (!flowState->justMeasuring)
				{
					RcDrawRectangle(renderRec, PalVioletDarker);
				}
				
				if (infoOut != nullptr)
				{
					if (infoOut->renderRec.width != 0) { infoOut->renderRec = RecBoth(infoOut->renderRec, renderRec); }
					else { infoOut->renderRec = renderRec; }
					if (infoOut->logicalRec.width != 0) { infoOut->logicalRec = RecBoth(infoOut->logicalRec, logicalRec); }
					else { infoOut->logicalRec = logicalRec; }
				}
				
				if (infoOut != nullptr) { infoOut->numCharactersRendered++; infoOut->numInvalidCharacters++; }
				flowState->position.x += renderRec.width + (2 * flowState->scale);
			}
			
			flowState->columnIndex += charColumnWidth;
			
		}
		
		flowState->charIndex++;
		flowState->byteIndex += numBytesInCodepoint;
		
		// Handle Line Wrap
		if (flowState->byteIndex == flowState->nextLineBreakIndex && flowState->nextLineBreakIndex < flowState->text.length)
		{
			Assert(flowState->maxWidth > 0 || flowState->alignment != TextAlignment_Left); //we shouldn't be in here if there is no maxWidth and no alignment is needed
			bool isCausedByNewLine = false;
			if (flowState->consumeCharAtLineBreak)
			{
				u32 consumedCodepoint = 0;
				u8 consumeCodepointSize = GetCodepointForUtf8(flowState->text.length - flowState->byteIndex, &flowState->text.pntr[flowState->byteIndex], &consumedCodepoint);
				if (consumedCodepoint == '\n') { isCausedByNewLine = true; }
				if (infoOut != nullptr)
				{
					if (isCausedByNewLine) { infoOut->numNewLineCharacters++; }
					else if (consumedCodepoint == ' ' || consumedCodepoint == '\t') { infoOut->numWhitespaceCharacters++; }
					else { /*TODO: Should we do a warning here of some kind?*/ }
				}
				flowState->charIndex++;
				flowState->byteIndex += consumeCodepointSize;
			}
			
			if (infoOut != nullptr && flowState->byteIndex < flowState->text.length)
			{
				infoOut->numLines++;
				if (!isCausedByNewLine) { infoOut->numLineWraps++; }
			}
			
			if (callbacks != nullptr && callbacks->afterLine != nullptr)
			{
				callbacks->afterLine(!isCausedByNewLine, flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
				flowState->calledAfterLineOnThisLine = true;
			}
			if (callbacks != nullptr && callbacks->betweenChar != nullptr)
			{
				callbacks->betweenChar(flowState->byteIndex, flowState->charIndex, flowState->position, flowState, callbacks->context);
			}
			
			FontFlow_DoLineBreak(flowState, callbacks);
			
			flowState->nextLineBreakIndex = flowState->byteIndex + FontFlow_FindNextLineBreak(flowState, &flowState->widthToLineBreak, &flowState->consumeCharAtLineBreak);
			Assert(flowState->nextLineBreakIndex > flowState->byteIndex || flowState->byteIndex >= flowState->text.length);
			if (flowState->alignment == TextAlignment_Right)
			{
				flowState->position.x -= flowState->widthToLineBreak;
				flowState->lineStartPos.x -= flowState->widthToLineBreak;
				flowState->underlineStartPos.x -= flowState->widthToLineBreak;
			}
			else if (flowState->alignment == TextAlignment_Center)
			{
				flowState->position.x -= RoundR32(flowState->widthToLineBreak/2);
				flowState->lineStartPos.x -= RoundR32(flowState->widthToLineBreak/2);
				flowState->underlineStartPos.x -= RoundR32(flowState->widthToLineBreak/2);
			}
			
			if (callbacks != nullptr && callbacks->beforeLine != nullptr)
			{
				callbacks->beforeLine(flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
				flowState->calledBeforeLineOnThisLine = true;
			}
		}
	}
	
	//if there were any characters on the last line then count that in numLines as well
	if (infoOut != nullptr)
	{
		infoOut->endPos = flowState->position;
		if (flowState->columnIndex > 0)
		{
			infoOut->numLines++;
		}
	}
	
	if (!flowState->calledBeforeLineOnThisLine && callbacks != nullptr && callbacks->beforeLine != nullptr)
	{
		callbacks->beforeLine(flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
		flowState->calledBeforeLineOnThisLine = true;
	}
	if (callbacks != nullptr && callbacks->betweenChar != nullptr)
	{
		callbacks->betweenChar(flowState->byteIndex, flowState->charIndex, flowState->position, flowState, callbacks->context);
	}
	if (!flowState->calledAfterLineOnThisLine && callbacks != nullptr && callbacks->afterLine != nullptr)
	{
		callbacks->afterLine(false, flowState->lineIndex, flowState->byteIndex, flowState, callbacks->context);
		flowState->calledAfterLineOnThisLine = true;
	}
}

void FontFlow_Initialize(FontFlowState_t* flowStateOut, MyStr_t text, const Font_t* font, FontFaceSelector_t selector, Color_t color, v2 position,
	TextAlignment_t alignment = TextAlignment_Left, r32 scale = 1.0f, r32 maxWidth = 0.0f)
{
	NotNull(flowStateOut);
	NotNullStr(&text);
	NotNull(font);
	
	ClearPointer(flowStateOut);
	flowStateOut->text = text;
	flowStateOut->font = font;
	flowStateOut->faceSelector = selector;
	flowStateOut->color = color;
	flowStateOut->startPos = position;
	flowStateOut->scale = scale;
	flowStateOut->maxWidth = maxWidth;
	flowStateOut->alignment = alignment;
	flowStateOut->justMeasuring = false;
	
	flowStateOut->byteIndex = 0;
	flowStateOut->charIndex = 0;
	flowStateOut->lineIndex = 0;
	flowStateOut->columnIndex = 0;
	flowStateOut->consumeCharAtLineBreak = false;
	flowStateOut->nextLineBreakIndex = 0;
	flowStateOut->widthToLineBreak = 0;
	flowStateOut->calledBeforeLineOnThisLine = false;
	flowStateOut->calledAfterLineOnThisLine = false;
	flowStateOut->position = position;
	flowStateOut->lineStartPos = position;
	flowStateOut->thisLineHeight = 0;
	flowStateOut->selectedFaceCache = GetFontFace(font, selector);
	flowStateOut->selectedBakeCache = nullptr;
}

void FontFlow_RenderText(MyStr_t text, const Font_t* font, FontFaceSelector_t selector, Color_t color, v2 position,
	TextAlignment_t alignment = TextAlignment_Left, r32 scale = 1.0f, r32 maxWidth = 0.0f,
	FontFlowCallbacks_t* callbacks = nullptr, FontFlowInfo_t* infoOut = nullptr)
{
	FontFlowState_t flowState;
	FontFlow_Initialize(&flowState, text, font, selector, color, position, alignment, scale, maxWidth);
	FontFlow_Main(&flowState, callbacks, infoOut);
}

// +--------------------------------------------------------------+
// |                      Measure Functions                       |
// +--------------------------------------------------------------+
TextMeasure_t MeasureTextInFont(MyStr_t text, const Font_t* font, FontFaceSelector_t faceSelector, r32 scale = 1.0f, r32 maxWidth = 0.0f, FontFlowInfo_t* flowInfoOut = nullptr, FontFlowCallbacks_t* callbacks = nullptr)
{
	FontFlowInfo_t flowInfo = {};
	
	FontFlowState_t flowState;
	FontFlow_Initialize(&flowState, text, font, faceSelector, White, Vec2_Zero, TextAlignment_Left, scale, maxWidth);
	flowState.justMeasuring = true;
	
	FontFlow_Main(&flowState, callbacks, &flowInfo);
	
	if (flowInfoOut != nullptr) { MyMemCopy(flowInfoOut, &flowInfo, sizeof(FontFlowInfo_t)); }
	
	TextMeasure_t result;
	result.size = flowInfo.logicalRec.size;
	result.offset = -flowInfo.logicalRec.topLeft;
	return result;
}
TextMeasure_t MeasureTextInFont(const char* nullptrText, const Font_t* font, FontFaceSelector_t faceSelector, r32 scale = 1.0f, r32 maxWidth = 0.0f, FontFlowInfo_t* flowInfoOut = nullptr, FontFlowCallbacks_t* callbacks = nullptr)
{
	return MeasureTextInFont(NewStr(nullptrText), font, faceSelector, scale, maxWidth, flowInfoOut, callbacks);
}

// +--------------------------------------------------------------+
// |                       FindCursorIndex                        |
// +--------------------------------------------------------------+
//TODO: This needs a little more fixup to make it work truly nicely.
//      It really should prioritize positions on the line this is shares a y-value with.
//      Right now it does that when deciding when line to do the index check against but it doesn't do
//      the same logic for multiple lines in a single dbgLine (like ones caused by a line wrap)
struct FindCursorIndexInFlowedTextContext_t
{
	v2 relativePixelPos;
	bool foundPosition;
	v2 closestPosition;
	u64 closestByteIndex;
	r32 closestDistance;
};
// | FindCursorIndexInFlowedTextBetweenCharCallback  |
// void FindCursorIndexInFlowedTextBetweenCharCallback(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
FFCB_BETWEEN_CHAR_DEFINITION(FindCursorIndexInFlowedTextBetweenCharCallback)
{
	UNUSED(charIndex);
	UNUSED(state);
	NotNull(context);
	FindCursorIndexInFlowedTextContext_t* contextPntr = (FindCursorIndexInFlowedTextContext_t*)context;
	r32 distanceToMouse = Vec2Length(contextPntr->relativePixelPos - position);
	if (!contextPntr->foundPosition || (distanceToMouse < contextPntr->closestDistance))
	{
		contextPntr->foundPosition = true;
		contextPntr->closestPosition = position;
		contextPntr->closestByteIndex = byteIndex;
		contextPntr->closestDistance = distanceToMouse;
	}
}

//returns a byteIndex, not a character index. Returns -1 when no suitable index is found (TODO: Should this, like, not happen??)
i64 FindCursorIndexInFlowedText(MyStr_t text, const Font_t* font, FontFaceSelector_t faceSelector, v2 relativePixelPos, r32 scale = 1.0f, r32 maxWidth = 0.0f, v2* foundPixelPosOut = nullptr, FontFlowInfo_t* flowInfoOut = nullptr)
{
	NotNullStr(&text);
	NotNull(font);
	
	FontFlowState_t flowState;
	FontFlow_Initialize(&flowState, text, font, faceSelector, White, Vec2_Zero, TextAlignment_Left, scale, maxWidth);
	flowState.justMeasuring = true;
	
	FindCursorIndexInFlowedTextContext_t context = {};
	context.foundPosition = false;
	context.relativePixelPos = relativePixelPos;
	
	FontFlowCallbacks_t flowCallbacks = {};
	flowCallbacks.context = (void*)&context;
	flowCallbacks.betweenChar = FindCursorIndexInFlowedTextBetweenCharCallback;
	
	FontFlowInfo_t flowInfo = {};
	FontFlow_Main(&flowState, &flowCallbacks, &flowInfo);
	
	if (flowInfoOut != nullptr) { MyMemCopy(flowInfoOut, &flowInfo, sizeof(FontFlowInfo_t)); }
	if (foundPixelPosOut != nullptr) { *foundPixelPosOut = context.closestPosition; }
	return (context.foundPosition ? context.closestByteIndex : -1);
}
