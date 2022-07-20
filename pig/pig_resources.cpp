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
void InitResourceStatus(ResourceStatus_t* status)
{
	NotNull(status);
	ClearPointer(status);
	status->state = ResourceState_Unloaded;
	status->lastAccessTime = 0;
	status->isPinned = false;
}

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
	
	pig->resources.textureStatus = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_TEXTURES);
	pig->resources.vectorStatus  = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_VECTORS);
	pig->resources.sheetStatus   = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SHEETS);
	pig->resources.shaderStatus  = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SHADERS);
	pig->resources.fontStatus    = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_FONTS);
	pig->resources.soundStatus   = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SOUNDS);
	pig->resources.musicStatus   = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_MUSICS);
	NotNull(pig->resources.textureStatus);
	NotNull(pig->resources.vectorStatus);
	NotNull(pig->resources.sheetStatus);
	NotNull(pig->resources.shaderStatus);
	NotNull(pig->resources.fontStatus);
	NotNull(pig->resources.soundStatus);
	NotNull(pig->resources.musicStatus);
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_TEXTURES; rIndex++) { InitResourceStatus(&pig->resources.textureStatus[rIndex]); }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_VECTORS;  rIndex++) { InitResourceStatus(&pig->resources.vectorStatus[rIndex]);  }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_SHEETS;   rIndex++) { InitResourceStatus(&pig->resources.sheetStatus[rIndex]);   }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_SHADERS;  rIndex++) { InitResourceStatus(&pig->resources.shaderStatus[rIndex]);  }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_FONTS;    rIndex++) { InitResourceStatus(&pig->resources.fontStatus[rIndex]);    }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_SOUNDS;   rIndex++) { InitResourceStatus(&pig->resources.soundStatus[rIndex]);   }
	for (u64 rIndex = 0; rIndex < RESOURCES_NUM_MUSICS;   rIndex++) { InitResourceStatus(&pig->resources.musicStatus[rIndex]);   }
	
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

const char* GetPathOrNameForResource(ResourceType_t type, u64 resourceIndex)
{
	const char* result = nullptr;
	switch (type)
	{
		case ResourceType_Texture:
		{
			bool isPixelated = false;
			bool isRepeating = false;
			result = Resources_GetPathForTexture(resourceIndex, &isPixelated, &isRepeating);
		} break;
		case ResourceType_VectorImage:
		{
			result = Resources_GetPathForVectorImg(resourceIndex);
		} break;
		case ResourceType_Sheet:
		{
			ResourceSheetMetaInfo_t metaInfo = {};
			result = Resources_GetPathForSheet(resourceIndex, &metaInfo);
		} break;
		case ResourceType_Shader:
		{
			ResourceShaderMetaInfo_t metaInfo = {};
			result = Resources_GetPathForShader(resourceIndex, &metaInfo);
		} break;
		case ResourceType_Font:
		{
			ResourceFontMetaInfo_t metaInfo = {};
			Resources_GetPathOrNameForFont(resourceIndex, &metaInfo);
			result = metaInfo.fontName;
		} break;
		case ResourceType_Sound:
		{
			result = Resources_GetPathForSound(resourceIndex);
		} break;
		case ResourceType_Music:
		{
			result = Resources_GetPathForMusic(resourceIndex);
		} break;
		default: Unimplemented(); break;
	}
	return result;
}

#if 0
const char* GetResourceStatus(ResourceType_t type, u64 resourceIndex)
{
	Texture_t* texture;
	VectorImg_t* vector;
	SpriteSheet_t* sheet;
	Shader_t* shader;
	Font_t* font;
	Sound_t* sound;
	Sound_t* music;
	switch (type)
	{
		case ResourceType_Texture:     texture = &pig->resources.textures->items[resourceIndex]; return texture->isValid               ? "Loaded" : "Error"; break;
		case ResourceType_VectorImage: vector  = &pig->resources.vectors->items[resourceIndex];  return vector->isValid                ? "Loaded" : "Error"; break;
		case ResourceType_Sheet:       sheet   = &pig->resources.sheets->items[resourceIndex];   return sheet->isValid                 ? "Loaded" : "Error"; break;
		case ResourceType_Shader:      shader  = &pig->resources.shaders->items[resourceIndex];  return shader->isValid                ? "Loaded" : "Error"; break;
		case ResourceType_Font:        font    = &pig->resources.fonts->items[resourceIndex];    return font->isValid                  ? "Loaded" : "Error"; break;
		case ResourceType_Sound:       sound   = &pig->resources.sounds->items[resourceIndex];   return (sound->allocArena != nullptr) ? "Loaded" : "Error"; break;
		case ResourceType_Music:       music   = &pig->resources.musics->items[resourceIndex];   return (music->allocArena != nullptr) ? "Loaded" : "Error"; break;
		default: Unimplemented(); return "Unknown"; break;
	}
}
#endif

