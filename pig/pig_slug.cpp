/*
File:   pig_slug.cpp
Author: Taylor Robbins
Date:   06\02\2022
Description: 
	** Holds functions that help us interop with the Slug font rendering library
*/

#if SLUG_SUPPORTED

#define SLUG_COMPRESSION_TYPE1 'TCMP'

bool CreateSlugPipelineShader(MemArena_t* memArena, Shader_t* shaderOut, u32 slugRenderFlags)
{
	u32 vertexShaderIndex = 0;
	u32 fragmentShaderIndex = 0;
	Terathon::Slug::GetShaderIndices(slugRenderFlags, &vertexShaderIndex, &fragmentShaderIndex);
	
	const char* vertShaderPieces[Terathon::Slug::kMaxVertexStringCount + 1]; //Add 1 line to store GLSL #version directive
	const char* fragShaderPieces[Terathon::Slug::kMaxFragmentStringCount + 1];
	vertShaderPieces[0] = "#version 330\n";
	fragShaderPieces[0] = "#version 330\n";
	u64 vertShaderPieceCount = (u64)Terathon::Slug::GetVertexShaderSourceCode(vertexShaderIndex, &vertShaderPieces[1]);
	u64 fragShaderPieceCount = (u64)Terathon::Slug::GetFragmentShaderSourceCode(fragmentShaderIndex, &fragShaderPieces[1]);
	vertShaderPieceCount++;
	fragShaderPieceCount++;
	
	// PrintLine_D("Vertex Shader (%llu pieces)", vertShaderPieceCount);
	// for (u64 lineIndex = 0; lineIndex < vertShaderPieceCount; lineIndex++)
	// {
	// 	Print_D("\t%s", vertShaderPieces[lineIndex]);
	// }
	// WriteLine_D("[End vertex shader]");
	// PrintLine_D("Fragment Shader (%llu pieces)", fragShaderPieceCount);
	// for (u64 lineIndex = 0; lineIndex < fragShaderPieceCount; lineIndex++)
	// {
	// 	Print_D("\t%s", fragShaderPieces[lineIndex]);
	// }
	// WriteLine_D("[End fragment shader]");
	
	CreateShaderMultiPieceStart(memArena, shaderOut, VertexType_Slug, ShaderUniform_None, vertShaderPieceCount, fragShaderPieceCount);
	// PlatOpenFile_t vertexShaderFile;
	// plat->OpenFile(NewStr("slug_vertex.glsl"), true, true, &vertexShaderFile);
	for (u64 pIndex = 0; pIndex < vertShaderPieceCount; pIndex++)
	{
		CreateShaderMultiPieceVertex(shaderOut, NewStr(vertShaderPieces[pIndex]));
		// plat->WriteToFile(&vertexShaderFile, MyStrLength64(vertShaderPieces[pIndex]), vertShaderPieces[pIndex], true);
	}
	// plat->CloseFile(&vertexShaderFile);
	// PlatOpenFile_t fragmentShaderFile;
	// plat->OpenFile(NewStr("slug_fragment.glsl"), true, true, &fragmentShaderFile);
	for (u64 pIndex = 0; pIndex < fragShaderPieceCount; pIndex++)
	{
		CreateShaderMultiPieceFragment(shaderOut, NewStr(fragShaderPieces[pIndex]));
		// plat->WriteToFile(&fragmentShaderFile, MyStrLength64(fragShaderPieces[pIndex]), fragShaderPieces[pIndex], true);
	}
	// plat->CloseFile(&fragmentShaderFile);
	if (!CreateShaderMultiPieceEnd(shaderOut))
	{
		//TODO: Print out the error infomration from the shader
		PrintLine_E("Failed to compile slug pipeline shader [%d, %d]", vertexShaderIndex, fragmentShaderIndex);
		return false;
	}
	
	return true;
}

