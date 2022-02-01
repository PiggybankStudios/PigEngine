/*
File:   pig_font.cpp
Author: Taylor Robbins
Date:   12\22\2021
Description: 
	** Holds the code that helps us load and organize information about fonts into Font_t structures
	** that can then be used by functions in pig_text_flow.cpp to render text on screen.
*/

#define DEFAULT_UNDERLINE_OFFSET     2 //px
#define DEFAULT_UNDERLINE_THICKNESS  1 //px

// +--------------------------------------------------------------+
// |                Creation/Destruction Functions                |
// +--------------------------------------------------------------+
void DestroyFont(Font_t* font)
{
	NotNull(font);
	if (font->faces.length > 0)
	{
		NotNull(font->allocArena);
		for (u64 fIndex = 0; fIndex < font->faces.length; fIndex++)
		{
			FontFace_t* face = VarArrayGet(&font->faces, fIndex, FontFace_t);
			for (u64 bIndex = 0; bIndex < face->bakes.length; bIndex++)
			{
				FontBake_t* bake = VarArrayGet(&face->bakes, bIndex, FontBake_t);
				DestroyTexture(&bake->bitmap);
				for (u64 rIndex = 0; rIndex < bake->ranges.length; rIndex++)
				{
					FontRange_t* range = VarArrayGet(&bake->ranges, rIndex, FontRange_t);
					FreeVarArray(&range->charInfos);
				}
				FreeVarArray(&bake->ranges);
			}
			FreeVarArray(&face->bakes);
		}
	}
	FreeVarArray(&font->faces);
	ClearPointer(font);
}

void StartFont(Font_t* font, MemArena_t* memArena, u64 numFacesExpected = 0)
{
	NotNull(font);
	NotNull(memArena);
	ClearPointer(font);
	CreateVarArray(&font->faces, memArena, sizeof(FontFace_t), numFacesExpected);
	font->isValid = false;
	font->hasFailures = false;
	font->allocArena = memArena;
}