void* GetResourcePntr(ResourceType_t type, u64 resourceIndex)
{
	switch (type)
	{
		case ResourceType_Texture:     Assert(resourceIndex < RESOURCES_NUM_TEXTURES); return (void*)(&pig->resources.textures->items[resourceIndex]);
		case ResourceType_VectorImage: Assert(resourceIndex < RESOURCES_NUM_VECTORS);  return (void*)(&pig->resources.vectors->items[resourceIndex]);
		case ResourceType_Sheet:       Assert(resourceIndex < RESOURCES_NUM_SHEETS);   return (void*)(&pig->resources.sheets->items[resourceIndex]);
		case ResourceType_Shader:      Assert(resourceIndex < RESOURCES_NUM_SHADERS);  return (void*)(&pig->resources.shaders->items[resourceIndex]);
		case ResourceType_Font:        Assert(resourceIndex < RESOURCES_NUM_FONTS);    return (void*)(&pig->resources.fonts->items[resourceIndex]);
		case ResourceType_Sound:       Assert(resourceIndex < RESOURCES_NUM_SOUNDS);   return (void*)(&pig->resources.sounds->items[resourceIndex]);
		case ResourceType_Music:       Assert(resourceIndex < RESOURCES_NUM_MUSICS);   return (void*)(&pig->resources.musics->items[resourceIndex]);
		default: Unimplemented(); return nullptr; break;
	}
}

ResourceType_t GetResourceByPntr(const void* resourcePntr, u64* resourceIndexOut = nullptr)
{
	ResourceType_t result = ResourceType_None;
	u64 resultIndex = 0;
	NotNull(resourcePntr);
	for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
	{
		ResourceType_t type = (ResourceType_t)tIndex;
		if (type != ResourceType_None)
		{
			u64 numOfType = GetNumResourcesOfType(type);
			for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
			{
				if (resourcePntr == GetResourcePntr(type, rIndex))
				{
					result = type;
					resultIndex = rIndex;
					break;
				}
			}
			if (result != ResourceType_None) { break; }
		}
	}
	if (resourceIndexOut != nullptr) { *resourceIndexOut = resultIndex; }
	return result;
}

ResourceStatus_t* GetResourceStatus(ResourceType_t type, u64 resourceIndex)
{
	switch (type)
	{
		case ResourceType_Texture:     Assert(resourceIndex < RESOURCES_NUM_TEXTURES); return &pig->resources.textureStatus[resourceIndex];
		case ResourceType_VectorImage: Assert(resourceIndex < RESOURCES_NUM_VECTORS);  return &pig->resources.vectorStatus[resourceIndex];
		case ResourceType_Sheet:       Assert(resourceIndex < RESOURCES_NUM_SHEETS);   return &pig->resources.sheetStatus[resourceIndex];
		case ResourceType_Shader:      Assert(resourceIndex < RESOURCES_NUM_SHADERS);  return &pig->resources.shaderStatus[resourceIndex];
		case ResourceType_Font:        Assert(resourceIndex < RESOURCES_NUM_FONTS);    return &pig->resources.fontStatus[resourceIndex];
		case ResourceType_Sound:       Assert(resourceIndex < RESOURCES_NUM_SOUNDS);   return &pig->resources.soundStatus[resourceIndex];
		case ResourceType_Music:       Assert(resourceIndex < RESOURCES_NUM_MUSICS);   return &pig->resources.musicStatus[resourceIndex];
		default: Unimplemented(); return nullptr;
	}
}

bool IsResourcePinned(ResourceType_t type, u64 resourceIndex)
{
	ResourceStatus_t* status = GetResourceStatus(type, resourceIndex);
	NotNull(status);
	return status->isPinned;
}

ResourceType_t FindResourceByPathOrName(MyStr_t searchStr, u64* resourceIndexOut, bool allowEndingPortionPartial = false, bool ignoreCase = true)
{
	NotNullStr(&searchStr);
	for (u64 tIndex = 0; tIndex < ResourceType_NumTypes; tIndex++)
	{
		ResourceType_t type = (ResourceType_t)tIndex;
		if (type != ResourceType_None)
		{
			u64 numOfType = GetNumResourcesOfType(type);
			for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
			{
				const char* pathOrName = GetPathOrNameForResource(type, rIndex);
				if (allowEndingPortionPartial && StrEndsWith(NewStr(pathOrName), searchStr, ignoreCase))
				{
					if (resourceIndexOut != nullptr) { *resourceIndexOut = rIndex; }
					return type;
				}
				else if (!allowEndingPortionPartial && ignoreCase && StrCompareIgnoreCase(NewStr(pathOrName), searchStr) == 0)
				{
					if (resourceIndexOut != nullptr) { *resourceIndexOut = rIndex; }
					return type;
				}
				else if (!allowEndingPortionPartial && !ignoreCase && StrEquals(NewStr(pathOrName), searchStr))
				{
					if (resourceIndexOut != nullptr) { *resourceIndexOut = rIndex; }
					return type;
				}
			}
		}
	}
	return ResourceType_None;
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
	NotNull2(pig->resources.textures, pig->resources.textureStatus);
	Assert(textureIndex < RESOURCES_NUM_TEXTURES);
	Texture_t* texture = &pig->resources.textures->items[textureIndex];
	ResourceStatus_t* textureStatus = &pig->resources.textureStatus[textureIndex];
	textureStatus->lastAccessTime = ProgramTime;
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
		textureStatus->state = ResourceStateWarnOrError(textureStatus->state);
		return;
	}
	if (texture->isValid)
	{
		textureStatus->state = ResourceState_Unloaded;
		DestroyTexture(texture);
	}
	MyMemCopy(texture, &newTexture, sizeof(Texture_t));
	textureStatus->state = ResourceState_Loaded;
	
	StopWatchingFilesForResource(ResourceType_Texture, textureIndex);
	WatchFileForResource(ResourceType_Texture, textureIndex, texturePathStr);
}
void Pig_LoadAllTextures(bool onlyPinned = false)
{
	for (u64 textureIndex = 0; textureIndex < RESOURCES_NUM_TEXTURES; textureIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Texture, textureIndex))
		{
			Pig_LoadTextureResource(textureIndex);
		}
	}
}

