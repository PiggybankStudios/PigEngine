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
	#if DEVELOPER_BUILD
	CreateVarArray(&pig->resources.watches, fixedHeap, sizeof(ResourceWatch_t), TOTAL_NUM_RESOURCES);
	#endif
	
	pig->resources.numTexturesAlloc   = RESOURCES_NUM_TEXTURES;
	pig->resources.numVectorImgsAlloc = RESOURCES_NUM_VECTORS;
	pig->resources.numSheetsAlloc     = RESOURCES_NUM_SHEETS;
	pig->resources.numShadersAlloc    = RESOURCES_NUM_SHADERS;
	pig->resources.numFontsAlloc      = RESOURCES_NUM_FONTS;
	pig->resources.numSoundsAlloc     = RESOURCES_NUM_SOUNDS;
	pig->resources.numMusicsAlloc     = RESOURCES_NUM_MUSICS;
	
	Assert(sizeof(ResourceTextures_t) == sizeof(Texture_t)     * RESOURCES_NUM_TEXTURES);
	Assert(sizeof(ResourceVectors_t)  == sizeof(VectorImg_t)   * RESOURCES_NUM_VECTORS);
	Assert(sizeof(ResourceSheets_t)   == sizeof(SpriteSheet_t) * RESOURCES_NUM_SHEETS);
	Assert(sizeof(ResourceShaders_t)  == sizeof(Shader_t)      * RESOURCES_NUM_SHADERS);
	Assert(sizeof(ResourceFonts_t)    == sizeof(Font_t)        * RESOURCES_NUM_FONTS);
	Assert(sizeof(ResourceSounds_t)   == sizeof(Sound_t)       * RESOURCES_NUM_SOUNDS);
	Assert(sizeof(ResourceMusics_t)   == sizeof(Sound_t)       * RESOURCES_NUM_MUSICS);
	
	pig->resources.textures = (ResourceTextures_t*)AllocArray(fixedHeap, Texture_t,     RESOURCES_NUM_TEXTURES);
	pig->resources.vectors  =  (ResourceVectors_t*)AllocArray(fixedHeap, VectorImg_t,   RESOURCES_NUM_VECTORS);
	pig->resources.sheets   =   (ResourceSheets_t*)AllocArray(fixedHeap, SpriteSheet_t, RESOURCES_NUM_SHEETS);
	pig->resources.shaders  =  (ResourceShaders_t*)AllocArray(fixedHeap, Shader_t,      RESOURCES_NUM_SHADERS);
	pig->resources.fonts    =    (ResourceFonts_t*)AllocArray(fixedHeap, Font_t,        RESOURCES_NUM_FONTS);
	pig->resources.sounds   =   (ResourceSounds_t*)AllocArray(fixedHeap, Sound_t,       RESOURCES_NUM_SOUNDS);
	pig->resources.musics   =   (ResourceMusics_t*)AllocArray(fixedHeap, Sound_t,       RESOURCES_NUM_MUSICS);
	NotNull(pig->resources.textures);
	NotNull(pig->resources.vectors);
	NotNull(pig->resources.sheets);
	NotNull(pig->resources.shaders);
	NotNull(pig->resources.fonts);
	NotNull(pig->resources.sounds);
	NotNull(pig->resources.musics);
	
	ClearPointer(pig->resources.textures);
	ClearPointer(pig->resources.vectors);
	ClearPointer(pig->resources.sheets);
	ClearPointer(pig->resources.shaders);
	ClearPointer(pig->resources.fonts);
	ClearPointer(pig->resources.sounds);
	ClearPointer(pig->resources.musics);
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
u64 GetNumResourcesOfType(ResourceType_t resourceType)
{
	switch (resourceType)
	{
		case ResourceType_Texture:     return RESOURCES_NUM_TEXTURES;
		case ResourceType_VectorImage: return RESOURCES_NUM_VECTORS;
		case ResourceType_Sheet:       return RESOURCES_NUM_SHEETS;
		case ResourceType_Shader:      return RESOURCES_NUM_SHADERS;
		case ResourceType_Font:        return RESOURCES_NUM_FONTS;
		case ResourceType_Sound:       return RESOURCES_NUM_SOUNDS;
		case ResourceType_Music:       return RESOURCES_NUM_MUSICS;
		default: Unimplemented(); return 0;
	}
}

