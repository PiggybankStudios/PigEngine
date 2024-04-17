/*
File:   pig_texture_array.cpp
Author: Taylor Robbins
Date:   09\26\2022
Description: 
	** Holds code that helps us load and create a texture array
	** (A single bindable texture object that contains multiple layers)
*/

u64 GetMaxNumTextureArrayLayers()
{
	u64 result = 0;
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			GLint maxLayers;
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayers);
			result = (u64)maxLayers;
		} break;
		#endif
		
		default:
		{
			AssertMsg(false, "Unsupported render API in GetMaxNumTextureArrayLayers");
		} break;
	}
	
	return result;
}

bool CreateTextureArray(MemArena_t* memArena, Texture_t* textureOut, u64 numLayers, const PlatImageData_t* layers, bool pixelated, bool repeating, bool reverseByteOrder = false, bool generateMipmaps = true)
{
	AssertSingleThreaded();
	NotNull2(memArena, textureOut);
	Assert(numLayers > 0);
	NotNull(layers);
	Assert(numLayers <= GetMaxNumTextureArrayLayers());
	Assert(layers[0].dataSize > 0);
	Assert(layers[0].rowSize > 0);
	Assert(layers[0].pixelSize > 0);
	Assert(layers[0].width > 0);
	Assert(layers[0].height > 0);
	
	v2i textureSize = layers[0].size;
	bool floatChannels = layers[0].floatChannels;
	u64 pixelSize = layers[0].pixelSize;
	#if GYLIB_ASSERTIONS_ENABLED
	for (u64 lIndex = 0; lIndex < numLayers; lIndex++)
	{
		Assert(layers[lIndex].size == textureSize);
		Assert(layers[lIndex].floatChannels == floatChannels);
		Assert(layers[lIndex].pixelSize == pixelSize);
	}
	#endif
	
	ClearPointer(textureOut);
	textureOut->allocArena = memArena;
	textureOut->id = pig->nextTextureId;
	pig->nextTextureId++;
	textureOut->isPixelated = pixelated;
	textureOut->isRepeating = repeating;
	textureOut->isHdrTexture = floatChannels;
	textureOut->antialiasingNumSamples = 0;
	textureOut->sizei = textureSize;
	textureOut->size = ToVec2(textureSize);
	textureOut->numLayers = numLayers;
	textureOut->error = Result_None;
	
	#if OPENGL_SUPPORTED
	const char* errorStr = nullptr;
	#define CreateTextureArray_CheckOpenGlError(apiCallStr)                                         \
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
			glGenTextures(1, &textureOut->glId);
			CreateTextureArray_CheckOpenGlError("glGenTextures()") { break; }
			glBindTexture(GL_TEXTURE_2D_ARRAY, textureOut->glId);
			CreateTextureArray_CheckOpenGlError("glBindTexture(GL_TEXTURE_2D_ARRAY)") { break; }
			
			GLenum dataFormat = (reverseByteOrder ? GL_BGRA : GL_RGBA);
			//NOTE: For some reason we have to specify GL_RGBA8 instead of GL_RGBA for texture arrays. Otherwise glTexStorage3D throws INVALID_ENUM
			GLenum internalFormat = (floatChannels ? GL_RGBA32F : GL_RGBA8);
			textureOut->hasAlpha = true;
			if (pixelSize == 1)
			{
				dataFormat = GL_RED;
				internalFormat = (floatChannels ? GL_R32F : GL_RED);
				textureOut->hasAlpha = false;
			}
			if (pixelSize == 3) //no-alpha
			{
				dataFormat = (reverseByteOrder ? GL_BGR : GL_RGB);
				internalFormat = (floatChannels ? GL_RGB32F : GL_RGB);
				textureOut->hasAlpha = false;
			}
			
			glTexStorage3D(
				GL_TEXTURE_2D_ARRAY,         //bound texture type
				4,                           //num mipmap levels TODO: Change me?
				internalFormat,              //internal format
				(GLsizei)textureSize.width,  //image width
				(GLsizei)textureSize.height, //image height
				(GLsizei)numLayers           //image depth
			);
			CreateTextureArray_CheckOpenGlError("glTexStorage3D(GL_TEXTURE_2D_ARRAY)") { break; }
			
			bool allLayersLoadedSuccessfully = true;
			for (u64 lIndex = 0; lIndex < numLayers; lIndex++)
			{
				const PlatImageData_t* imageData = &layers[lIndex];
				NotNull(imageData->data8);
				glTexSubImage3D(
					GL_TEXTURE_2D_ARRAY,         //bound texture type
					0,                           //image level
					0,                           //xoffset
					0,                           //yoffset
					(GLsizei)lIndex,             //zoffset
					(GLsizei)textureSize.width,  //image width
					(GLsizei)textureSize.height, //image height
					1,                           //image depth
					dataFormat,                  //format
					(floatChannels ? GL_FLOAT : GL_UNSIGNED_BYTE), //channel type
					imageData->data8             //data
				);
				CreateTextureArray_CheckOpenGlError("glTexSubImage3D(GL_TEXTURE_2D_ARRAY)") { allLayersLoadedSuccessfully = false; break; }
			}
			if (!allLayersLoadedSuccessfully) { break; }
			
			if (generateMipmaps)
			{
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
				CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MIN_FILTER)") { break; }
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
				CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MIN_FILTER)") { break; }
			}
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_MAG_FILTER)") { break; }
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_S)") { break; }
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
			CreateTexture_CheckOpenGlError("glTexParameteri(GL_TEXTURE_WRAP_T)") { break; }
			
			if (generateMipmaps)
			{
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
				CreateTexture_CheckOpenGlError("glGenerateMipmap()") { break; }
			}
			
			if (pixelSize == 1)
			{
				//TODO: This is not supported by WebGL! We need to figure out an alternative or only use this when running on OpenGL proper?
				GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
				glTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
				CreateTexture_CheckOpenGlError("glTexParameteriv(GL_TEXTURE_SWIZZLE_RGBA)") { break; }
			}
			
			textureOut->isValid = true;
		}
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

