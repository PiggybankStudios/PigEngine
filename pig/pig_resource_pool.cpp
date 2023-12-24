/*
File:   pig_resource_pool.cpp
Author: Taylor Robbins
Date:   01\23\2023
Description: 
	** Traditionally we have opted to name all of our resources and put them into a proper structure in code.
	** This is really useful so that we can treat resources as a well tooled runtime concept.
	** However sometimes we want to let the artists and designers add content to the game in a very generic way
	** without requiring an engineer to do any actual runtime hook-up. These kinds of assets are normally
	** referenced by other assets (like a level definition, or a cutscene file) and therefore do not
	** need a proper name or a dedicated slot in the Resources_t structure.
	**
	** A resource pool is meant to handle loading resources in this content-driven manner.
	** Every item in the resource pool is manually reference counted.
	** The "id" for a resource is it's file path so the reference is created using that string.
	** Every reference counts as a tracked reference increment and should be released when the
	** resource is no longer required by a particular party. If multiple parties use the same resource
	** then it may outlive a single references getting released (since other reference still exists),
	** but eventually when all refrences are released, the resource will get released
	**
	** This file relies on some types from pig_resources.h
*/

#define DEFAULT_RESOURCE_POOL_FREE_DELAY 1000 //ms

// +--------------------------------------------------------------+
// |                             Free                             |
// +--------------------------------------------------------------+
void FreeResourcePoolEntry(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	if (entry->id != 0)
	{
		NotNull(pool->allocArena);
		PrintLine_W("Releasing Resource Pool %s[%llu]", GetResourceTypeStr(entry->type), entry->arrayIndex);
		FreeString(pool->allocArena, &entry->filePath);
		switch (entry->type)
		{
			case ResourceType_Texture:     DestroyTexture(&entry->texture);         break;
			case ResourceType_VectorImage: DestroyVectorImg(&entry->vectorImg);     break;
			case ResourceType_Sheet:       DestroySpriteSheet(&entry->spriteSheet); break;
			case ResourceType_Shader:      DestroyShader(&entry->shader);           break;
			case ResourceType_Font:        DestroyFont(&entry->font);               break;
			case ResourceType_Sound:       FreeSound(&entry->sound);                break;
			case ResourceType_Music:       FreeSound(&entry->music);                break;
			case ResourceType_Model:       DestroyModel(&entry->model);             break;
			default: AssertMsg(false, "Unhandle ResourceType_t in FreeResourcePoolEntry"); break;
		}
		entry->id = 0;
	}
	//NOTE: We DON'T ClearPointer here because we want to type and arrayIndex members to stay intact
}
void ClearResourcePoolArray(ResourcePool_t* pool, ResourceType_t resourceType, bool deallocate, bool printLeaks = false)
{
	NotNull(pool);
	Assert(resourceType < ResourceType_NumTypes);
	Assert(resourceType != ResourceType_None);
	BktArray_t* array = &pool->arrays[resourceType];
	for (u64 eIndex = 0; eIndex < array->length; eIndex++)
	{
		ResourcePoolEntry_t* entry = BktArrayGet(array, ResourcePoolEntry_t, eIndex);
		DebugAssert(entry->type == resourceType);
		DebugAssert(entry->arrayIndex == eIndex);
		if (printLeaks && entry->id != 0 && entry->refCount > 0)
		{
			PrintLine_E("Resource Pool %s[%llu] leaked %llu reference%s! \"%.*s\"", GetResourceTypeStr(resourceType), eIndex, entry->refCount, ((entry->refCount == 1) ? "" : "s"), StrPrint(entry->filePath));
		}
		if (resourceType == ResourceType_Sound || resourceType == ResourceType_Music)
		{
			StopAllSoundInstancesForSound(&entry->sound);
		}
		FreeResourcePoolEntry(pool, entry);
	}
	if (deallocate) { FreeBktArray(array); }
	else { BktArrayClear(array); }
	pool->resourceCounts[resourceType] = 0;
}
void ClearResourcePool(ResourcePool_t* pool, bool deallocate = false, bool printLeaks = false)
{
	for (u64 tIndex = 1; tIndex < ResourceType_NumTypes; tIndex++)
	{
		ClearResourcePoolArray(pool, (ResourceType_t)tIndex, deallocate, printLeaks);
	}
}
void FreeResourcePool(ResourcePool_t* pool, bool printLeaks = true)
{
	ClearResourcePool(pool, true, printLeaks);
	ClearPointer(pool);
}