// +==============================+
// |          VectorImg           |
// +==============================+
void Pig_LoadVectorImgResource(u64 vectorImgIndex)
{
	NotNull2(pig->resources.vectors, pig->resources.vectorStatus);
	Assert(vectorImgIndex < RESOURCES_NUM_VECTORS);
	VectorImg_t* image = &pig->resources.vectors->items[vectorImgIndex];
	ResourceStatus_t* vectorStatus = &pig->resources.vectorStatus[vectorImgIndex];
	vectorStatus->lastAccessTime = ProgramTime;
	
	const char* vectorImgPath = Resources_GetPathForVectorImg(vectorImgIndex);
	NotNull(vectorImgPath);
	MyStr_t vectorImgPathStr = NewStr(vectorImgPath);
	
	PlatFileContents_t svgFile = {};
	if (!plat->ReadFileContents(vectorImgPathStr, &svgFile))
	{
		PrintLine_E("Failed to open vector image[%llu] at \"%s\"", vectorImgIndex, GetFileNamePart(vectorImgPathStr).pntr);
		DebugAssert(false);
		vectorStatus->state = ResourceStateWarnOrError(vectorStatus->state);
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
		vectorStatus->state = ResourceStateWarnOrError(vectorStatus->state);
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
		vectorStatus->state = ResourceStateWarnOrError(vectorStatus->state);
		return;
	}
	FreeSvgData(&svgData);
	
	if (image->isValid)
	{
		vectorStatus->state = ResourceState_Unloaded;
		DestroyVectorImg(image);
	}
	MyMemCopy(image, &newImage, sizeof(VectorImg_t));
	vectorStatus->state = ResourceState_Loaded;
	
	StopWatchingFilesForResource(ResourceType_VectorImage, vectorImgIndex);
	WatchFileForResource(ResourceType_VectorImage, vectorImgIndex, vectorImgPathStr);
	
	TempPopMark();
	u64 fixedHeapUsageAfter = fixedHeap->used; //TODO: Remove me!
	PrintLine_D("Vector image[%llu] took %s", vectorImgIndex, FormatBytesNt(fixedHeapUsageAfter - fixedHeapUsageBefore, TempArena));
}
void Pig_LoadAllVectorImgs(bool onlyPinned = false)
{
	for (u64 vectorImgIndex = 0; vectorImgIndex < RESOURCES_NUM_VECTORS; vectorImgIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_VectorImage, vectorImgIndex))
		{
			Pig_LoadVectorImgResource(vectorImgIndex);
		}
	}
}

// +==============================+
// |         SpriteSheet          |
// +==============================+
void Pig_LoadSpriteSheetResource(u64 sheetIndex)
{
	NotNull2(pig->resources.sheets, pig->resources.sheetStatus);
	Assert(sheetIndex < RESOURCES_NUM_SHEETS);
	SpriteSheet_t* sheet = &pig->resources.sheets->items[sheetIndex];
	ResourceStatus_t* sheetStatus = &pig->resources.sheetStatus[sheetIndex];
	sheetStatus->lastAccessTime = ProgramTime;
	
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
		sheetStatus->state = ResourceStateWarnOrError(sheetStatus->state);
		return;
	}
	
	if (sheet->isValid)
	{
		sheetStatus->state = ResourceState_Unloaded;
		DestroySpriteSheet(sheet);
	}
	MyMemCopy(sheet, &tempSheet, sizeof(SpriteSheet_t));
	sheetStatus->state = ResourceState_Loaded;
	
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
				sheetStatus->state = ResourceState_Warning;
			}
			
			FreeProcessLog(&metaParseLog);
			plat->FreeFileContents(&metaFile);
		}
		else
		{
			NotifyPrint_E("Failed to load meta file for sheet[%llu] at \"%.*s\"", sheetIndex, metaInfo.metaFilePath.length, metaInfo.metaFilePath.pntr);
			sheetStatus->state = ResourceState_Warning;
		}
	}
	
	StopWatchingFilesForResource(ResourceType_Sheet, sheetIndex);
	WatchFileForResource(ResourceType_Sheet, sheetIndex, sheetPathStr);
	if (!IsEmptyStr(metaInfo.metaFilePath)) 
	{
		WatchFileForResource(ResourceType_Sheet, sheetIndex, metaInfo.metaFilePath);
	}
}
void Pig_LoadAllSpriteSheets(bool onlyPinned = false)
{
	for (u64 sheetIndex = 0; sheetIndex < RESOURCES_NUM_SHEETS; sheetIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Sheet, sheetIndex))
		{
			Pig_LoadSpriteSheetResource(sheetIndex);
		}
	}
}

