/*
File:   win32_files.cpp
Author: Taylor Robbins
Date:   09\27\2021
Description: 
	** Holds functions that help the platform layer and engine manipulate and deal with files
	** Functions related to file watching are in win32_file_watching.cpp
*/

//TODO: Add support for opening a file but not reading all of it's contents immediately and keeping it open for a while

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
// bool Win32_DoesFileExist(MyStr_t filePath, bool* isFolderOut)
PLAT_API_DOES_FILE_EXIST_DEF(Win32_DoesFileExist)
{
	//NOTE: This function should be multi-thread safe!
	NotNullStr(&filePath);
	if (isFolderOut != nullptr) { *isFolderOut = false; }
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, true);
	DWORD fileType = GetFileAttributesA(fullPath.pntr);
	if (fileType == INVALID_FILE_ATTRIBUTES)
	{
		TempPopMark();
		return false;
	}
	else if (fileType & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (isFolderOut != nullptr) { *isFolderOut = true; }
		TempPopMark();
		return false;
	}
	else
	{
		TempPopMark();
		return true;
	}
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
		Assert(contentsOut->errorCode != ERROR_FILE_NOT_FOUND); //already checked for this
		PrintLine_E("Failed to open file that exists at \"%.*s\". Error code: %s", fullPath.length, fullPath.pntr, Win32_GetErrorCodeStr(contentsOut->errorCode, true));
		//The file might have permissions that prevent us from reading it
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
// void Win32_FreeImageData(PlatImageData_t* imageData)
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
	
	u64 executeResult = (u64)ShellExecute(
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
