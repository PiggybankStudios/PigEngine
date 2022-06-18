/*
File:   pig_resources.cpp
Author: Taylor Robbins
Date:   11\04\2021
Description: 
	** Holds a bunch of functions that help us manage loading and unloading resources from disk
*/

#define INTERNATIONAL_FONT_NAME "Yu Mincho"

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void Pig_InitResources()
{
	ClearStruct(pig->resources);
	#if DEBUG_BUILD
	CreateVarArray(&pig->resources.watches, fixedHeap, sizeof(ResourceWatch_t), TOTAL_NUM_RESOURCES);
	#endif
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
u64 GetNumResourcesOfType(ResourceType_t resourceType)
{
	switch (resourceType)
	{
		case ResourceType_Texture: return RESOURCES_NUM_TEXTURES;
		case ResourceType_Sheet:   return RESOURCES_NUM_SHEETS;
		case ResourceType_Shader:  return RESOURCES_NUM_SHADERS;
		case ResourceType_Font:    return RESOURCES_NUM_FONTS;
		default: return 0;
	}
}

// +--------------------------------------------------------------+
// |                            Watch                             |
// +--------------------------------------------------------------+
#if DEBUG_BUILD
void StopWatchingFilesForResource(ResourceType_t resourceType, u64 resourceIndex)
{
	for (u64 wIndex = 0; wIndex < pig->resources.watches.length; )
	{
		ResourceWatch_t* watch = VarArrayGet(&pig->resources.watches, wIndex, ResourceWatch_t);
		if (watch->type == resourceType && watch->resourceIndex == resourceIndex)
		{
			plat->UnwatchFile(watch->watchedFile);
			VarArrayRemove(&pig->resources.watches, wIndex, ResourceWatch_t);
			//don't increment wIndex
		}
		else
		{
			wIndex++;
		}
	}
}
ResourceWatch_t* WatchFileForResource(ResourceType_t resourceType, u64 resourceIndex, MyStr_t filePath)
{
	Assert(resourceType < ResourceType_NumTypes);
	Assert(resourceType != ResourceType_None);
	
	PlatWatchedFile_t* newWatch = plat->WatchFile(filePath);
	if (newWatch == nullptr) { return nullptr; }
	
	PrintLine_D("Watching resource file \"%.*s\"", filePath.length, filePath.pntr);
	ResourceWatch_t* newResourceWatch = VarArrayAdd(&pig->resources.watches, ResourceWatch_t);
	NotNull(newResourceWatch);
	ClearPointer(newResourceWatch);
	newResourceWatch->type = resourceType;
	newResourceWatch->resourceIndex = resourceIndex;
	newResourceWatch->watchedFile = newWatch;
	return newResourceWatch;
}
#else
#define StopWatchingFilesForResource(resourceType, resourceIndex) //nothing
#define WatchFileForResource(resourceType, resourceIndex, filePath) //nothing
#endif

// +--------------------------------------------------------------+
// |                             Load                             |
// +--------------------------------------------------------------+
// +==============================+
// |           Texture            |
// +==============================+
void Pig_LoadTextureResource(u64 textureIndex)
{
	Assert(textureIndex < RESOURCES_NUM_TEXTURES);
	const char* texturePath = Resources_GetPathForTexture(textureIndex);
	NotNull(texturePath);
	MyStr_t texturePathStr = NewStr(texturePath);
	Texture_t newTexture = {};
	if (!LoadTexture(fixedHeap, &newTexture, texturePathStr, true, true))
	{
		PrintLine_E("Failed to load texture[%u] from \"%s\"! Error %s%s%s",
			textureIndex,
			GetFileNamePart(texturePathStr).pntr,
			GetTextureErrorStr(newTexture.error),
			(newTexture.error == TextureError_ApiError) ? ": " : "",
			(newTexture.error == TextureError_ApiError) ? newTexture.apiErrorStr.pntr : ""
		);
		DestroyTexture(&newTexture);
		return;
	}
	Texture_t* texture = &pig->resources.textures[textureIndex];
	if (texture->isValid)
	{
		DestroyTexture(texture);
	}
	MyMemCopy(texture, &newTexture, sizeof(Texture_t));
	StopWatchingFilesForResource(ResourceType_Texture, textureIndex);
	WatchFileForResource(ResourceType_Texture, textureIndex, texturePathStr);
}
void Pig_LoadAllTextures()
{
	for (u64 textureIndex = 0; textureIndex < RESOURCES_NUM_TEXTURES; textureIndex++)
	{
		Pig_LoadTextureResource(textureIndex);
	}
}

// +==============================+
// |          VectorImg           |
// +==============================+
void Pig_LoadVectorImgResource(u64 vectorImgIndex)
{
	Assert(vectorImgIndex < RESOURCES_NUM_VECTORS);
	
	const char* vectorImgPath = Resources_GetPathForVectorImg(vectorImgIndex);
	NotNull(vectorImgPath);
	MyStr_t vectorImgPathStr = NewStr(vectorImgPath);
	
	PlatFileContents_t svgFile = {};
	if (!plat->ReadFileContents(vectorImgPathStr, &svgFile))
	{
		PrintLine_E("Failed to open vector image[%llu] at \"%s\"", vectorImgIndex, GetFileNamePart(vectorImgPathStr).pntr);
		DebugAssert(false);
		return;
	}
	
	u64 fixedHeapUsageBefore = fixedHeap->used; //TODO: Remove me!
	TempPushMark();
	ProcessLog_t svgParseLog = {};
	CreateProcessLog(&svgParseLog, Kilobytes(8), TempArena, fixedHeap);
	
	SvgData_t svgData = {};
	if (!TryDeserSvgFile(NewStr(svgFile.size, svgFile.chars), &svgParseLog, &svgData, mainHeap))
	{
		PrintLine_E("Failed to load vector image[%llu] from \"%s\"! Error %s%s%s%s",
			vectorImgIndex,
			GetFileNamePart(vectorImgPathStr).pntr,
			GetTryDeserSvgFileErrorStr((TryDeserSvgFileError_t)svgParseLog.errorCode),
			(svgParseLog.errorCode == TryDeserSvgFileError_XmlParsingError || svgParseLog.errorCode == TryDeserSvgFileError_TryParseError) ? ": " : "",
			(svgParseLog.errorCode == TryDeserSvgFileError_XmlParsingError) ? GetXmlParsingErrorStr(svgParseLog.xmlParsingError) : "",
			(svgParseLog.errorCode == TryDeserSvgFileError_TryParseError) ? GetTryParseFailureReasonStr(svgParseLog.parseFailureReason) : ""
		);
		DumpProcessLog(&svgParseLog, "SVG Parse Log", DbgLevel_Warning);
		FreeProcessLog(&svgParseLog);
		FreeSvgData(&svgData);
		TempPopMark();
		plat->FreeFileContents(&svgFile);
		return;
	}
	if (svgParseLog.hadWarnings || svgParseLog.hadErrors)
	{
		DumpProcessLog(&svgParseLog, "SVG Parse Log", DbgLevel_Warning);
	}
	FreeProcessLog(&svgParseLog);
	plat->FreeFileContents(&svgFile);
	
	VectorImg_t newImage = {};
	if (!CreateVectorImgFromSvg(fixedHeap, &newImage, &svgData))
	{
		PrintLine_E("Failed to load vector image[%llu] from \"%s\"!", vectorImgIndex, GetFileNamePart(vectorImgPathStr).pntr);
		DestroyVectorImg(&newImage);
		FreeSvgData(&svgData);
		TempPopMark();
		return;
	}
	FreeSvgData(&svgData);
	
	VectorImg_t* image = &pig->resources.vectorImgs[vectorImgIndex];
	if (image->isValid)
	{
		DestroyVectorImg(image);
	}
	
	MyMemCopy(image, &newImage, sizeof(VectorImg_t));
	StopWatchingFilesForResource(ResourceType_VectorImage, vectorImgIndex);
	WatchFileForResource(ResourceType_VectorImage, vectorImgIndex, vectorImgPathStr);
	
	TempPopMark();
	u64 fixedHeapUsageAfter = fixedHeap->used; //TODO: Remove me!
	PrintLine_D("Vector image[%llu] took %s", vectorImgIndex, FormatBytesNt(fixedHeapUsageAfter - fixedHeapUsageBefore, TempArena));
}
void Pig_LoadAllVectorImgs()
{
	for (u64 vectorImgIndex = 0; vectorImgIndex < RESOURCES_NUM_VECTORS; vectorImgIndex++)
	{
		Pig_LoadVectorImgResource(vectorImgIndex);
	}
}

// +==============================+
// |         SpriteSheet          |
// +==============================+
void Pig_LoadSpriteSheetResource(u64 sheetIndex)
{
	Assert(sheetIndex < RESOURCES_NUM_SHEETS);
	SpriteSheet_t* sheet = &pig->resources.sheets[sheetIndex];
	
	ResourceSheetMetaInfo_t metaInfo = {};
	const char* sheetPath = Resources_GetPathForSheet(sheetIndex, &metaInfo);
	NotNull(sheetPath);
	MyStr_t sheetPathStr = NewStr(sheetPath);
	
	SpriteSheet_t tempSheet = {};
	if (!LoadSpriteSheet(mainHeap, &tempSheet, sheetPathStr, metaInfo.padding, metaInfo.numFrames, metaInfo.pixelated))
	{
		NotifyPrint_E("Failed to load sheet[%u] from \"%s\"! Error %s%s%s",
			sheetIndex,
			GetFileNamePart(sheetPathStr).pntr,
			GetSpriteSheetErrorStr(tempSheet.error),
			(tempSheet.error == SpriteSheetError_TextureError) ? ": " : "",
			(tempSheet.error == SpriteSheetError_TextureError) ? GetTextureErrorStr(tempSheet.texture.error) : ""
		);
		return;
	}
	
	if (sheet->isValid) { DestroySpriteSheet(sheet); }
	MyMemCopy(sheet, &tempSheet, sizeof(SpriteSheet_t));
	
	if (!IsEmptyStr(metaInfo.metaFilePath))
	{
		PlatFileContents_t metaFile = {};
		if (plat->ReadFileContents(metaInfo.metaFilePath, &metaFile))
		{
			ProcessLog_t metaParseLog = {};
			CreateProcessLog(&metaParseLog, Kilobytes(8), TempArena, mainHeap);
			SetProcessLogFilePath(&metaParseLog, metaInfo.metaFilePath);
			
			if (TryDeserSpriteSheetMeta(NewStr(metaFile.length, metaFile.chars), sheet, &metaParseLog))
			{
				//Do nothing i guess
			}
			
			if (metaParseLog.hadWarnings || metaParseLog.hadErrors)
			{
				NotifyPrint_E("Sprite Sheet[%llu] meta file had parsing %s!", sheetIndex, (metaParseLog.hadErrors ? "errors" : "warnings"));
				DumpProcessLog(&metaParseLog, "Sprite Sheet Meta Parse Log", DbgLevel_Warning);
			}
			
			FreeProcessLog(&metaParseLog);
			plat->FreeFileContents(&metaFile);
		}
		else
		{
			NotifyPrint_E("Failed to load meta file for sheet[%llu] at \"%.*s\"", sheetIndex, metaInfo.metaFilePath.length, metaInfo.metaFilePath.pntr);
		}
	}
	
	StopWatchingFilesForResource(ResourceType_Sheet, sheetIndex);
	WatchFileForResource(ResourceType_Sheet, sheetIndex, sheetPathStr);
	if (!IsEmptyStr(metaInfo.metaFilePath)) 
	{
		WatchFileForResource(ResourceType_Sheet, sheetIndex, metaInfo.metaFilePath);
	}
}
void Pig_LoadAllSpriteSheets()
{
	for (u64 sheetIndex = 0; sheetIndex < RESOURCES_NUM_SHEETS; sheetIndex++)
	{
		Pig_LoadSpriteSheetResource(sheetIndex);
	}
}

// +==============================+
// |            Shader            |
// +==============================+
void Pig_LoadShaderResource(u64 shaderIndex)
{
	Assert(shaderIndex < RESOURCES_NUM_SHADERS);
	Shader_t* shader = &pig->resources.shaders[shaderIndex];
	
	ResourceShaderMetaInfo_t metaInfo;
	const char* shaderPath = Resources_GetPathForShader(shaderIndex, &metaInfo);
	NotNull(shaderPath);
	MyStr_t shaderPathStr = NewStr(shaderPath);
	
	Shader_t tempShader = {};
	if (!LoadShader(mainHeap, &tempShader, shaderPathStr, metaInfo.vertexType, metaInfo.requiredUniforms))
	{
		if (tempShader.error == ShaderError_MissingAttribute)
		{
			PrintLine_W("Loaded shader[%u] from %s but it is missing an attribute", shaderIndex, GetFileNamePart(shaderPathStr).pntr);
		}
		else if (tempShader.error == ShaderError_MissingUniform)
		{
			PrintLine_W("Loaded shader[%u] from %s but it is missing a uniform", shaderIndex, GetFileNamePart(shaderPathStr).pntr);
		}
		else
		{
			NotifyPrint_E("Failed to load shader[%u] from %s! Error %s%s%s", shaderIndex, GetFileNamePart(shaderPathStr).pntr, GetShaderErrorStr(tempShader.error), (tempShader.error == ShaderError_ApiError) ? ": " : "", (tempShader.error == ShaderError_ApiError) ? tempShader.apiErrorStr.pntr : "");
			DestroyShader(&tempShader);
			MyDebugBreak();
			return;
		}
	}
	
	if (shader->isValid) { DestroyShader(shader); }
	MyMemCopy(shader, &tempShader, sizeof(Shader_t));
	
	StopWatchingFilesForResource(ResourceType_Shader, shaderIndex);
	WatchFileForResource(ResourceType_Shader, shaderIndex, shaderPathStr);
}
void Pig_LoadAllShaders()
{
	for (u64 shaderIndex = 0; shaderIndex < RESOURCES_NUM_SHADERS; shaderIndex++)
	{
		Pig_LoadShaderResource(shaderIndex);
	}
}

// +==============================+
// |             Font             |
// +==============================+
bool TryLoadSpriteSheetAndMeta(MemArena_t* memArena, MyStr_t filePath, MyStr_t metaFilePath, v2i sheetSize, v2i padding, bool pixelated, SpriteSheet_t* spriteSheetOut, bool dumpLogOnFailure = true)
{
	NotNull(memArena);
	NotNull(spriteSheetOut);
	bool result = false;
	if (!LoadSpriteSheet(mainHeap, spriteSheetOut, filePath, padding, sheetSize, pixelated))
	{
		if (dumpLogOnFailure)
		{
			PrintLine_E("Failed to load sprite sheet from \"%.*s\"! Error %s%s%s",
				filePath.length, filePath.pntr,
				GetSpriteSheetErrorStr(spriteSheetOut->error),
				(spriteSheetOut->error == SpriteSheetError_TextureError) ? ": " : "",
				(spriteSheetOut->error == SpriteSheetError_TextureError) ? GetTextureErrorStr(spriteSheetOut->texture.error) : ""
			);
		}
		DebugAssert(false);
		return result;
	}
	
	if (!IsEmptyStr(metaFilePath))
	{
		PlatFileContents_t metaFile = {};
		if (plat->ReadFileContents(metaFilePath, &metaFile))
		{
			ProcessLog_t metaParseLog = {};
			CreateProcessLog(&metaParseLog, Kilobytes(8), TempArena, mainHeap);
			SetProcessLogFilePath(&metaParseLog, metaFilePath);
			if (TryDeserSpriteSheetMeta(NewStr(metaFile.length, metaFile.chars), spriteSheetOut, &metaParseLog))
			{
				result = true;
			}
			if (dumpLogOnFailure && (metaParseLog.hadWarnings || metaParseLog.hadErrors))
			{
				DumpProcessLog(&metaParseLog, "Sprite Sheet Meta Parse Log", DbgLevel_Warning);
			}
			FreeProcessLog(&metaParseLog);
			plat->FreeFileContents(&metaFile);
		}
		else if (dumpLogOnFailure)
		{
			PrintLine_E("Failed to open meta file for sprite sheet at \"%.*s\"", metaFilePath.length, metaFilePath.pntr);
		}
	}
	
	return result;
}
void Pig_LoadFontResource(u64 fontIndex)
{
	Assert(fontIndex < RESOURCES_NUM_FONTS);
	Font_t* font = &pig->resources.fonts[fontIndex];
	if (font->isValid) { DestroyFont(font); }
	
	ResourceFontMetaInfo_t metaInfo = {};
	const char* fontName = Resources_GetPathOrNameForFont(fontIndex, &metaInfo);
	NotNull(fontName);
	StartFont(font, mainHeap, 1); //TODO: Should we go through and figure out how many faces have a size > 0 and feed that value in here instead of 1
	
	StopWatchingFilesForResource(ResourceType_Font, fontIndex);
	
	Assert(metaInfo.faces[0].size > 0);
	for (u64 faceIndex = 0; faceIndex < MAX_NUM_RESOURCE_FONT_FACES; faceIndex++)
	{
		ResourceFontFaceMetaInfo_t* faceMetaInfo = &metaInfo.faces[faceIndex];
		if (faceMetaInfo->isSpriteFont)
		{
			FontFace_t* face = FontStartFace(font, faceMetaInfo->name, faceMetaInfo->size, faceMetaInfo->bold, faceMetaInfo->italic, 1);
			NotNull(face);
			
			u64 numBakesFound = 0;
			for (u64 bIndex = 0; bIndex < MAX_NUM_RESOURCE_FONT_BAKES; bIndex++)
			{
				MyStr_t pngFilePath = faceMetaInfo->filePaths[bIndex];
				MyStr_t metaFilePath = faceMetaInfo->metaFilePaths[bIndex];
				
				if (IsEmptyStr(pngFilePath)) { break; } //that's the end
				numBakesFound++;
				
				SpriteSheet_t spriteSheet = {};
				bool loadedSpriteSheet = TryLoadSpriteSheetAndMeta(
					mainHeap,
					pngFilePath, metaFilePath,
					faceMetaInfo->sheetSizes[bIndex], faceMetaInfo->paddings[bIndex], faceMetaInfo->isPixelated[bIndex],
					&spriteSheet
				);
				
				if (loadedSpriteSheet)
				{
					FontBake_t* spriteSheetBake = AddSpriteSheetAsFontBake(font, face, &spriteSheet, faceMetaInfo->scalables[bIndex], faceMetaInfo->colored[bIndex]);
					UNUSED(spriteSheetBake);
					DebugAssert(spriteSheetBake != nullptr);
					
					WatchFileForResource(ResourceType_Font, fontIndex, pngFilePath);
					WatchFileForResource(ResourceType_Font, fontIndex, metaFilePath);
				}
				
				DestroySpriteSheet(&spriteSheet);
			}
			DebugAssert(numBakesFound > 0);
			
			FontFinishFace(font, face);
		}
		else if (faceMetaInfo->size > 0)
		{
			FontFace_t* face = FontStartFace(font, NewStr(fontName), faceMetaInfo->size, faceMetaInfo->bold, faceMetaInfo->italic, 2);
			NotNull(face);
			
			PlatFontRange_t ranges[2] = {};
			ranges[0].fontSize = (float)faceMetaInfo->size; //TODO: This really shouldn't need to be defined
			ranges[1].fontSize = (float)faceMetaInfo->size; //TODO: This really shouldn't need to be defined
			
			ranges[0].firstCodepoint = 0x20;
			ranges[0].numChars = 0x7E - ranges[0].firstCodepoint;
			ranges[1].firstCodepoint = UNICODE_LATIN_EXT_START;
			ranges[1].numChars = UNICODE_LATIN_EXT_COUNT;
			FontBake_t* mainBake = FontAddBakeToActiveFace(font, face, faceMetaInfo->bakeSize, 2, &ranges[0]);
			UNUSED(mainBake);
			
			if (faceMetaInfo->includeCyrillicBake)
			{
				ranges[0].firstCodepoint = UNICODE_CYRILLIC_START;
				ranges[0].numChars = UNICODE_CYRILLIC_COUNT;
				FontBake_t* cyrillicBake = FontAddBakeToActiveFace(font, face, faceMetaInfo->bakeSize, 1, &ranges[0]);
				UNUSED(cyrillicBake);
			}
			
			if (faceMetaInfo->includeJapaneseKanaBake)
			{
				ChangeFontFileForActiveFace(font, face, NewStr(INTERNATIONAL_FONT_NAME), faceMetaInfo->size, faceMetaInfo->bold, faceMetaInfo->italic);
				ranges[0].firstCodepoint = UNICODE_HIRAGANA_START;
				ranges[0].numChars = UNICODE_KATAKANA_END - ranges[0].firstCodepoint;
				FontBake_t* japaneseBake = FontAddBakeToActiveFace(font, face, faceMetaInfo->bakeSize, 1, &ranges[0]);
				UNUSED(japaneseBake);
			}
			
			if (faceMetaInfo->includeBtnsSheet)
			{
				SpriteSheet_t spriteSheet = {};
				bool loadedSpriteSheet = false;
				if (face->fontSize < 24) //TODO: Tune this number! How tall does the line actually have to be before the high-res icons look good?
				{
					loadedSpriteSheet = TryLoadSpriteSheetAndMeta(
						mainHeap,
						NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns_white.png"), NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns.meta"),
						NewVec2i(16, 16), Vec2i_One, false,
						&spriteSheet
					);
				}
				else
				{
					loadedSpriteSheet = TryLoadSpriteSheetAndMeta(
						mainHeap,
						NewStr(RESOURCE_FOLDER_FONTS "/btns_light.png"), NewStr(RESOURCE_FOLDER_FONTS "/btns_light.meta"),
						NewVec2i(16, 16), Vec2i_One, false,
						&spriteSheet
					);
				}
				
				if (loadedSpriteSheet)
				{
					FontBake_t* spriteSheetBake = AddSpriteSheetAsFontBake(font, face, &spriteSheet, true, false);
					UNUSED(spriteSheetBake);
					DebugAssert(spriteSheetBake != nullptr);
				}
				
				DestroySpriteSheet(&spriteSheet);
			}
			
			//TODO: Do we want to watch any files so we can auto-reload this kind of font?
			
			FontFinishFace(font, face);
			if (faceIndex == 0) { MakeFontFaceDefault(font, face); }
		}
	}
	
	EndFont(font);
}
void Pig_LoadAllFonts()
{
	for (u64 fontIndex = 0; fontIndex < RESOURCES_NUM_FONTS; fontIndex++)
	{
		Pig_LoadFontResource(fontIndex);
	}
}

