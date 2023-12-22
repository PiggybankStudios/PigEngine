/*
File:   pig_file.cpp
Author: Taylor Robbins
Date:   10\25\2023
Description: 
	** Holds functions that help us open and read files from the Playdate file system
*/

//NOTE: The "data" directory is read/writable, while the default directory is only readable and ships with the game

bool DoesFileExist(bool fromDataDir, MyStr_t path)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	
	FileStat fileStats = {};
	int statResult = pd->file->stat(pathNullTerm.chars, &fileStats);
	
	FreeScratchArena(scratch);
	return (statResult == 0 && fileStats.isdir == 0);
}

bool ReadEntireFile(bool fromDataDir, MyStr_t path, MyStr_t* contentsOut, MemArena_t* memArena)
{
	NotNullStr(&path);
	NotNull2(contentsOut, memArena);
	
	MemArena_t* scratch = GetScratchArena(memArena);
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	
	SDFile* fileHandle = pd->file->open(pathNullTerm.chars, (fromDataDir ? kFileReadData : kFileRead));
	if (fileHandle == nullptr)
	{
		PrintLine_E("Failed to open file at \"%s\": %s", pathNullTerm.chars, pd->file->geterr());
		FreeScratchArena(scratch);
		return false;
	}
	
	// Find the size of the file
	int seekEndSuccess = pd->file->seek(fileHandle, 0, SEEK_END);
	Assert(seekEndSuccess == 0);
	int tellResult = pd->file->tell(fileHandle);
	Assert(tellResult >= 0);
	u64 fileSize = (u64)tellResult;
	int seekStartSuccess = pd->file->seek(fileHandle, 0, SEEK_SET);
	Assert(seekStartSuccess == 0);
	
	if (fileSize == 0)
	{
		*contentsOut = MyStr_Empty;
		FreeScratchArena(scratch);
		return true;
	}
	
	contentsOut->length = fileSize;
	contentsOut->chars = AllocArray(memArena, char, fileSize+1);
	NotNull(contentsOut->chars);
	
	int readResult = pd->file->read(fileHandle, contentsOut->chars, (unsigned int)contentsOut->length);
	if (readResult < 0 || (u64)readResult < fileSize)
	{
		PrintLine_E("Failed to read %llu byte file contents at \"%s\": %s", fileSize, pathNullTerm.chars, pd->file->geterr());
		FreeScratchArena(scratch);
		return false;
	}
	
	contentsOut->chars[contentsOut->length] = '\0';
	pd->file->close(fileHandle);
	FreeScratchArena(scratch);
	
	return true;
}

bool WriteEntireFile(MyStr_t path, MyStr_t contents)
{
	NotNullStr(&path);
	MemArena_t* scratch = GetScratchArena();
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	
	SDFile* fileHandle = pd->file->open(pathNullTerm.chars, kFileWrite);
	if (fileHandle == nullptr)
	{
		PrintLine_E("Failed to open file at \"%s\": %s", pathNullTerm.chars, pd->file->geterr());
		FreeScratchArena(scratch);
		return false;
	}
	
	int writeResult = pd->file->write(fileHandle, contents.pntr, (unsigned int)contents.length);
	if (writeResult < 0)
	{
		PrintLine_E("Failed to write %llu bytes to file at \"%s\": %s", contents.length, pathNullTerm.chars, pd->file->geterr());
		pd->file->close(fileHandle);
		FreeScratchArena(scratch);
		return false;
	}
	
	pd->file->close(fileHandle);
	FreeScratchArena(scratch);
	return true;
}

struct FilesInDir_t
{
	bool success;
	u64 count;
	MyStr_t basePath; //allocation not tracked! Copied straight from GetFilesInDirectory path parameter
	MyStr_t* paths;
	MemArena_t* allocArena;
	MyStr_t mainAllocation;
};
struct FilesInDirContext_t
{
	FilesInDir_t* result;
	u64 fileIndex;
	u64 charIndex;
	bool includeFolders;
	bool fileNameOnly;
};