// +==============================+
// |            Shader            |
// +==============================+
void Pig_LoadShaderResource(u64 shaderIndex)
{
	NotNull2(pig->resources.shaders, pig->resources.shaderStatus);
	Assert(shaderIndex < RESOURCES_NUM_SHADERS);
	Shader_t* shader = &pig->resources.shaders->items[shaderIndex];
	ResourceStatus_t* shaderStatus = &pig->resources.shaderStatus[shaderIndex];
	shaderStatus->lastAccessTime = ProgramTime;
	
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
			shaderStatus->state = ResourceStateWarnOrError(shaderStatus->state);
			return;
		}
	}
	
	if (shader->isValid)
	{
		shaderStatus->state = ResourceState_Unloaded;
		DestroyShader(shader);
	}
	MyMemCopy(shader, &tempShader, sizeof(Shader_t));
	shaderStatus->state = ResourceState_Loaded;
	
	StopWatchingFilesForResource(ResourceType_Shader, shaderIndex);
	WatchFileForResource(ResourceType_Shader, shaderIndex, shaderPathStr);
}
void Pig_LoadAllShaders(bool onlyPinned = false)
{
	for (u64 shaderIndex = 0; shaderIndex < RESOURCES_NUM_SHADERS; shaderIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Shader, shaderIndex))
		{
			Pig_LoadShaderResource(shaderIndex);
		}
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
	NotNull2(pig->resources.fonts, pig->resources.fontStatus);
	Assert(fontIndex < RESOURCES_NUM_FONTS);
	Font_t* font = &pig->resources.fonts->items[fontIndex];
	ResourceStatus_t* fontStatus = &pig->resources.fontStatus[fontIndex];
	fontStatus->lastAccessTime = ProgramTime;
	
	if (font->isValid)
	{
		fontStatus->state = ResourceState_Unloaded;
		DestroyFont(font);
	}
	
	ResourceFontMetaInfo_t metaInfo = {};
	const char* fontName = Resources_GetPathOrNameForFont(fontIndex, &metaInfo);
	NotNull(fontName);
	StartFont(font, mainHeap, 1); //TODO: Should we go through and figure out how many faces have a size > 0 and feed that value in here instead of 1
	
	StopWatchingFilesForResource(ResourceType_Font, fontIndex);
	
	Assert(metaInfo.faces[0].size > 0);
	if (metaInfo.faces[0].size == 0) { fontStatus->state = ResourceStateWarnOrError(fontStatus->state); }
	
	//TODO: We should probably check to make sure there is at least 1 filled out face meta info
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
				else
				{
					fontStatus->state = ResourceStateWarnOrError(fontStatus->state);
				}
				
				DestroySpriteSheet(&spriteSheet);
			}
			if (numBakesFound == 0)
			{
				DebugAssert(false);
				fontStatus->state = ResourceStateWarnOrError(fontStatus->state);
			}
			
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
						NewVec2i(16, 16), Vec2i_Zero, true,
						&spriteSheet
					);
				}
				else
				{
					loadedSpriteSheet = TryLoadSpriteSheetAndMeta(
						mainHeap,
						NewStr(RESOURCE_FOLDER_FONTS "/btns_light.png"), NewStr(RESOURCE_FOLDER_FONTS "/btns_light.meta"),
						NewVec2i(16, 16), Vec2i_Zero, false,
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
	fontStatus->state = ResourceState_Loaded;
}
void Pig_LoadAllFonts(bool onlyPinned = false)
{
	for (u64 fontIndex = 0; fontIndex < RESOURCES_NUM_FONTS; fontIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Font, fontIndex))
		{
			Pig_LoadFontResource(fontIndex);
		}
	}
}

// +==============================+
// |            Sound             |
// +==============================+
void Pig_LoadSoundResource(u64 soundIndex)
{
	NotNull2(pig->resources.sounds, pig->resources.soundStatus);
	Assert(soundIndex < RESOURCES_NUM_SOUNDS);
	Sound_t* sound = &pig->resources.sounds->items[soundIndex];
	ResourceStatus_t* soundStatus = &pig->resources.soundStatus[soundIndex];
	soundStatus->lastAccessTime = ProgramTime;
	
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
		if (!parseSuccess)
		{
			soundStatus->state = ResourceStateWarnOrError(soundStatus->state);
			return;
		}
	}
	else
	{
		PrintLine_E("Missing sound[%llu] file at \"%.*s\"", soundIndex, soundPathStr.length, soundPathStr.pntr);
		DebugAssert(false);
		soundStatus->state = ResourceStateWarnOrError(soundStatus->state);
		return;
	}
	
	if (sound->allocArena != nullptr) //TODO: Change this to something like isValid
	{
		soundStatus->state = ResourceState_Unloaded;
		FreeSound(sound);
	}
	MyMemCopy(sound, &newSound, sizeof(Sound_t));
	soundStatus->state = ResourceState_Loaded;
	
	StopWatchingFilesForResource(ResourceType_Sound, soundIndex);
	WatchFileForResource(ResourceType_Sound, soundIndex, soundPathStr);
}
void Pig_LoadAllSounds(bool onlyPinned = false)
{
	for (u64 soundIndex = 0; soundIndex < RESOURCES_NUM_SOUNDS; soundIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Sound, soundIndex))
		{
			Pig_LoadSoundResource(soundIndex);
		}
	}
}

