/*
File:   game_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the GameHandleDebugCommand function which is where the game is
	** able to try processing a debug command input to the console.
	** See pig_debug_commands.cpp for engine level debug commands
*/

const char* GameDebugCommandInfoStrs[] = {
	//"command", "description", "arg1", "arg2", ... "\n",
	"test", "Serves as a dedicated spot to place temporary test code", "\n",
};
DebugCommandInfoList_t GameGetDebugCommandInfoList() //pre-declared in game_main.h
{
	return CreateDebugCommandInfoList(GameDebugCommandInfoStrs, ArrayCount(GameDebugCommandInfoStrs));
}

//Pre-declared in game_main.h
bool GameHandleDebugCommand(MyStr_t command, u64 numArguments, MyStr_t* arguments, MyStr_t fullInputStr)
{
	UNUSED(fullInputStr);
	bool isValidCommand = true;
	
	// +==============================+
	// |             test             |
	// +==============================+
	if (StrCompareIgnoreCase(command, "test") == 0)
	{
		WriteLine_D("Nothing to test right now");
	}
	
	// +==============================+
	// |       Unknown Command        |
	// +==============================+
	else
	{
		isValidCommand = false;
	}
	
	return isValidCommand;
}
