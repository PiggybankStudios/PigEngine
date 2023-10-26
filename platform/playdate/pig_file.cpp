/*
File:   pig_file.cpp
Author: Taylor Robbins
Date:   10\25\2023
Description: 
	** Holds functions that help us open and read files from the Playdate file system
*/

//NOTE: The "data" directory is read/writable, while the default directory is only readable and ships with the game

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