void FreeSlugAlbum(SlugAlbum_t* album)
{
	NotNull(album);
	DestroyTexture(&album->curveTexture);
	DestroyTexture(&album->bandTexture);
	if (album->pictureData.pntr != nullptr)
	{
		NotNull(album->allocArena);
		FreeMem(album->allocArena, album->pictureData.pntr, album->pictureData.length);
	}
	if (album->meshVertexData.pntr != nullptr)
	{
		NotNull(album->allocArena);
		FreeMem(album->allocArena, album->meshVertexData.pntr, album->meshVertexData.length);
	}
	if (album->meshTriangleData.pntr != nullptr)
	{
		NotNull(album->allocArena);
		FreeMem(album->allocArena, album->meshTriangleData.pntr, album->meshTriangleData.length);
	}
	ClearPointer(album);
}

void UntransformTextureData(u8* textureData, v2i textureSize)
{
	TempPushMark();
	u8* workspace = TempArray(u8, 4096 * sizeof(Terathon::Slug::Texel32));
	
	u64 size = (textureSize.width * sizeof(Terathon::Slug::Texel16)) >> 2;
	
	for (i64 yIndex = 0; yIndex < textureSize.height; yIndex++)
	{
		u8* source = workspace;
		MyMemCopy(source, textureData, textureSize.width * sizeof(Terathon::Slug::Texel16));
		
		for (i64 channelIndex = 0; channelIndex < 4; channelIndex++)
		{
			for (u64 xIndex = 1; xIndex < size; xIndex++)
			{
				source[xIndex] += source[xIndex - 1];
			}
			
			for (u64 xIndex = 0; xIndex < size; xIndex++)
			{
				textureData[xIndex * 4] = source[xIndex];
			}
			
			source += size;
			textureData++;
		}
		
		textureData += textureSize.width * sizeof(Terathon::Slug::Texel16) - 4;
	}
	TempPopMark();
}

