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
		default: return "Unknown";
	}
}

struct ResourceWatch_t
{
	ResourceType_t type;
	u64 resourceIndex;
	PlatWatchedFile_t* watchedFile;
};

struct ResourceSheetMetaInfo_t
{
	v2i numFrames;
	v2i padding;
	bool pixelated;
	MyStr_t metaFilePath;
};

struct ResourceShaderMetaInfo_t
{
	VertexType_t vertexType;
	u32 requiredUniforms;
};

#define MAX_NUM_RESOURCE_FONT_FACES 8
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
	bool requestFromPlatform;
	ResourceFontFaceMetaInfo_t faces[MAX_NUM_RESOURCE_FONT_FACES];
};

#endif //  _PIG_RESOURCES_H