void Pig_LoadAllResources()
{
	Pig_LoadAllTextures();
	Pig_LoadAllVectorImgs();
	Pig_LoadAllShaders();
	Pig_LoadAllFonts();
	Pig_LoadAllSpriteSheets();
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Pig_UpdateResources()
{
	#if DEBUG_BUILD
	VarArrayLoop(&pig->resources.watches, wIndex)
	{
		VarArrayLoopGet(ResourceWatch_t, watch, &pig->resources.watches, wIndex);
		NotNull(watch->watchedFile);
		u32 watchChanged = plat->InterlockedExchange(&watch->watchedFile->changed, 0);
		if (watchChanged != 0)
		{
			switch (watch->type)
			{
				case ResourceType_Texture:
				{
					PrintLine_N("Reloading Texture[%llu]...", watch->resourceIndex);
					Pig_LoadTextureResource(watch->resourceIndex);
				} break;
				case ResourceType_VectorImage:
				{
					PrintLine_N("Reloading VectorImg[%llu]...", watch->resourceIndex);
					Pig_LoadVectorImgResource(watch->resourceIndex);
				} break;
				case ResourceType_Sheet:
				{
					PrintLine_N("Reloading Sheet[%llu]...", watch->resourceIndex);
					Pig_LoadSpriteSheetResource(watch->resourceIndex);
				} break;
				case ResourceType_Shader:
				{
					PrintLine_N("Reloading Shader[%llu]...", watch->resourceIndex);
					Pig_LoadShaderResource(watch->resourceIndex);
				} break;
				case ResourceType_Font:
				{
					PrintLine_N("Reloading Font[%llu]...", watch->resourceIndex);
					Pig_LoadFontResource(watch->resourceIndex);
				} break;
				default: DebugAssert(false); break;
			}
		}
	}
	#endif
}