bool ParseSlugAlbum(MemArena_t* memArena, MyStr_t fileContents, SlugAlbum_t* albumOut)
{
	NotNull(memArena);
	NotNullStr(&fileContents);
	NotNull(albumOut);
	
	ClearPointer(albumOut);
	albumOut->allocArena = memArena;
	albumOut->isValid = false;
	
	const u64 baseFileOffset = 16;
	if (fileContents.length < baseFileOffset + sizeof(const Terathon::Slug::AlbumHeader)) { return false; }
	const Terathon::Slug::AlbumHeader* albumHeader = (const Terathon::Slug::AlbumHeader*)(&fileContents.bytes[baseFileOffset]);
	/*
		struct AlbumHeader:
		uint32    albumFlags;              // Flags indicating various properties of the album.
		int32     iconCount;               // The total number of icons in the album.
		int32     iconDataOffset;          // The offset to the table of IconData structures.
		int32     pictureCount;            // The total number of pictures in the album.
		int32     pictureDataOffset;       // The offset to the table of PictureData structures.
		int32     meshVertexOffset;        // The offset to the mesh vertex data.
		int32     meshTriangleOffset;      // The offset to the mesh triangle data.
		int32     reserved[1];
		Integer2D curveTextureSize;        // The dimensions of the texture map containing the control points for the quadratic Bézier curves composing all icons in the album.
		uint32    curveCompressionType;    // The compression type applied to the curve texture map.
		uint32    curveCompressedDataSize; // The size of the compressed curve texture map data, in bytes.
		int32     curveTextureOffset;      // The offset to the compressed curve texture map data.
		Integer2D bandTextureSize;         // The dimensions of the texture map containing the multi-band Bézier curve index data for all icons in the album.
		uint32    bandCompressionType;     // The compression type applied to the band texture map.
		uint32    bandCompressedDataSize;  // The size of the compressed band texture map data, in bytes.
		int32     bandTextureOffset;       // The offset to the compressed band texture map data.
	*/
	PrintLine_D("AlbumHeader:\n"
		"  albumFlags:              0x%08X\n"
		"  iconCount:               %d\n"
		"  iconDataOffset:          0x%08X\n"
		"  pictureCount:            %d\n"
		"  pictureDataOffset:       0x%08X\n"
		"  meshVertexOffset:        0x%08X\n"
		"  meshTriangleOffset:      0x%08X\n"
		"  curveTextureSize:        (%d, %d)\n"
		"  curveCompressionType:    0x%08X\n"
		"  curveCompressedDataSize: %u\n"
		"  curveTextureOffset:      0x%08X\n"
		"  bandTextureSize:         (%d, %d)\n"
		"  bandCompressionType:     0x%08X\n"
		"  bandCompressedDataSize:  %u\n"
		"  bandTextureOffset:       0x%08X\n",
		albumHeader->albumFlags,
		albumHeader->iconCount,
		albumHeader->iconDataOffset,
		albumHeader->pictureCount,
		albumHeader->pictureDataOffset,
		albumHeader->meshVertexOffset,
		albumHeader->meshTriangleOffset,
		albumHeader->curveTextureSize.x, albumHeader->curveTextureSize.y,
		albumHeader->curveCompressionType,
		albumHeader->curveCompressedDataSize,
		albumHeader->curveTextureOffset,
		albumHeader->bandTextureSize.x, albumHeader->bandTextureSize.y,
		albumHeader->bandCompressionType,
		albumHeader->bandCompressedDataSize,
		albumHeader->bandTextureOffset
	);
	
	bool curvesAre32bit = IsFlagSet(albumHeader->albumFlags, Terathon::Slug::kAlbumCurveTextureFloat32);
	v2i curveTextureSize = ToVec2i(albumHeader->curveTextureSize);
	Assert(curveTextureSize.width > 0 && curveTextureSize.height > 0);
	u64 curveTexelSize = (curvesAre32bit ? sizeof(Terathon::Slug::Texel32) : sizeof(Terathon::Slug::Texel16));
	u64 curveNumTexels = (u64)(curveTextureSize.width * curveTextureSize.height);
	u8* curveTexels = (u8*)AllocMem(&pig->stdHeap, curveTexelSize * curveNumTexels);
	NotNull(curveTexels);
	
	v2i bandTextureSize = ToVec2i(albumHeader->bandTextureSize);
	Assert(bandTextureSize.width > 0 && bandTextureSize.height > 0);
	u64 bandNumTexels = (u64)(bandTextureSize.width * bandTextureSize.height);
	u8* bandTexels = (u8*)AllocMem(&pig->stdHeap, sizeof(Terathon::Slug::Texel16) * bandNumTexels);
	NotNull(bandTexels);
	
	u32 curveCompressionType = albumHeader->curveCompressionType;
	const u8* curveData = &fileContents.bytes[baseFileOffset + albumHeader->curveTextureOffset];
	if (curveCompressionType == 0)
	{
		MyMemCopy(curveTexels, curveData, curveTexelSize * curveNumTexels);
	}
	else
	{
		Terathon::Compression::DecompressData(curveData, albumHeader->curveCompressedDataSize, curveTexels);
		if (curveCompressionType == SLUG_COMPRESSION_TYPE1)
		{
			UntransformTextureData(curveTexels, curveTextureSize);
		}
	}
	
	u32 bandCompressionType = albumHeader->bandCompressionType;
	const u8* bandData = &fileContents.bytes[baseFileOffset + albumHeader->bandTextureOffset];
	if (bandCompressionType == 0)
	{
		MyMemCopy(bandTexels, bandData, sizeof(Terathon::Slug::Texel16) * bandNumTexels);
	}
	else
	{
		Terathon::Compression::DecompressData(bandData, albumHeader->bandCompressedDataSize, bandTexels);
		if (bandCompressionType == SLUG_COMPRESSION_TYPE1)
		{
			UntransformTextureData(bandTexels, bandTextureSize);
		}
	}
	
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			GLuint textureIds[2];
			glGenTextures(2, textureIds);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			albumOut->curveTexture.glId = textureIds[0];
			albumOut->curveTexture.allocArena = memArena;
			albumOut->curveTexture.sizei = curveTextureSize;
			albumOut->curveTexture.size = ToVec2(curveTextureSize);
			albumOut->curveTexture.isValid = false;
			albumOut->curveTexture.id = pig->nextTextureId;
			pig->nextTextureId++;
			albumOut->curveTexture.isPixelated = false;
			albumOut->curveTexture.isRepeating = false;
			albumOut->curveTexture.hasAlpha = true;
			albumOut->curveTexture.isHdrTexture = false;
			albumOut->curveTexture.antialiasingNumSamples = 0;
			albumOut->curveTexture.isFlippedY = false;
			albumOut->curveTexture.singleChannel = false;
			
			albumOut->bandTexture.glId = textureIds[1];
			
			glBindTexture(GL_TEXTURE_2D, albumOut->curveTexture.glId);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				(curvesAre32bit ? GL_RGBA32F : GL_RGBA16F),
				curveTextureSize.width, curveTextureSize.height,
				0,
				GL_RGBA,
				(curvesAre32bit ? GL_FLOAT : GL_HALF_FLOAT),
				curveTexels
			);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			albumOut->curveTexture.isValid = true;
			
			glBindTexture(GL_TEXTURE_2D, albumOut->bandTexture.glId);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA16UI,
				bandTextureSize.width, bandTextureSize.height,
				0,
				GL_RGBA_INTEGER,
				GL_UNSIGNED_SHORT,
				bandTexels
			);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //required because of Intel driver not having defined behavior for integer textures if we don't set it
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //required because of Intel driver not having defined behavior for integer textures if we don't set it
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			albumOut->bandTexture.isValid = true;
			
			albumOut->isValid = true;
			
		} break;
		#endif
		
		default: AssertMsg(false, "Unsupported API in LoadSlugAlbum"); break;
	}
	
	FreeMem(&pig->stdHeap, curveTexels);
	FreeMem(&pig->stdHeap, bandTexels);
	
	Terathon::Slug::PictureData* pictureDataHeader = (Terathon::Slug::PictureData*)&fileContents.bytes[baseFileOffset + albumHeader->pictureDataOffset];
	/*
		struct PictureData:
		uint32 pictureFlags;
		Box2D  canvasBox;
		Box2D  boundingBox;
		int32  componentCount;
		int32  componentDataOffset;
		int32  meshDataOffset;
		int32  reserved[4];
	*/
	PrintLine_D("PictureDataHeader:\n"
		"  pictureFlags:        0x%08X\n"
		"  canvasBox:           (%f, %f, %f, %f)\n"
		"  boundingBox:         (%f, %f, %f, %f)\n"
		"  componentCount:      %d\n"
		"  componentDataOffset: 0x%08X\n"
		"  meshDataOffset:      0x%08X\n",
		pictureDataHeader->pictureFlags,
		pictureDataHeader->canvasBox.min.x, pictureDataHeader->canvasBox.min.y, pictureDataHeader->canvasBox.max.x, pictureDataHeader->canvasBox.max.y,
		pictureDataHeader->boundingBox.min.x, pictureDataHeader->boundingBox.min.y, pictureDataHeader->boundingBox.max.x, pictureDataHeader->boundingBox.max.y,
		pictureDataHeader->componentCount,
		pictureDataHeader->componentDataOffset,
		pictureDataHeader->meshDataOffset
	);
	
	Terathon::Slug::ComponentData* componentDataHeader = (Terathon::Slug::ComponentData*)&fileContents.bytes[baseFileOffset + albumHeader->pictureDataOffset + pictureDataHeader->componentDataOffset];
	/*
		struct ComponentData:
		int32    componentIndex;
		Color4U  componentColor;
		Matrix2D componentMatrix;
		Point2D  componentPosition;
	*/
	PrintLine_D("ComponentDataHeader:\n"
		"  componentIndex:    %d\n"
		"  componentColor:    %02X%02X%02X%02X\n"
		"  componentMatrix:   [%f, %f]\n"
		"                     [%f, %f]\n"
		"  componentPosition: (%f, %f)\n",
		componentDataHeader->componentIndex,
		componentDataHeader->componentColor.alpha, componentDataHeader->componentColor.red, componentDataHeader->componentColor.green, componentDataHeader->componentColor.blue,
		componentDataHeader->componentMatrix.m00, componentDataHeader->componentMatrix.m01, componentDataHeader->componentMatrix.m10, componentDataHeader->componentMatrix.m11,
		componentDataHeader->componentPosition.x, componentDataHeader->componentPosition.y
	);
	Terathon::Slug::MeshData* meshDataHeader = (Terathon::Slug::MeshData*)&fileContents.bytes[baseFileOffset + albumHeader->pictureDataOffset + pictureDataHeader->meshDataOffset];
	/*
		struct MeshData:
		int32 vertexBase;
		int32 vertexCount;
		int32 triangleBase;
		int32 triangleCount;
	*/
	PrintLine_D("MeshDataHeader:\n"
		"  vertexBase: %d\n"
		"  vertexCount: %d\n"
		"  triangleBase: %d\n"
		"  triangleCount: %d\n",
		meshDataHeader->vertexBase,
		meshDataHeader->vertexCount,
		meshDataHeader->triangleBase,
		meshDataHeader->triangleCount
	);
	
	u32 renderFlags = (Terathon::Slug::kRenderSymmetricBands | Terathon::Slug::kRenderStrokes);
	if (!CreateSlugPipelineShader(memArena, &albumOut->pipelineShader, renderFlags))
	{
		albumOut->isValid = false;
	}
	
	return albumOut->isValid;
}