// +==============================+
// |            Music             |
// +==============================+
void Pig_LoadMusicResource(u64 musicIndex)
{
	NotNull2(pig->resources.musics, pig->resources.musicStatus);
	Assert(musicIndex < RESOURCES_NUM_MUSICS);
	Sound_t* music = &pig->resources.musics->items[musicIndex];
	ResourceStatus_t* musicStatus = &pig->resources.musicStatus[musicIndex];
	musicStatus->lastAccessTime = ProgramTime;
	
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
			OggAudioData_t oggData = {};
			if (TryDeserOggFile(musicFile.size, musicFile.data, &musicParseLog, &oggData, &pig->stdHeap))
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
			FreeOggAudioData(&oggData);
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
		if (!parseSuccess)
		{
			musicStatus->state = ResourceStateWarnOrError(musicStatus->state);
			return;
		}
	}
	else
	{
		PrintLine_E("Missing sound[%llu] file at \"%.*s\"", musicIndex, musicPathStr.length, musicPathStr.pntr);
		DebugAssert(false);
		musicStatus->state = ResourceStateWarnOrError(musicStatus->state);
		return;
	}
	
	if (music->allocArena != nullptr) //TODO: Change this to something like isValid
	{
		musicStatus->state = ResourceState_Unloaded;
		FreeSound(music);
	}
	MyMemCopy(music, &newMusic, sizeof(Sound_t));
	musicStatus->state = ResourceState_Loaded;
	
	StopWatchingFilesForResource(ResourceType_Music, musicIndex);
	WatchFileForResource(ResourceType_Music, musicIndex, musicPathStr);
}
void Pig_LoadAllMusics(bool onlyPinned = false)
{
	for (u64 musicIndex = 0; musicIndex < RESOURCES_NUM_MUSICS; musicIndex++)
	{
		if (!onlyPinned || IsResourcePinned(ResourceType_Music, musicIndex))
		{
			Pig_LoadMusicResource(musicIndex);
		}
	}
}

