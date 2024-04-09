/*
File:   win32_performance.cpp
Author: Taylor Robbins
Date:   01\04\2022
Description: 
	** Holds functions that help us measure and interpret timing information
	** for the purposes of measuring out performance
*/

// +==============================+
// |     Win32_GetProgramTime     |
// +==============================+
// u64 GetProgramTime(r64* programTimeR64Out, bool ignoreFixedTimeScaleEffects)
PLAT_API_GET_PROGRAM_TIME_DEFINITION(Win32_GetProgramTime) //pre-declared in win32_func_defs.cpp
{
	if (InitPhase >= Win32InitPhase_GlfwInitialized)
	{
		//NOTE: A 64 bit floating point number can hold a maximum whole number of 9007199254740992 before losing precision.
		//      Since glfwGetTime gives us microseconds this should be "okay" for about 285 years since program start.
		//      (Some of fractional microsecond precision will be lost before that of course)
		r64 resultR64 = glfwGetTime() * 1000.0;
		u64 result = (u64)(resultR64);
		if (!ignoreFixedTimeScaleEffects)
		{
			if (Platform->programTimeIsBehind)
			{
				resultR64 -= Platform->absProgramTimeDiffF;
				result -= Platform->absProgramTimeDiff;
			}
			else if (Platform->programTimeIsAhead)
			{
				resultR64 += Platform->absProgramTimeDiffF;
				result += Platform->absProgramTimeDiff;
			}
		}
		SetOptionalOutPntr(programTimeR64Out, resultR64);
		return result;
	}
	else
	{
		SetOptionalOutPntr(programTimeR64Out, 0.0);
		return 0;
	}
}

u64 Win32_TimeSince(u64 programTimeSnapshot, bool ignoreFixedTimeScaleEffects) //pre-declared in win32_func_defs.cpp
{
	u64 currentProgramTime = Win32_GetProgramTime(nullptr, ignoreFixedTimeScaleEffects);
	if (programTimeSnapshot <= currentProgramTime)
	{
		return currentProgramTime - programTimeSnapshot;
	}
	else 
	{
		return 0;
	}
}