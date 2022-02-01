/*
File:   win32_fonts.cpp
Author: Taylor Robbins
Date:   10\16\2021
Description: 
	** Holds a bunch of functions that help us deal with finding, opening, parsing, and baking fonts to bitmaps
*/

// +==============================+
// |      Win32_FreeFontData      |
// +==============================+
// void FreeFontData(PlatFontData_t* fontData)
PLAT_API_FREE_FONT_DATA_DEF(Win32_FreeFontData)
{
	NotNull(fontData);
	Win32_FreeImageData(&fontData->imageData);
	if (fontData->ranges != nullptr)
	{
		Assert(fontData->numRanges > 0);
		for (u64 rIndex = 0; rIndex < fontData->numRanges; rIndex++)
		{
			PlatFontRange_t* range = &fontData->ranges[rIndex];
			Assert(range->numChars > 0);
			if (range->codePointList != nullptr)
			{
				FreeMem(&Platform->stdHeap, range->codePointList, sizeof(u32) * range->numChars);
			}
		}
		FreeMem(&Platform->stdHeap, fontData->ranges, sizeof(PlatFontRange_t) * fontData->numRanges);
	}
	if (fontData->charData != nullptr)
	{
		Assert(fontData->numChars > 0);
		FreeMem(&Platform->stdHeap, fontData->charData, sizeof(PlatFontCharData_t) * fontData->numChars);
	}
	ClearPointer(fontData);
}

// +==============================+
// |    Win32_ReadPlatformFont    |
// +==============================+
// bool ReadPlatformFont(MyStr_t fontName, i32 fontSize, bool bold, bool italic, PlatFileContents_t* fileContentsOut)
PLAT_API_READ_PLATFORM_FONT_DEF(Win32_ReadPlatformFont)
{
	AssertSingleThreaded();
	NotNull(fileContentsOut);
	NotNullStr(&fontName);
	AssertNullTerm(&fontName);
	PlatFileContents_t result = {};
	
	//TODO: Call EnumFontFamiliesEx first to determine which font name exactly we will be getting
	
	HFONT fontHandle = CreateFontA(
		fontSize,                   //height
		0,                          //width
		0,                          //escapement
		0,                          //orientation
		bold ? FW_BOLD : FW_NORMAL, //weight
		italic,                     //italic
		false,                      //underline
		false,                      //strikeout
		ANSI_CHARSET,               //charset
		OUT_DEFAULT_PRECIS,         //output precision
		CLIP_DEFAULT_PRECIS,        //clipping precision
		DEFAULT_QUALITY,            //quality
		DEFAULT_PITCH|FF_DONTCARE,  //pitch and family
		fontName.pntr               //Face Name
	);
	if (fontHandle == NULL)
	{
		PrintLine_E("Failed to find font with name \"%s\"", fontName.pntr);
		return false;
	}
	
	PlatFileContents_t fontFile = {};
	
	Assert(Platform->winDeviceContext != NULL);
	HGDIOBJ selectResult = SelectObject(Platform->winDeviceContext, fontHandle);
	if (selectResult == NULL)
	{
		PrintLine_E("Failed to select platform font after creation! \"%s\"", fontName.pntr);
		BOOL deleteResult = DeleteObject(fontHandle);
		DebugAssertAndUnused(deleteResult != 0, deleteResult);
		return false;
	}
	
	DWORD getFontDataResult = GetFontData(
		Platform->winDeviceContext, //hardware device context
		0,                          //table
		0,                          //offset
		nullptr,                    //buffer
		0                           //buffer size
	);
	if (getFontDataResult == GDI_ERROR)
	{
		PrintLine_E("Failed to get/measure platform font contents for font \"%s\"", fontName.pntr);
		BOOL deleteResult = DeleteObject(fontHandle);
		DebugAssertAndUnused(deleteResult != 0, deleteResult);
		return false;
	}
	if (getFontDataResult <= 0)
	{
		PrintLine_E("There was no file contents for font \"%s\"", fontName.pntr);
		BOOL deleteResult = DeleteObject(fontHandle);
		DebugAssertAndUnused(deleteResult != 0, deleteResult);
		return false;
	}
	
	u64 fontDataSize = (u64)getFontDataResult;
	u8* fontDataPntr = AllocArray(&Platform->stdHeap, u8, fontDataSize);
	if (fontDataPntr == nullptr)
	{
		PrintLine_E("Failed to allocate space to store font file data for font \"%s\": %llu bytes", fontName.pntr, fontDataSize);
		BOOL deleteResult = DeleteObject(fontHandle);
		DebugAssertAndUnused(deleteResult != 0, deleteResult);
		return false;
	}
	
	getFontDataResult = GetFontData(
		Platform->winDeviceContext, //hardware device context
		0,                          //table
		0,                          //offset
		fontDataPntr,               //buffer
		(DWORD)fontDataSize         //buffer size
	);
	Assert(getFontDataResult == (DWORD)fontDataSize);
	
	BOOL deleteResult = DeleteObject(fontHandle);
	DebugAssertAndUnused(deleteResult != 0, deleteResult);
	
	fileContentsOut->id = Platform->nextFileContentsId;
	Platform->nextFileContentsId++;
	fileContentsOut->path = AllocString(&Platform->stdHeap, &fontName);
	NotNullStr(&fileContentsOut->path);
	fileContentsOut->data = fontDataPntr;
	fileContentsOut->size = fontDataSize;
	return true;
}

