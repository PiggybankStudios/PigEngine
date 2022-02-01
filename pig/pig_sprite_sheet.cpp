/*
File:   pig_sprite_sheet.cpp
Author: Taylor Robbins
Date:   01\17\2022
Description: 
	** Holds the functions that help us load sprite sheets and query
	** or manipulate them in various ways. Sprite sheets are a single
	** texture which holds an array of sprites in a fixed frame size grid
*/

void DestroySpriteSheet(SpriteSheet_t* sheet)
{
	NotNull(sheet);
	VarArrayLoop(&sheet->frames, fIndex)
	{
		VarArrayLoopGet(SpriteSheetFrame_t, frame, &sheet->frames, fIndex);
		VarArrayLoop(&frame->points, pIndex)
		{
			VarArrayLoopGet(SpriteSheetFramePoint_t, point, &frame->points, pIndex);
			FreeString(sheet->allocArena, &point->name);
		}
		FreeVarArray(&frame->points);
		FreeString(sheet->allocArena, &frame->name);
	}
	FreeVarArray(&sheet->frames);
	DestroyTexture(&sheet->texture);
	SpriteSheetError_t error = sheet->error;
	TextureError_t textureError = sheet->texture.error;
	ClearPointer(sheet);
	sheet->error = error;
	sheet->texture.error = textureError;
}

bool CreateSpriteSheet(MemArena_t* memArena, SpriteSheet_t* sheetOut, const PlatImageData_t* imageData, v2i padding, v2i numFrames, bool pixelated)
{
	AssertSingleThreaded();
	NotNull(memArena);
	NotNull(sheetOut);
	NotNull(imageData);
	Assert(imageData->width > 0 && imageData->height > 0);
	Assert(imageData->pixelSize == sizeof(u32));
	Assert(padding.x >= 0 && padding.y >= 0);
	Assert(numFrames.x > 0 && numFrames.y > 0);
	
	ClearPointer(sheetOut);
	sheetOut->isValid = false;
	sheetOut->allocArena = memArena;
	CreateVarArray(&sheetOut->frames, memArena, sizeof(SpriteSheetFrame_t), numFrames.x * numFrames.y);
	sheetOut->numFrames = numFrames;
	sheetOut->frameSize = NewVec2i(imageData->width / numFrames.x, imageData->height / numFrames.y);
	sheetOut->padding = padding;
	sheetOut->numFilledFrames = 0;
	
	v2i frameSizeWithPadding = sheetOut->frameSize + padding*2;
	v2i textureSizeWithPadding = Vec2iMultiply(frameSizeWithPadding, numFrames);
	
	TempPushMark();
	
	u32* newPixels = AllocArray(TempArena, u32, textureSizeWithPadding.width*textureSizeWithPadding.height);
	if (newPixels == nullptr)
	{
		DebugAssert(false);
		sheetOut->error = SpriteSheetError_AllocFailure;
		DestroySpriteSheet(sheetOut);
		TempPopMark();
		return false;
	}
	MyMemSet(newPixels, 0x00, sizeof(u32)*textureSizeWithPadding.width*textureSizeWithPadding.height);
	
	for (i32 frameY = 0; frameY < numFrames.y; frameY++)
	{
		for (i32 frameX = 0; frameX < numFrames.x; frameX++)
		{
			v2i gridPos = NewVec2i(frameX, frameY);
			v2i framePos = Vec2iMultiply(sheetOut->frameSize, gridPos);
			v2i framePosWithPadding = Vec2iMultiply(frameSizeWithPadding, gridPos) + padding;
			
			SpriteSheetFrame_t* frame = VarArrayAdd(&sheetOut->frames, SpriteSheetFrame_t);
			NotNull(frame);
			ClearPointer(frame);
			frame->index = sheetOut->frames.length-1;
			frame->gridPos = gridPos;
			frame->filled = false;
			CreateVarArray(&frame->points, memArena, sizeof(SpriteSheetFramePoint_t));
			
			for (i32 yPos = 0; yPos < sheetOut->frameSize.y; yPos++)
			{
				for (i32 xPos = 0; xPos < sheetOut->frameSize.x; xPos++)
				{
					v2i sourcePos = framePos + NewVec2i(xPos, yPos);
					const u32* source = &(imageData->data32)[(sourcePos.y*imageData->rowSize)/sizeof(u32) + sourcePos.x];
					
					v2i newPos = framePosWithPadding + NewVec2i(xPos, yPos);
					if (!frame->filled && ((*source) & 0xFF000000) != 0)
					{
						frame->filled = true;
						sheetOut->numFilledFrames++;
						// PrintLine_D("pixel (%d, %d) 0x%08X fills frame (%d, %d)", sourcePos.x, sourcePos.y, *source, frameX, frameY);
					}
					newPixels[(newPos.y * textureSizeWithPadding.width) + newPos.x] = *source;
					
					if (xPos == 0)
					{
						for (i32 padIndex = 1; padIndex <= sheetOut->padding.x; padIndex++)
						{
							v2i padPos = newPos + NewVec2i(-padIndex, 0);
							newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
						}
					}
					if (yPos == 0)
					{
						for (i32 padIndex = 1; padIndex <= sheetOut->padding.x; padIndex++)
						{
							v2i padPos = newPos + NewVec2i(0, -padIndex);
							newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
						}
					}
					if (xPos == sheetOut->frameSize.width-1)
					{
						for (i32 padIndex = 1; padIndex <= sheetOut->padding.x; padIndex++)
						{
							v2i padPos = newPos + NewVec2i(padIndex, 0);
							newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
						}
					}
					if (yPos == sheetOut->frameSize.height-1)
					{
						for (i32 padIndex = 1; padIndex <= sheetOut->padding.x; padIndex++)
						{
							v2i padPos = newPos + NewVec2i(0, padIndex);
							newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
						}
					}
					
					if (xPos == 0 && yPos == 0)
					{
						for (i32 padY = 1; padY <= sheetOut->padding.y; padY++)
						{
							for (i32 padX = 1; padX <= sheetOut->padding.x; padX++)
							{
								v2i padPos = newPos + NewVec2i(-padX, -padY);
								newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
							}
						}
					}
					if (xPos == sheetOut->frameSize.width-1 && yPos == 0)
					{
						for (i32 padY = 1; padY <= sheetOut->padding.y; padY++)
						{
							for (i32 padX = 1; padX <= sheetOut->padding.x; padX++)
							{
								v2i padPos = newPos + NewVec2i(padX, -padY);
								newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
							}
						}
					}
					if (xPos == 0 && yPos == sheetOut->frameSize.height-1)
					{
						for (i32 padY = 1; padY <= sheetOut->padding.y; padY++)
						{
							for (i32 padX = 1; padX <= sheetOut->padding.x; padX++)
							{
								v2i padPos = newPos + NewVec2i(-padX, padY);
								newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
							}
						}
					}
					if (xPos == sheetOut->frameSize.width-1 && yPos == sheetOut->frameSize.height-1)
					{
						for (i32 padY = 1; padY <= sheetOut->padding.y; padY++)
						{
							for (i32 padX = 1; padX <= sheetOut->padding.x; padX++)
							{
								v2i padPos = newPos + NewVec2i(padX, padY);
								newPixels[(padPos.y * textureSizeWithPadding.width) + padPos.x] = *source;
							}
						}
					}
				}
			}
		}
	}
	
	PlatImageData_t paddedImageData = {};
	paddedImageData.size = textureSizeWithPadding;
	paddedImageData.pixelSize = sizeof(u32);
	paddedImageData.rowSize = paddedImageData.pixelSize * paddedImageData.width;
	paddedImageData.dataSize = paddedImageData.rowSize * paddedImageData.height;
	paddedImageData.data32 = newPixels;
	
	if (!CreateTexture(memArena, &sheetOut->texture, &paddedImageData, pixelated, false))
	{
		sheetOut->error = SpriteSheetError_TextureError;
		DestroySpriteSheet(sheetOut);
		TempPopMark();
		return false;
	}
	
	TempPopMark();
	
	sheetOut->isValid = true;
	sheetOut->id = pig->nextSpriteSheetId;
	pig->nextSpriteSheetId++;
	return true;
}