FontFace_t* FontStartFace(Font_t* font, MyStr_t fontName, i32 fontSize, bool bold, bool italic, u64 numBakesExpected = 0)
{
	NotNull(font);
	NotNull(font->allocArena);
	NotNullStr(&fontName);
	Assert(fontSize > 0);
	
	if (font->defaultFaceIndex >= font->faces.length)
	{
		font->defaultFaceIndex++; //maintain invalid index
	}
	
	FontFace_t* result = VarArrayAdd(&font->faces, FontFace_t);
	NotNull(result);
	ClearPointer(result);
	result->faceIndex = (font->faces.length - 1);
	result->fontSize = fontSize;
	result->flags = (bold ? FontFaceFlag_IsBold : 0) | (italic ? FontFaceFlag_IsItalic : 0);
	result->underlineOffset = DEFAULT_UNDERLINE_OFFSET;
	result->underlineThickness = DEFAULT_UNDERLINE_THICKNESS;
	
	result->minCodepoint = UINT32_MAX;
	result->maxCodepoint = UINT32_MAX;
	result->numCharacters = 0;
	CreateVarArray(&result->bakes, font->allocArena, sizeof(FontBake_t), numBakesExpected);
	
	if (!plat->ReadPlatformFont(fontName, fontSize, bold, italic, &result->fontFile))
	{
		PrintLine_E("Failed to open platform font file by name \"%.*s\" for new face in font", fontName.length, fontName.pntr);
		FreeVarArray(&result->bakes);
		VarArrayPop(&font->faces, FontFace_t);
		return nullptr;
	}
	FlagSet(result->flags, FontFaceFlag_IsActive);
	
	// PrintLine_D("Font file \"%s\" at %d is %u bytes", fontName.pntr, fontSize, result->fontFile.size);
	
	FlagSet(result->flags, FontFaceFlag_IsValid);
	return result;
}
void ChangeFontFileForActiveFace(Font_t* font, FontFace_t* face, MyStr_t fontName, i32 fontSize, bool bold, bool italic)
{
	NotNull(font);
	NotNull(face);
	Assert(IsFlagSet(face->flags, FontFaceFlag_IsActive));
	PlatFileContents_t newFontFile = {};
	if (!plat->ReadPlatformFont(fontName, fontSize, bold, italic, &newFontFile))
	{
		PrintLine_E("Failed to open platform font file by name \"%.*s\" for changing face in font", fontName.length, fontName.pntr);
		DebugAssert(false);
		return;
	}
	plat->FreeFileContents(&face->fontFile);
	face->fontFile = newFontFile;
}
FontBake_t* FontAddBakeToActiveFace(Font_t* font, FontFace_t* face, v2i bakeSize, u64 numRanges, PlatFontRange_t* ranges)
{
	NotNull(font);
	NotNull(font->allocArena);
	NotNull(face);
	DebugAssert(VarArrayContains(&font->faces, face));
	Assert(IsFlagSet(face->flags, FontFaceFlag_IsActive));
	Assert(bakeSize.width > 0 && bakeSize.height > 0);
	Assert(numRanges > 0);
	NotNull(ranges);
	
	FontBake_t* result = VarArrayAdd(&face->bakes, FontBake_t);
	NotNull(result);
	ClearPointer(result);
	result->bakeIndex = (face->bakes.length - 1);
	CreateVarArray(&result->ranges, font->allocArena, sizeof(FontRange_t), numRanges);
	result->flags = FontBakeFlag_None;
	
	u32 newFaceMinCodepoint = face->minCodepoint;
	u32 newFaceMaxCodepoint = face->maxCodepoint;
	for (u64 rIndex = 0; rIndex < numRanges; rIndex++)
	{
		PlatFontRange_t* inRange = &ranges[rIndex];
		FontRange_t* range = VarArrayAdd(&result->ranges, FontRange_t);
		NotNull(range);
		range->rangeIndex = (result->ranges.length - 1);
		range->contiguous = (inRange->codePointList == nullptr);
		if (range->contiguous)
		{
			range->minCodepoint = inRange->firstCodepoint;
			range->maxCodepoint = inRange->firstCodepoint + inRange->numChars;
		}
		else
		{
			range->minCodepoint = UINT32_MAX;
			range->maxCodepoint = UINT32_MAX;
			for (u64 cIndex = 0; cIndex < numRanges; cIndex++)
			{
				u32 codepoint = inRange->codePointList[cIndex];
				Assert(codepoint < UINT32_MAX);
				if (codepoint < range->minCodepoint || range->minCodepoint == UINT32_MAX) { range->minCodepoint = codepoint; }
				if (codepoint >= range->maxCodepoint || range->maxCodepoint == UINT32_MAX) { range->maxCodepoint = codepoint+1; }
			}
		}
		if (range->minCodepoint < newFaceMinCodepoint || newFaceMinCodepoint == UINT32_MAX) { newFaceMinCodepoint = range->minCodepoint; }
		if (range->maxCodepoint > newFaceMaxCodepoint || newFaceMaxCodepoint == UINT32_MAX) { newFaceMaxCodepoint = range->maxCodepoint; }
		CreateVarArray(&range->charInfos, font->allocArena, sizeof(FontCharInfo_t), inRange->numChars);
	}
	
	PlatFontData_t fontData = {};
	bool expandTo32bit = false;
	if (!plat->BakeFont(&face->fontFile, bakeSize, expandTo32bit, numRanges, ranges, &fontData, &result->rectPackContext))
	{
		MyDebugBreak();
		font->hasFailures = true;
		FlagUnset(face->flags, FontFaceFlag_IsValid);
		return result;
	}
	
	if (!CreateTexture(font->allocArena, &result->bitmap, &fontData.imageData, false, false))
	{
		MyDebugBreak();
		font->hasFailures = true;
		FlagUnset(face->flags, FontFaceFlag_IsValid);
		return result;
	}
	
	u64 charInfoIndex = 0;
	for (u64 rIndex = 0; rIndex < numRanges; rIndex++)
	{
		PlatFontRange_t* inRange = &ranges[rIndex];
		FontRange_t* range = VarArrayGet(&result->ranges, rIndex, FontRange_t);
		for (u32 cIndex = 0; cIndex < inRange->numChars; cIndex++)
		{
			u32 codepoint = (inRange->codePointList != nullptr) ? (inRange->codePointList[cIndex]) : (inRange->firstCodepoint + cIndex);
			Assert(charInfoIndex < fontData.numChars);
			PlatFontCharData_t* platCharInfo = &fontData.charData[charInfoIndex];
			Assert(platCharInfo->codepoint == codepoint);
			
			FontCharInfo_t* charInfo = VarArrayAdd(&range->charInfos, FontCharInfo_t);
			NotNull(charInfo);
			ClearPointer(charInfo);
			charInfo->charIndex = (range->charInfos.length - 1);
			charInfo->codepoint = codepoint;
			
			charInfo->sourceRec = platCharInfo->sourceRec;
			charInfo->logicalOffset = platCharInfo->offset;
			charInfo->logicalSize = ToVec2(platCharInfo->sourceRec.size);
			charInfo->renderOffset = platCharInfo->offset;
			charInfo->advanceX = platCharInfo->advanceX;
			
			if (charInfo->logicalSize.width > 0 && charInfo->logicalSize.height > 0)
			{
				r32 maxAscend = MaxR32(0, -charInfo->logicalOffset.y);
				r32 maxDescend = MaxR32(0, charInfo->logicalOffset.y + charInfo->logicalSize.height);
				r32 lineHeight = maxAscend + maxDescend;
				r32 renderMaxAscend = MaxR32(0, -charInfo->renderOffset.y);
				r32 renderMaxDescend = MaxR32(0, charInfo->renderOffset.y + charInfo->renderSize.height);
				r32 renderLineHeight = renderMaxAscend + renderMaxDescend;
				if (maxAscend  > face->maxAscend)  { face->maxAscend  = maxAscend;  }
				if (maxDescend > face->maxDescend) { face->maxDescend = maxDescend; }
				if (lineHeight > face->lineHeight) { face->lineHeight = lineHeight; }
				if (renderMaxAscend > face->renderMaxAscend)   { face->renderMaxAscend  = renderMaxAscend; }
				if (renderMaxDescend > face->renderMaxDescend) { face->renderMaxDescend = renderMaxDescend; }
				if (renderLineHeight > face->renderLineHeight) { face->renderLineHeight = renderLineHeight; }
			}
			
			charInfoIndex++;
		}
	}
	
	plat->FreeFontData(&fontData);
	
	face->minCodepoint = newFaceMinCodepoint;
	face->maxCodepoint = newFaceMaxCodepoint;
	
	FlagSet(result->flags, FontBakeFlag_IsValid);
	return result;
}
// NOTE: spriteSheet gets some members copied out and cleared, thus making the spriteSheet
//       unusable by the calling code afterwards, but it can be passed to DestroySpriteSheet just fine
// TODO: If we could copy a texture easily then this wouldn't be required I guess ^^^
FontBake_t* AddSpriteSheetAsFontBake(Font_t* font, FontFace_t* face, SpriteSheet_t* spriteSheet, bool scalable, bool colored)
{
	NotNull(font);
	NotNull(font->allocArena);
	NotNull(face);
	DebugAssert(VarArrayContains(&font->faces, face));
	Assert(IsFlagSet(face->flags, FontFaceFlag_IsActive));
	NotNull(spriteSheet);
	Assert(spriteSheet->isValid);
	Assert(spriteSheet->texture.isValid);
	
	FontBake_t* result = VarArrayAdd(&face->bakes, FontBake_t);
	NotNull(result);
	ClearPointer(result);
	result->bakeIndex = (face->bakes.length - 1);
	CreateVarArray(&result->ranges, font->allocArena, sizeof(FontRange_t), 1);
	result->flags = FontBakeFlag_IsSpriteFont | (u8)(scalable ? FontBakeFlag_IsScalable : FontBakeFlag_None) | (u8)(colored ? FontBakeFlag_IsColored : FontBakeFlag_None);
	
	u32 newFaceMinCodepoint = face->minCodepoint;
	u32 newFaceMaxCodepoint = face->maxCodepoint;
	
	FontRange_t* newRange = VarArrayAdd(&result->ranges, FontRange_t);
	NotNull(newRange);
	ClearPointer(newRange);
	newRange->rangeIndex = (result->ranges.length - 1);
	newRange->contiguous = false;
	CreateVarArray(&newRange->charInfos, font->allocArena, sizeof(FontCharInfo_t), (u64)(spriteSheet->numFramesX * spriteSheet->numFramesY));
	newRange->minCodepoint = UINT32_MAX;
	newRange->maxCodepoint = UINT32_MAX;
	VarArrayLoop(&spriteSheet->frames, fIndex)
	{
		VarArrayLoopGet(SpriteSheetFrame_t, frame, &spriteSheet->frames, fIndex);
		if (frame->codepoint != 0)
		{
			if (frame->codepoint < newRange->minCodepoint || newRange->minCodepoint == UINT32_MAX) { newRange->minCodepoint = frame->codepoint; }
			if (frame->codepoint >= newRange->maxCodepoint || newRange->maxCodepoint == UINT32_MAX) { newRange->maxCodepoint = frame->codepoint+1; }
			FontCharInfo_t* newCharInfo = VarArrayAdd(&newRange->charInfos, FontCharInfo_t);
			NotNull(newCharInfo);
			ClearPointer(newCharInfo);
			newCharInfo->charIndex = (newRange->charInfos.length - 1);
			newCharInfo->codepoint = frame->codepoint;
			v2i frameTopLeft = Vec2iMultiply(frame->gridPos, spriteSheet->frameSize + spriteSheet->padding*2) + spriteSheet->padding;
			newCharInfo->sourceRec = NewReci(frameTopLeft + frame->charBounds.topLeft, frame->charBounds.size);
			newCharInfo->renderOffset = NewVec2(0, -(r32)newCharInfo->sourceRec.height);
			newCharInfo->renderOffset += ToVec2(frame->charOffset);
			if (frame->logicalBounds == Reci_Zero)
			{
				newCharInfo->logicalSize = ToVec2(newCharInfo->renderSize);
				newCharInfo->logicalOffset = newCharInfo->renderOffset;
			}
			else
			{
				newCharInfo->logicalSize = ToVec2(frame->logicalBounds.size);
				newCharInfo->logicalOffset = newCharInfo->renderOffset + ToVec2(frame->logicalBounds.topLeft - frame->charBounds.topLeft);
			}
			newCharInfo->advanceX = frame->advanceX;
			
			if (!scalable && newCharInfo->logicalSize.width > 0 && newCharInfo->logicalSize.height > 0)
			{
				r32 maxAscend = MaxR32(0, -newCharInfo->logicalOffset.y);
				r32 maxDescend = MaxR32(0, newCharInfo->logicalOffset.y + newCharInfo->logicalSize.height);
				r32 lineHeight = maxAscend + maxDescend;
				r32 renderMaxAscend = MaxR32(0, -newCharInfo->renderOffset.y);
				r32 renderMaxDescend = MaxR32(0, newCharInfo->renderOffset.y + newCharInfo->renderSize.height);
				r32 renderLineHeight = renderMaxAscend + renderMaxDescend;
				if (maxAscend  > face->maxAscend)  { face->maxAscend  = maxAscend;  }
				if (maxDescend > face->maxDescend) { face->maxDescend = maxDescend; }
				if (lineHeight > face->lineHeight) { face->lineHeight = lineHeight; }
				if (renderMaxAscend > face->renderMaxAscend)   { face->renderMaxAscend  = renderMaxAscend; }
				if (renderMaxDescend > face->renderMaxDescend) { face->renderMaxDescend = renderMaxDescend; }
				if (renderLineHeight > face->renderLineHeight) { face->renderLineHeight = renderLineHeight; }
			}
		}
	}
	if (newRange->minCodepoint < newFaceMinCodepoint || newFaceMinCodepoint == UINT32_MAX) { newFaceMinCodepoint = newRange->minCodepoint; }
	if (newRange->maxCodepoint > newFaceMaxCodepoint || newFaceMaxCodepoint == UINT32_MAX) { newFaceMaxCodepoint = newRange->maxCodepoint; }
	
	//Copy and clear out the texture in the spriteSheet so it doesn't get freed later, we now own this texture
	MyMemCopy(&result->bitmap, &spriteSheet->texture, sizeof(Texture_t));
	MyMemSet(&spriteSheet->texture, 0x00, sizeof(Texture_t));
	
	face->minCodepoint = newFaceMinCodepoint;
	face->maxCodepoint = newFaceMaxCodepoint;
	
	FlagSet(result->flags, FontBakeFlag_IsValid);
	return result;
}
void FontFinishFace(Font_t* font, FontFace_t* face)
{
	NotNull(font);
	NotNull(face);
	DebugAssert(VarArrayContains(&font->faces, face));
	Assert(IsFlagSet(face->flags, FontFaceFlag_IsActive));
	plat->FreeFileContents(&face->fontFile);
	FlagUnset(face->flags, FontFaceFlag_IsActive);
}

