/*
File:   pig_slug.h
Author: Taylor Robbins
Date:   06\02\2022
*/

#ifndef _PIG_SLUG_H
#define _PIG_SLUG_H

struct SlugAlbum_t
{
	bool isValid;
	MemArena_t* allocArena;
	Texture_t curveTexture;
	Texture_t bandTexture;
	MyStr_t pictureData;
	MyStr_t meshVertexData;
	MyStr_t meshTriangleData;
	Shader_t pipelineShader;
};

struct SlugFont_t
{
	bool isValid;
	MemArena_t* allocArena;
	u64 fileContentsSize;
	union
	{
		void* fileContentsVoidPntr;
		u8* fileContentsBytePntr;
	};
	Terathon::Slug::FontHeader* fontHeader;
	Texture_t curveTexture;
	Texture_t bandTexture;
	Shader_t pipelineShader;
};

struct SlugText_t
{
	MemArena_t* allocArena;
	MyStr_t text;
	SlugFont_t* font;
	Terathon::Slug::LayoutData layoutData;
	VertBuffer_t vertBuffer;
};

#endif //  _PIG_SLUG_H