// +--------------------------------------------------------------+
// |                            Create                            |
// +--------------------------------------------------------------+
//NOTE: Although you can define a memArena the pool itself to use, the resources
// will get allocated mostly through the mainHeap (audio being the exception on the audioHeap)
void CreateResourcePool(MemArena_t* memArena, ResourcePool_t* poolOut, u64 resourceFreeDelay = DEFAULT_RESOURCE_POOL_FREE_DELAY)
{
	ClearPointer(poolOut);
	poolOut->allocArena = memArena;
	poolOut->resourceFreeDelay = resourceFreeDelay;
	for (u64 typeIndex = 1; typeIndex < ResourceType_NumTypes; typeIndex++)
	{
		BktArray_t* array = &poolOut->arrays[typeIndex];
		CreateBktArray(array, memArena, sizeof(ResourcePoolEntry_t));
		poolOut->nextId[typeIndex] = 1;
		poolOut->resourceCounts[typeIndex] = 0;
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateResourcePool(ResourcePool_t* pool)
{
	for (u64 typeIndex = 1; typeIndex < ResourceType_NumTypes; typeIndex++)
	{
		ResourceType_t resourceType = (ResourceType_t)typeIndex;
		BktArray_t* array = &pool->arrays[typeIndex];
		for (u64 eIndex = 0; eIndex < array->length; eIndex++)
		{
			ResourcePoolEntry_t* entry = BktArrayGet(array, ResourcePoolEntry_t, eIndex);
			if (entry->id != 0 && entry->refCount == 0 && TimeSince(entry->lastRefCountChangeTime) > pool->resourceFreeDelay)
			{
				if (resourceType == ResourceType_Sound || resourceType == ResourceType_Music)
				{
					StopAllSoundInstancesForSound(&entry->sound);
				}
				FreeResourcePoolEntry(pool, entry);
				DebugAssert(pool->resourceCounts[typeIndex] > 0);
				Decrement(pool->resourceCounts[typeIndex]);
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
ResourcePoolEntry_t* FindEmptyResourcePoolEntry(BktArray_t* array, bool addIfFull = true)
{
	for (u64 eIndex = 0; eIndex < array->length; eIndex++)
	{
		ResourcePoolEntry_t* entry = BktArrayGet(array, ResourcePoolEntry_t, eIndex);
		if (entry->id == 0) { return entry; }
	}
	if (addIfFull)
	{
		ResourcePoolEntry_t* newEntry = BktArrayAdd(array, ResourcePoolEntry_t);
		newEntry->id = 0; // will probably get overridden by caller, but let's make sure it's cleared to 0 to show empty
		newEntry->arrayIndex = array->length-1;
		return newEntry;
	}
	return nullptr;
}

ResourcePoolEntry_t* FindResourcePoolEntryByPath(BktArray_t* array, MyStr_t filePath)
{
	for (u64 eIndex = 0; eIndex < array->length; eIndex++)
	{
		ResourcePoolEntry_t* entry = BktArrayGet(array, ResourcePoolEntry_t, eIndex);
		if (entry->id != 0 && StrEqualsIgnoreCase(entry->filePath, filePath)) { return entry; }
	}
	return nullptr;
}
ResourcePoolEntry_t* FindResourcePoolEntryByFilename(BktArray_t* array, MyStr_t filename)
{
	for (u64 eIndex = 0; eIndex < array->length; eIndex++)
	{
		ResourcePoolEntry_t* entry = BktArrayGet(array, ResourcePoolEntry_t, eIndex);
		if (StrEqualsIgnoreCase(GetFileNamePart(entry->filePath), filename)) { return entry; }
	}
	return nullptr;
}

// +--------------------------------------------------------------+
// |                TakeRef and Release Functions                 |
// +--------------------------------------------------------------+
TextureRef_t TakeRefTexture(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Texture);
	TextureRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->texture;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
VectorImgRef_t TakeRefVectorImg(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_VectorImage);
	VectorImgRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->vectorImg;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
SpriteSheetRef_t TakeRefSpriteSheet(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Sheet);
	SpriteSheetRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->spriteSheet;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
ShaderRef_t TakeRefShader(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Shader);
	ShaderRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->shader;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
FontRef_t TakeRefFont(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Font);
	FontRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->font;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
SoundRef_t TakeRefSound(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Sound);
	SoundRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->sound;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
MusicRef_t TakeRefMusic(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Music);
	MusicRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->music;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}
ModelRef_t TakeRefModel(ResourcePool_t* pool, ResourcePoolEntry_t* entry)
{
	NotNull2(pool, entry);
	Assert(entry->id != 0);
	Assert(entry->type == ResourceType_Model);
	ModelRef_t result;
	result.pool = pool;
	result.arrayIndex = entry->arrayIndex;
	result.pntr = &entry->model;
	IncrementU64(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	return result;
}

TextureRef_t DuplicateRefTexture(const TextureRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->textures.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->textures, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefTexture(reference->pool, entry);
}
VectorImgRef_t DuplicateRefVectorImg(const VectorImgRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->vectorImages.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->vectorImages, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefVectorImg(reference->pool, entry);
}
SpriteSheetRef_t DuplicateRefSpriteSheet(const SpriteSheetRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->sheets.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->sheets, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefSpriteSheet(reference->pool, entry);
}
ShaderRef_t DuplicateRefShader(const ShaderRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->shaders.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->shaders, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefShader(reference->pool, entry);
}
FontRef_t DuplicateRefFont(const FontRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->fonts.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->fonts, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefFont(reference->pool, entry);
}
SoundRef_t DuplicateRefSound(const SoundRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->sounds.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->sounds, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefSound(reference->pool, entry);
}
MusicRef_t DuplicateRefMusic(const MusicRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->musics.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->musics, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefMusic(reference->pool, entry);
}
ModelRef_t DuplicateRefModel(const ModelRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	Assert(reference->arrayIndex < reference->pool->models.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->models, ResourcePoolEntry_t, reference->arrayIndex);
	return TakeRefModel(reference->pool, entry);
}

void ReleaseRef(TextureRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->textures.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->textures, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool texture was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(VectorImgRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->vectorImages.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->vectorImages, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool vectorImg was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(SpriteSheetRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->sheets.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->sheets, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool sheet was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(ShaderRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->shaders.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->shaders, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool shader was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(FontRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->fonts.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->fonts, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool font was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(SoundRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->sounds.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->sounds, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool sound was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(MusicRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->musics.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->musics, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool music was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}
void ReleaseRef(ModelRef_t* reference)
{
	NotNull(reference);
	NotNull(reference->pool);
	NotNull(reference->pntr);
	Assert(reference->arrayIndex < reference->pool->models.length);
	ResourcePoolEntry_t* entry = BktArrayGet(&reference->pool->models, ResourcePoolEntry_t, reference->arrayIndex);
	AssertMsg(entry->id != 0, "The resource pool model was already freed! This is a double release situation!");
	Assert(entry->refCount > 0);
	Decrement(entry->refCount);
	entry->lastRefCountChangeTime = ProgramTime;
	ClearPointer(reference);
}

void SoftReleaseRef(TextureRef_t*     reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(VectorImgRef_t*   reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(SpriteSheetRef_t* reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(ShaderRef_t*      reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(FontRef_t*        reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(SoundRef_t*       reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(MusicRef_t*       reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }
void SoftReleaseRef(ModelRef_t*       reference) { NotNull(reference); if (!IsValidRef(*reference)) { return; } ReleaseRef(reference); }

// +--------------------------------------------------------------+
// |                       Main Loading API                       |
// +--------------------------------------------------------------+
//NOTE: Load functions will NOT prevent you from adding multiple entries for the same file.
// Though if you try and look up the entry by filePath later you will get whichever entry was
// added first until it has been fully released

//NOTE: In GetOrLoad functions, if your loading options do not match a previously loaded version of this file,
// you will silently get a resource that was loaded with different options than you asked for

TextureRef_t ResourcePoolLoadTexture(ResourcePool_t* pool, MyStr_t filePath, bool pixelated, bool repeating, PlatImageData_t* imageDataOut = nullptr)
{
	Texture_t tempTexture;
	bool loadSuccess = LoadTexture(mainHeap, &tempTexture, filePath, pixelated, repeating, imageDataOut);
	if (!loadSuccess)
	{
		PrintLine_E("Failed to load Texture for pool from \"%.*s\": Error %s", StrPrint(filePath), GetTextureErrorStr(tempTexture.error));
		return TextureRef_Invalid;
	}
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->textures, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextTextureId;
	pool->nextTextureId++;
	pool->numTextures++;
	newEntry->type = ResourceType_Texture;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->texture, &tempTexture, sizeof(Texture_t));
	
	return TakeRefTexture(pool, newEntry);
}
TextureRef_t ResourcePoolGetOrLoadTexture(ResourcePool_t* pool, MyStr_t filePath, bool pixelated, bool repeating, PlatImageData_t* imageDataOut = nullptr)
{
	if (imageDataOut == nullptr)
	{
		ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->textures, filePath);
		if (existingEntry != nullptr) { return TakeRefTexture(pool, existingEntry); }
	}
	return ResourcePoolLoadTexture(pool, filePath, pixelated, repeating, imageDataOut);
}

VectorImgRef_t ResourcePoolLoadVectorImg(ResourcePool_t* pool, MyStr_t filePath)
{
	VectorImg_t tempVectorImg = {};
	Unimplemented(); //TODO: Implement me!
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->vectorImages, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextVectorImageId;
	pool->nextVectorImageId++;
	pool->numVectorImages++;
	newEntry->type = ResourceType_VectorImage;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->vectorImg, &tempVectorImg, sizeof(VectorImg_t));
	
	return TakeRefVectorImg(pool, newEntry);
}
VectorImgRef_t ResourcePoolGetOrLoadVectorImg(ResourcePool_t* pool, MyStr_t filePath)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->vectorImages, filePath);
	if (existingEntry != nullptr) { return TakeRefVectorImg(pool, existingEntry); }
	return ResourcePoolLoadVectorImg(pool, filePath);
}

SpriteSheetRef_t ResourcePoolLoadSpriteSheet(ResourcePool_t* pool, MyStr_t filePath, v2i padding, v2i numFrames, bool pixelated, bool useTextureArray)
{
	SpriteSheet_t tempSheet;
	bool loadSuccess = LoadSpriteSheet(mainHeap, &tempSheet, filePath, padding, numFrames, pixelated, useTextureArray);
	if (!loadSuccess)
	{
		PrintLine_E("Failed to load SpriteSheet for pool from \"%.*s\": Error %s", StrPrint(filePath), GetSpriteSheetErrorStr(tempSheet.error));
		return SpriteSheetRef_Invalid;
	}
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->sheets, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextSheetId;
	pool->nextSheetId++;
	pool->numSheets++;
	newEntry->type = ResourceType_Sheet;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->spriteSheet, &tempSheet, sizeof(SpriteSheet_t));
	
	return TakeRefSpriteSheet(pool, newEntry);
}
SpriteSheetRef_t ResourcePoolGetOrLoadSpriteSheet(ResourcePool_t* pool, MyStr_t filePath, v2i padding, v2i numFrames, bool pixelated, bool useTextureArray)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->sheets, filePath);
	if (existingEntry != nullptr) { return TakeRefSpriteSheet(pool, existingEntry); }
	return ResourcePoolLoadSpriteSheet(pool, filePath, padding, numFrames, pixelated, useTextureArray);
}

ShaderRef_t ResourcePoolLoadShader(ResourcePool_t* pool, MyStr_t filePath, VertexType_t vertexType, u32 requiredUniforms)
{
	Shader_t tempShader;
	bool loadSuccess = LoadShader(mainHeap, &tempShader, filePath, vertexType, requiredUniforms);
	if (!loadSuccess)
	{
		//TODO: Copy the nice error output from Pig_LoadShaderResource
		PrintLine_E("Failed to load Shader for pool from \"%.*s\": Error %s", StrPrint(filePath), GetShaderErrorStr(tempShader.error));
		return ShaderRef_Invalid;
	}
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->shaders, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextShaderId;
	pool->nextShaderId++;
	pool->numShaders++;
	newEntry->type = ResourceType_Shader;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->shader, &tempShader, sizeof(Shader_t));
	
	return TakeRefShader(pool, newEntry);
}
ShaderRef_t ResourcePoolGetOrLoadShader(ResourcePool_t* pool, MyStr_t filePath, VertexType_t vertexType, u32 requiredUniforms)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->shaders, filePath);
	if (existingEntry != nullptr) { return TakeRefShader(pool, existingEntry); }
	return ResourcePoolLoadShader(pool, filePath, vertexType, requiredUniforms);
}

FontRef_t ResourcePoolLoadFont(ResourcePool_t* pool, MyStr_t filePath)
{
	Font_t tempFont = {};
	Unimplemented(); //TODO: Implement me!
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->fonts, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextFontId;
	pool->nextFontId++;
	pool->numFonts++;
	newEntry->type = ResourceType_Font;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->font, &tempFont, sizeof(Font_t));
	
	return TakeRefFont(pool, newEntry);
}
FontRef_t ResourcePoolGetOrLoadFont(ResourcePool_t* pool, MyStr_t filePath)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->fonts, filePath);
	if (existingEntry != nullptr) { return TakeRefFont(pool, existingEntry); }
	return ResourcePoolLoadFont(pool, filePath);
}

SoundRef_t ResourcePoolLoadSound(ResourcePool_t* pool, MyStr_t filePath)
{
	ProcessLog_t soundParseLog = {}; CreateDefaultProcessLog(&soundParseLog);
	Sound_t tempSound = {};
	if (!TryLoadSoundOggOrWav(&soundParseLog, filePath, &pig->audioHeap, &tempSound))
	{
		PrintLine_E("Failed to load sound for pool from \"%.*s\"", StrPrint(filePath));
		DumpProcessLog(&soundParseLog, "Sound Parse Log");
		return SoundRef_Invalid;
	}
	else if (soundParseLog.hadErrors || soundParseLog.hadWarnings) { DumpProcessLog(&soundParseLog, "Sound Parse Log"); }
	FreeProcessLog(&soundParseLog);
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->sounds, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextSoundId;
	pool->nextSoundId++;
	pool->numSounds++;
	newEntry->type = ResourceType_Sound;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->sound, &tempSound, sizeof(Sound_t));
	
	return TakeRefSound(pool, newEntry);
}
SoundRef_t ResourcePoolGetOrLoadSound(ResourcePool_t* pool, MyStr_t filePath)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->sounds, filePath);
	AssertIf(existingEntry != nullptr, existingEntry->id != 0);
	if (existingEntry != nullptr) { return TakeRefSound(pool, existingEntry); }
	return ResourcePoolLoadSound(pool, filePath);
}

MusicRef_t ResourcePoolLoadMusic(ResourcePool_t* pool, MyStr_t filePath)
{
	ProcessLog_t musicParseLog = {}; CreateDefaultProcessLog(&musicParseLog);
	Sound_t tempMusic = {};
	if (!TryLoadSoundOggOrWav(&musicParseLog, filePath, &pig->audioHeap, &tempMusic))
	{
		PrintLine_E("Failed to load music for pool from \"%.*s\"", StrPrint(filePath));
		DumpProcessLog(&musicParseLog, "Music Parse Log");
		return MusicRef_Invalid;
	}
	else if (musicParseLog.hadErrors || musicParseLog.hadWarnings) { DumpProcessLog(&musicParseLog, "Music Parse Log"); }
	FreeProcessLog(&musicParseLog);
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->musics, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextMusicId;
	pool->nextMusicId++;
	pool->numMusics++;
	newEntry->type = ResourceType_Music;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->music, &tempMusic, sizeof(Sound_t));
	
	return TakeRefMusic(pool, newEntry);
}
MusicRef_t ResourcePoolGetOrLoadMusic(ResourcePool_t* pool, MyStr_t filePath)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->musics, filePath);
	if (existingEntry != nullptr) { return TakeRefMusic(pool, existingEntry); }
	return ResourcePoolLoadMusic(pool, filePath);
}

