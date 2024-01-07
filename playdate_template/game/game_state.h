/*
File:   game_state.h
Author: Taylor Robbins
Date:   12\19\2023
*/

#ifndef _GAME_STATE_H
#define _GAME_STATE_H

struct GameState_t
{
	bool initialized;
	
	PDMenuItem* mainMenuItem;
	bool mainMenuRequested;
};

#endif //  _GAME_STATE_H