bool LoadSlugAlbum(MemArena_t* memArena, MyStr_t filePath, SlugAlbum_t* albumOut)
{
	bool result = false;
	PlatFileContents_t albumFile;
	if (plat->ReadFileContents(filePath, &albumFile))
	{
		if (ParseSlugAlbum(memArena, NewStr(albumFile.size, albumFile.chars), albumOut))
		{
			result = true;
		}
		plat->FreeFileContents(&albumFile);
	}
	else
	{
		AssertMsg(false, "Failed to find/open slug album file at requested path");
	}
	return result;
}

void FreeSlugFont(SlugFont_t* font)
{
	NotNull(font);
	Unimplemented(); //TODO: Implement me!
}

bool ParseSlugFont(MemArena_t* memArena, MyStr_t fileContents, SlugFont_t* fontOut)
{
	NotNull(memArena);
	NotNullStr(&fileContents);
	NotNull(fontOut);
	
	ClearPointer(fontOut);
	fontOut->allocArena = memArena;
	fontOut->isValid = false;
	
	const u64 baseFileOffset = 16;
	if (fileContents.length < baseFileOffset + sizeof(const Terathon::Slug::FontHeader)) { return false; }
	
	fontOut->fileContentsSize = fileContents.length;
	fontOut->fileContentsVoidPntr = AllocMem(memArena, fontOut->fileContentsSize);
	NotNull(fontOut->fileContentsVoidPntr);
	MyMemCopy(fontOut->fileContentsVoidPntr, fileContents.pntr, fontOut->fileContentsSize);
	fontOut->fontHeader = (Terathon::Slug::FontHeader*)(&fontOut->fileContentsBytePntr[baseFileOffset]);
	
	v2i curveTextureSize = ToVec2i(fontOut->fontHeader->curveTextureSize);
	Assert(curveTextureSize.width > 0 && curveTextureSize.height > 0);
	u64 curveNumTexels = (u64)(curveTextureSize.width * curveTextureSize.height);
	u8* curveTexels = (u8*)AllocMem(&pig->stdHeap, sizeof(Terathon::Slug::Texel16) * curveNumTexels);
	NotNull(curveTexels);
	
	v2i bandTextureSize = ToVec2i(fontOut->fontHeader->bandTextureSize);
	Assert(bandTextureSize.width > 0 && bandTextureSize.height > 0);
	u64 bandNumTexels = (u64)(bandTextureSize.width * bandTextureSize.height);
	u8* bandTexels = (u8*)AllocMem(&pig->stdHeap, sizeof(Terathon::Slug::Texel16) * bandNumTexels);
	NotNull(bandTexels);
	
	u32 curveCompressionType = fontOut->fontHeader->curveCompressionType;
	const u8* curveData = &fileContents.bytes[baseFileOffset + fontOut->fontHeader->curveTextureOffset];
	if (curveCompressionType == 0)
	{
		MyMemCopy(curveTexels, curveData, sizeof(Terathon::Slug::Texel16) * curveNumTexels);
	}
	else
	{
		Terathon::Compression::DecompressData(curveData, fontOut->fontHeader->curveCompressedDataSize, curveTexels);
		if (curveCompressionType == SLUG_COMPRESSION_TYPE1)
		{
			UntransformTextureData(curveTexels, curveTextureSize);
		}
	}
	
	u32 bandCompressionType = fontOut->fontHeader->bandCompressionType;
	const u8* bandData = &fileContents.bytes[baseFileOffset + fontOut->fontHeader->bandTextureOffset];
	if (bandCompressionType == 0)
	{
		MyMemCopy(bandTexels, bandData, sizeof(Terathon::Slug::Texel16) * bandNumTexels);
	}
	else
	{
		Terathon::Compression::DecompressData(bandData, fontOut->fontHeader->bandCompressedDataSize, bandTexels);
		if (bandCompressionType == SLUG_COMPRESSION_TYPE1)
		{
			UntransformTextureData(bandTexels, bandTextureSize);
		}
	}
	
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			GLuint textureIds[2];
			glGenTextures(2, textureIds);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			fontOut->curveTexture.glId = textureIds[0];
			fontOut->curveTexture.allocArena = memArena;
			fontOut->curveTexture.sizei = curveTextureSize;
			fontOut->curveTexture.size = ToVec2(curveTextureSize);
			fontOut->curveTexture.isValid = false;
			fontOut->curveTexture.id = pig->nextTextureId;
			pig->nextTextureId++;
			fontOut->curveTexture.isPixelated = false;
			fontOut->curveTexture.isRepeating = false;
			fontOut->curveTexture.hasAlpha = true;
			fontOut->curveTexture.isHdrTexture = false;
			fontOut->curveTexture.antialiasingNumSamples = 0;
			fontOut->curveTexture.isFlippedY = false;
			fontOut->curveTexture.singleChannel = false;
			
			fontOut->bandTexture.glId = textureIds[1];
			
			glBindTexture(GL_TEXTURE_2D, fontOut->curveTexture.glId);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA16F,
				curveTextureSize.width, curveTextureSize.height,
				0,
				GL_RGBA,
				GL_HALF_FLOAT,
				curveTexels
			);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			fontOut->curveTexture.isValid = true;
			
			glBindTexture(GL_TEXTURE_2D, fontOut->bandTexture.glId);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA16UI,
				bandTextureSize.width, bandTextureSize.height,
				0,
				GL_RGBA_INTEGER,
				GL_UNSIGNED_SHORT,
				bandTexels
			);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //required because of Intel driver not having defined behavior for integer textures if we don't set it
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //required because of Intel driver not having defined behavior for integer textures if we don't set it
			AssertNoOpenGlError(); //TODO: Make this an error case instead?
			
			fontOut->bandTexture.isValid = true;
			
			fontOut->isValid = true;
			
		} break;
		#endif
		
		default: AssertMsg(false, "Unsupported API in LoadSlugAlbum"); break;
	}
	
	FreeMem(&pig->stdHeap, curveTexels);
	FreeMem(&pig->stdHeap, bandTexels);
	
	u32 renderFlags = (Terathon::Slug::kRenderSymmetricBands | Terathon::Slug::kRenderStrokes);
	if (!CreateSlugPipelineShader(memArena, &fontOut->pipelineShader, renderFlags))
	{
		fontOut->isValid = false;
	}
	
	return fontOut->isValid;
}

