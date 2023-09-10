/*
File:   sprite_sheet.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _SPRITE_SHEET_H
#define _SPRITE_SHEET_H

struct SpriteSheet_t
{
	bool isValid;
	LCDBitmapTable* table;
	union
	{
		v2i numFrames;
		struct { i32 numFramesX, numFramesY; };
	};
	v2i frameSize;
};

#endif //  _SPRITE_SHEET_H
