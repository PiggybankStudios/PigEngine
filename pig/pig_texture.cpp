/*
File:   pig_texture.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description: 
	** Holds functions that help us load textures from disk and manipulate them
*/

void DestroyTexture(Texture_t* texture)
{
	NotNull(texture);
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			if (texture->glId != 0)
			{
				glDeleteTextures(1, &texture->glId);
				AssertNoOpenGlError();
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in DestroyTexture"); break;
	}
	if (texture->apiErrorStr.pntr != nullptr)
	{
		NotNull(texture->allocArena);
		FreeString(texture->allocArena, &texture->apiErrorStr);
	}
	ClearPointer(texture);
}

bool CreateTexture(MemArena_t* memArena, Texture_t* textureOut, const PlatImageData_t* imageData, bool pixelated, bool repeating, bool reverseByteOrder = false)
{
	NotNull(memArena);
	NotNull(textureOut);
	NotNull(imageData);
	Assert(imageData->dataSize > 0);
	Assert(imageData->rowSize > 0);
	Assert(imageData->pixelSize > 0);
	Assert(imageData->width > 0);
	Assert(imageData->height > 0);
	NotNull(imageData->data8);
	ClearPointer(textureOut);
	
	const char* errorStr = nullptr;
	#if OPENGL_SUPPORTED
	#define CreateTexture_CheckOpenGlError(apiCallStr)                                            \
	{                                                                                             \
		errorStr = CheckOpenGlError(true);                                                        \
		if (errorStr != nullptr)                                                                  \
		{                                                                                         \
			textureOut->apiErrorStr = PrintInArenaStr(memArena, apiCallStr " error: ", errorStr); \
			textureOut->error = TextureError_ApiError;                                            \
		}                                                                                         \
	} if (errorStr != nullptr)
	#endif
	
	bool result = false;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			// +==============================+
			// |        Create Texture        |
			// +==============================+
			GLuint textureId = 0;
			glGenTextures(1, &textureId);
			CreateTexture_CheckOpenGlError("glGenTextures()")
			{
				return false;
			}
			glBindTexture(GL_TEXTURE_2D, textureId);
			CreateTexture_CheckOpenGlError("glBindTexture(GL_TEXTURE_2D)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			
			GLenum dataFormat = reverseByteOrder ? GL_BGRA : GL_RGBA;
			GLenum internalFormat = GL_RGBA;
			bool hasAlpha = true;
			if (imageData->pixelSize == 1)
			{
				dataFormat = GL_RED;
				internalFormat = GL_RED;
				hasAlpha = false;
			}
			if (imageData->pixelSize == 3) //no-alpha
			{
				dataFormat = reverseByteOrder ? GL_BGR : GL_RGB;
				internalFormat = GL_RGB;
				hasAlpha = false;
			}
			glTexImage2D(
				GL_TEXTURE_2D,      //bound texture type
				0,                  //image level
				internalFormat,     //internal format
				imageData->width,   //image width
				imageData->height,  //image height
				0,                  //border
				dataFormat,         //format
				GL_UNSIGNED_BYTE,   //type
				imageData->data8    //data
			);
			CreateTexture_CheckOpenGlError("glTexImage2D(...)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MIN_FILTER)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MAG_FILTER)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_S)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_T)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			glGenerateMipmap(GL_TEXTURE_2D);
			CreateTexture_CheckOpenGlError("glGenerateMipmap(GL_TEXTURE_2D)")
			{
				glDeleteTextures(1, &textureId);
				return false;
			}
			if (imageData->pixelSize == 1)
			{
				GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
				glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
				CreateTexture_CheckOpenGlError("glTexParameteriv(GL_TEXTURE_SWIZZLE_RGBA)")
				{
					glDeleteTextures(1, &textureId);
					return false;
				}
			}
			
			textureOut->allocArena = memArena;
			textureOut->glId = textureId;
			textureOut->id = pig->nextTextureId;
			pig->nextTextureId++;
			textureOut->isPixelated = pixelated;
			textureOut->isRepeating = repeating;
			textureOut->hasAlpha = hasAlpha;
			textureOut->sizei = imageData->size;
			textureOut->size = ToVec2(imageData->size);
			textureOut->isValid = true;
			result = true;
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			textureOut->error = TextureError_UnsupportedApi;
			return false;
		} break;
	}
	
	return result;
}