bool LoadSlugFont(MemArena_t* memArena, MyStr_t filePath, SlugFont_t* fontOut)
{
	bool result = false;
	PlatFileContents_t fontFile;
	if (plat->ReadFileContents(filePath, &fontFile))
	{
		if (ParseSlugFont(memArena, NewStr(fontFile.size, fontFile.chars), fontOut))
		{
			result = true;
		}
		plat->FreeFileContents(&fontFile);
	}
	else
	{
		AssertMsg(false, "Failed to find/open slug font file at requested path");
	}
	return result;
}

void FreeSlugText(SlugText_t* text)
{
	NotNull(text);
	if (text->text.pntr != nullptr)
	{
		NotNull(text->allocArena);
		FreeString(text->allocArena, &text->text);
	}
	ClearPointer(text);
}

SlugText_t CreateSlugText(MemArena_t* memArena, MyStr_t text, SlugFont_t* font, const Terathon::Slug::LayoutData* layoutData = nullptr)
{
	NotNull(memArena);
	NotNullStr(&text);
	NotNull(font);
	SlugText_t result = {};
	result.allocArena = memArena;
	result.text = AllocString(memArena, &text);
	result.font = font;
	if (layoutData != nullptr)
	{
		MyMemCopy(&result.layoutData, layoutData, sizeof(Terathon::Slug::LayoutData));
	}
	else
	{
		Terathon::Slug::SetDefaultLayoutData(&result.layoutData);
	}
	
	i32 maxVertexCount;
	i32 maxTriangleCount;
	i32 glyphCount = Terathon::Slug::CountSlug(font->fontHeader, &result.layoutData, result.text.pntr, result.text.length, &maxVertexCount, &maxTriangleCount);
	
	if (maxVertexCount > 0)
	{
		TempPushMarkNotArena(memArena);
		Terathon::Slug::GeometryBuffer slugGeometryBuffer = {};
		Terathon::Slug::Vertex* verticesPntr = TempArray(Terathon::Slug::Vertex, maxVertexCount);
		Terathon::Slug::Triangle* trianglesPntr = TempArray(Terathon::Slug::Triangle, maxTriangleCount);
		slugGeometryBuffer.vertexData = verticesPntr;
		slugGeometryBuffer.triangleData = trianglesPntr;
		MyMemSet((void*)slugGeometryBuffer.vertexData, 0x00, maxVertexCount * sizeof(Terathon::Slug::Vertex));
		MyMemSet((void*)slugGeometryBuffer.triangleData, 0x00, maxTriangleCount * sizeof(Terathon::Slug::Triangle));
		slugGeometryBuffer.vertexIndex = 0;
		
		Terathon::Slug::BuildSlug(font->fontHeader, &result.layoutData, result.text.pntr, result.text.length, SlugPoint2D_Zero, &slugGeometryBuffer);
		
		Assert(sizeof(Terathon::Slug::Triangle) == sizeof(u16)*3);
		bool createVertBufferResult = CreateVertBufferWithIndices_(memArena, &result.vertBuffer, false,
			(u64)maxVertexCount, VertexType_Slug, VertexType_Slug_Size, (void*)verticesPntr, false,
			(u64)maxTriangleCount*3, sizeof(u16), (void*)trianglesPntr, false
		);
		Assert(createVertBufferResult == true);
		TempPopMarkNotArena(memArena);
	}
	else
	{
		MyDebugBreak();
	}
	
	return result;
}

#endif //SLUG_SUPPORTED