bool LoadTextureArrayFromMultipleFiles(MemArena_t* memArena, Texture_t* textureOut, u64 numFiles, const MyStr_t* filePaths, bool pixelated, bool repeating)
{
	NotNull2(memArena, textureOut);
	Assert(numFiles > 0);
	NotNull(filePaths);
	bool result = false;
	
	TempPushMark();
	PlatImageData_t* imageDatas = TempArray(PlatImageData_t, numFiles);
	NotNull(imageDatas);
	MyMemSet(imageDatas, 0x00, sizeof(PlatImageData_t) * numFiles);
	
	bool allImagesLoadedSuccessfully = true;
	for (u64 fIndex = 0; fIndex < numFiles; fIndex++)
	{
		PlatImageData_t* imageDataPntr = &imageDatas[fIndex];
		MyStr_t filePath = filePaths[fIndex];
		NotNullStr(&filePath);
		
		PlatFileContents_t imageFileContents = {};
		if (plat->ReadFileContents(filePath, nullptr, false, &imageFileContents))
		{
			if (imageFileContents.size == 0)
			{
				PrintLine_E("Image file %llu/%llu for texture array was empty at \"%.*s\"", fIndex+1, numFiles, StrPrint(filePath));
				textureOut->error = Result_EmptyFile;
				allImagesLoadedSuccessfully = false;
			}
			else if (!plat->TryParseImageFile(&imageFileContents, 4, imageDataPntr))
			{
				PrintLine_E("Failed to parse image %llu/%llu for texture array at \"%.*s\"", fIndex+1, numFiles, StrPrint(filePath));
				textureOut->error = Result_ParseFailure;
				allImagesLoadedSuccessfully = false;
			}
			plat->FreeFileContents(&imageFileContents);
		}
		else
		{
			PrintLine_E("Failed to open file %llu/%llu for texture array at \"%.*s\"", fIndex+1, numFiles, StrPrint(filePath));
			textureOut->error = Result_CouldntOpenFile;
			allImagesLoadedSuccessfully = false;
		}
		
		if (!allImagesLoadedSuccessfully) { break; }
	}
	
	if (allImagesLoadedSuccessfully)
	{
		bool createResult = CreateTextureArray(memArena, textureOut, numFiles, imageDatas, pixelated, repeating);
		if (createResult)
		{
			result = true;
		}
		else
		{
			PrintLine_E("Failed to create Texture Array from %llu images: %s", numFiles, PrintTextureError(textureOut));
			//textureOut->error was filled by CreateTextureArray
		}
	}
	
	for (u64 fIndex = 0; fIndex < numFiles; fIndex++)
	{
		plat->FreeImageData(&imageDatas[fIndex]);
	}
	TempPopMark();
	
	return result;
}
