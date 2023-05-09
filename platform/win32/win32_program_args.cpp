/*
File:   win32_program_args.cpp
Author: Taylor Robbins
Date:   09\24\2021
Description: 
	** Holds functions that handle parsing and checking program arguments that were passed on the command line
*/

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+

// +--------------------------------------------------------------+
// |                           Process                            |
// +--------------------------------------------------------------+
void Win32_ProcessProgramArguments(int argc, char* argv[])
{
	NotNull(Platform);
	Platform->programArgs.count = (u64)(argc-1);
	if (Platform->programArgs.count > 0)
	{
		Platform->programArgs.args = AllocArray(&Platform->mainHeap, MyStr_t, Platform->programArgs.count);
		for (u32 aIndex = 0; aIndex < Platform->programArgs.count; aIndex++)
		{
			Platform->programArgs.args[aIndex] = NewStringInArenaNt(&Platform->mainHeap, argv[1+aIndex]);
		}
	}
}
void Win32_ProcessProgramArguments(LPSTR lpCmdLine, int nCmdShow)
{
	NotNull(Platform);
	UNUSED(nCmdShow);
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(&Platform->mainHeap, lpCmdLine, " ", &numPieces); //TODO: Switch this out for SplitStringQuoted? TODO: Move this to TempArena
	if (numPieces == 1 && pieces[0].length == 0)
	{
		//this is a degenerate case where nothing was really passed
		numPieces = 0;
	}
	if (numPieces > 0)
	{
		Platform->programArgs.count = numPieces;
		Platform->programArgs.args = AllocArray(&Platform->mainHeap, MyStr_t, numPieces);
		NotNull(Platform->programArgs.args);
		for (u32 aIndex = 0; aIndex < numPieces; aIndex++)
		{
			Platform->programArgs.args[aIndex] = AllocString(&Platform->mainHeap, &pieces[aIndex]);
		}
	}
	if (pieces != nullptr) { FreeMem(&Platform->mainHeap, pieces, sizeof(MyStr_t) * numPieces); }
}

// +--------------------------------------------------------------+
// |                            Lookup                            |
// +--------------------------------------------------------------+
// +==============================+
// |     Win32_GetProgramArg      |
// +==============================+
// bool GetProgramArg(MemArena_t* memArena, MyStr_t argName, MyStr_t* valueOut)
PLAT_API_GET_PROGRAM_ARG_DEF(Win32_GetProgramArg)
{
	Assert(InitPhase >= Win32InitPhase_ProgramArgsParsed);
	NotNull(Platform);
	
	for (u64 argIndex = 0; argIndex < Platform->programArgs.count; argIndex++)
	{
		MyStr_t inputStr = Platform->programArgs.args[argIndex];
		
		bool isNamedArg = false;
		if (inputStr.length >= 2 && inputStr.pntr[0] == '-' && inputStr.pntr[1] == '-')
		{
			isNamedArg = true;
			inputStr.pntr += 2;
			inputStr.length -= 2;
		}
		else if (inputStr.length >= 1 && (inputStr.pntr[0] == '-' || inputStr.pntr[0] == '/'))
		{
			isNamedArg = true;
			inputStr.pntr += 1;
			inputStr.length -= 1;
		}
		
		bool argHasValue = false;
		MyStr_t currArgName = (isNamedArg ? inputStr : MyStr_Empty);
		MyStr_t currArgValue = (isNamedArg ? MyStr_Empty : inputStr);
		if (isNamedArg)
		{
			for (u64 cIndex = 0; cIndex < inputStr.length; cIndex++)
			{
				if (inputStr.pntr[cIndex] == '=')
				{
					argHasValue = true;
					currArgValue = StrSubstring(&inputStr, cIndex+1);
					currArgName = StrSubstring(&inputStr, 0, cIndex);
					break;
				}
			}
		}
		
		if (!IsEmptyStr(currArgName) && !IsStringValidIdentifier(currArgName.length, currArgName.pntr))
		{
			//Invalid argument name
			continue;
		}
		
		UnescapeQuotedStringInPlace(&currArgValue, true); //removeQuotes: true
		
		if ((isNamedArg && StrEquals(currArgName, argName)) ||
			(!isNamedArg && IsEmptyStr(argName)))
		{
			if (valueOut != nullptr)
			{
				NotNullMsg(memArena, "You must pass memArena to Win32_GetProgramArg if you pass a valueOut pntr");
				*valueOut = AllocString(memArena, &currArgValue);
			}
			return true;
		}
	}
	return false;
}
