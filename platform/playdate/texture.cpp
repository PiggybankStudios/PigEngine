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
		pd->graphics->getBitmapData(
			result.bitmap,
			&result.width,
			&result.height,
			nullptr, //rowbytes
			nullptr, //mask
			nullptr //data
		);
		
		result.isValid = true;
	}
	else
	{
		pd->system->error("Failed to load texture from \"%s\": %s", pathNullTerm.chars, loadBitmapErrorStr);
	}
	
	FreeScratchArena(scratch);
	return result;
}