void MakeFontFaceDefault(Font_t* font, FontFace_t* face)
{
	NotNull(font);
	NotNull(face);
	Assert(font->defaultFaceIndex >= font->faces.length); //Make sure there isn't another face that was already set as default
	font->defaultFaceIndex = face->faceIndex;
	FlagSet(face->flags, FontFaceFlag_IsDefault);
}

//TODO: Add some functions that help us create font faces from custom bitmap font files or from local .ttf files rather than platform provided file lookups

// +--------------------------------------------------------------+
// |                    Usage/Access Functions                    |
// +--------------------------------------------------------------+
FontFace_t* GetFontFace(Font_t* font, FontFaceSelector_t selector)
{
	NotNull(font);
	
	if (selector.selectDefault)
	{
		if (font->defaultFaceIndex < font->faces.length)
		{
			return VarArrayGet(&font->faces, font->defaultFaceIndex, FontFace_t);
		}
		else if (font->faces.length > 0)
		{
			//If there is no default face, then we will treat the first face as the default
			return VarArrayGet(&font->faces, 0, FontFace_t);
		}
		else
		{
			return nullptr;
		}
	}
	
	FontFace_t* result = nullptr;
	u64 resultFitness = UINT64_MAX;
	VarArrayLoop(&font->faces, fIndex)
	{
		VarArrayLoopGet(FontFace_t, face, &font->faces, fIndex);
		
		u64 fitness = 0;
		if (selector.fontSize != 0)
		{
			fitness += (u64)AbsI32(face->fontSize - selector.fontSize);
		}
		fitness <<= 2; //shift up by two to give 2-bit space for bold and italic mismatch
		if (selector.bold != IsFlagSet(face->flags, FontFaceFlag_IsBold)) { fitness++; }
		if (selector.italic != IsFlagSet(face->flags, FontFaceFlag_IsItalic)) { fitness++; }
		
		if (fitness < resultFitness)
		{
			result = face;
			resultFitness = fitness;
		}
	}
	return result;
}
const FontFace_t* GetFontFace(const Font_t* font, FontFaceSelector_t selector) //const-variant
{
	return (const FontFace_t*)GetFontFace((Font_t*)font, selector);
}