// +--------------------------------------------------------------+
// |                            Watch                             |
// +--------------------------------------------------------------+
#if DEVELOPER_BUILD
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
	bool isPixelated = true;
	bool isRepeating = true;
	const char* texturePath = Resources_GetPathForTexture(textureIndex, &isPixelated, &isRepeating);
	NotNull(texturePath);
	MyStr_t texturePathStr = NewStr(texturePath);
	Texture_t newTexture = {};
	if (!LoadTexture(fixedHeap, &newTexture, texturePathStr, isPixelated, isRepeating))
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
	Texture_t* texture = &pig->resources.textures->items[textureIndex];
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
	
	VectorImg_t* image = &pig->resources.vectors->items[vectorImgIndex];
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
	SpriteSheet_t* sheet = &pig->resources.sheets->items[sheetIndex];
	
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
	Shader_t* shader = &pig->resources.shaders->items[shaderIndex];
	
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
	Font_t* font = &pig->resources.fonts->items[fontIndex];
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

// +==============================+
// |            Sound             |
// +==============================+
void Pig_LoadSoundResource(u64 soundIndex)
{
	Assert(soundIndex < RESOURCES_NUM_SOUNDS);
	const char* soundPath = Resources_GetPathForSound(soundIndex);
	NotNull(soundPath);
	MyStr_t soundPathStr = NewStr(soundPath);
	Sound_t newSound = {};
	
	PlatFileContents_t soundFile = {};
	if (plat->ReadFileContents(soundPathStr, &soundFile))
	{
		bool parseSuccess = false;
		ProcessLog_t soundParseLog = {};
		CreateProcessLog(&soundParseLog, Kilobytes(8), TempArena, fixedHeap);
		
		if (StrEndsWith(soundPathStr, ".ogg"))
		{
			TempPushMark();
			OggAudioData_t oggData = {};
			if (TryDeserOggFile(soundFile.size, soundFile.data, &soundParseLog, &oggData, TempArena))
			{
				CreateSoundFromOggAudioData(&oggData, platInfo->audioFormat, &newSound, &pig->audioHeap);
				parseSuccess = true;
			}
			else
			{
				PrintLine_E("Failed to deserialize ogg sound[%llu] at \"%.*s\"", soundIndex, soundPathStr.length, soundPathStr.pntr);
				DebugAssert(false);
			}
			if (soundParseLog.hadErrors || soundParseLog.hadWarnings) { DumpProcessLog(&soundParseLog, "OGG Parse Log"); }
			TempPopMark();
		}
		else if (StrEndsWith(soundPathStr, ".wav"))
		{
			TempPushMark();
			WavAudioData_t wavData = {};
			if (TryDeserWavFile(soundFile.size, soundFile.data, &soundParseLog, &wavData, TempArena))
			{
				CreateSoundFromWavAudioData(&wavData, platInfo->audioFormat, &newSound, &pig->audioHeap);
				parseSuccess = true;
			}
			else
			{
				PrintLine_E("Failed to deserialize wav sound[%llu] at \"%.*s\"", soundIndex, soundPathStr.length, soundPathStr.pntr);
				DebugAssert(false);
			}
			if (soundParseLog.hadErrors || soundParseLog.hadWarnings) { DumpProcessLog(&soundParseLog, "WAV Parse Log"); }
			TempPopMark();
		}
		else
		{
			AssertMsg(false, "Unknown file format extension found on sound resource path");
		}
		
		FreeProcessLog(&soundParseLog);
		plat->FreeFileContents(&soundFile);
		if (!parseSuccess) { return; }
	}
	else
	{
		PrintLine_E("Missing sound[%llu] file at \"%.*s\"", soundIndex, soundPathStr.length, soundPathStr.pntr);
		DebugAssert(false);
		return;
	}
	
	Sound_t* sound = &pig->resources.sounds->items[soundIndex];
	if (sound->allocArena != nullptr) //TODO: Change this to something like isValid
	{
		FreeSound(sound);
	}
	MyMemCopy(sound, &newSound, sizeof(Sound_t));
	StopWatchingFilesForResource(ResourceType_Sound, soundIndex);
	WatchFileForResource(ResourceType_Sound, soundIndex, soundPathStr);
}
void Pig_LoadAllSounds()
{
	for (u64 soundIndex = 0; soundIndex < RESOURCES_NUM_SOUNDS; soundIndex++)
	{
		Pig_LoadSoundResource(soundIndex);
	}
}

