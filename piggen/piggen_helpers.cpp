/*
File:   piggen_helpers.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds functions that we need in piggen_main.cpp
*/

void ProcessProgramArguments(int argc, char* argv[])
{
	NotNull(pig);
	pig->programArgs.count = (u64)(argc-1);
	if (pig->programArgs.count > 0)
	{
		pig->programArgs.args = AllocArray(mainHeap, MyStr_t, pig->programArgs.count);
		for (u32 aIndex = 0; aIndex < pig->programArgs.count; aIndex++)
		{
			pig->programArgs.args[aIndex] = NewStringInArenaNt(mainHeap, argv[1+aIndex]);
		}
	}
}

void ProcessProgramArguments(LPSTR lpCmdLine, int nCmdShow)
{
	NotNull(pig);
	UNUSED(nCmdShow);
	u64 numPieces = 0;
	MyStr_t* pieces = SplitString(mainHeap, lpCmdLine, " ", &numPieces); //TODO: Switch this out for SplitStringQuoted? TODO: Move this to TempArena
	if (numPieces == 1 && pieces[0].length == 0)
	{
		//this is a degenerate case where nothing was really passed
		numPieces = 0;
	}
	if (numPieces > 0)
	{
		pig->programArgs.count = numPieces;
		pig->programArgs.args = AllocArray(mainHeap, MyStr_t, numPieces);
		NotNull(pig->programArgs.args);
		for (u32 aIndex = 0; aIndex < numPieces; aIndex++)
		{
			pig->programArgs.args[aIndex] = AllocString(mainHeap, &pieces[aIndex]);
		}
	}
	if (pieces != nullptr) { FreeMem(mainHeap, pieces, sizeof(MyStr_t) * numPieces); }
}

bool GetProgramArg(MemArena_t* memArena, MyStr_t argName, MyStr_t* valueOut, u64 skip = 0)
{
	NotNull(pig);
	
	u64 numSkipped = 0;
	for (u64 argIndex = 0; argIndex < pig->programArgs.count; argIndex++)
	{
		MyStr_t inputStr = pig->programArgs.args[argIndex];
		
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
		
		if ((isNamedArg && StrEquals(currArgName, argName)) ||
			(!isNamedArg && IsEmptyStr(argName)))
		{
			if (numSkipped >= skip)
			{
				if (valueOut != nullptr)
				{
					NotNullMsg(memArena, "You must pass memArena to Win32_GetProgramArg if you pass a valueOut pntr");
					// MyStr_t UnescapeQuotedStringInArena(MemArena_t* memArena, MyStr_t target, bool removeQuotes = true, bool allowNewLineEscapes = true, bool allowOtherEscapeCodes = false)
					*valueOut = UnescapeQuotedStringInArena(memArena, currArgValue, true); //removeQuotes: true
				}
				return true;
			}
			numSkipped++;
		}
	}
	
	return false;
}

bool DoesPathMatchExclusionPatterns(MyStr_t path, VarArray_t* exclusionPatterns)
{
	if (exclusionPatterns == nullptr) { return false; }
	Assert(exclusionPatterns->itemSize == sizeof(ExclusionPattern_t));
	VarArrayLoop(exclusionPatterns, pIndex)
	{
		VarArrayLoopGet(ExclusionPattern_t, exclusionPattern, exclusionPatterns, pIndex);
		re_t pattern = re_compile(exclusionPattern->patternStr.chars); //TODO: It seems the regex library does not allow multiple compiled regexes to sit around
		int reMatchLength = 0; //unused, but required by re_matchp
		if (re_matchp(pattern, path.chars, &reMatchLength) >= 0)
		{
			if (pig->verboseEnabled) { PrintLine_W("Excluding \"%.*s\" due to regex \"%.*s\"", path.length, path.chars, exclusionPattern->patternStr.length, exclusionPattern->patternStr.chars); }
			return true;
		}
	}
	return false;
}

void FindAllCodeFilesInFolder(VarArray_t* arrayOut, MyStr_t folderPath, VarArray_t* exclusionPatterns, bool recursive, u64 depth = 0)
{
	NotNull(arrayOut);
	Assert(arrayOut->itemSize == sizeof(FileToProcess_t));
	NotNullStr(&folderPath);
	Assert(exclusionPatterns == nullptr || exclusionPatterns->itemSize == sizeof(ExclusionPattern_t));
	
	FileEnumerator_t fileEnumerator = StartEnumeratingFiles(folderPath, true, false);
	MyStr_t filePath;
	while (EnumerateFiles(&fileEnumerator, &filePath, mainHeap, true))
	{
		bool isFileToProcess = false;
		if (StrEndsWith(filePath, ".cpp", true) || StrEndsWith(filePath, ".h", true))
		{
			if (!DoesPathMatchExclusionPatterns(filePath, exclusionPatterns))
			{
				FileToProcess_t* newFileToProcess = VarArrayAdd(arrayOut, FileToProcess_t);
				NotNull(newFileToProcess);
				ClearPointer(newFileToProcess);
				newFileToProcess->path = filePath;
				isFileToProcess = true;
			}
		}
		if (!isFileToProcess) { FreeString(mainHeap, &filePath); }
	}
	
	if (recursive)
	{
		FileEnumerator_t folderEnumerator = StartEnumeratingFiles(folderPath, false, true);
		MyStr_t subFolderPath;
		while (EnumerateFiles(&folderEnumerator, &subFolderPath, mainHeap, true))
		{
			FindAllCodeFilesInFolder(arrayOut, subFolderPath, exclusionPatterns, true, depth+1);
			FreeString(mainHeap, &subFolderPath);
		}
	}
}

i32 CompareFuncFileToProcess(const void* left, const void* right, void* contextPntr)
{
	FileToProcess_t* leftFileToProcess = (FileToProcess_t*)left;
	FileToProcess_t* rightFileToProcess = (FileToProcess_t*)right;
	return CompareFuncMyStr(&leftFileToProcess->path, &rightFileToProcess->path, contextPntr);
}

void DumpProcessLog(const ProcessLog_t* log, const char* headerAndFooterStr = nullptr, DbgLevel_t minLevel = DbgLevel_Debug)
{
	NotNull(log);
	Assert(minLevel < DbgLevel_NumLevels);
	if (headerAndFooterStr != nullptr) { PrintLine_R("v========= %s =========v", headerAndFooterStr); }
	const StringFifoLine_t* logLine = log->fifo.firstLine;
	while (logLine != nullptr)
	{
		u64 newMetaStructSize = 0;
		MyStr_t text = GetFifoLineText(logLine);
		const ProcessLogLine_t* logLineMeta = GetFifoLineMetaStruct(logLine, ProcessLogLine_t);
		if (logLineMeta->dbgLevel >= minLevel) { PrintLineAt(logLineMeta->dbgLevel, "%.*s", text.length, text.chars); }
		logLine = logLine->next;
	}
	if (headerAndFooterStr != nullptr) { PrintLine_R("^========= %s =========^", headerAndFooterStr); }
}

MyStr_t GetOutputFileName(MyStr_t sourceCodePath, u64 outputFileCount, MemArena_t* memArena)
{
	MyStr_t sourceCodeFileNameNoExt = GetFileNamePart(sourceCodePath, false);
	return PrintInArenaStr(memArena, "%.*s_%llu.h", sourceCodeFileNameNoExt.length, sourceCodeFileNameNoExt.chars, outputFileCount);
}