FontBake_t* GetFontBakeForChar(Font_t* font, FontFace_t* face, u32 codepoint, FontCharInfo_t** infoOut = nullptr, FontRange_t** rangeOut = nullptr)
{
	NotNull(font);
	NotNull(face);
	DebugAssert(VarArrayContains(&font->faces, face));
	
	VarArrayLoop(&face->bakes, bIndex)
	{
		VarArrayLoopGet(FontBake_t, bake, &face->bakes, bIndex);
		VarArrayLoop(&bake->ranges, rIndex)
		{
			VarArrayLoopGet(FontRange_t, range, &bake->ranges, rIndex);
			if (range->minCodepoint <= codepoint && range->maxCodepoint > codepoint)
			{
				if (range->contiguous)
				{
					if (infoOut != nullptr) { *infoOut = VarArrayGet(&range->charInfos, codepoint - range->minCodepoint, FontCharInfo_t); }
					if (rangeOut != nullptr) { *rangeOut = range; }
					return bake;
				}
				else
				{
					VarArrayLoop(&range->charInfos, cIndex)
					{
						VarArrayLoopGet(FontCharInfo_t, charInfo, &range->charInfos, cIndex);
						if (charInfo->codepoint == codepoint)
						{
							if (infoOut != nullptr) { *infoOut = charInfo; }
							if (rangeOut != nullptr) { *rangeOut = range; }
							return bake;
						}
					}
				}
			}
		}
	}
	return nullptr;
}
const FontBake_t* GetFontBakeForChar(const Font_t* font, const FontFace_t* face, u32 codepoint, const FontCharInfo_t** infoOut = nullptr, const FontRange_t** rangeOut = nullptr) //const-variant
{
	return (const FontBake_t*)GetFontBakeForChar((Font_t*)font, (FontFace_t*)face, codepoint, (FontCharInfo_t**)infoOut, (FontRange_t**)rangeOut);
}