ModelRef_t ResourcePoolLoadModel(ResourcePool_t* pool, MyStr_t filePath, ModelTextureType_t textureType, bool copyVertices, bool flipUvY, ProcessLog_t* log = nullptr)
{
	ProcessLog_t modelParseLog = {};
	if (log == nullptr) { CreateDefaultProcessLog(&modelParseLog); log = &modelParseLog; }
	Model_t tempModel = {};
	if (!TryLoadModel(log, filePath, textureType, copyVertices, flipUvY, pool->allocArena, &tempModel))
	{
		PrintLine_E("Failed to load model for pool from \"%.*s\"", StrPrint(filePath));
		if (log == nullptr) { DumpProcessLog(&modelParseLog, "Model Parse Log"); }
		return ModelRef_Invalid;
	}
	else if (log == nullptr && (modelParseLog.hadErrors || modelParseLog.hadWarnings)) { DumpProcessLog(&modelParseLog, "Model Parse Log"); }
	if (log == nullptr) { FreeProcessLog(&modelParseLog); }
	
	ResourcePoolEntry_t* newEntry = FindEmptyResourcePoolEntry(&pool->models, true);
	DebugAssert(newEntry != nullptr);
	newEntry->id = pool->nextModelId;
	pool->nextModelId++;
	pool->numModels++;
	newEntry->type = ResourceType_Model;
	newEntry->refCount = 0;
	newEntry->lastRefCountChangeTime = ProgramTime;
	newEntry->filePath = AllocString(pool->allocArena, &filePath);
	MyMemCopy(&newEntry->model, &tempModel, sizeof(Model_t));
	
	return TakeRefModel(pool, newEntry);
}
ModelRef_t ResourcePoolGetOrLoadModel(ResourcePool_t* pool, MyStr_t filePath, ModelTextureType_t textureType, bool copyVertices, bool flipUvY, ProcessLog_t* log = nullptr)
{
	ResourcePoolEntry_t* existingEntry = FindResourcePoolEntryByPath(&pool->models, filePath);
	if (existingEntry != nullptr) { return TakeRefModel(pool, existingEntry); }
	return ResourcePoolLoadModel(pool, filePath, textureType, copyVertices, flipUvY, log);
}