void ListFilesCallback(const char* pathNullTerm, void* userdata)
{
	NotNull2(pathNullTerm, userdata);
	FilesInDirContext_t* context = (FilesInDirContext_t*)userdata;
	bool basePathHasSlash = (context->result->basePath.length > 0 && context->result->basePath.chars[context->result->basePath.length-1] == '/');
	u64 pathLength = MyStrLength64(pathNullTerm);
	if (context->includeFolders || !(pathLength > 0 && pathNullTerm[pathLength-1] == '/'))
	{
		if (context->result->mainAllocation.pntr == nullptr)
		{
			context->result->count++;
			if (!context->fileNameOnly) { context->result->mainAllocation.length += context->result->basePath.length + (basePathHasSlash ? 0 : 1); }
			context->result->mainAllocation.length += pathLength + 1;
		}
		else
		{
			Assert(context->fileIndex < context->result->count);
			Assert(context->charIndex <= context->result->mainAllocation.length);
			NotNull(context->result->paths);
			
			MyStr_t* outPath = &context->result->paths[context->fileIndex];
			context->fileIndex++;
			char* writePntr = context->result->mainAllocation.chars + context->charIndex;
			outPath->chars = writePntr;
			outPath->length = 0;
			
			if (!context->fileNameOnly)
			{
				Assert(context->charIndex + context->result->basePath.length <= context->result->mainAllocation.length);
				MyMemCopy(writePntr, context->result->basePath.chars, context->result->basePath.length);
				context->charIndex += context->result->basePath.length;
				writePntr += context->result->basePath.length;
				outPath->length += context->result->basePath.length;
				
				if (!basePathHasSlash)
				{
					Assert(context->charIndex + 1 <= context->result->mainAllocation.length);
					*writePntr = '/';
					context->charIndex++;
					writePntr++;
					outPath->length++;
				}
			}
			
			Assert(context->charIndex + pathLength <= context->result->mainAllocation.length);
			MyMemCopy(writePntr, pathNullTerm, pathLength);
			context->charIndex += pathLength;
			writePntr += pathLength;
			outPath->length += pathLength;
			
			Assert(context->charIndex + 1 <= context->result->mainAllocation.length);
			*writePntr = '\0';
			context->charIndex++;
			writePntr++;
		}
	}
}

void FreeFilesInDir(FilesInDir_t* files)
{
	NotNull(files);
	if (files->allocArena != nullptr && files->mainAllocation.pntr != nullptr)
	{
		FreeMem(files->allocArena, files->mainAllocation.pntr, files->mainAllocation.length);
	}
	ClearPointer(files);
}

//Pass nullptr for memArena to count files only
FilesInDir_t GetFilesInDirectory(MyStr_t path, MemArena_t* memArena, bool includeFolders = false, bool fileNameOnly = false, bool showHidden = false)
{
	NotNullStr(&path);
	FilesInDir_t result = {};
	result.basePath = path;
	result.allocArena = memArena;
	MemArena_t* scratch = GetScratchArena(memArena);
	MyStr_t pathNullTerm = AllocString(scratch, &path);
	
	FilesInDirContext_t context = {};
	context.result = &result;
	context.fileNameOnly = fileNameOnly;
	context.includeFolders = includeFolders;
	
	//First count the files and find out how much space is needed for their path strings
	int listResult1 = pd->file->listfiles(pathNullTerm.chars, ListFilesCallback, &context, showHidden);
	if (listResult1 != 0)
	{
		//TODO: Maybe call pd->file->geterr()?
		result.success = false;
		FreeScratchArena(scratch);
		return result;
	}
	
	if (result.count == 0 || memArena == nullptr)
	{
		result.success = true;
		FreeScratchArena(scratch);
		return result;
	}
	
	result.mainAllocation.length += sizeof(MyStr_t) * result.count;
	result.mainAllocation.pntr = (char*)AllocMem(memArena, result.mainAllocation.length);
	if (result.mainAllocation.pntr == nullptr)
	{
		result.success = false;
		FreeScratchArena(scratch);
		return result;
	}
	result.paths = (MyStr_t*)result.mainAllocation.pntr;
	
	context.fileIndex = 0;
	context.charIndex = (sizeof(MyStr_t) * result.count);
	int listResult2 = pd->file->listfiles(pathNullTerm.chars, ListFilesCallback, &context, showHidden);
	Assert(listResult2 == 0); //if the first call succeeded, then the second one should as well!
	Assert(context.fileIndex == result.count);
	Assert(context.charIndex == result.mainAllocation.length);
	
	FreeScratchArena(scratch);
	result.success = true;
	return result;
}
