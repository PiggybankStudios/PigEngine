/*
File:   texture.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _TEXTURE_H
#define _TEXTURE_H

struct Texture_t
{
	bool isValid;
	LCDBitmap* bitmap;
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
};

#endif //  _TEXTURE_H
