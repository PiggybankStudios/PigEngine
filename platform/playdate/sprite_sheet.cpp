/*
File:   sprite_sheet.cpp
Author: Taylor Robbins
Date:   09\08\2023
Description: 
	** Functions for SpriteSheet_t structure (which wraps LCDBitmapTable*)
*/

SpriteSheet_t LoadSpriteSheet(MyStr_t path, i32 numFramesX)
{
	Assert(IsStrNullTerminated(path)); //TODO: Allocate it somewhere if it's not!
	Assert(numFramesX > 0);
	SpriteSheet_t result = {};
	
	const char* loadBitmapTableErrorStr = nullptr;
	result.table = pd->graphics->loadBitmapTable(path.chars, &loadBitmapTableErrorStr);
	if (loadBitmapTableErrorStr == nullptr)
	{
	result.isValid = true;
		
		i32 frameIndex = 0;
		while (true)
		{
			LCDBitmap* frameBitmap = pd->graphics->getTableBitmap(result.table, frameIndex);
			if (frameBitmap == nullptr) { break; }
			v2i frameGridPos = NewVec2i(frameIndex % numFramesX, frameIndex / numFramesX);
			if (result.numFrames.x < frameGridPos.x+1) { result.numFrames.x = frameGridPos.x+1; }
			if (result.numFrames.y < frameGridPos.y+1) { result.numFrames.y = frameGridPos.y+1; }
			if (frameIndex == 0) { result.frameSize = GetBitmapSize(frameBitmap); }
			else { DebugAssert(result.frameSize == GetBitmapSize(frameBitmap)); }
			frameIndex++;
		}
		
		if (frameIndex == 0)
		{
			pd->system->error("The sprite sheet at \"%.*s\" had no frames?", path.length, path.chars);
			result.isValid = false;
		}
	}
	else
	{
		pd->system->error("Failed to load sprite sheet from \"%.*s\": %s", path.length, path.chars, loadBitmapTableErrorStr);
	}
	
	return result;
}
