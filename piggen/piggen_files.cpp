/*
File:   piggen_files.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds functions that help us manipulate files in the OS file system
*/

const char* GetWin32ErrorCodeStr(DWORD windowsErrorCode, bool printUnknownValue = false)
{
	switch (windowsErrorCode)
	{
		case ERROR_FILE_NOT_FOUND:    return "ERROR_FILE_NOT_FOUND";    //2
		case ERROR_FILE_EXISTS:       return "ERROR_FILE_EXISTS";       //80
		case ERROR_ALREADY_EXISTS:    return "ERROR_ALREADY_EXISTS";    //183
		case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION"; //?
		case ERROR_PIPE_BUSY:         return "ERROR_PIPE_BUSY";         //?
		case ERROR_ACCESS_DENIED:     return "ERROR_ACCESS_DENIED";     //?
		case ERROR_DIRECTORY:         return "ERROR_DIRECTORY";         //267
		default: return (printUnknownValue ? TempPrint("(0x%08X)", windowsErrorCode) : "UNKNOWN");
	}
}

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

// This function will automatically add a null-term char at the end of the file in case you want to read it as a string
bool ReadFileContents(MyStr_t filePath, FileContents_t* contentsOut)
{
	NotEmptyStr(&filePath);
	NotNull(contentsOut);
	ClearPointer(contentsOut);
	
	TempPushMark();
	MyStr_t fullPath = GetFullPath(GetTempArena(), filePath, true);
	
	//TODO: This check maybe takes a lot of time? Should we just attempt to open the file and be done with it?
	#if 0
	if (!Win32_DoesFileExist(fullPath, nullptr))
	{
		//The file doesn't exist
		contentsOut->readSuccess = false;
		contentsOut->errorCode = ERROR_FILE_NOT_FOUND;
		TempPopMark();
		return false;
	}
	#endif
	
	HANDLE fileHandle = CreateFileA(
		fullPath.pntr,         //lpFileName
		GENERIC_READ,          //dwDesiredAccess
		FILE_SHARE_READ,       //dwShareMode
		NULL,                  //lpSecurityAttributes (NULL: no sub process access)
		OPEN_EXISTING,         //dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL, //dwFlagsAndAttributes
		NULL                   //hTemplateFile
	);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		contentsOut->readSuccess = false;
		contentsOut->errorCode = GetLastError();
		if (contentsOut->errorCode == ERROR_FILE_NOT_FOUND)
		{
			PrintLine_E("File not found at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, GetWin32ErrorCodeStr(contentsOut->errorCode, true));
		}
		else
		{
			//The file might have permissions that prevent us from reading it
			PrintLine_E("Failed to open file that exists at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, GetWin32ErrorCodeStr(contentsOut->errorCode, true));
		}
		TempPopMark();
		CloseHandle(fileHandle);
		return false;
	}
	
	LARGE_INTEGER fileSize;
	BOOL getFileSizeResult = GetFileSizeEx(
		fileHandle, //hFile
		&fileSize   //lpFileSize
	);
	if (getFileSizeResult == 0)
	{
		contentsOut->readSuccess = false;
		contentsOut->errorCode = GetLastError();
		PrintLine_E("Failed to size of file at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, GetWin32ErrorCodeStr(contentsOut->errorCode, true));
		TempPopMark();
		CloseHandle(fileHandle);
		return false;
	}
	
	contentsOut->size = fileSize.QuadPart;
	void* resultData = AllocArray(&pig->stdHeap, u8, contentsOut->size+1); //+1 for null-term
	NotNullMsg(resultData, "Failed to allocate space to hold file contents. The application probably tried to open a massive file");
	
	if (contentsOut->size > 0)
	{
		//TODO: What about files that are larger than DWORD_MAX? Will we just fail to read these?
		DWORD bytesRead = 0;
		BOOL readFileResult = ReadFile(
			fileHandle,               //hFile
			resultData, //lpBuffer
			(DWORD)contentsOut->size, //nNumberOfBytesToRead
			&bytesRead,               //lpNumberOfBytesRead
			NULL                      //lpOverlapped
		);
		if (readFileResult == 0)
		{
			contentsOut->readSuccess = false;
			contentsOut->errorCode = GetLastError();
			PrintLine_E("Failed to ReadFile contents at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, GetWin32ErrorCodeStr(contentsOut->errorCode, true));
			FreeMem(&pig->stdHeap, resultData);
			TempPopMark();
			CloseHandle(fileHandle);
			return false;
		}
		if (bytesRead < contentsOut->size)
		{
			contentsOut->readSuccess = false;
			contentsOut->errorCode = GetLastError();
			PrintLine_E("Failed to all of the file at \"%.*s\". Error code: %s. Read %u/%llu bytes", fullPath.length, fullPath.pntr, GetWin32ErrorCodeStr(contentsOut->errorCode, true), bytesRead, contentsOut->size);
			FreeMem(&pig->stdHeap, resultData);
			TempPopMark();
			CloseHandle(fileHandle);
			return false;
		}
	}
	
	TempPopMark();
	CloseHandle(fileHandle);
	
	contentsOut->data = (u8*)resultData;
	contentsOut->data[contentsOut->size] = '\0'; //add null-term
	contentsOut->path = AllocString(&pig->stdHeap, &filePath);
	NotNullStr(&contentsOut->path);
	
	contentsOut->id = pig->nextFileContentsId;
	pig->nextFileContentsId++;
	
	contentsOut->readSuccess = true;
	return true;
}

void FreeFileContents(FileContents_t* fileContents)
{
	//NOTE: This function should be multi-thread safe!
	NotNull(fileContents);
	if (fileContents->data != nullptr)
	{
		AssertMsg(fileContents->size > 0, "FileContents_t.size was 0 when calling FreeFileContents");
		FreeMem(&pig->stdHeap, fileContents->data, fileContents->size+1);
	}
	if (fileContents->path.pntr != nullptr)
	{
		FreeString(&pig->stdHeap, &fileContents->path);
	}
	ClearPointer(fileContents);
}

//TODO: We should probably convert \n to \r\n on windows so the file can be opened by all widows programs
bool WriteEntireFile(MyStr_t filePath, const void* memory, u64 memorySize)
{
	NotNullStr(&filePath);
	Assert(memorySize > 0);
	NotNull(memory);
	bool result = false;
	
	TempPushMark();
	MyStr_t fullPath = GetFullPath(TempArena, filePath, true);
	HANDLE fileHandle = CreateFileA(
		fullPath.pntr,         //Name of the file
		GENERIC_WRITE,         //Open for writing
		0,                     //Do not share
		NULL,                  //Default security
		CREATE_ALWAYS,         //Always overwrite
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		0                      //No Template File
	);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		//TODO: Should we assert if memorySize > max value of DWORD?
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, (DWORD)memorySize, &bytesWritten, 0))
		{
			if ((u64)bytesWritten == memorySize)
			{
				result = true;
			}
			else
			{
				PrintLine_W("Only wrote %u/%llu bytes to file at \"%.*s\"", bytesWritten, memorySize, fullPath.length, fullPath.pntr);
			}
		}
		else
		{
			PrintLine_E("Failed to write %u bytes to file at \"%.*s\"", memorySize, fullPath.length, fullPath.pntr);
		}
		CloseHandle(fileHandle);
	}
	else
	{
		PrintLine_E("Failed to open file for writing at \"%.*s\"", fullPath.length, fullPath.pntr);
	}
	TempPopMark();
	
	return result;
}