// +==============================+
// |        Win32_BakeFont        |
// +==============================+
// bool BakeFont(PlatFileContents_t* fontFile, v2i bakeSize, bool expandTo32bit, u64 numRanges, PlatFontRange_t* ranges, PlatFontData_t* fontDataOut, PlatRectPackContext_t* packContextOut)
PLAT_API_BAKE_FONT_DEF(Win32_BakeFont)
{
	AssertSingleThreaded();
	NotNull(fontFile);
	NotNull(fontFile->data);
	NotNull(fontDataOut);
	Assert(numRanges > 0);
	Assert(numRanges <= INT_MAX);
	NotNull(ranges);
	
	PlatImageData_t imageData = {};
	imageData.id = Platform->nextImageId;
	Platform->nextImageId++;
	imageData.allocArena = &Platform->stdHeap;
	imageData.size = bakeSize;
	imageData.pixelSize = (expandTo32bit ? sizeof(u32) : sizeof(u8));
	imageData.rowSize = (imageData.pixelSize * imageData.width);
	imageData.dataSize = (imageData.rowSize * imageData.height);
	imageData.data8 = AllocArray(imageData.allocArena, u8, imageData.dataSize);
	if (imageData.data8 == nullptr)
	{
		PrintLine_E("Failed to allocate space for pixel data for font bake of \"%s\": size (%d, %d)%s", fontFile->path.pntr, bakeSize.width, bakeSize.height, expandTo32bit ? " 32-bit" : "");
		return false;
	}
	
	stbtt_pack_context packContext = {};
	int beginResult = stbtt_PackBegin(
		&packContext,                                //context
		imageData.data8,                             //pixels
		(int)imageData.width, (int)imageData.height, //width, height
		(int)imageData.width * sizeof(u8),           //stride_in_bytes
		1,                                           //padding (between chars)
		&Platform->stdHeap                           //alloc_context
	);
	UNUSED(beginResult);//TODO: We should probably check or assert on this
	
	//TODO: Should we call this?
	// stbtt_PackSetOversample() //-- for improved quality on small fonts
	
	stbtt_pack_range* stbRanges = AllocArray(&Platform->stdHeap, stbtt_pack_range, numRanges);
	NotNull(stbRanges);
	u64 totalNumChars = 0;
	for (u64 rIndex = 0; rIndex < numRanges; rIndex++)
	{
		stbRanges[rIndex].font_size = (float)ranges[rIndex].fontSize;
		if (ranges[rIndex].codePointList != nullptr)
		{
			Assert(ranges[rIndex].firstCodepoint == 0);
			stbRanges[rIndex].first_unicode_codepoint_in_range = 0;
			stbRanges[rIndex].array_of_unicode_codepoints = AllocArray(&Platform->stdHeap, int, ranges[rIndex].numChars);
			NotNull(stbRanges[rIndex].array_of_unicode_codepoints);
			for (u64 cIndex = 0; cIndex < ranges[rIndex].numChars; cIndex++)
			{
				Assert(ranges[rIndex].codePointList[cIndex] <= INT_MAX);
				stbRanges[rIndex].array_of_unicode_codepoints[cIndex] = (int)ranges[rIndex].codePointList[cIndex];
			}
		}
		else
		{
			Assert(ranges[rIndex].firstCodepoint != 0);
			Assert(ranges[rIndex].firstCodepoint <= INT_MAX);
			stbRanges[rIndex].first_unicode_codepoint_in_range = (int)ranges[rIndex].firstCodepoint;
			stbRanges[rIndex].array_of_unicode_codepoints = nullptr;
		}
		Assert(ranges[rIndex].numChars <= INT_MAX);
		stbRanges[rIndex].num_chars = (int)ranges[rIndex].numChars;
		
		stbRanges[rIndex].chardata_for_range = AllocArray(&Platform->stdHeap, stbtt_packedchar, ranges[rIndex].numChars);
		NotNull(stbRanges[rIndex].chardata_for_range);
		totalNumChars += ranges[rIndex].numChars;
	}
	
	int packResult = stbtt_PackFontRanges(&packContext,
		fontFile->data, //fontdata
		0,              //font_index
		&stbRanges[0],  //ranges
		(int)numRanges  //num_ranges
	);
	if (packResult <= 0)
	{
		PrintLine_E("Failed to pack %llu font range(s) for font \"%s\" at %.1f in (%d, %d) bitmap. Result: %d",
			numRanges,
			fontFile->path.pntr,
			ranges[0].fontSize,
			bakeSize.width, bakeSize.height,
			packResult
		);
		for (u64 rIndex = 0; rIndex < ArrayCount(stbRanges); rIndex++)
		{
			if (stbRanges[rIndex].array_of_unicode_codepoints != nullptr) { FreeMem(&Platform->stdHeap, stbRanges[rIndex].array_of_unicode_codepoints, sizeof(int) * stbRanges[rIndex].num_chars); }
			FreeMem(&Platform->stdHeap, stbRanges[rIndex].chardata_for_range, sizeof(stbtt_packedchar) * stbRanges[rIndex].num_chars);
		}
		Win32_FreeImageData(&imageData);
		return false;
	}
	
	// TODO: Do we want to use this function and glean some sort of sub-pixel alignment information from it?
	// void stbtt_GetPackedQuad(stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer)
	
	if (packContextOut != nullptr)
	{
		ClearPointer(packContextOut);
		packContextOut->id = Platform->nextPackContextId;
		Platform->nextPackContextId++;
		packContextOut->packSize = bakeSize;
		NotNull(packContext.pack_info);
		MyMemCopy(&packContextOut->stbContext, (stbrp_context*)packContext.pack_info, sizeof(stbrp_context));
	}
	
	stbtt_PackEnd(&packContext);
	
	if (expandTo32bit)
	{
		//NOTE: We go backwards because the image data is garunteed to expand in size and going backwards makes sure
		//      the writePos never catches up to the readPos
		for (i32 yPos = imageData.height; yPos > 0; yPos--)
		{
			for (i32 xPos = imageData.width; xPos > 0; xPos--)
			{
				u8* readPntr       =           &imageData.data8 [(yPos-1) * imageData.width + (xPos-1)];
				Color_t* writePntr = (Color_t*)&imageData.data32[(yPos-1) * imageData.width + (xPos-1)];
				writePntr->value = 0xFFFFFFFFUL;
				writePntr->a = *readPntr;
			}
		}
	}
	
	fontDataOut->id = Platform->nextFontDataId;
	Platform->nextFontDataId++;
	fontDataOut->imageData = imageData;
	fontDataOut->numChars = totalNumChars;
	fontDataOut->charData = AllocArray(&Platform->stdHeap, PlatFontCharData_t, totalNumChars);
	NotNull(fontDataOut->charData);
	fontDataOut->numRanges = numRanges;
	fontDataOut->ranges = AllocArray(&Platform->stdHeap, PlatFontRange_t, numRanges);
	NotNull(fontDataOut->ranges);
	u64 cIndex = 0;
	for (u64 rIndex = 0; rIndex < numRanges; rIndex++)
	{
		stbtt_pack_range* stbRange = &stbRanges[rIndex];
		PlatFontRange_t* originalRange = &ranges[rIndex];
		PlatFontRange_t* outRange = &fontDataOut->ranges[rIndex];
		MyMemCopy(outRange, originalRange, sizeof(PlatFontRange_t));
		if (originalRange->codePointList != nullptr)
		{
			outRange->codePointList = AllocArray(&Platform->stdHeap, u32, originalRange->numChars);
			NotNull(outRange->codePointList);
			MyMemCopy(outRange->codePointList, originalRange->codePointList, sizeof(u32) * originalRange->numChars);
		}
		Assert(stbRange->num_chars == (int)outRange->numChars);
		NotNull(stbRange->chardata_for_range);
		for (u64 rcIndex = 0; rcIndex < stbRange->num_chars; rcIndex++)
		{
			stbtt_packedchar* stbChar = &stbRange->chardata_for_range[rcIndex];
			Assert(cIndex < totalNumChars);
			PlatFontCharData_t* charData = &fontDataOut->charData[cIndex];
			if (stbRange->array_of_unicode_codepoints != nullptr)
			{
				Assert(stbRange->array_of_unicode_codepoints[rcIndex] >= 0);
				charData->codepoint = (u32)stbRange->array_of_unicode_codepoints[rcIndex];
			}
			else
			{
				charData->codepoint = (u32)(stbRange->first_unicode_codepoint_in_range + rcIndex);
			}
			charData->sourceRec = NewReci(stbChar->x0, stbChar->y0, stbChar->x1 - stbChar->x0, stbChar->y1 - stbChar->y0);
			charData->offset = NewVec2(stbChar->xoff, stbChar->yoff);
			charData->offset2 = NewVec2(stbChar->xoff2, stbChar->yoff2);
			charData->advanceX = stbChar->xadvance;
			cIndex++;
		}
		if (stbRange->array_of_unicode_codepoints != nullptr) { FreeMem(&Platform->stdHeap, stbRange->array_of_unicode_codepoints, sizeof(int) * stbRange->num_chars); }
		FreeMem(&Platform->stdHeap, stbRange->chardata_for_range, sizeof(stbtt_packedchar) * stbRange->num_chars);
	}
	FreeMem(&Platform->stdHeap, stbRanges, sizeof(stbtt_pack_range) * numRanges);
	
	return true;
}
