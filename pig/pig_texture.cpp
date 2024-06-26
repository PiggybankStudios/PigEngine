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

bool CreateTexture(MemArena_t* memArena, Texture_t* textureOut, const PlatImageData_t* imageData, bool pixelated, bool repeating, bool reverseByteOrder = false, bool generateMipmap = true, u64 antialiasingNumSamples = 0)
{
	NotNull(memArena);
	NotNull(textureOut);
	NotNull(imageData);
	Assert(imageData->dataSize > 0);
	Assert(imageData->rowSize > 0);
	Assert(imageData->pixelSize > 0);
	Assert(imageData->width > 0);
	Assert(imageData->height > 0);
	Assert(antialiasingNumSamples == 0 || antialiasingNumSamples == 4 || antialiasingNumSamples == 8);
	Assert(!(imageData->floatChannels && antialiasingNumSamples > 0));
	// NotNull(imageData->data8); //This can actually be null if you want to create a texture without filling it immediately
	
	ClearPointer(textureOut);
	textureOut->allocArena = memArena;
	textureOut->id = pig->nextTextureId;
	pig->nextTextureId++;
	textureOut->isPixelated = pixelated;
	textureOut->isRepeating = repeating;
	textureOut->isHdrTexture = imageData->floatChannels;
	textureOut->antialiasingNumSamples = antialiasingNumSamples;
	textureOut->sizei = imageData->size;
	textureOut->size = ToVec2(imageData->size);
	textureOut->numLayers = 1;
	textureOut->error = Result_None;
	bool hasAntialiasing = (antialiasingNumSamples > 0);
	
	const char* errorStr = nullptr;
	#if OPENGL_SUPPORTED
	#define CreateTexture_CheckOpenGlError(apiCallStr)                                              \
	{                                                                                               \
		errorStr = CheckOpenGlError(true);                                                          \
		if (errorStr != nullptr)                                                                    \
		{                                                                                           \
			textureOut->apiErrorStr = PrintInArenaStr(memArena, apiCallStr " error: %s", errorStr); \
			textureOut->error = Result_ApiError;                                              \
		}                                                                                           \
	} if (errorStr != nullptr)
	#endif
	
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
			glGenTextures(1, &textureOut->glId);
			CreateTexture_CheckOpenGlError("glGenTextures()") { break; }
			GLenum targetEnum = (hasAntialiasing ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
			glBindTexture(targetEnum, textureOut->glId);
			CreateTexture_CheckOpenGlError("glBindTexture()") { break; }
			
			GLenum dataFormat = reverseByteOrder ? GL_BGRA : GL_RGBA;
			GLenum internalFormat = imageData->floatChannels ? GL_RGBA32F : GL_RGBA;
			textureOut->hasAlpha = true;
			if (imageData->pixelSize == 1)
			{
				dataFormat = GL_RED;
				internalFormat = imageData->floatChannels ? GL_R32F : GL_RED;
				textureOut->hasAlpha = false;
			}
			if (imageData->pixelSize == 3) //no-alpha
			{
				dataFormat = reverseByteOrder ? GL_BGR : GL_RGB;
				internalFormat = imageData->floatChannels ? GL_RGB32F : GL_RGB;
				textureOut->hasAlpha = false;
			}
			
			if (hasAntialiasing)
			{
				//TODO: This makes our minimum OpenGL version 3.2 or greater!
				glTexImage2DMultisample(
					targetEnum,                      //bound texture type
					(GLsizei)antialiasingNumSamples, //samples
					internalFormat,                  //internal format
					imageData->width,                //image width
					imageData->height,               //image height
					GL_TRUE                          //fixed sample locations
				);
				CreateTexture_CheckOpenGlError("glTexImage2DMultisample(...)") { break; }
			}
			else
			{
				glTexImage2D(
					targetEnum,         //bound texture type
					0,                  //image level
					internalFormat,     //internal format
					imageData->width,   //image width
					imageData->height,  //image height
					0,                  //border
					dataFormat,         //format
					(imageData->floatChannels ? GL_FLOAT : GL_UNSIGNED_BYTE), //type
					imageData->data8    //data
				);
				CreateTexture_CheckOpenGlError("glTexImage2D(...)") { break; }
			}
			
			if (!hasAntialiasing)
			{
				if (generateMipmap)
				{
					glTexParameteri(targetEnum, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
					CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MIN_FILTER)") { break; }
				}
				else
				{
					glTexParameteri(targetEnum, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
					CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MIN_FILTER)") { break; }
				}
				glTexParameteri(targetEnum, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
				CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MAG_FILTER)") { break; }
				glTexParameteri(targetEnum, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
				CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_S)") { break; }
				glTexParameteri(targetEnum, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
				CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_T)") { break; }
				
				if (generateMipmap)
				{
					glGenerateMipmap(targetEnum);
					CreateTexture_CheckOpenGlError("glGenerateMipmap()") { break; }
				}
			}
			
			if (imageData->pixelSize == 1)
			{
				//TODO: This is not supported by WebGL! We need to figure out an alternative or only use this when running on OpenGL proper?
				GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
				glTexParameteriv(targetEnum, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
				CreateTexture_CheckOpenGlError("glTexParameteriv(GL_TEXTURE_SWIZZLE_RGBA)") { break; }
			}
			
			textureOut->isValid = true;
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			textureOut->error = Result_UnsupportedApi;
		} break;
	}
	
	AssertIf(!textureOut->isValid, textureOut->error != Result_None);
	return textureOut->isValid;
}