bool LoadSpriteSheet(MemArena_t* memArena, SpriteSheet_t* sheetOut, MyStr_t filePath, v2i padding, v2i numFrames, bool pixelated)
{
	NotNull(memArena);
	NotNull(sheetOut);
	NotEmptyStr(&filePath);
	
	PlatFileContents_t textureFile;
	if (!plat->ReadFileContents(filePath, &textureFile))
	{
		sheetOut->error = SpriteSheetError_CouldntOpenFile;
		return false;
	}
	if (textureFile.size == 0)
	{
		sheetOut->error = SpriteSheetError_EmptyFile;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	NotNull(textureFile.data);
	
	PlatImageData_t imageData;
	if (!plat->TryParseImageFile(&textureFile, sizeof(u32), &imageData))
	{
		sheetOut->error = SpriteSheetError_ParseFailure;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	
	bool result = CreateSpriteSheet(memArena, sheetOut, &imageData, padding, numFrames, pixelated);
	
	plat->FreeImageData(&imageData);
	plat->FreeFileContents(&textureFile);
	
	return result;
}

SpriteSheetFrame_t* TryGetSpriteSheetFrame(SpriteSheet_t* sheet, v2i gridPos)
{
	NotNull(sheet);
	VarArrayLoop(&sheet->frames, fIndex)
	{
		VarArrayLoopGet(SpriteSheetFrame_t, frame, &sheet->frames, fIndex);
		if (frame->gridPos == gridPos)
		{
			return frame;
		}
	}
	return nullptr;
}

SpriteSheetFrame_t* TryGetSpriteSheetFrame(SpriteSheet_t* sheet, MyStr_t frameName)
{
	NotNull(sheet);
	VarArrayLoop(&sheet->frames, fIndex)
	{
		VarArrayLoopGet(SpriteSheetFrame_t, frame, &sheet->frames, fIndex);
		if (StrCompareIgnoreCase(frame->name, frameName) == 0)
		{
			return frame;
		}
	}
	return nullptr;
}
v2i GetSpriteSheetFrame(SpriteSheet_t* sheet, MyStr_t frameName, bool assertOnFailure = false)
{
	NotNull(sheet);
	VarArrayLoop(&sheet->frames, fIndex)
	{
		VarArrayLoopGet(SpriteSheetFrame_t, frame, &sheet->frames, fIndex);
		if (StrCompareIgnoreCase(frame->name, frameName) == 0)
		{
			return frame->gridPos;
		}
	}
	AssertIf(assertOnFailure, false);
	return Vec2i_Zero;
}
