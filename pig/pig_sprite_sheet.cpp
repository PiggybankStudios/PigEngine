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

bool CreateSpriteSheet(MemArena_t* memArena, SpriteSheet_t* sheetOut, const PlatImageData_t* imageData, v2i padding, v2i numFrames, bool pixelated, bool useTextureArray)
{
	AssertSingleThreaded();
	NotNull(memArena);
	Assert(memArena != TempArena);
	NotNull(sheetOut);
	NotNull(imageData);
	Assert(imageData->width > 0 && imageData->height > 0);
	Assert(imageData->pixelSize == sizeof(u32));
	Assert(padding.x >= 0 && padding.y >= 0);
	Assert(numFrames.x > 0 && numFrames.y > 0);
	AssertIf(useTextureArray, padding.x == 0 && padding.y == 0); //we don't support padding in texture array backed sprite sheets (there's no need?)
	
	ClearPointer(sheetOut);
	sheetOut->isValid = false;
	sheetOut->allocArena = memArena;
	CreateVarArray(&sheetOut->frames, memArena, sizeof(SpriteSheetFrame_t), numFrames.x * numFrames.y);
	sheetOut->numFrames = numFrames;
	sheetOut->frameSize = NewVec2i(imageData->width / numFrames.x, imageData->height / numFrames.y);
	sheetOut->padding = padding;
	sheetOut->numFilledFrames = 0;
	
	if (useTextureArray)
	{
		TempPushMark();
		
		u64 totalNumFrames = (u64)(numFrames.x * numFrames.y);
		PlatImageData_t* frameDatas = TempArray(PlatImageData_t, totalNumFrames);
		NotNull(frameDatas);
		MyMemSet(frameDatas, 0x00, sizeof(PlatImageData_t) * totalNumFrames);
		
		u8 pixelSize = sizeof(u32);
		u64 framePixelsSize = pixelSize * (sheetOut->frameSize.width * sheetOut->frameSize.height);
		u64 pixelBufferSize = framePixelsSize * totalNumFrames;
		u32* pixelBuffer = AllocArray(mainHeap, u32, pixelBufferSize/pixelSize);
		NotNull(pixelBuffer);
		
		for (i32 frameY = 0; frameY < numFrames.y; frameY++)
		{
			for (i32 frameX = 0; frameX < numFrames.x; frameX++)
			{
				v2i gridPos = NewVec2i(frameX, frameY);
				v2i framePos = Vec2iMultiply(sheetOut->frameSize, gridPos);
				u64 frameIndex = (u64)((frameY * numFrames.x) + frameX);
				
				PlatImageData_t* frameData = &frameDatas[frameIndex];
				frameData->size = sheetOut->frameSize;
				frameData->pixelSize = pixelSize;
				frameData->rowSize = frameData->pixelSize * (u64)frameData->size.width;
				frameData->dataSize = frameData->rowSize * (u64)frameData->size.height;
				frameData->data32 = &pixelBuffer[frameIndex * (framePixelsSize / pixelSize)];
				Assert(((u8*)frameData->data32) + frameData->dataSize <= ((u8*)pixelBuffer) + pixelBufferSize);
				
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
						
						if (!frame->filled && ((*source) & 0xFF000000) != 0)
						{
							frame->filled = true;
							sheetOut->numFilledFrames++;
							// PrintLine_D("pixel (%d, %d) 0x%08X fills frame (%d, %d)", sourcePos.x, sourcePos.y, *source, frameX, frameY);
						}
						
						frameData->data32[(yPos * sheetOut->frameSize.width) + xPos] = *source;
					}
				}
			}
		}
		
		//TODO: Should we have the calling code decide if the textures should repeat? I think we may just always want this, it's part of the reason we would use a texture array in the first place
		const bool repeating = true;
		if (!CreateTextureArray(memArena, &sheetOut->texture, totalNumFrames, frameDatas, pixelated, repeating))
		{
			sheetOut->error = SpriteSheetError_TextureError;
			DestroySpriteSheet(sheetOut);
			FreeMem(mainHeap, pixelBuffer, pixelBufferSize);
			TempPopMark();
			return false;
		}
		
		FreeMem(mainHeap, pixelBuffer, pixelBufferSize);
		TempPopMark();
	}
	else
	{
		v2i frameSizeWithPadding = sheetOut->frameSize + padding*2;
		v2i textureSizeWithPadding = padding*2 + Vec2iMultiply(frameSizeWithPadding, numFrames);
		
		u64 newPixelsSize = sizeof(u32) * textureSizeWithPadding.width*textureSizeWithPadding.height;
		u32* newPixels = AllocArray(mainHeap, u32, textureSizeWithPadding.width*textureSizeWithPadding.height);
		if (newPixels == nullptr)
		{
			DebugAssert(false);
			sheetOut->error = SpriteSheetError_AllocFailure;
			DestroySpriteSheet(sheetOut);
			FreeMem(mainHeap, newPixels, newPixelsSize);
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
		
		const bool repeating = false;
		if (!CreateTexture(memArena, &sheetOut->texture, &paddedImageData, pixelated, repeating))
		{
			sheetOut->error = SpriteSheetError_TextureError;
			DestroySpriteSheet(sheetOut);
			FreeMem(mainHeap, newPixels, newPixelsSize);
			return false;
		}
		
		FreeMem(mainHeap, newPixels, newPixelsSize);
	}
	
	sheetOut->isValid = true;
	sheetOut->id = pig->nextSpriteSheetId;
	pig->nextSpriteSheetId++;
	return true;
}

