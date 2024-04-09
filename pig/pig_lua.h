/*
File:   pig_lua.h
Author: Taylor Robbins
Date:   04\01\2024
*/

#ifndef _PIG_LUA_H
#define _PIG_LUA_H

#if LUA_SUPPORTED

struct LuaState_t
{
	lua_State* handle;
};

#endif

#endif //  _PIG_LUA_H