void Pig_LoadResource(ResourceType_t type, u64 resourceIndex)
{
	switch (type)
	{
		case ResourceType_Texture:     Pig_LoadTextureResource(resourceIndex);     break;
		case ResourceType_VectorImage: Pig_LoadVectorImgResource(resourceIndex);   break;
		case ResourceType_Sheet:       Pig_LoadSpriteSheetResource(resourceIndex); break;
		case ResourceType_Shader:      Pig_LoadShaderResource(resourceIndex);      break;
		case ResourceType_Font:        Pig_LoadFontResource(resourceIndex);        break;
		case ResourceType_Sound:       Pig_LoadSoundResource(resourceIndex);       break;
		case ResourceType_Music:       Pig_LoadMusicResource(resourceIndex);       break;
		default: Unimplemented(); break;
	}
}
void Pig_LoadAllResources(bool onlyPinned = false)
{
	Pig_LoadAllTextures(onlyPinned);
	Pig_LoadAllVectorImgs(onlyPinned);
	Pig_LoadAllSpriteSheets(onlyPinned);
	Pig_LoadAllShaders(onlyPinned);
	Pig_LoadAllFonts(onlyPinned);
	Pig_LoadAllSounds(onlyPinned);
	Pig_LoadAllMusics(onlyPinned);
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
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_TEXTURES);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.textures, sizeof(Texture_t) * MinU64(RESOURCES_NUM_TEXTURES, oldTextureCount));
		MyMemCopy(newStatusSpace, pig->resources.textureStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_TEXTURES, oldTextureCount));
		FreeMem(fixedHeap, pig->resources.textures, sizeof(Texture_t) * oldTextureCount);
		FreeMem(fixedHeap, pig->resources.textureStatus, sizeof(ResourceStatus_t) * oldTextureCount);
		pig->resources.numTexturesAlloc = RESOURCES_NUM_TEXTURES;
		pig->resources.textures = (ResourceTextures_t*)newSpace;
		pig->resources.textureStatus = newStatusSpace;
		u64 numNewTextures = RESOURCES_NUM_TEXTURES - oldTextureCount;
		if (oldTextureCount < RESOURCES_NUM_TEXTURES) { NotifyPrint_N("Loading %llu new texture resource%s...", numNewTextures, (numNewTextures == 1) ? "" : "s"); }
		for (u64 textureIndex = oldTextureCount; textureIndex < RESOURCES_NUM_TEXTURES; textureIndex++) { InitResourceStatus(&pig->resources.textureStatus[textureIndex]); Pig_LoadTextureResource(textureIndex); }
	}
	if (pig->resources.numVectorImgsAlloc != RESOURCES_NUM_VECTORS)
	{
		PrintLine_N("Vector resource count changed: %llu -> %llu", pig->resources.numVectorImgsAlloc, RESOURCES_NUM_VECTORS);
		u64 oldVectorImgCount = pig->resources.numVectorImgsAlloc;
		VectorImg_t* newSpace = AllocArray(fixedHeap, VectorImg_t, RESOURCES_NUM_VECTORS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_VECTORS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.vectors, sizeof(VectorImg_t) * MinU64(RESOURCES_NUM_VECTORS, oldVectorImgCount));
		MyMemCopy(newStatusSpace, pig->resources.vectorStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_VECTORS, oldVectorImgCount));
		FreeMem(fixedHeap, pig->resources.vectors, sizeof(VectorImg_t) * oldVectorImgCount);
		FreeMem(fixedHeap, pig->resources.vectorStatus, sizeof(ResourceStatus_t) * oldVectorImgCount);
		pig->resources.numVectorImgsAlloc = RESOURCES_NUM_VECTORS;
		pig->resources.vectors = (ResourceVectors_t*)newSpace;
		pig->resources.vectorStatus = newStatusSpace;
		u64 numNewVectorImgs = RESOURCES_NUM_VECTORS - oldVectorImgCount;
		if (oldVectorImgCount < RESOURCES_NUM_VECTORS) { NotifyPrint_I("Loading %llu new vector resource%s...", numNewVectorImgs, (numNewVectorImgs == 1) ? "" : "s"); }
		for (u64 vectorIndex = oldVectorImgCount; vectorIndex < RESOURCES_NUM_VECTORS; vectorIndex++) { InitResourceStatus(&pig->resources.vectorStatus[vectorIndex]); Pig_LoadVectorImgResource(vectorIndex); }
	}
	if (pig->resources.numSheetsAlloc != RESOURCES_NUM_SHEETS)
	{
		PrintLine_N("Sprite Sheet resource count changed: %llu -> %llu", pig->resources.numSheetsAlloc, RESOURCES_NUM_SHEETS);
		u64 oldSheetsCount = pig->resources.numSheetsAlloc;
		SpriteSheet_t* newSpace = AllocArray(fixedHeap, SpriteSheet_t, RESOURCES_NUM_SHEETS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SHEETS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.sheets, sizeof(SpriteSheet_t) * MinU64(RESOURCES_NUM_SHEETS, oldSheetsCount));
		MyMemCopy(newStatusSpace, pig->resources.sheetStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_SHEETS, oldSheetsCount));
		FreeMem(fixedHeap, pig->resources.sheets, sizeof(SpriteSheet_t) * oldSheetsCount);
		FreeMem(fixedHeap, pig->resources.sheetStatus, sizeof(ResourceStatus_t) * oldSheetsCount);
		pig->resources.numSheetsAlloc = RESOURCES_NUM_SHEETS;
		pig->resources.sheets = (ResourceSheets_t*)newSpace;
		pig->resources.sheetStatus = newStatusSpace;
		u64 numNewSheets = RESOURCES_NUM_SHEETS - oldSheetsCount;
		if (oldSheetsCount < RESOURCES_NUM_SHEETS) { NotifyPrint_I("Loading %llu new sprite sheet resource%s...", numNewSheets, (numNewSheets == 1) ? "" : "s"); }
		for (u64 sheetIndex = oldSheetsCount; sheetIndex < RESOURCES_NUM_SHEETS; sheetIndex++) { InitResourceStatus(&pig->resources.sheetStatus[sheetIndex]); Pig_LoadSpriteSheetResource(sheetIndex); }
	}
	if (pig->resources.numShadersAlloc != RESOURCES_NUM_SHADERS)
	{
		PrintLine_N("Shader resource count changed: %llu -> %llu", pig->resources.numShadersAlloc, RESOURCES_NUM_SHADERS);
		u64 oldShadersCount = pig->resources.numShadersAlloc;
		Shader_t* newSpace = AllocArray(fixedHeap, Shader_t, RESOURCES_NUM_SHADERS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SHADERS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.shaders, sizeof(Shader_t) * MinU64(RESOURCES_NUM_SHADERS, oldShadersCount));
		MyMemCopy(newStatusSpace, pig->resources.shaderStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_SHADERS, oldShadersCount));
		FreeMem(fixedHeap, pig->resources.shaders, sizeof(Shader_t) * oldShadersCount);
		FreeMem(fixedHeap, pig->resources.shaderStatus, sizeof(ResourceStatus_t) * oldShadersCount);
		pig->resources.numShadersAlloc = RESOURCES_NUM_SHADERS;
		pig->resources.shaders = (ResourceShaders_t*)newSpace;
		pig->resources.shaderStatus = newStatusSpace;
		u64 numNewShaders = RESOURCES_NUM_SHADERS - oldShadersCount;
		if (oldShadersCount < RESOURCES_NUM_SHADERS) { NotifyPrint_I("Loading %llu new shader resource%s...", numNewShaders, (numNewShaders == 1) ? "" : "s"); }
		for (u64 shaderIndex = oldShadersCount; shaderIndex < RESOURCES_NUM_SHADERS; shaderIndex++) { InitResourceStatus(&pig->resources.shaderStatus[shaderIndex]); Pig_LoadShaderResource(shaderIndex); }
	}
	if (pig->resources.numFontsAlloc != RESOURCES_NUM_FONTS)
	{
		PrintLine_N("Font resource count changed: %llu -> %llu", pig->resources.numFontsAlloc, RESOURCES_NUM_FONTS);
		u64 oldFontsCount = pig->resources.numFontsAlloc;
		Font_t* newSpace = AllocArray(fixedHeap, Font_t, RESOURCES_NUM_FONTS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_FONTS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.fonts, sizeof(Font_t) * MinU64(RESOURCES_NUM_FONTS, oldFontsCount));
		MyMemCopy(newStatusSpace, pig->resources.fontStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_FONTS, oldFontsCount));
		FreeMem(fixedHeap, pig->resources.fonts, sizeof(Font_t) * oldFontsCount);
		FreeMem(fixedHeap, pig->resources.fontStatus, sizeof(ResourceStatus_t) * oldFontsCount);
		pig->resources.numFontsAlloc = RESOURCES_NUM_FONTS;
		pig->resources.fonts = (ResourceFonts_t*)newSpace;
		pig->resources.fontStatus = newStatusSpace;
		u64 numNewFonts = RESOURCES_NUM_FONTS - oldFontsCount;
		if (oldFontsCount < RESOURCES_NUM_FONTS) { NotifyPrint_I("Loading %llu new font resource%s...", numNewFonts, (numNewFonts == 1) ? "" : "s"); }
		for (u64 fontIndex = oldFontsCount; fontIndex < RESOURCES_NUM_FONTS; fontIndex++) { InitResourceStatus(&pig->resources.fontStatus[fontIndex]); Pig_LoadFontResource(fontIndex); }
	}
	if (pig->resources.numSoundsAlloc != RESOURCES_NUM_SOUNDS)
	{
		PrintLine_N("Sound resource count changed: %llu -> %llu", pig->resources.numSoundsAlloc, RESOURCES_NUM_SOUNDS);
		u64 oldSoundsCount = pig->resources.numSoundsAlloc;
		Sound_t* newSpace = AllocArray(fixedHeap, Sound_t, RESOURCES_NUM_SOUNDS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_SOUNDS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.sounds, sizeof(Sound_t) * MinU64(RESOURCES_NUM_SOUNDS, oldSoundsCount));
		MyMemCopy(newStatusSpace, pig->resources.soundStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_SOUNDS, oldSoundsCount));
		FreeMem(fixedHeap, pig->resources.sounds, sizeof(Sound_t) * oldSoundsCount);
		FreeMem(fixedHeap, pig->resources.soundStatus, sizeof(ResourceStatus_t) * oldSoundsCount);
		pig->resources.numSoundsAlloc = RESOURCES_NUM_SOUNDS;
		pig->resources.sounds = (ResourceSounds_t*)newSpace;
		pig->resources.soundStatus = newStatusSpace;
		u64 numNewSounds = RESOURCES_NUM_SOUNDS - oldSoundsCount;
		if (oldSoundsCount < RESOURCES_NUM_SOUNDS) { NotifyPrint_I("Loading %llu new sound resource%s...", numNewSounds, (numNewSounds == 1) ? "" : "s"); }
		for (u64 soundIndex = oldSoundsCount; soundIndex < RESOURCES_NUM_SOUNDS; soundIndex++) { InitResourceStatus(&pig->resources.soundStatus[soundIndex]); Pig_LoadSoundResource(soundIndex); }
	}
	if (pig->resources.numMusicsAlloc != RESOURCES_NUM_MUSICS)
	{
		PrintLine_N("Music resource count changed: %llu -> %llu", pig->resources.numMusicsAlloc, RESOURCES_NUM_MUSICS);
		u64 oldMusicsCount = pig->resources.numMusicsAlloc;
		Sound_t* newSpace = AllocArray(fixedHeap, Sound_t, RESOURCES_NUM_MUSICS);
		ResourceStatus_t* newStatusSpace = AllocArray(fixedHeap, ResourceStatus_t, RESOURCES_NUM_MUSICS);
		NotNull2(newSpace, newStatusSpace);
		MyMemCopy(newSpace, pig->resources.musics, sizeof(Sound_t) * MinU64(RESOURCES_NUM_MUSICS, oldMusicsCount));
		MyMemCopy(newStatusSpace, pig->resources.musicStatus, sizeof(ResourceStatus_t) * MinU64(RESOURCES_NUM_MUSICS, oldMusicsCount));
		FreeMem(fixedHeap, pig->resources.musics, sizeof(Sound_t) * oldMusicsCount);
		FreeMem(fixedHeap, pig->resources.musicStatus, sizeof(ResourceStatus_t) * oldMusicsCount);
		pig->resources.numMusicsAlloc = RESOURCES_NUM_MUSICS;
		pig->resources.musics = (ResourceMusics_t*)newSpace;
		pig->resources.musicStatus = newStatusSpace;
		u64 numNewMusics = RESOURCES_NUM_MUSICS - oldMusicsCount;
		if (oldMusicsCount < RESOURCES_NUM_MUSICS) { NotifyPrint_I("Loading %llu new music resource%s...", numNewMusics, (numNewMusics == 1) ? "" : "s"); }
		for (u64 musicIndex = oldMusicsCount; musicIndex < RESOURCES_NUM_MUSICS; musicIndex++) { InitResourceStatus(&pig->resources.musicStatus[musicIndex]); Pig_LoadMusicResource(musicIndex); }
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
// |                      On-Demand Loading                       |
// +--------------------------------------------------------------+
bool IsResourceReady(ResourceType_t type, u64 resourceIndex)
{
	ResourceStatus_t* status = GetResourceStatus(type, resourceIndex);
	NotNull(status);
	return (status->state == ResourceState_Loaded || status->state == ResourceState_Warning);
}
bool IsResourceReady(const void* resourcePntr)
{
	u64 resourceIndex = 0;
	ResourceType_t type = GetResourceByPntr(resourcePntr, &resourceIndex);
	Assert(type != ResourceType_None);
	return IsResourceReady(type, resourceIndex);
}

void PinResource(ResourceType_t type, u64 resourceIndex)
{
	ResourceStatus_t* status = GetResourceStatus(type, resourceIndex);
	NotNull(status);
	status->isPinned = true;
}
void PinResource(const void* resourcePntr)
{
	u64 resourceIndex = 0;
	ResourceType_t type = GetResourceByPntr(resourcePntr, &resourceIndex);
	Assert(type != ResourceType_None);
	PinResource(type, resourceIndex);
}

void PinAllResourcesOfType(ResourceType_t type)
{
	u64 numOfType = GetNumResourcesOfType(type);
	for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
	{
		PinResource(type, rIndex);
	}
}

void AccessResource(ResourceType_t type, u64 resourceIndex)
{
	ResourceStatus_t* status = GetResourceStatus(type, resourceIndex);
	NotNull(status);
	if (status->state == ResourceState_Unloaded)
	{
		Pig_LoadResource(type, resourceIndex);
	}
}
//TODO: We don't really need all these overloads. We could probably just take a const void* straight up.
//      But it is a little more compile-time type safe this way I guess.
void AccessResource(const Texture_t* texture)
{
	u64 textureIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)texture, &textureIndex);
	Assert(type == ResourceType_Texture);
	AccessResource(ResourceType_Texture, textureIndex);
}
void AccessResource(const VectorImg_t* vectorImg)
{
	u64 vectorImgIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)vectorImg, &vectorImgIndex);
	Assert(type == ResourceType_VectorImage);
	AccessResource(ResourceType_VectorImage, vectorImgIndex);
}
void AccessResource(const SpriteSheet_t* sheet)
{
	u64 sheetIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)sheet, &sheetIndex);
	Assert(type == ResourceType_Sheet);
	AccessResource(ResourceType_Sheet, sheetIndex);
}
void AccessResource(const Shader_t* shader)
{
	u64 shaderIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)shader, &shaderIndex);
	Assert(type == ResourceType_Shader);
	AccessResource(ResourceType_Shader, shaderIndex);
}
void AccessResource(const Font_t* font)
{
	u64 fontIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)font, &fontIndex);
	Assert(type == ResourceType_Font);
	AccessResource(ResourceType_Font, fontIndex);
}
void AccessResource(const Sound_t* soundOrMusic)
{
	u64 soundOrMusicIndex = 0;
	ResourceType_t type = GetResourceByPntr((const void*)soundOrMusic, &soundOrMusicIndex);
	Assert(type == ResourceType_Sound || type == ResourceType_Music);
	AccessResource(type, soundOrMusicIndex);
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
bool IsHandleFilled(const SoundHandle_t* handle)
{
	NotNull(handle);
	return (handle->reloadIndex != 0);
}
bool IsHandleFilled(const MusicHandle_t* handle)
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
SoundHandle_t GetSoundHandle(const Sound_t* soundPointer)
{
	SoundHandle_t result = {};
	if (soundPointer == nullptr && pig->resources.sounds != nullptr) { return result; }
	Assert(soundPointer >= (Sound_t*)pig->resources.sounds);
	if (pig->resources.sounds == nullptr) { Assert(soundPointer < ((Sound_t*)pig->resources.sounds) + RESOURCES_NUM_SOUNDS); }
	else { Assert(soundPointer < ((Sound_t*)pig->resources.sounds) + pig->resources.numSoundsAlloc); }
	result.index = (u64)(soundPointer - ((Sound_t*)pig->resources.sounds));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (Sound_t*)soundPointer;
	return result;
}
MusicHandle_t GetMusicHandle(const Sound_t* musicPointer)
{
	MusicHandle_t result = {};
	if (musicPointer == nullptr && pig->resources.musics != nullptr) { return result; }
	Assert(musicPointer >= (Sound_t*)pig->resources.musics);
	if (pig->resources.musics == nullptr) { Assert(musicPointer < ((Sound_t*)pig->resources.musics) + RESOURCES_NUM_MUSICS); }
	else { Assert(musicPointer < ((Sound_t*)pig->resources.musics) + pig->resources.numMusicsAlloc); }
	result.index = (u64)(musicPointer - ((Sound_t*)pig->resources.musics));
	result.reloadIndex = pig->reloadIndex;
	result.pntr = (Sound_t*)musicPointer;
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
Sound_t* GetPointer(SoundHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numSoundsAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.sounds == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.sounds->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
Sound_t* GetPointer(MusicHandle_t* handle)
{
	if (handle->reloadIndex == pig->reloadIndex && handle->pntr != nullptr) { return handle->pntr; }
	Assert(handle->index < pig->resources.numMusicsAlloc);
	if (handle->reloadIndex == 0) { return nullptr; }
	if (pig->resources.musics == nullptr) { return nullptr; }
	handle->pntr = &pig->resources.musics->items[handle->index];
	handle->reloadIndex = pig->reloadIndex;
	return handle->pntr;
}