// +==============================+
// |            Music             |
// +==============================+
void Pig_LoadMusicResource(u64 musicIndex)
{
	Assert(musicIndex < RESOURCES_NUM_MUSICS);
	const char* musicPath = Resources_GetPathForMusic(musicIndex);
	NotNull(musicPath);
	MyStr_t musicPathStr = NewStr(musicPath);
	Sound_t newMusic = {};
	
	PlatFileContents_t musicFile = {};
	if (plat->ReadFileContents(musicPathStr, &musicFile))
	{
		bool parseSuccess = false;
		ProcessLog_t musicParseLog = {};
		CreateProcessLog(&musicParseLog, Kilobytes(8), TempArena, fixedHeap);
		
		if (StrEndsWith(musicPathStr, ".ogg"))
		{
			TempPushMark();
			OggAudioData_t oggData = {};
			if (TryDeserOggFile(musicFile.size, musicFile.data, &musicParseLog, &oggData, TempArena))
			{
				CreateSoundFromOggAudioData(&oggData, platInfo->audioFormat, &newMusic, &pig->audioHeap);
				parseSuccess = true;
			}
			else
			{
				PrintLine_E("Failed to deserialize ogg music[%llu] at \"%.*s\"", musicIndex, musicPathStr.length, musicPathStr.pntr);
				DebugAssert(false);
			}
			if (musicParseLog.hadErrors || musicParseLog.hadWarnings) { DumpProcessLog(&musicParseLog, "OGG Parse Log"); }
			TempPopMark();
		}
		else if (StrEndsWith(musicPathStr, ".wav"))
		{
			TempPushMark();
			WavAudioData_t wavData = {};
			if (TryDeserWavFile(musicFile.size, musicFile.data, &musicParseLog, &wavData, TempArena))
			{
				CreateSoundFromWavAudioData(&wavData, platInfo->audioFormat, &newMusic, &pig->audioHeap);
				parseSuccess = true;
			}
			else
			{
				PrintLine_E("Failed to deserialize wav music[%llu] at \"%.*s\"", musicIndex, musicPathStr.length, musicPathStr.pntr);
				DebugAssert(false);
			}
			if (musicParseLog.hadErrors || musicParseLog.hadWarnings) { DumpProcessLog(&musicParseLog, "WAV Parse Log"); }
			TempPopMark();
		}
		else
		{
			AssertMsg(false, "Unknown file format extension found on music resource path");
		}
		
		FreeProcessLog(&musicParseLog);
		plat->FreeFileContents(&musicFile);
		if (!parseSuccess) { return; }
	}
	else
	{
		PrintLine_E("Missing sound[%llu] file at \"%.*s\"", musicIndex, musicPathStr.length, musicPathStr.pntr);
		DebugAssert(false);
		return;
	}
	
	Sound_t* music = &pig->resources.musics->items[musicIndex];
	if (music->allocArena != nullptr) //TODO: Change this to something like isValid
	{
		FreeSound(music);
	}
	MyMemCopy(music, &newMusic, sizeof(Sound_t));
	StopWatchingFilesForResource(ResourceType_Music, musicIndex);
	WatchFileForResource(ResourceType_Music, musicIndex, musicPathStr);
}
void Pig_LoadAllMusics()
{
	for (u64 musicIndex = 0; musicIndex < RESOURCES_NUM_MUSICS; musicIndex++)
	{
		Pig_LoadMusicResource(musicIndex);
	}
}

