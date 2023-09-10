/*
File:   game.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _GAME_H
#define _GAME_H

#define MAIN_FONT_PATH  "/System/Fonts/Asheville-Sans-14-Bold.pft"

struct GameState_t
{
	bool initialized;
	
	Font_t mainFont;
	SpriteSheet_t testSheet;
	Texture_t backgroundTexture;
	Texture_t pigTexture;
	Sound_t testSound;
	
	MyStr_t pigEngineText;
	v2 pigPos;
	v2 pigVel;
	v2i testSheetFrame;
	bool isInverted;
};

#endif //  _GAME_H
