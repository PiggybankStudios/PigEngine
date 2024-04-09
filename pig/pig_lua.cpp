/*
File:   pig_lua.cpp
Author: Taylor Robbins
Date:   04\01\2024
Description: 
	** Holds functions that help games in Pig Engine interact with the Lua scripting language
*/

#if LUA_SUPPORTED

void* LuaAllocMemCallback(void* userPntr, void* oldPntr, size_t oldSize, size_t newSize)
{
	UNUSED(userPntr);
	MemArenaVerify(&pig->luaHeap, true);
	if (newSize == 0)
	{
		if (oldSize != 0)
		{
			NotNull(oldPntr);
			FreeMem(&pig->luaHeap, oldPntr, oldSize);
			// PrintLine_D("LuaFreed: %p (%lld) (%llu allocs)", oldPntr, oldSize, pig->luaHeap.numAllocations);
		}
		return nullptr;
	}
	else if (oldPntr != nullptr)
	{
		void* result = ReallocMem(&pig->luaHeap, oldPntr, newSize, oldSize);
		// PrintLine_D("LuaRealloc: %p (%lld -> %lld) -> %p (%llu allocs)", oldPntr, oldSize, newSize, result, pig->luaHeap.numAllocations);
		return result;
	}
	else
	{
		void* result = AllocMem(&pig->luaHeap, newSize);
		// PrintLine_D("LuaAlloc: %lld -> %p (%llu allocs)", newSize, result, pig->luaHeap.numAllocations);
		return result;
	}
}

void PigInitLua()
{
	ClearStruct(pig->lua);
	pig->lua.handle = lua_newstate(LuaAllocMemCallback, nullptr);
	NotNull(pig->lua.handle);
	luaL_openlibs(pig->lua.handle);
}

void PigLuaHandleReload()
{
	lua_setallocf(pig->lua.handle, LuaAllocMemCallback, nullptr);
}

#endif //LUA_SUPPORTED