void Pig_LoadAllResources()
{
	Pig_LoadAllTextures();
	Pig_LoadAllVectorImgs();
	Pig_LoadAllSpriteSheets();
	Pig_LoadAllShaders();
	Pig_LoadAllFonts();
	Pig_LoadAllSounds();
	Pig_LoadAllMusics();
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Pig_HandleResourcesOnReload()
{
	if (pig->resources.numTexturesAlloc != RESOURCES_NUM_TEXTURES)
	{
		PrintLine_N("Texture resource count changed: %llu -> %llu", pig->resources.numTexturesAlloc, RESOURCES_NUM_TEXTURES);
		u64 oldTextureCount = pig->resources.numTexturesAlloc;
		Texture_t* newSpace = AllocArray(fixedHeap, Texture_t, RESOURCES_NUM_TEXTURES);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.textures, sizeof(Texture_t) * MinU64(RESOURCES_NUM_TEXTURES, oldTextureCount));
		FreeMem(fixedHeap, pig->resources.textures, sizeof(Texture_t) * oldTextureCount);
		pig->resources.numTexturesAlloc = RESOURCES_NUM_TEXTURES;
		pig->resources.textures = (ResourceTextures_t*)newSpace;
		u64 numNewTextures = RESOURCES_NUM_TEXTURES - oldTextureCount;
		if (oldTextureCount < RESOURCES_NUM_TEXTURES) { NotifyPrint_N("Loading %llu new texture resource%s...", numNewTextures, (numNewTextures == 1) ? "" : "s"); }
		for (u64 textureIndex = oldTextureCount; textureIndex < RESOURCES_NUM_TEXTURES; textureIndex++) { Pig_LoadTextureResource(textureIndex); }
	}
	if (pig->resources.numVectorImgsAlloc != RESOURCES_NUM_VECTORS)
	{
		PrintLine_N("Vector resource count changed: %llu -> %llu", pig->resources.numVectorImgsAlloc, RESOURCES_NUM_VECTORS);
		u64 oldVectorImgCount = pig->resources.numVectorImgsAlloc;
		VectorImg_t* newSpace = AllocArray(fixedHeap, VectorImg_t, RESOURCES_NUM_VECTORS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.vectors, sizeof(VectorImg_t) * MinU64(RESOURCES_NUM_VECTORS, oldVectorImgCount));
		FreeMem(fixedHeap, pig->resources.vectors, sizeof(VectorImg_t) * oldVectorImgCount);
		pig->resources.numVectorImgsAlloc = RESOURCES_NUM_VECTORS;
		pig->resources.vectors = (ResourceVectors_t*)newSpace;
		u64 numNewVectorImgs = RESOURCES_NUM_VECTORS - oldVectorImgCount;
		if (oldVectorImgCount < RESOURCES_NUM_VECTORS) { NotifyPrint_I("Loading %llu new vector resource%s...", numNewVectorImgs, (numNewVectorImgs == 1) ? "" : "s"); }
		for (u64 vectorIndex = oldVectorImgCount; vectorIndex < RESOURCES_NUM_VECTORS; vectorIndex++) { Pig_LoadVectorImgResource(vectorIndex); }
	}
	if (pig->resources.numSheetsAlloc != RESOURCES_NUM_SHEETS)
	{
		PrintLine_N("Sprite Sheet resource count changed: %llu -> %llu", pig->resources.numSheetsAlloc, RESOURCES_NUM_SHEETS);
		u64 oldSheetsCount = pig->resources.numSheetsAlloc;
		SpriteSheet_t* newSpace = AllocArray(fixedHeap, SpriteSheet_t, RESOURCES_NUM_SHEETS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.sheets, sizeof(SpriteSheet_t) * MinU64(RESOURCES_NUM_SHEETS, oldSheetsCount));
		FreeMem(fixedHeap, pig->resources.sheets, sizeof(SpriteSheet_t) * oldSheetsCount);
		pig->resources.numSheetsAlloc = RESOURCES_NUM_SHEETS;
		pig->resources.sheets = (ResourceSheets_t*)newSpace;
		u64 numNewSheets = RESOURCES_NUM_SHEETS - oldSheetsCount;
		if (oldSheetsCount < RESOURCES_NUM_SHEETS) { NotifyPrint_I("Loading %llu new sprite sheet resource%s...", numNewSheets, (numNewSheets == 1) ? "" : "s"); }
		for (u64 sheetIndex = oldSheetsCount; sheetIndex < RESOURCES_NUM_SHEETS; sheetIndex++) { Pig_LoadSpriteSheetResource(sheetIndex); }
	}
	if (pig->resources.numShadersAlloc != RESOURCES_NUM_SHADERS)
	{
		PrintLine_N("Shader resource count changed: %llu -> %llu", pig->resources.numShadersAlloc, RESOURCES_NUM_SHADERS);
		u64 oldShadersCount = pig->resources.numShadersAlloc;
		Shader_t* newSpace = AllocArray(fixedHeap, Shader_t, RESOURCES_NUM_SHADERS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.shaders, sizeof(Shader_t) * MinU64(RESOURCES_NUM_SHADERS, oldShadersCount));
		FreeMem(fixedHeap, pig->resources.shaders, sizeof(Shader_t) * oldShadersCount);
		pig->resources.numShadersAlloc = RESOURCES_NUM_SHADERS;
		pig->resources.shaders = (ResourceShaders_t*)newSpace;
		u64 numNewShaders = RESOURCES_NUM_SHADERS - oldShadersCount;
		if (oldShadersCount < RESOURCES_NUM_SHADERS) { NotifyPrint_I("Loading %llu new shader resource%s...", numNewShaders, (numNewShaders == 1) ? "" : "s"); }
		for (u64 shaderIndex = oldShadersCount; shaderIndex < RESOURCES_NUM_SHADERS; shaderIndex++) { Pig_LoadShaderResource(shaderIndex); }
	}
	if (pig->resources.numFontsAlloc != RESOURCES_NUM_FONTS)
	{
		PrintLine_N("Font resource count changed: %llu -> %llu", pig->resources.numFontsAlloc, RESOURCES_NUM_FONTS);
		u64 oldFontsCount = pig->resources.numFontsAlloc;
		Font_t* newSpace = AllocArray(fixedHeap, Font_t, RESOURCES_NUM_FONTS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.fonts, sizeof(Font_t) * MinU64(RESOURCES_NUM_FONTS, oldFontsCount));
		FreeMem(fixedHeap, pig->resources.fonts, sizeof(Font_t) * oldFontsCount);
		pig->resources.numFontsAlloc = RESOURCES_NUM_FONTS;
		pig->resources.fonts = (ResourceFonts_t*)newSpace;
		u64 numNewFonts = RESOURCES_NUM_FONTS - oldFontsCount;
		if (oldFontsCount < RESOURCES_NUM_FONTS) { NotifyPrint_I("Loading %llu new font resource%s...", numNewFonts, (numNewFonts == 1) ? "" : "s"); }
		for (u64 fontIndex = oldFontsCount; fontIndex < RESOURCES_NUM_FONTS; fontIndex++) { Pig_LoadFontResource(fontIndex); }
	}
	if (pig->resources.numSoundsAlloc != RESOURCES_NUM_SOUNDS)
	{
		PrintLine_N("Sound resource count changed: %llu -> %llu", pig->resources.numSoundsAlloc, RESOURCES_NUM_SOUNDS);
		u64 oldSoundsCount = pig->resources.numSoundsAlloc;
		Sound_t* newSpace = AllocArray(fixedHeap, Sound_t, RESOURCES_NUM_SOUNDS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.sounds, sizeof(Sound_t) * MinU64(RESOURCES_NUM_SOUNDS, oldSoundsCount));
		FreeMem(fixedHeap, pig->resources.sounds, sizeof(Sound_t) * oldSoundsCount);
		pig->resources.numSoundsAlloc = RESOURCES_NUM_SOUNDS;
		pig->resources.sounds = (ResourceSounds_t*)newSpace;
		u64 numNewSounds = RESOURCES_NUM_SOUNDS - oldSoundsCount;
		if (oldSoundsCount < RESOURCES_NUM_SOUNDS) { NotifyPrint_I("Loading %llu new sound resource%s...", numNewSounds, (numNewSounds == 1) ? "" : "s"); }
		for (u64 soundIndex = oldSoundsCount; soundIndex < RESOURCES_NUM_SOUNDS; soundIndex++) { Pig_LoadSoundResource(soundIndex); }
	}
	if (pig->resources.numMusicsAlloc != RESOURCES_NUM_MUSICS)
	{
		PrintLine_N("Music resource count changed: %llu -> %llu", pig->resources.numMusicsAlloc, RESOURCES_NUM_MUSICS);
		u64 oldMusicsCount = pig->resources.numMusicsAlloc;
		Sound_t* newSpace = AllocArray(fixedHeap, Sound_t, RESOURCES_NUM_MUSICS);
		NotNull(newSpace);
		MyMemCopy(newSpace, pig->resources.musics, sizeof(Sound_t) * MinU64(RESOURCES_NUM_MUSICS, oldMusicsCount));
		FreeMem(fixedHeap, pig->resources.musics, sizeof(Sound_t) * oldMusicsCount);
		pig->resources.numMusicsAlloc = RESOURCES_NUM_MUSICS;
		pig->resources.musics = (ResourceMusics_t*)newSpace;
		u64 numNewMusics = RESOURCES_NUM_MUSICS - oldMusicsCount;
		if (oldMusicsCount < RESOURCES_NUM_MUSICS) { NotifyPrint_I("Loading %llu new music resource%s...", numNewMusics, (numNewMusics == 1) ? "" : "s"); }
		for (u64 musicIndex = oldMusicsCount; musicIndex < RESOURCES_NUM_MUSICS; musicIndex++) { Pig_LoadMusicResource(musicIndex); }
	}
}