bool LoadTexture(MemArena_t* memArena, Texture_t* textureOut, MyStr_t filePath, bool pixelated, bool repeating, PlatImageData_t* imageDataOut = nullptr)
{
	NotNull(memArena);
	NotNull(textureOut);
	NotEmptyStr(&filePath);
	ClearPointer(textureOut);
	
	PlatFileContents_t textureFile;
	if (!plat->ReadFileContents(filePath, nullptr, false, &textureFile))
	{
		textureOut->error = Result_CouldntOpenFile;
		return false;
	}
	if (textureFile.size == 0)
	{
		textureOut->error = Result_EmptyFile;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	NotNull(textureFile.data);
	
	PlatImageData_t imageData;
	if (!plat->TryParseImageFile(&textureFile, sizeof(u32), &imageData))
	{
		textureOut->error = Result_ParseFailure;
		plat->FreeFileContents(&textureFile);
		return false;
	}
	
	bool result = CreateTexture(memArena, textureOut, &imageData, pixelated, repeating);
	
	if (imageDataOut != nullptr && result)
	{
		NotNull(imageDataOut->allocArena);
		Assert(imageDataOut->allocArena->type != MemArenaType_None || imageDataOut->allocArena->type < MemArenaType_NumTypes);
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

void TextureGenerateMipmaps(Texture_t* texture)
{
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			if (texture->antialiasingNumSamples == 0)
			{
				glBindTexture(GL_TEXTURE_2D, texture->glId);
				AssertNoOpenGlError();
				glGenerateMipmap(GL_TEXTURE_2D);
				AssertNoOpenGlError();
			}
			else
			{
				WriteLine_W("Warning: Trying to generate mipmaps for texture with antialiasing, which doesn't support mipmaps");
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in TextureGenerateMipmaps"); break;
	}
	
}

const char* PrintTextureError(const Texture_t* texture)
{
	NotNull(texture);
	if (texture->error == Result_ApiError)
	{
		return TempPrint("Api: %.*s", StrPrint(texture->apiErrorStr));
	}
	else
	{
		return GetResultStr(texture->error);
	}
}

bool GetTextureDataSubPart(Texture_t* texture, reci subPartRec, MemArena_t* memArena, PlatImageData_t* imageDataOut)
{
	NotNull(texture);
	NotNull(memArena);
	NotNull(imageDataOut);
	Assert(texture->isValid);
	Assert(texture->widthi > 0 && texture->heighti > 0);
	Assert(!texture->singleChannel); //TODO: we don't support single channel stuff in this function yet, because we don't need it
	Assert(subPartRec.width > 0 && subPartRec.height > 0);
	Assert(subPartRec.x >= 0 && subPartRec.y >= 0);
	Assert(subPartRec.x + subPartRec.width <= texture->widthi && subPartRec.y + subPartRec.height <= texture->heighti);
	
	ClearPointer(imageDataOut);
	imageDataOut->allocArena = memArena;
	imageDataOut->size = subPartRec.size;
	imageDataOut->pixelSize = (texture->hasAlpha ? 4 : 3);
	imageDataOut->rowSize = imageDataOut->pixelSize * subPartRec.width;
	imageDataOut->dataSize = imageDataOut->rowSize * subPartRec.height;
	imageDataOut->data8 = AllocArray(memArena, u8, imageDataOut->dataSize);
	if (imageDataOut->data8 == nullptr) { return false; }
	
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			GLint sourceY = (GLint)(texture->isFlippedY ? texture->height - (subPartRec.y + subPartRec.height) : subPartRec.y);
			GLint sourceHeight = (GLint)subPartRec.height;
			glGetTextureSubImage(
				texture->glId,                          //texture,
				0,                                      //level,
				subPartRec.x,                           //xoffset,
				sourceY,                                //yoffset,
				0,                                      //zoffset,
				subPartRec.width,                       //width,
				sourceHeight,                           //height,
				1,                                      //depth,
				(texture->hasAlpha ? GL_RGBA : GL_RGB), //format,
				GL_UNSIGNED_BYTE,                       //type,
				(GLsizei)imageDataOut->dataSize,        //bufSize,
				(GLvoid*)imageDataOut->data8            //pixels
			);
			AssertNoOpenGlError();
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in GetTextureData"); break;
	}
	
	if (texture->isFlippedY)
	{
		//NOTE: We need to reverse the rows in the image data since this texture
		//      is stored internally with rows going bottom up
		TempPushMark();
		u8* tempRowBuffer = AllocArray(TempArena, u8, imageDataOut->rowSize);
		NotNull(tempRowBuffer);
		for (u64 rIndex = 0; rIndex < (u64)imageDataOut->height/2; rIndex++)
		{
			MyMemCopy(tempRowBuffer, &imageDataOut->data8[rIndex * imageDataOut->rowSize], imageDataOut->rowSize);
			MyMemCopy(&imageDataOut->data8[rIndex * imageDataOut->rowSize], &imageDataOut->data8[((u64)imageDataOut->height-1 - rIndex) * imageDataOut->rowSize], imageDataOut->rowSize);
			MyMemCopy(&imageDataOut->data8[((u64)imageDataOut->height-1 - rIndex) * imageDataOut->rowSize], tempRowBuffer, imageDataOut->rowSize);
		}
		TempPopMark();
	}
	
	return true;
}

bool GetTextureData(Texture_t* texture, MemArena_t* memArena, PlatImageData_t* imageDataOut)
{
	NotNull(texture);
	return GetTextureDataSubPart(texture, NewReci(Vec2i_Zero, texture->sizei), memArena, imageDataOut);
}

// +--------------------------------------------------------------+
// |                   PlatImageData_t Helpers                    |
// +--------------------------------------------------------------+
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
