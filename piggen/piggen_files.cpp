/*
File:   piggen_files.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds functions that help us manipulate files in the OS file system
*/

MyStr_t GetFullPath(MemArena_t* memArena, MyStr_t relativePath, bool giveBackslashes)
{
	NotNull(memArena);
	NotNullStr(&relativePath);
	NotNullStr(&pig->defaultDirectory);
	MyStr_t result;
	
	bool endedWithSlash = (relativePath.length > 0 && (relativePath.pntr[relativePath.length-1] == '/' || relativePath.pntr[relativePath.length-1] == '\\'));
	bool isRelative = false;
	if (relativePath.length >= 1 && relativePath.pntr[0] == '.' && !(relativePath.length >= 2 && relativePath.pntr[1] == '.'))
	{
		isRelative = true;
		if (relativePath.length >= 2 && (relativePath.pntr[1] == '/' || relativePath.pntr[1] == '\\'))
		{
			relativePath.pntr += 2;
			relativePath.length -= 2;
		}
		else
		{
			relativePath.pntr += 1;
			relativePath.length -= 1;
		}
	}
	
	if (relativePath.length == 0)
	{
		result.length = pig->defaultDirectory.length - ((endedWithSlash || pig->defaultDirectory.length == 0) ? 0 : 1);
		result.pntr = AllocChars(memArena, result.length+1);
		NotNull(result.pntr);
		MyMemCopy(result.pntr, pig->defaultDirectory.pntr, result.length);
		result.pntr[result.length] = '\0';
		StrReplaceInPlace(result, "\\", "/");
		return result;
	}
	
	if (relativePath.length >= 2 && relativePath.pntr[1] == ':')
	{
		Assert(!isRelative); //can't have something like ./C:
	}
	else
	{
		isRelative = true;
	}
	
	if (isRelative)
	{
		//Remove leading / from the relative path if it exists
		if (relativePath.length >= 0 && (relativePath.pntr[0] == '/' || relativePath.pntr[0] == '\\'))
		{
			relativePath.pntr++;
			relativePath.length--;
		}
		
		result = CombineStrs(memArena, pig->defaultDirectory, relativePath);
		NotNullStr(&result);
	}
	else
	{
		result = AllocString(memArena, &relativePath);
		NotNullStr(&result);
	}
	
	//TODO: Contract .. sections of the path
	
	if (giveBackslashes) { StrReplaceInPlace(result, "/", "\\"); }
	else { StrReplaceInPlace(result, "\\", "/"); }
	return result;
}

bool DoesFileExist(MyStr_t filePath, bool* isFolderOut)
{
	//NOTE: This function should be multi-thread safe!
	NotNullStr(&filePath);
	if (isFolderOut != nullptr) { *isFolderOut = false; }
	TempPushMark();
	MyStr_t fullPath = GetFullPath(GetTempArena(), filePath, true);
	DWORD fileType = GetFileAttributesA(fullPath.pntr);
	TempPopMark();
	if (fileType == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}
	else if (fileType & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (isFolderOut != nullptr) { *isFolderOut = true; }
		return false;
	}
	else
	{
		return true;
	}
}

FileEnumerator_t StartEnumeratingFiles(MyStr_t folderPath, bool enumerateFiles, bool enumerateFolders)
{
	FileEnumerator_t result = {};
	result.folderPath = GetFullPath(GetTempArena(), folderPath, true);
	NotNullStr(&result.folderPath);
	//NOTE: File enumeration in windows requires that we have a slash on the end and a * wildcard character
	result.folderPath = PrintInArenaStr(GetTempArena(), "%.*s%s", result.folderPath.length, result.folderPath.pntr, (StrEndsWith(result.folderPath, "\\") ? "" : "\\"));
	NotNullStr(&result.folderPath);
	result.folderPathWithWildcard = PrintInArenaStr(GetTempArena(), "%.*s*", result.folderPath.length, result.folderPath.pntr);
	NotNullStr(&result.folderPathWithWildcard);
	result.enumerateFiles = enumerateFiles;
	result.enumerateFolders = enumerateFolders;
	result.index = UINT64_MAX;
	result.nextIndex = 0;
	result.finished = false;
	return result;
}

bool EnumerateFiles(FileEnumerator_t* enumerator, MyStr_t* pathOut, MemArena_t* pathOutArena, bool giveFullPath)
{
	NotNull(enumerator);
	NotNull2(pathOut, pathOutArena);
	if (enumerator->finished) { return false; }
	while (true)
	{
		bool firstEnumeration = (enumerator->index == UINT64_MAX);
		enumerator->index = enumerator->nextIndex;
		if (firstEnumeration)
		{
			enumerator->handle = FindFirstFileA(enumerator->folderPathWithWildcard.pntr, &enumerator->findData);
			if (enumerator->handle == INVALID_HANDLE_VALUE)
			{
				enumerator->finished = true;
				return false;
			}
		}
		else
		{
			BOOL findNextResult = FindNextFileA(enumerator->handle, &enumerator->findData);
			if (findNextResult == 0)
			{
				enumerator->finished = true;
				return false;
			}
		}
		
		//Skip the generic "this" folder listing
		if (enumerator->findData.cFileName[0] == '.' && enumerator->findData.cFileName[1] == '\0')
		{
			continue;
		}
		//Skip the parent folder listing
		if (enumerator->findData.cFileName[0] == '.' && enumerator->findData.cFileName[1] == '.' && enumerator->findData.cFileName[2] == '\0')
		{
			continue;
		}
		
		bool isFolder = IsFlagSet(enumerator->findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
		if ((isFolder && enumerator->enumerateFolders) || (!isFolder && enumerator->enumerateFiles))
		{
			if (giveFullPath)
			{
				*pathOut = PrintInArenaStr(pathOutArena, "%.*s%s", enumerator->folderPath.length, enumerator->folderPath.pntr, enumerator->findData.cFileName);
				NotNullStr(pathOut);
				StrReplaceInPlace(*pathOut, "\\", "/");
			}
			else
			{
				*pathOut = NewStringInArenaNt(pathOutArena, enumerator->findData.cFileName);
				NotNullStr(pathOut);
				StrReplaceInPlace(*pathOut, "\\", "/");
			}
			enumerator->nextIndex = enumerator->index+1;
			return true;
		}
	}
	Assert(false); //Shouldn't be possible to get here
	return false;
}