bool LoadSpriteSheet(MemArena_t* memArena, SpriteSheet_t* sheetOut, MyStr_t filePath, v2i padding, v2i numFrames, bool pixelated, bool useTextureArray)
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
	
	bool result = CreateSpriteSheet(memArena, sheetOut, &imageData, padding, numFrames, pixelated, useTextureArray);
	
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
const SpriteSheetFrame_t* TryGetSpriteSheetFrame(const SpriteSheet_t* sheet, v2i gridPos) //const variant
{
	return (const SpriteSheetFrame_t*)TryGetSpriteSheetFrame((SpriteSheet_t*)sheet, gridPos);
}
const SpriteSheetFrame_t* TryGetSpriteSheetFrame(const SpriteSheet_t* sheet, MyStr_t frameName) //const variant
{
	return (const SpriteSheetFrame_t*)TryGetSpriteSheetFrame((SpriteSheet_t*)sheet, frameName);
}

v2i GetSpriteSheetFrame(const SpriteSheet_t* sheet, MyStr_t frameName, bool assertOnFailure = false)
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
	AssertIfMsg(assertOnFailure, false, "Failed to find sprite frame by name");
	return Vec2i_Zero;
}

rec GetSpriteSheetFrameSourceRec(const SpriteSheet_t* sheet, v2i gridPos)
{
	NotNull(sheet);
	Assert(sheet->texture.numLayers <= 1);
	rec result = NewRec(
		ToVec2(Vec2iMultiply(gridPos, sheet->frameSize + sheet->padding*2) + sheet->padding),
		ToVec2(sheet->frameSize)
	);
	return result;
}
rec GetSpriteSheetFrameSourceRec(const SpriteSheet_t* sheet, MyStr_t frameName, bool assertOnFailure = true)
{
	NotNull(sheet);
	const SpriteSheetFrame_t* frame = TryGetSpriteSheetFrame(sheet, frameName);
	if (frame != nullptr)
	{
		return GetSpriteSheetFrameSourceRec(sheet, frame->gridPos);
	}
	AssertIfMsg(assertOnFailure, false, "Failed to find sprite frame by name");
	return Rec_Zero;
}

box GetSpriteSheetFrameArraySourceRec(const SpriteSheet_t* sheet, v2i gridPos)
{
	u64 frameIndex = (u64)((gridPos.y * sheet->numFramesX) + gridPos.x);
	box result = NewBox(
		0, 0, (r32)frameIndex,
		(r32)sheet->frameSize.width, (r32)sheet->frameSize.height, 0
	);
	return result;
}
box GetSpriteSheetFrameArraySourceRec(const SpriteSheet_t* sheet, MyStr_t frameName, bool assertOnFailure = true)
{
	NotNull(sheet);
	const SpriteSheetFrame_t* frame = TryGetSpriteSheetFrame(sheet, frameName);
	if (frame != nullptr)
	{
		return GetSpriteSheetFrameArraySourceRec(sheet, frame->gridPos);
	}
	AssertIfMsg(assertOnFailure, false, "Failed to find sprite frame by name");
	return Box_Zero;
}

//NOTE: "Encoded" means we use a 2D texture coordinate but we map it into a 3D sample coord by pretending that each texture exists at (2x the width of a single texture) * frameIndex
//      This gives us enough slop between texture coordinates for each frame that we generally won't get any bleed from antialiasing or floating point error
rec GetSpriteSheetFrameArrayEncodedSourceRec(const SpriteSheet_t* sheet, v2i gridPos)
{
	u64 frameIndex = (u64)((gridPos.y * sheet->numFramesX) + gridPos.x);
	rec result = NewRec(
		(r32)(frameIndex * sheet->frameSize.width * 2) + (sheet->frameSize.width / 2.0f), 0,
		(r32)sheet->frameSize.width, (r32)sheet->frameSize.height
	);
	return result;
}
rec GetSpriteSheetFrameArrayEncodedSourceRec(const SpriteSheet_t* sheet, MyStr_t frameName, bool assertOnFailure = true)
{
	NotNull(sheet);
	const SpriteSheetFrame_t* frame = TryGetSpriteSheetFrame(sheet, frameName);
	if (frame != nullptr)
	{
		return GetSpriteSheetFrameArrayEncodedSourceRec(sheet, frame->gridPos);
	}
	AssertIfMsg(assertOnFailure, false, "Failed to find sprite frame by name");
	return Rec_Zero;
}
