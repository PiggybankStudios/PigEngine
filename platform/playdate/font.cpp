/*
File:   font.cpp
Author: Taylor Robbins
Date:   09\09\2023
Description: 
	** Functions for Font_t structure (which wraps LCDFont*)
*/

Font_t LoadFont(MyStr_t path)
{
	MemArena_t* scratch = GetScratchArena();
	const char* loadFontErrorStr = nullptr;
	Font_t result = {};
	
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	result.font = pd->graphics->loadFont(pathNullTerm.chars, &loadFontErrorStr);
	if (result.font != nullptr)
	{
		result.lineHeight = pd->graphics->getFontHeight(result.font);
		
		result.numChars = 0;
		result.ranges = FontRange_None;
		result.partialRanges = FontRange_None;
		result.numRanges = 0;
		result.numPartialRanges = 0;
		for (u8 rangeIndex = 0; rangeIndex < FontRange_NumRanges; rangeIndex++)
		{
			FontRange_t range = FontRangeByIndex(rangeIndex);
			u8 numCharsInRange = GetNumCharsInFontRange(range);
			bool allCharsFound = true;
			bool anyCharsFound = false;
			for (u8 charIndex = 0; charIndex < numCharsInRange; charIndex++)
			{
				u32 codepoint = GetFontRangeChar(range, charIndex);
				if (codepoint == 0) { PrintLine_E("0 codepoint at %s[%u]", GetFontRangeStr(range), charIndex); }
				DebugAssert(codepoint != 0);
				LCDFontPage* fontPage = pd->graphics->getFontPage(result.font, codepoint);
				if (fontPage != nullptr)
				{
					LCDBitmap* glyphBitmap = nullptr;
					i32 glyphAdvance = 0;
					LCDFontGlyph* fontGlyph = pd->graphics->getPageGlyph(fontPage, codepoint, &glyphBitmap, &glyphAdvance);
					if (fontGlyph != nullptr)// && glyphBitmap != nullptr && glyphAdvance > 0)
					{
						result.numChars++;
						anyCharsFound = true;
					}
					else { allCharsFound = false; }
				}
				else { allCharsFound = false; }
			}
			
			if (allCharsFound) { result.ranges |= range; result.numRanges++; }
			if (anyCharsFound) { result.partialRanges |= range; result.numPartialRanges++; }
		}
		
		result.isValid = true;
	}
	else
	{
		PrintLine_E("Couldn't load font %s: %s", pathNullTerm.chars, loadFontErrorStr);
	}
	
	FreeScratchArena(scratch);
	return result;
}
