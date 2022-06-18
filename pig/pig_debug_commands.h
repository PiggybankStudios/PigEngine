/*
File:   pig_debug_commands.h
Author: Taylor Robbins
Date:   06\17\2022
*/

#ifndef _PIG_DEBUG_COMMANDS_H
#define _PIG_DEBUG_COMMANDS_H

struct DebugCommandInfo_t
{
	MyStr_t name;
	MyStr_t description;
	u64 numArguments;
	MyStr_t* arguments;
};
struct DebugCommandInfoList_t
{
	u64 numCommands;
	u64 totalNumArguments;
	DebugCommandInfo_t* commands;
	MyStr_t* allArguments;
};

#endif //  _PIG_DEBUG_COMMANDS_H