FontCharInfo_t* GetFontCharInfoInBake(Font_t* font, FontFace_t* face, FontBake_t* bake, u32 codepoint, FontRange_t** rangeOut = nullptr)
{
	NotNull(font);
	NotNull(face);
	NotNull(bake);
	DebugAssert(VarArrayContains(&font->faces, face));
	DebugAssert(VarArrayContains(&face->bakes, bake));
	VarArrayLoop(&bake->ranges, rIndex)
	{
		VarArrayLoopGet(FontRange_t, range, &bake->ranges, rIndex);
		if (range->minCodepoint <= codepoint && range->maxCodepoint > codepoint)
		{
			if (range->contiguous)
			{
				FontCharInfo_t* charInfo = VarArrayGet(&range->charInfos, codepoint - range->minCodepoint, FontCharInfo_t);
				if (rangeOut != nullptr) { *rangeOut = range; }
				return charInfo; 
			}
			else
			{
				VarArrayLoop(&range->charInfos, cIndex)
				{
					VarArrayLoopGet(FontCharInfo_t, charInfo, &range->charInfos, cIndex);
					if (charInfo->codepoint == codepoint)
					{
						if (rangeOut != nullptr) { *rangeOut = range; }
						return charInfo;
					}
				}
			}
		}
	}
	return nullptr;
}
const FontCharInfo_t* GetFontCharInfoInBake(const Font_t* font, const FontFace_t* face, const FontBake_t* bake, u32 codepoint, const FontRange_t** rangeOut = nullptr)
{
	return (const FontCharInfo_t*)GetFontCharInfoInBake((Font_t*)font, (FontFace_t*)face, (FontBake_t*)bake, codepoint, (FontRange_t**)rangeOut);
}