void Pig_UpdateResources()
{
	#if DEVELOPER_BUILD
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
				case ResourceType_Sound:
				{
					PrintLine_N("Reloading Sound[%llu]...", watch->resourceIndex);
					Pig_LoadSoundResource(watch->resourceIndex);
				} break;
				case ResourceType_Music:
				{
					PrintLine_N("Reloading Music[%llu]...", watch->resourceIndex);
					Pig_LoadMusicResource(watch->resourceIndex);
				} break;
				default: DebugAssert(false); break;
			}
		}
	}
	#endif
}

// +--------------------------------------------------------------+
// |                           Handles                            |
// +--------------------------------------------------------------+
bool IsHandleFilled(const TextureHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}
bool IsHandleFilled(const VectorImgHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}
bool IsHandleFilled(const SpriteSheetHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}
bool IsHandleFilled(const ShaderHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}
bool IsHandleFilled(const FontHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}

#define AssertFilledHandle(handlePntr) Assert(IsHandleFilled(handlePntr))

//NOTE: Handles to resources CAN be grabbed before the resource lists are allocated!
TextureHandle_t GetTextureHandle(const Texture_t* texturePointer)
{
	TextureHandle_t result = {};
	if (texturePointer == nullptr && pig->resources.textures != nullptr) { return result; }
	Assert(texturePointer >= (Texture_t*)pig->resources.textures);
	if (pig->resources.textures == nullptr) { Assert(texturePointer < ((Texture_t*)pig->resources.textures) + RESOURCES_NUM_TEXTURES); }
	else { Assert(texturePointer < ((Texture_t*)pig->resources.textures) + pig->resources.numTexturesAlloc); }
	result.index = (u64)(texturePointer - ((Texture_t*)pig->resources.textures));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (Texture_t*)texturePointer;
	return result;
}
VectorImgHandle_t GetVectorImgHandle(const VectorImg_t* vectorImgPointer)
{
	VectorImgHandle_t result = {};
	if (vectorImgPointer == nullptr && pig->resources.vectors != nullptr) { return result; }
	Assert(vectorImgPointer >= (VectorImg_t*)pig->resources.vectors);
	if (pig->resources.vectors == nullptr) { Assert(vectorImgPointer < ((VectorImg_t*)pig->resources.vectors) + RESOURCES_NUM_VECTORS); }
	else { Assert(vectorImgPointer < ((VectorImg_t*)pig->resources.vectors) + pig->resources.numVectorImgsAlloc); }
	result.index = (u64)(vectorImgPointer - ((VectorImg_t*)pig->resources.vectors));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (VectorImg_t*)vectorImgPointer;
	return result;
}
SpriteSheetHandle_t GetSpriteSheetHandle(const SpriteSheet_t* spriteSheetPointer)
{
	SpriteSheetHandle_t result = {};
	if (spriteSheetPointer == nullptr && pig->resources.sheets != nullptr) { return result; }
	Assert(spriteSheetPointer >= (SpriteSheet_t*)pig->resources.sheets);
	if (pig->resources.sheets == nullptr) { Assert(spriteSheetPointer < ((SpriteSheet_t*)pig->resources.sheets) + RESOURCES_NUM_SHEETS); }
	else { Assert(spriteSheetPointer < ((SpriteSheet_t*)pig->resources.sheets) + pig->resources.numSheetsAlloc); }
	result.index = (u64)(spriteSheetPointer - ((SpriteSheet_t*)pig->resources.sheets));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (SpriteSheet_t*)spriteSheetPointer;
	return result;
}
ShaderHandle_t GetShaderHandle(const Shader_t* shaderPointer)
{
	ShaderHandle_t result = {};
	if (shaderPointer == nullptr && pig->resources.shaders != nullptr) { return result; }
	Assert(shaderPointer >= (Shader_t*)pig->resources.shaders);
	if (pig->resources.shaders == nullptr) { Assert(shaderPointer < ((Shader_t*)pig->resources.shaders) + RESOURCES_NUM_SHADERS); }
	else { Assert(shaderPointer < ((Shader_t*)pig->resources.shaders) + pig->resources.numShadersAlloc); }
	result.index = (u64)(shaderPointer - ((Shader_t*)pig->resources.shaders));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (Shader_t*)shaderPointer;
	return result;
}
FontHandle_t GetFontHandle(const Font_t* fontPointer)
{
	FontHandle_t result = {};
	if (fontPointer == nullptr && pig->resources.fonts != nullptr) { return result; }
	Assert(fontPointer >= (Font_t*)pig->resources.fonts);
	if (pig->resources.fonts == nullptr) { Assert(fontPointer < ((Font_t*)pig->resources.fonts) + RESOURCES_NUM_FONTS); }
	else { Assert(fontPointer < ((Font_t*)pig->resources.fonts) + pig->resources.numFontsAlloc); }
	result.index = (u64)(fontPointer - ((Font_t*)pig->resources.fonts));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (Font_t*)fontPointer;
	return result;
}

Texture_t* GetPointer(TextureHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numTexturesAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.textures == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.textures->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
VectorImg_t* GetPointer(VectorImgHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numVectorImgsAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.vectors == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.vectors->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
SpriteSheet_t* GetPointer(SpriteSheetHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numSheetsAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.sheets == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.sheets->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
Shader_t* GetPointer(ShaderHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numShadersAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.shaders == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.shaders->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
Font_t* GetPointer(FontHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numFontsAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.fonts == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.fonts->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