void CloseFile(OpenFile_t* openFile)
{
	NotNull(openFile);
	if (openFile->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(openFile->handle);
	}
	if (openFile->path.chars != nullptr) { FreeString(mainHeap, &openFile->path); }
	ClearPointer(openFile);
	openFile->handle = INVALID_HANDLE_VALUE;
}

bool OpenFile(MyStr_t filePath, OpenFile_t* openFileOut)
{
	NotNullStr(&filePath);
	NotNull(openFileOut);
	
	MyStr_t fullPath = GetFullPath(mainHeap, filePath, true);
	NotNull(fullPath.chars);
	
	HANDLE fileHandle = CreateFileA(
		fullPath.pntr,         //lpFileName
		GENERIC_WRITE,         //dwDesiredAccess
		0,                     //dwShareMode
		NULL,                  //lpSecurityAttributes
		CREATE_ALWAYS,         //dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL, //dwFlagsAndAttributes
		NULL                   //hTemplateFile
	);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		PrintLine_E("Failed to Create file at \"%.*s\"", fullPath.length, fullPath.pntr);
		FreeString(mainHeap, &fullPath);
		return false;
	}
	
	ClearPointer(openFileOut);
	openFileOut->isOpen = true;
	openFileOut->handle = fileHandle;
	openFileOut->id = pig->nextOpenFileId; //TODO: Make this thread safe!
	pig->nextOpenFileId++;
	openFileOut->path = fullPath;
	return true;
}

bool WriteToFile(OpenFile_t* openFile, u64 numBytes, const void* bytesPntr)
{
	NotNull(openFile);
	if (!openFile->isOpen) { return false; }
	Assert(openFile->handle != INVALID_HANDLE_VALUE);
	AssertIf(numBytes > 0, bytesPntr != nullptr);
	if (numBytes == 0) { return true; } //no bytes to write always succeeds
	Assert(numBytes <= UINT32_MAX);
	
	DWORD numBytesToWrite = (DWORD)numBytes;
	DWORD numBytesWritten = 0;
	BOOL writeResult = WriteFile(
		openFile->handle, //hFile
		bytesPntr,        //lpBuffer
		numBytesToWrite,  //nNumberOfBytesToWrite
		&numBytesWritten, //lpNumberOfnumBytesWritten
		NULL              //lpOverlapped
	);
	if (writeResult == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("WriteFile failed: 0x%08X (%u)", errorCode, errorCode);
		return false;
	}
	
	Assert(numBytesWritten <= numBytesToWrite);
	if (numBytesWritten < numBytesToWrite)
	{
		PrintLine_E("Partial write occurred: %u/%llu", numBytesWritten, numBytesToWrite);
		return false;
	}
	
	return true;
}

bool IsRelativePath(MyStr_t path)
{
	if (path.length >= 2 && path.chars[1] == ':' && IsCharAlphabetic(path.chars[0]))
	{
		return false;
	}
	else
	{
		return true;
	}
}