bool GetFontInfoForCharAndSelector(Font_t* font, FontFaceSelector_t selector, u32 codepoint, FontFace_t** faceOut = nullptr, FontBake_t** bakeOut = nullptr, FontRange_t** rangeOut = nullptr, FontCharInfo_t** charInfoOut = nullptr)
{
	NotNull(font);
	FontFace_t* face = GetFontFace(font, selector);
	if (face == nullptr) { return false; }
	FontCharInfo_t* charInfo = nullptr;
	FontRange_t* range = nullptr;
	FontBake_t* bake = GetFontBakeForChar(font, face, codepoint, &charInfo, &range);
	if (bake == nullptr || range == nullptr || charInfo == nullptr) { return false; }
	
	if (faceOut != nullptr) { *faceOut = face; }
	if (bakeOut != nullptr) { *bakeOut = bake; }
	if (rangeOut != nullptr) { *rangeOut = range; }
	if (charInfoOut != nullptr) { *charInfoOut = charInfo; }
	return true;
}
bool GetFontInfoForCharAndSelector(const Font_t* font, FontFaceSelector_t selector, u32 codepoint, const FontFace_t** faceOut = nullptr, const FontBake_t** bakeOut = nullptr, const FontRange_t** rangeOut = nullptr, const FontCharInfo_t** charInfoOut = nullptr) //const-variant
{
	return GetFontInfoForCharAndSelector((Font_t*)font, selector, codepoint, (FontFace_t**)faceOut, (FontBake_t**)bakeOut, (FontRange_t**)rangeOut, (FontCharInfo_t**)charInfoOut);
}