bool LoadTexture(MemArena_t* memArena, Texture_t* textureOut, MyStr_t filePath, bool pixelated, bool repeating, PlatImageData_t* imageDataOut = nullptr)
{
	NotNull(memArena);
	NotNull(textureOut);
	NotEmptyStr(&filePath);
	ClearPointer(textureOut);
	AssertIf(imageDataOut != nullptr, imageDataOut->allocArena != nullptr);
	
	PlatFileContents_t textureFile;
	if (!plat->ReadFileContents(filePath, &textureFile))
	{
		textureOut->error = TextureError_CouldntOpenFile;
		return false;
	}
	if (textureFile.size == 0)
	{
		textureOut->error = TextureError_EmptyFile;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	NotNull(textureFile.data);
	
	PlatImageData_t imageData;
	if (!plat->TryParseImageFile(&textureFile, sizeof(u32), &imageData))
	{
		textureOut->error = TextureError_ParseFailure;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	
	bool result = CreateTexture(memArena, textureOut, &imageData, pixelated, repeating, false);
	
	if (imageDataOut != nullptr && result)
	{
		imageDataOut->size      = imageData.size;
		imageDataOut->pixelSize = imageData.pixelSize;
		imageDataOut->rowSize   = imageData.rowSize;
		imageDataOut->dataSize  = imageData.dataSize;
		imageDataOut->data8 = AllocArray(imageDataOut->allocArena, u8, imageDataOut->dataSize);
		MyMemCopy(imageDataOut->data8, imageData.data8, imageDataOut->dataSize);
	}
	
	plat->FreeImageData(&imageData);
	plat->FreeFileContents(&textureFile);
	
	return result;
}

void CreateSubImageData(const PlatImageData_t* sourceImageData, reci sourceRec, PlatImageData_t* imageDataOut, MemArena_t* allocArena)
{
	NotNull(sourceImageData);
	NotNull(sourceImageData->data8);
	NotNull(imageDataOut);
	NotNull(allocArena);
	//TODO: Make a RecFullyContains function or something similar and assert on that
	
	ClearPointer(imageDataOut);
	imageDataOut->allocArena = allocArena;
	imageDataOut->size = sourceRec.size;
	imageDataOut->pixelSize = sizeof(u32);
	imageDataOut->rowSize = imageDataOut->pixelSize * imageDataOut->width;
	imageDataOut->dataSize = imageDataOut->rowSize * imageDataOut->height;
	imageDataOut->data8 = AllocArray(allocArena, u8, imageDataOut->dataSize);
	NotNull(imageDataOut->data8);
	
	for (i32 yOffset = 0; yOffset < sourceRec.height; yOffset++)
	{
		for (i32 xOffset = 0; xOffset < sourceRec.width; xOffset++)
		{
			v2i pixelPos = sourceRec.topLeft + NewVec2i(xOffset, yOffset);
			u64 srcPixelIndex = (pixelPos.y * sourceImageData->rowSize) + (pixelPos.x * sourceImageData->pixelSize);
			Assert(srcPixelIndex + sourceImageData->pixelSize <= sourceImageData->dataSize);
			const u32* srcPixelPntr = (u32*)&sourceImageData->data8[srcPixelIndex];
			u64 destPixelIndex = (yOffset * imageDataOut->rowSize) + (xOffset * imageDataOut->pixelSize);
			Assert(destPixelIndex + imageDataOut->pixelSize <= imageDataOut->dataSize);
			u32* destPixelPntr = (u32*)&imageDataOut->data8[destPixelIndex];
			*destPixelPntr = *srcPixelPntr;
		}
	}
}

bool IsPlatImageDataPixelFilled(PlatImageData_t* imageData, v2i pixelPos, u8 minAlpha = 1, u8* alphaValueOut = nullptr)
{
	NotNull(imageData->data8);
	Assert(imageData->pixelSize == sizeof(u32));
	u64 pixelIndex = ((u64)pixelPos.y * imageData->rowSize) + ((u64)pixelPos.x * imageData->pixelSize);
	Assert(pixelIndex + imageData->pixelSize <= imageData->dataSize);
	u8 alphaValue = (u8)(((*((u32*)&imageData->data8[pixelIndex])) & 0xFF000000) >> 24);
	if (alphaValueOut != nullptr) { *alphaValueOut = alphaValue; }
	return (alphaValue >= minAlpha);
}
