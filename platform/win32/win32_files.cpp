/*
File:   win32_files.cpp
Author: Taylor Robbins
Date:   09\27\2021
Description: 
	** Holds functions that help the platform layer and engine manipulate and deal with files
	** Functions related to file watching are in win32_file_watching.cpp
*/

//TODO: Handle "/" case a bit better
//TODO: Convert all forward slashes to back slashes on windows!
// +==============================+
// |      Win32_GetFullPath       |
// +==============================+
// Never returns null string
// MyStr_t GetFullPath(MemArena_t* memArena, MyStr_t relativePath, bool giveBackslashes)
PLAT_API_GET_FULL_PATH_DEF(Win32_GetFullPath)
{
	NotNull(memArena);
	NotNullStr(&relativePath);
	NotNullStr(&Platform->defaultDirectory);
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
		result.length = Platform->defaultDirectory.length - ((endedWithSlash || Platform->defaultDirectory.length == 0) ? 0 : 1);
		result.pntr = AllocChars(memArena, result.length+1);
		NotNull(result.pntr);
		MyMemCopy(result.pntr, Platform->defaultDirectory.pntr, result.length);
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
		
		result = CombineStrs(memArena, Platform->defaultDirectory, relativePath);
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

// +==============================+
// |     Win32_DoesFileExist      |
// +==============================+
// bool DoesFileExist(MyStr_t filePath, bool* isFolderOut)
PLAT_API_DOES_FILE_EXIST_DEF(Win32_DoesFileExist)
{
	//NOTE: This function should be multi-thread safe!
	NotNullStr(&filePath);
	if (isFolderOut != nullptr) { *isFolderOut = false; }
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
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

// +==============================+
// | Win32_StartEnumeratingFiles  |
// +==============================+
// PlatFileEnumerator_t StartEnumeratingFiles(MyStr_t folderPath, bool enumerateFiles, bool enumerateFolders)
PLAT_API_START_ENUMERATING_FILES_DEF(Win32_StartEnumeratingFiles)
{
	PlatFileEnumerator_t result = {};
	result.folderPath = Win32_GetFullPath(GetTempArena(), folderPath, true);
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

// +==============================+
// |     Win32_EnumerateFiles     |
// +==============================+
// bool EnumerateFiles(PlatFileEnumerator_t* enumerator, MyStr_t* pathOut, MemArena_t* pathOutArena, bool giveFullPath)
PLAT_API_ENUMERATE_FILES_DEF(Win32_EnumerateFiles)
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

// +==============================+
// |      Win32_CreateFolder      |
// +==============================+
// bool CreateFolder(MyStr_t folderPath)
PLAT_API_CREATE_FOLDER(Win32_CreateFolder)
{
	NotNullStr(&folderPath);
	TempPushMark();
	MyStr_t fullFolderPath = Win32_GetFullPath(GetTempArena(), folderPath, true);
	
	BOOL createResult = CreateDirectoryA(
		fullFolderPath.pntr, //lpPathName
		NULL //lpSecurityAttributes
	);
	TempPopMark();
	
	bool result = (createResult == TRUE);
	if (createResult != TRUE)
	{
		PrintLine_E("Failed to create new folder at \"%.*s\"", fullFolderPath.length, fullFolderPath.pntr);
	}
	
	return result;
}

// +==============================+
// |    Win32_ReadFileContents    |
// +==============================+
// This function will automatically add a null-term char at the end of the file in case you want to read it as a string
// bool ReadFileContents(MyStr_t filePath, PlatFileContents_t* contentsOut)
PLAT_API_READ_FILE_CONTENTS_DEF(Win32_ReadFileContents)
{
	//NOTE: This function should be multi-thread safe!
	NotEmptyStr(&filePath);
	NotNull(contentsOut);
	ClearPointer(contentsOut);
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	
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
			PrintLine_E("File not found at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true));
		}
		else
		{
			//The file might have permissions that prevent us from reading it
			PrintLine_E("Failed to open file that exists at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true));
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
		PrintLine_E("Failed to size of file at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true));
		TempPopMark();
		CloseHandle(fileHandle);
		return false;
	}
	
	contentsOut->size = fileSize.QuadPart;
	void* resultData = AllocArray(&Platform->stdHeap, u8, contentsOut->size+1); //+1 for null-term
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
			PrintLine_E("Failed to ReadFile contents at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true));
			FreeMem(&Platform->stdHeap, resultData);
			TempPopMark();
			CloseHandle(fileHandle);
			return false;
		}
		if (bytesRead < contentsOut->size)
		{
			contentsOut->readSuccess = false;
			contentsOut->errorCode = GetLastError();
			PrintLine_E("Failed to all of the file at \"%.*s\". Error code: %s. Read %u/%llu bytes", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true), bytesRead, contentsOut->size);
			FreeMem(&Platform->stdHeap, resultData);
			TempPopMark();
			CloseHandle(fileHandle);
			return false;
		}
	}
	
	TempPopMark();
	CloseHandle(fileHandle);
	
	contentsOut->data = (u8*)resultData;
	contentsOut->data[contentsOut->size] = '\0'; //add null-term
	contentsOut->path = AllocString(&Platform->stdHeap, &filePath);
	NotNullStr(&contentsOut->path);
	
	//TODO: Make this thread safe!
	contentsOut->id = Platform->nextFileContentsId;
	Platform->nextFileContentsId++;
	
	contentsOut->readSuccess = true;
	return true;
}

// +==============================+
// |    Win32_FreeFileContents    |
// +==============================+
// void FreeFileContents(PlatFileContents_t* fileContents)
PLAT_API_FREE_FILE_CONTENTS_DEF(Win32_FreeFileContents)
{
	//NOTE: This function should be multi-thread safe!
	NotNull(fileContents);
	if (fileContents->data != nullptr)
	{
		AssertMsg(fileContents->size > 0, "PlatFileContents_t.size was 0 when calling Win32_FreeFileContents");
		FreeMem(&Platform->stdHeap, fileContents->data, fileContents->size+1);
	}
	if (fileContents->path.pntr != nullptr)
	{
		FreeString(&Platform->stdHeap, &fileContents->path);
	}
	ClearPointer(fileContents);
}

//TODO: We should probably convert \n to \r\n on windows so the file can be opened by all widows programs
// +==============================+
// |    Win32_WriteEntireFile     |
// +==============================+
// bool WriteEntireFile(MyStr_t filePath, const void* memory, u64 memorySize)
WRITE_ENTIRE_FILE_DEFINITION(Win32_WriteEntireFile)
{
	NotNullStr(&filePath);
	Assert(memorySize > 0);
	NotNull(memory);
	bool result = false;
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
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

// +==============================+
// |        Win32_OpenFile        |
// +==============================+
// bool OpenFile(MyStr_t filePath, bool forWriting, PlatOpenFile_t* openFileOut)
PLAT_API_OPEN_FILE_DEFINITION(Win32_OpenFile)
{
	NotNullStr(&filePath);
	NotNull(openFileOut);
	
	TempPushMark(); //TODO: Make this thread safe!
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	
	HANDLE fileHandle = CreateFileA(
		fullPath.pntr,                               //lpFileName
		(forWriting ? GENERIC_WRITE : GENERIC_READ), //dwDesiredAccess
		(forWriting ? 0 : FILE_SHARE_READ),          //dwShareMode
		NULL,                                        //lpSecurityAttributes
		(forWriting ? CREATE_ALWAYS : OPEN_ALWAYS),  //dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL,                       //dwFlagsAndAttributes
		NULL                                         //hTemplateFile
	);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		PrintLine_E("Failed to %s file at \"%.*s\"", (forWriting ? "Create" : "Open"), fullPath.length, fullPath.pntr);
		TempPopMark();
		return false;
	}
	
	//TODO: Seems like this seek to end and back stuff was assuming that we were opening a file for appending when doing writing.
	//      But we do CREATE_ALWAYS as the option above which ensures we overwrite/reset the file when opening it.
	//      Should we have an option for appending?? Or should we reframe this code to not act like the length matters for writing?
	//Seek to the end of the file
	LONG newCursorPosHighOrder = 0;
	DWORD newCursorPos = SetFilePointer(
		fileHandle,             //hFile
		0,                      //lDistanceToMove
		&newCursorPosHighOrder, //lDistanceToMoveHigh
		FILE_END                //dMoveMethod
	);
	if (newCursorPos == INVALID_SET_FILE_POINTER)
	{
		PrintLine_E("Failed to seek to the end of the file when opened for %s \"%.*s\"!", (forWriting ? "writing" : "reading"), fullPath.length, fullPath.pntr);
		CloseHandle(fileHandle);
		TempPopMark();
		return false;
	}
	
	u64 fileSize = (((u64)newCursorPosHighOrder << 32) | (u64)newCursorPos);
	u64 cursorIndex = fileSize;
	if (!forWriting)
	{
		DWORD beginMove = SetFilePointer(
			fileHandle, //hFile
			0, //lDistanceToMove,
			NULL, //lDistanceToMoveHigh
			FILE_BEGIN
		);
		Assert(beginMove != INVALID_SET_FILE_POINTER);
		cursorIndex = 0;
	}
	
	TempPopMark();
	
	ClearPointer(openFileOut);
	openFileOut->isOpen = true;
	openFileOut->handle = fileHandle;
	openFileOut->id = Platform->nextOpenFileId; //TODO: Make this thread safe!
	Platform->nextOpenFileId++;
	openFileOut->openedForWriting = forWriting;
	openFileOut->cursorIndex = cursorIndex;
	openFileOut->fileSize = fileSize;
	openFileOut->path = AllocString(&Platform->mainHeap, &filePath);
	NotNull(openFileOut->path.pntr);
	openFileOut->fullPath = Win32_GetFullPath(&Platform->mainHeap, filePath, false);
	NotNull(openFileOut->fullPath.pntr);
	return true;
}

// +==============================+
// |      Win32_WriteToFile       |
// +==============================+
// bool WriteToFile(PlatOpenFile_t* openFile, u64 numBytes, const void* bytesPntr, bool convertNewLines)
PLAT_API_WRITE_TO_FILE_DEFINITION(Win32_WriteToFile)
{
	NotNull(openFile);
	if (!openFile->isOpen) { return false; }
	Assert(openFile->openedForWriting);
	Assert(openFile->handle != INVALID_HANDLE_VALUE);
	AssertIf(numBytes > 0, bytesPntr != nullptr);
	if (numBytes == 0) { return true; } //no bytes to write always succeeds
	Assert(numBytes <= UINT32_MAX);
	
	DWORD numBytesToWrite = (DWORD)numBytes;
	const void* bytesToWrite = bytesPntr;
	if (convertNewLines)
	{
		//TODO: Make this TempArena usage thread safe!
		TempPushMark();
		MyStr_t newLinesReplacedStr = StrReplace(NewStr(numBytesToWrite, (char*)bytesToWrite), "\r\n", "\n", TempArena);
		NotNullStr(&newLinesReplacedStr);
		numBytesToWrite = (DWORD)newLinesReplacedStr.length;
		bytesToWrite = (const void*)newLinesReplacedStr.pntr;
	}
	
	DWORD numBytesWritten = 0;
	BOOL writeResult = WriteFile(
		openFile->handle, //hFile
		bytesToWrite,     //lpBuffer
		numBytesToWrite,  //nNumberOfBytesToWrite
		&numBytesWritten, //lpNumberOfnumBytesWritten
		NULL              //lpOverlapped
	);
	if (convertNewLines) { TempPopMark(); }
	if (writeResult == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("WriteFile failed: 0x%08X (%u)", errorCode, errorCode);
		return false;
	}
	
	openFile->cursorIndex += numBytesWritten;
	openFile->fileSize += numBytesWritten;
	
	Assert(numBytesWritten <= numBytesToWrite);
	if (numBytesWritten < numBytesToWrite)
	{
		PrintLine_E("Partial write occurred: %u/%llu", numBytesWritten, numBytesToWrite);
		return false;
	}
	
	return true;
}

// +==============================+
// |      Win32_ReadFromFile      |
// +==============================+
// Return is numBytes actually placed into the bufferPntr. This might be smaller for 3 reasons:
//  1. End of the file was reached (shouldn't be a problem if fileSize and cursorIndex in OpenFile_t are respected)
//  2. ReadFile gave a partial read (not sure why this might happen)
//  3. convertNewLines is true and we replaced 1 or more instances of "\r\n" to "\n"
// u64 ReadFromFile(PlatOpenFile_t* openFile, u64 numBytes, void* bufferPntr, bool convertNewLines)
PLAT_API_READ_FROM_FILE_DEFINITION(Win32_ReadFromFile)
{
	NotNull(openFile);
	NotNull(bufferPntr);
	Assert(numBytes < UINT32_MAX);
	if (!openFile->isOpen) { return 0; }
	Assert(!openFile->openedForWriting);
	if (numBytes == 0) { return 0; }
	
	DWORD numBytesToRead = (DWORD)numBytes;
	if (openFile->cursorIndex + numBytes > openFile->fileSize)
	{
		numBytesToRead = (DWORD)(openFile->fileSize - openFile->cursorIndex);
	}
	
	DWORD numBytesRead = 0;
	BOOL readResult = ReadFile(
		openFile->handle, //hFile
		bufferPntr,       //lpBuffer
		numBytesToRead,   //nNumberOfBytesToRead
		&numBytesRead,    //lpNumberOfBytesRead
		NULL              //lpOverlapped
	);
	if (readResult == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("ReadFile failed: 0x%08X (%u)", errorCode, errorCode);
		return false;
	}
	Assert(numBytesRead <= numBytesToRead);
	
	openFile->cursorIndex += numBytesRead;
	
	u64 result = numBytesRead;
	if (convertNewLines)
	{
		u64 numNewLinesReplaced = StrReplaceInPlace(NewStr(numBytesRead, (char*)bufferPntr), "\r\n", "\n");
		Assert(result > numNewLinesReplaced);
		result -= numNewLinesReplaced;  //1 byte smaller for each "\r\n" pair to "\n"
	}
	
	if (numBytesRead < numBytesToRead) { PrintLine_W("Partial read occurred: %u/%llu", numBytesRead, numBytesToRead); }
	return numBytesRead;
}

// +==============================+
// |     Win32_MoveFileCursor     |
// +==============================+
// bool MoveFileCursor(PlatOpenFile_t* openFile, i64 moveAmount)
PLAT_API_MOVE_FILE_CURSOR_DEFINITION(Win32_MoveFileCursor)
{
	NotNull(openFile);
	if (!openFile->isOpen) { return false; }
	Assert(!openFile->openedForWriting);
	if (moveAmount == 0) { return true; }
	Assert(openFile->handle != INVALID_HANDLE_VALUE);
	if (moveAmount < 0 && (u64)(-moveAmount) > openFile->cursorIndex) { return false; } //seek past beginning
	if (moveAmount > 0 && openFile->cursorIndex + (u64)moveAmount > openFile->fileSize) { return false; } //seek past end
	
	LARGE_INTEGER distanceToMoveLarge = {};
	distanceToMoveLarge.QuadPart = (LONGLONG)moveAmount;
	
	LARGE_INTEGER newCursorPosLarge = {};
	BOOL setResult = SetFilePointerEx(
		openFile->handle, //hFile
		distanceToMoveLarge, //liDistanceToMove
		&newCursorPosLarge, //lpNewFilePointer
		FILE_CURRENT //dwMoveMethod
	);
	if (setResult == 0)
	{
		DWORD errorCode = GetLastError();
		PrintLine_E("SetFilePointerEx failed: 0x%08X (%u)", errorCode, errorCode);
		return false;
	}
	
	openFile->cursorIndex += moveAmount;
	return false;
}

// +==============================+
// |   Win32_SeekToOffsetInFile   |
// +==============================+
// bool SeekToOffsetInFile(PlatOpenFile_t* openFile, u64 offset)
PLAT_API_SEEK_TO_OFFSET_IN_FILE_DEFINITION(Win32_SeekToOffsetInFile)
{
	NotNull(openFile);
	if (!openFile->isOpen) { return false; }
	Assert(offset < openFile->fileSize);
	i64 moveAmount = 0;
	if (openFile->cursorIndex >= offset) { moveAmount = -(i64)(openFile->cursorIndex - offset); }
	else { moveAmount = (i64)(offset - openFile->cursorIndex); }
	return Win32_MoveFileCursor(openFile, moveAmount);
}

// +==============================+
// |       Win32_CloseFile        |
// +==============================+
// void CloseFile(PlatOpenFile_t* openFile)
PLAT_API_CLOSE_FILE_DEFINITION(Win32_CloseFile)
{
	NotNull(openFile);
	if (openFile->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(openFile->handle);
	}
	if (openFile->path.pntr != nullptr) { FreeString(&Platform->mainHeap, &openFile->path); }
	if (openFile->fullPath.pntr != nullptr) { FreeString(&Platform->mainHeap, &openFile->fullPath); }
	ClearPointer(openFile);
	openFile->handle = INVALID_HANDLE_VALUE;
}

// +==============================+
// |   Win32_TryParseImageFile    |
// +==============================+
// bool TryParseImageFile(const PlatFileContents_t* contents, u8 desiredPixelSize, PlatImageData_t* imageOut)
PLAT_API_TRY_PARSE_IMAGE_FILE_DEF(Win32_TryParseImageFile)
{
	//NOTE: This function should be multi-thread safe!
	NotNull(contents);
	NotNull(imageOut);
	ClearPointer(imageOut);
	if (contents->size == 0 || contents->data == nullptr) { return false; }
	
	//TODO: Should we upgrade stbi_load_from_memory to support u64 sized files?
	AssertMsg(contents->size <= INT_MAX, "Image file was too big to parse. STB Image doesn't support super large file sizes");
	
	int stbNumChannels;
	int stbWidth, stbHeight;
	u8* stbImageData = stbi_load_from_memory(
		contents->data, (int)contents->size,
		&stbWidth, &stbHeight,
		&stbNumChannels,
		desiredPixelSize
	);
	if (stbImageData == nullptr)
	{
		//TODO: Write some errorCode to the imageOut?
		return false;
	}
	Assert(stbWidth > 0 && stbHeight > 0);
	
	imageOut->data8 = stbImageData;
	imageOut->size = NewVec2i(stbWidth, stbHeight);
	imageOut->pixelSize = desiredPixelSize;
	imageOut->rowSize = (u64)(imageOut->pixelSize * imageOut->width); //TODO: Is there ever any padding added by stb_image?
	imageOut->dataSize = imageOut->rowSize * (u64)imageOut->height;
	
	//TODO: Make this thread safe!
	imageOut->id = Platform->nextImageId;
	Platform->nextImageId++;
	
	return true;
}

struct Win32_StbImageWriteContext_t
{
	MyStr_t filePath;
	PlatImageFormat_t imageFormat;
	const PlatImageData_t* imageData;
	bool hadFailures;
	u64 fileSize;
	HANDLE fileHandle;
};
void Win32_StbImageWriteFunctionCallback(void* contextPntr, void* data, int size)
{
	NotNull(contextPntr);
	if (size == 0) { return; }
	NotNull(data);
	Win32_StbImageWriteContext_t* context = (Win32_StbImageWriteContext_t*)contextPntr;
	
	DWORD bytesWritten = 0;
	if (WriteFile(context->fileHandle, data, (DWORD)size, &bytesWritten, 0))
	{
		if (bytesWritten == (DWORD)size)
		{
			context->fileSize += (DWORD)size;
		}
		else
		{
			PrintLine_E("Failed to write all bytes in Win32_StbImageWriteFunctionCallback: %u/%u bytes written", bytesWritten, (DWORD)size);
			context->hadFailures = true;
		}
	}
	else
	{
		PrintLine_E("WriteFile failed in Win32_StbImageWriteFunctionCallback: %u/%u bytes written", bytesWritten, (DWORD)size);
		context->hadFailures = true;
	}
}

// +==============================+
// |  Win32_SaveImageDataToFile   |
// +==============================+
// bool SaveImageDataToFile(MyStr_t filePath, const PlatImageData_t* imageData, PlatImageFormat_t imageFormat, u64* fileSizeOut)
PLAT_API_SAVE_IMAGE_DATA_TO_FILE(Win32_SaveImageDataToFile)
{
	NotNullStr(&filePath);
	NotNull(imageData);
	NotNull(imageData->data8);
	Assert(imageFormat < PlatImageFormat_NumFormats);
	
	Win32_StbImageWriteContext_t context = {};
	context.filePath = filePath;
	context.imageData = imageData;
	context.imageFormat = imageFormat;
	context.hadFailures = false;
	context.fileSize = 0;
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	
	context.fileHandle = CreateFileA(
		fullPath.pntr,         //Name of the file
		GENERIC_WRITE,         //Open for writing
		0,                     //Do not share
		NULL,                  //Default security
		CREATE_ALWAYS,         //Create always
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		NULL                   //No Template File
	);
	
	if (context.fileHandle == INVALID_HANDLE_VALUE)
	{
		PrintLine_E("Failed to open file for writing image into: \"%.*s\"", fullPath.length, fullPath.pntr);
		TempPopMark();
		return false;
	}
	
	switch (imageFormat)
	{
		case PlatImageFormat_Png:
		{
			int writeResult = stbi_write_png_to_func(
				Win32_StbImageWriteFunctionCallback,
				&context,
				imageData->width,
				imageData->height,
				sizeof(u32),
				imageData->data8,
				(int)imageData->rowSize
			);
			if (writeResult == 0)
			{
				PrintLine_E("stbi_write_png failed for unknown reason! %d", writeResult);
				context.hadFailures = true;
			}
		} break;
		
		case PlatImageFormat_Bmp:
		{
			int writeResult = stbi_write_bmp_to_func(
				Win32_StbImageWriteFunctionCallback,
				&context,
				imageData->width,
				imageData->height,
				sizeof(u32),
				imageData->data8
			);
			if (writeResult == 0)
			{
				PrintLine_E("stbi_write_bmp failed for unknown reason! %d", writeResult);
				context.hadFailures = true;
			}
		} break;
		
		case PlatImageFormat_Tga:
		{
			int writeResult = stbi_write_tga_to_func(
				Win32_StbImageWriteFunctionCallback,
				&context,
				imageData->width,
				imageData->height,
				sizeof(u32),
				imageData->data8
			);
			if (writeResult == 0)
			{
				PrintLine_E("stbi_write_tga failed for unknown reason! %d", writeResult);
				context.hadFailures = true;
			}
		} break;
		
		default:
		{
			Unimplemented();
		} break;
	}
	
	CloseHandle(context.fileHandle);
	TempPopMark();
	
	if (fileSizeOut != nullptr) { *fileSizeOut = context.fileSize; }
	return !context.hadFailures;
}

// +==============================+
// |     Win32_FreeImageData      |
// +==============================+
// void FreeImageData(PlatImageData_t* imageData)
PLAT_API_FREE_IMAGE_DATA_DEF(Win32_FreeImageData)
{
	//NOTE: This function should be multi-thread safe!
	NotNull(imageData);
	if (imageData->data8 != nullptr)
	{
		AssertMsg(imageData->dataSize > 0, "PlatImageData_t.dataSize was 0 when calling Win32_FreeImageData");
		if (imageData->allocArena != nullptr)
		{
			FreeMem(imageData->allocArena, imageData->data8, imageData->dataSize);
		}
		else
		{
			stbi_image_free(imageData->data8);
		}
	}
	ClearPointer(imageData);
}

// +==============================+
// |        Win32_ShowFile        |
// +==============================+
// bool ShowFile(MyStr_t filePath)
SHOW_FILE_DEFINITION(Win32_ShowFile)
{
	NotNullStr(&filePath);
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	MyStr_t commandStr = TempPrintStr("C:\\Windows\\explorer.exe /select,\"%.*s\"", fullPath.length, fullPath.pntr);
	
	int systemResult = system(commandStr.pntr);
	
	if (systemResult >= 0)
	{
		TempPopMark();
		return true;
	}
	else
	{
		PrintLine_D("Command \"%.*s\" returned an error: %d", commandStr.length, commandStr.pntr, errno);
		TempPopMark();
		return false;
	}
}

// +==============================+
// |     Win32_ShowSourceFile     |
// +==============================+
// bool ShowSourceFile(MyStr_t filePath, u64 lineNumber)
SHOW_SOURCE_FILE_DEFINITION(Win32_ShowSourceFile)
{
	NotNullStr(&filePath);
	bool result = false;
	if (filePath.length == 0) { return false; }
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	MyStr_t commandStr = TempPrintStr("%.*s:%llu", fullPath.length, fullPath.pntr, lineNumber);
	
	u64 executeResult = (u64)ShellExecuteA(
		NULL,   //No parent window
		"open", //The action verb
		"\"C:\\Program Files\\Sublime Text 3\\subl.exe\"", //The target file
		commandStr.pntr, //No parameters
		NULL, //Use default working directory
		SW_SHOWNORMAL //Show command is normal
	);
	
	if (executeResult > 32)
	{
		result = true;
	}
	else
	{
		switch (executeResult)
		{
			case ERROR_FILE_NOT_FOUND:   { WriteLine_E("ShellExecute returned ERROR_FILE_NOT_FOUND"); } break;
			case ERROR_PATH_NOT_FOUND:   { WriteLine_E("ShellExecute returned ERROR_PATH_NOT_FOUND"); } break;
			case ERROR_BAD_FORMAT:       { WriteLine_E("ShellExecute returned ERROR_BAD_FORMAT"); } break;
			case SE_ERR_ACCESSDENIED:    { WriteLine_E("ShellExecute returned SE_ERR_ACCESSDENIED"); } break;
			case SE_ERR_ASSOCINCOMPLETE: { WriteLine_E("ShellExecute returned SE_ERR_ASSOCINCOMPLETE"); } break;
			case SE_ERR_DDEBUSY:         { WriteLine_E("ShellExecute returned SE_ERR_DDEBUSY"); } break;
			case SE_ERR_DDEFAIL:         { WriteLine_E("ShellExecute returned SE_ERR_DDEFAIL"); } break;
			case SE_ERR_DDETIMEOUT:      { WriteLine_E("ShellExecute returned SE_ERR_DDETIMEOUT"); } break;
			case SE_ERR_DLLNOTFOUND:     { WriteLine_E("ShellExecute returned SE_ERR_DLLNOTFOUND"); } break;
			// case SE_ERR_FNF:             { WriteLine_E("ShellExecute returned SE_ERR_FNF"); } break;
			case SE_ERR_NOASSOC:         { WriteLine_E("ShellExecute returned SE_ERR_NOASSOC"); } break;
			case SE_ERR_OOM:             { WriteLine_E("ShellExecute returned SE_ERR_OOM"); } break;
			// case SE_ERR_PNF:             { WriteLine_E("ShellExecute returned SE_ERR_PNF"); } break;
			case SE_ERR_SHARE:           { WriteLine_E("ShellExecute returned SE_ERR_SHARE"); } break;
			
			default:
			{
				PrintLine_E("ShellExecute failed with result 0x%02X for file at \"%.*s\"", executeResult, fullPath.length, fullPath.pntr);
			} break;
		};
	}
	
	TempPopMark();
	return result;
}

// +==============================+
// |  Win32_GetSpecialFolderPath  |
// +==============================+
//NOTE: applicationName needs to be folder path friendly (no invalid characters)
// MyStr_t GetSpecialFolderPath(SpecialFolder_t specialFolder, MyStr_t applicationName, MemArena_t* memArena)
PLAT_API_GET_SPECIAL_FOLDER_PATH(Win32_GetSpecialFolderPath)
{
	NotNull(memArena);
	NotNullStr(&applicationName);
	Assert(specialFolder != SpecialFolder_None && specialFolder < SpecialFolder_NumTypes);
	
	int specialFolderId = 0;
	switch (specialFolder)
	{
		case SpecialFolder_SavesAndSettings: specialFolderId = CSIDL_APPDATA;     break;
		case SpecialFolder_Screenshots:      specialFolderId = CSIDL_MYPICTURES;  break;
		case SpecialFolder_Share:            specialFolderId = CSIDL_MYDOCUMENTS; break;
		default:
		{
			PrintLine_E("SpecialFolder_%s (%u) is not implemented by the win32 platform layer!", GetSpecialFolderStr(specialFolder), specialFolder);
			DebugAssert(false);
			return MyStr_Empty;
		} break;
	}
	
	char pathBuffer[MAX_PATH+1];
	MyMemSet(&pathBuffer[0], 0xFF, MAX_PATH);
	pathBuffer[MAX_PATH] = '\0';
	
	BOOL getFolderPathResult = SHGetSpecialFolderPathA(
		NULL, //hwnd
		&pathBuffer[0], //pszPath
		specialFolderId, //csidl
		false //fCreate
	);
	if (getFolderPathResult == FALSE)
	{
		PrintLine_E("SHGetSpecialFolderPathA failed when trying to get SpecialFolder_%s", GetSpecialFolderStr(specialFolder));
		return MyStr_Empty;
	}
	Assert(BufferIsNullTerminated(ArrayCount(pathBuffer), pathBuffer));
	MyStr_t pathBufferStr = NewStr(MyStrLength64(&pathBuffer[0]), &pathBuffer[0]);
	StrReplaceInPlace(pathBufferStr, "\\", "/");
	
	MyStr_t result = pathBufferStr;
	switch (specialFolder)
	{
		case SpecialFolder_SavesAndSettings: result = PrintInArenaStr(memArena, "%.*s/%.*s",             pathBufferStr.length, pathBufferStr.pntr, applicationName.length, applicationName.pntr); break;
		case SpecialFolder_Screenshots:      result = PrintInArenaStr(memArena, "%.*s/%.*s Screenshots", pathBufferStr.length, pathBufferStr.pntr, applicationName.length, applicationName.pntr); break;
		case SpecialFolder_Share:            result = PrintInArenaStr(memArena, "%.*s/%.*s",             pathBufferStr.length, pathBufferStr.pntr, applicationName.length, applicationName.pntr); break;
		default: Assert(false); break;
	}
	
	return result;
}
