/*
File:   texture.cpp
Author: Taylor Robbins
Date:   09\08\2023
Description: 
	** Functions for Texture_t structure (which wraps LCDBitmap*)
*/

Texture_t LoadTexture(MyStr_t path)
{
	MemArena_t* scratch = GetScratchArena();
	Texture_t result = {};
	
	const char* loadBitmapErrorStr = nullptr;
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	result.bitmap = pd->graphics->loadBitmap(pathNullTerm.chars, &loadBitmapErrorStr);
	if (loadBitmapErrorStr == nullptr)
	{
		int width, height;
		pd->graphics->getBitmapData(
			result.bitmap,
			&width,
			&height,
			nullptr, //rowbytes
			nullptr, //mask
			nullptr //data
		);
		result.width = width;
		result.height = height;
		
		result.isValid = true;
	}
	else
	{
		pd->system->error("Failed to load texture from \"%s\": %s", pathNullTerm.chars, loadBitmapErrorStr);
	}
	
	FreeScratchArena(scratch);
	return result;
}

// You can pass dataSize=0 and pixelData=nullptr if you want an empty bitmap
Texture_t CreateTexture(v2i size, u64 dataSize, const u8* pixelData, const u8* maskData = nullptr)
{
	Assert(size.width > 0 && size.height > 0);
	Texture_t result = {};
	result.size = size;
	
	result.bitmap = pd->graphics->newBitmap(size.width, size.height, kColorWhite);
	if (result.bitmap != nullptr)
	{
		Assert((dataSize == 0) == (pixelData == nullptr));
		if (pixelData != nullptr)
		{
			BitmapData_t bitmapData = GetBitmapData(result.bitmap);
			if (dataSize == (u64)bitmapData.rowWidth * size.height)
			{
				NotNull(bitmapData.data);
				MyMemCopy(bitmapData.data, pixelData, dataSize);
				if (maskData != nullptr && bitmapData.mask != nullptr) { MyMemCopy(bitmapData.mask, maskData, dataSize); }
				
				result.isValid = true;
			}
			else
			{
				pd->system->error("Passed %llu bytes instead of expected %llu to CreateTexture", dataSize, (u64)bitmapData.rowWidth * size.height);
			}
		}
	}
	else
	{
		pd->system->error("Failed to create %dx%d texture!", size.width, size.height);
	}
	
	return result;
}