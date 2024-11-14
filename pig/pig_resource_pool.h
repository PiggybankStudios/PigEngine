/*
File:   pig_resource_pool.h
Author: Taylor Robbins
Date:   01\23\2023
*/

#ifndef _PIG_RESOURCE_POOL_H
#define _PIG_RESOURCE_POOL_H

struct ResourcePoolEntry_t
{
	u64 id; //doubles as a "filled" flag by using 0 to mean unfilled
	ResourceType_t type;
	u64 arrayIndex;
	u64 refCount;
	u64 lastRefCountChangeTime;
	MyStr_t filePath;
	union
	{
		Texture_t texture;
		VectorImg_t vectorImg;
		SpriteSheet_t spriteSheet;
		Shader_t shader;
		Font_t font;
		Sound_t sound;
		Sound_t music;
		Model_t model;
		VoxFrameSet_t voxFrameSet;
	};
};

struct ResourcePool_t
{
	MemArena_t* allocArena;
	u64 resourceFreeDelay;
	union
	{
		u64 nextId[ResourceType_NumTypes];
		struct
		{
			u64 nextNoneId; //not used
			u64 nextTextureId;
			u64 nextVectorImageId;
			u64 nextSheetId;
			u64 nextShaderId;
			u64 nextFontId;
			u64 nextSoundId;
			u64 nextMusicId;
			u64 nextModelId;
			u64 nextVoxelFrameSetId;
		};
	};
	union
	{
		u64 resourceCounts[ResourceType_NumTypes];
		struct
		{
			u64 numNones; //not used
			u64 numTextures;
			u64 numVectorImages;
			u64 numSheets;
			u64 numShaders;
			u64 numFonts;
			u64 numSounds;
			u64 numMusics;
			u64 numModels;
			u64 numVoxelFrameSets;
		};
	};
	union
	{
		BktArray_t arrays[ResourceType_NumTypes]; //ResourcePoolEntry_t
		struct
		{
			//NOTE: These BktArrays never shrink unless you free the pool.
			// That way we never invalidate pointers to elements because we never
			// remove items from the array, only add or recycle empty slots.
			BktArray_t nones; //not used
			BktArray_t textures;
			BktArray_t vectorImages;
			BktArray_t sheets;
			BktArray_t shaders;
			BktArray_t fonts;
			BktArray_t sounds;
			BktArray_t musics;
			BktArray_t models;
			BktArray_t voxelFrameSets;
		};
	};
};
CompileAssert(ResourceType_NumTypes == 10); //Update the above code if ResourceType_NumTypes changes

struct TextureRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Texture_t* pntr;
};
struct VectorImgRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	VectorImg_t* pntr;
};
struct SpriteSheetRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	SpriteSheet_t* pntr;
};
struct ShaderRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Shader_t* pntr;
};
struct FontRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Font_t* pntr;
};
struct SoundRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Sound_t* pntr;
};
struct MusicRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Sound_t* pntr;
};
struct ModelRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	Model_t* pntr;
};
struct VoxFrameSetRef_t
{
	ResourcePool_t* pool;
	u64 arrayIndex;
	VoxFrameSet_t* pntr;
};

bool IsValidRef(const TextureRef_t&     reference) { return (reference.pool != nullptr); }
bool IsValidRef(const VectorImgRef_t&   reference) { return (reference.pool != nullptr); }
bool IsValidRef(const SpriteSheetRef_t& reference) { return (reference.pool != nullptr); }
bool IsValidRef(const ShaderRef_t&      reference) { return (reference.pool != nullptr); }
bool IsValidRef(const FontRef_t&        reference) { return (reference.pool != nullptr); }
bool IsValidRef(const SoundRef_t&       reference) { return (reference.pool != nullptr); }
bool IsValidRef(const MusicRef_t&       reference) { return (reference.pool != nullptr); }
bool IsValidRef(const ModelRef_t&       reference) { return (reference.pool != nullptr); }
bool IsValidRef(const VoxFrameSetRef_t& reference) { return (reference.pool != nullptr); }

#define TextureRef_Invalid     TextureRef_t()
#define VectorImgRef_Invalid   VectorImgRef_t()
#define SpriteSheetRef_Invalid SpriteSheetRef_t()
#define ShaderRef_Invalid      ShaderRef_t()
#define FontRef_Invalid        FontRef_t()
#define SoundRef_Invalid       SoundRef_t()
#define MusicRef_Invalid       MusicRef_t()
#define ModelRef_Invalid       ModelRef_t()
#define VoxFrameSetRef_Invalid VoxFrameSetRef_t()

#endif //  _PIG_RESOURCE_POOL_H
