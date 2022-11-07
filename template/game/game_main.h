/*
File:   game_main.h
Author: Taylor Robbins
Date:   01\31\2022
Description: 
	** Holds all the .h includes for the game portion of the engine DLL
*/

#ifndef _GAME_MAIN_H
#define _GAME_MAIN_H

#include "game_version.h"
#include "game_defines.h"

#include "game_tasks.h"

#include "app_state_list.h"
#include "main_menu/main_menu_state.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
struct AppStateStructs_t
{
	u64 mainMenuAppStateSize;
	MainMenuAppState_t* mainMenuAppStatePntr;
};

struct AppGlobals_t
{
	bool placeholder; //TODO: Remove me!
};

static AppGlobals_t*          gl        = nullptr;
static MainMenuAppState_t*    mmenu     = nullptr;

DebugCommandInfoList_t GameGetDebugCommandInfoList();
bool GameHandleDebugCommand(MyStr_t command, u64 numArguments, MyStr_t* arguments, MyStr_t fullInputStr);

#endif //  _GAME_MAIN_H
