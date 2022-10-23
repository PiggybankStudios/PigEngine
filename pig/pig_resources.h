/*
File:   pig_resources.h
Author: Taylor Robbins
Date:   11\04\2021
*/

#ifndef _PIG_RESOURCES_H
#define _PIG_RESOURCES_H

// +--------------------------------------------------------------+
// |                     Resources Structure                      |
// +--------------------------------------------------------------+
enum ResourceType_t
{
	ResourceType_None,
	ResourceType_Texture,
	ResourceType_VectorImage,
	ResourceType_Sheet,
	ResourceType_Shader,
	ResourceType_Font,
	ResourceType_Sound,
	ResourceType_Music,
	ResourceType_NumTypes,
};
const char* GetResourceTypeStr(ResourceType_t resourceType)
{
	switch (resourceType)
	{
		case ResourceType_None:        return "None";
		case ResourceType_Texture:     return "Texture";
		case ResourceType_VectorImage: return "VectorImage";
		case ResourceType_Sheet:       return "Sheet";
		case ResourceType_Shader:      return "Shader";
		case ResourceType_Font:        return "Font";
		case ResourceType_Sound:       return "Sound";
		case ResourceType_Music:       return "Music";
		default: return "Unknown";
	}
}

enum ResourceState_t
{
	ResourceState_None = 0,
	ResourceState_Unloaded, //we've never tried to load, or we have unloaded it for non-use
	ResourceState_Error,    //we've tried to load, but failed
	ResourceState_Warning,  //failed reload, or failed to load all metadata, but we have a valid value (still probably usable)
	ResourceState_Loaded,   //Loaded and ready to go
	ResourceState_NumStates,
};
const char* GetResourceStateStr(ResourceState_t resourceState)
{
	switch (resourceState)
	{
		case ResourceState_None:     return "None";
		case ResourceState_Unloaded: return "Unloaded";
		case ResourceState_Error:    return "Error";
		case ResourceState_Warning:  return "Warning";
		case ResourceState_Loaded:   return "Loaded";
		default: return "Unknown";
	}
}

ResourceState_t ResourceStateWarnOrError(ResourceState_t state)
{
	switch (state)
	{
		case ResourceState_Unloaded: return ResourceState_Error;
		case ResourceState_Error:    return ResourceState_Error;
		case ResourceState_Warning:  return ResourceState_Warning;
		case ResourceState_Loaded:   return ResourceState_Warning;
		default: return ResourceState_Error;
	}
}

struct ResourceStatus_t
{
	ResourceState_t state;
	u64 lastAccessTime;
	bool isPinned; //keeps the resource loaded without needing to call AccessResource
};

struct ResourceWatch_t
{
	ResourceType_t type;
	u64 resourceIndex;
	PlatWatchedFile_t* watchedFile;
};

#define RESOURCE_TEXTURE_MAX_NUM_LAYERS   256

struct ResourceTextureMetaInfo_t
{
	bool pixelated;
	bool repeating;
	u64 numLayers;
	const char* filePaths[RESOURCE_TEXTURE_MAX_NUM_LAYERS];
};

struct ResourceSheetMetaInfo_t
{
	v2i numFrames;
	v2i padding;
	bool pixelated;
	bool useTextureArray;
	MyStr_t metaFilePath;
};

struct ResourceShaderMetaInfo_t
{
	VertexType_t vertexType;
	u32 requiredUniforms;
};

#define MAX_NUM_RESOURCE_FONT_FACES 12
#define MAX_NUM_RESOURCE_FONT_BAKES 4

struct ResourceFontFaceMetaInfo_t
{
	bool isSpriteFont;
	
	//shared options
	bool bold;
	bool italic;
	MyStr_t name;
	
	//TTF Options
	i32 size;
	MyStr_t fontName;
	v2i bakeSize;
	bool includeCyrillicBake;
	bool includeJapaneseKanaBake;
	bool includeBtnsSheet;
	
	//Sprite Font options
	MyStr_t filePaths[MAX_NUM_RESOURCE_FONT_BAKES]; //overrides the const char* returned from Resources_GetPathOrNameForFont
	MyStr_t metaFilePaths[MAX_NUM_RESOURCE_FONT_BAKES];
	v2i sheetSizes[MAX_NUM_RESOURCE_FONT_BAKES];
	v2i paddings[MAX_NUM_RESOURCE_FONT_BAKES];
	bool isPixelated[MAX_NUM_RESOURCE_FONT_BAKES];
	bool scalables[MAX_NUM_RESOURCE_FONT_BAKES];
	bool colored[MAX_NUM_RESOURCE_FONT_BAKES];
};
struct ResourceFontMetaInfo_t
{
	const char* fontName;
	bool requestFromPlatform;
	ResourceFontFaceMetaInfo_t faces[MAX_NUM_RESOURCE_FONT_FACES];
};

//TODO: We could make these be straight up typedefs in release mode?
//NOTE: The reloadIndex is also acting like a "filled" flag since 0 is an invalid reloadIndex
struct TextureHandle_t
{
	u64 index;
	u64 reloadIndex;
	Texture_t* pntr;
};
struct VectorImgHandle_t
{
	u64 index;
	u64 reloadIndex;
	VectorImg_t* pntr;
};
struct SpriteSheetHandle_t
{
	u64 index;
	u64 reloadIndex;
	SpriteSheet_t* pntr;
};
struct ShaderHandle_t
{
	u64 index;
	u64 reloadIndex;
	Shader_t* pntr;
};
struct FontHandle_t
{
	u64 index;
	u64 reloadIndex;
	Font_t* pntr;
};
struct SoundHandle_t
{
	u64 index;
	u64 reloadIndex;
	Sound_t* pntr;
};
struct MusicHandle_t
{
	u64 index;
	u64 reloadIndex;
	Sound_t* pntr;
};

#include "game_resources.h"

struct Resources_t
{
	#if DEVELOPER_BUILD
	VarArray_t watches; //ResourceWatch_t
	#endif
	
	u64 numTexturesAlloc;
	u64 numVectorImgsAlloc;
	u64 numSheetsAlloc;
	u64 numShadersAlloc;
	u64 numFontsAlloc;
	u64 numSoundsAlloc;
	u64 numMusicsAlloc;
	
	ResourceStatus_t* textureStatus;
	ResourceStatus_t* vectorStatus;
	ResourceStatus_t* sheetStatus;
	ResourceStatus_t* shaderStatus;
	ResourceStatus_t* fontStatus;
	ResourceStatus_t* soundStatus;
	ResourceStatus_t* musicStatus;
	
	ResourceTextures_t* textures;
	ResourceVectors_t*  vectors;
	ResourceSheets_t*   sheets;
	ResourceShaders_t*  shaders;
	ResourceFonts_t*    fonts;
	ResourceSounds_t*   sounds;
	ResourceMusics_t*   musics;
};

#endif //  _PIG_RESOURCES_H
