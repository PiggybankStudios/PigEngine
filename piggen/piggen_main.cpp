/*
File:   piggen_main.cpp
Author: Taylor Robbins
Date:   05\09\2023
Description: 
	** Holds the main entry point for piggen.exe
	** (this is also the main compilable file that all other files are #included into)
*/

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#if !defined(ASSERTIONS_ENABLED)
#error You must define ASSERTIONS_ENABLED in the build options
#endif
#define GYLIB_ASSERTIONS_ENABLED ASSERTIONS_ENABLED
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#include "gylib/gy_defines_check.h"
#include "gylib/gy_basic_macros.h"

#if WINDOWS_COMPILATION
	#include <windows.h>
#elif OSX_COMPILATION
	//TODO: Implement me!
#elif LINUX_COMPILATION
	//TODO: Implement me!
#elif WASM_COMPILATION
	//TODO: Implement me!
#endif

#define GYLIB_LOOKUP_PRIMES_100
#include "gylib/gy.h"

#include "regex/re.h"
#include "regex/re.c"

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "piggen_version.h"

#include "piggen_parse.h"

#include "piggen_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
#include "gylib/gy_temp_memory.cpp"
PigGenState_t* pig = nullptr;
MemArena_t* mainHeap = nullptr;

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "piggen_debug.cpp"
#include "piggen_files.cpp"
#include "piggen_generate.cpp"
#include "piggen_parse.cpp"
#include "piggen_helpers.cpp"

// +--------------------------------------------------------------+
// |                             Help                             |
// +--------------------------------------------------------------+
void ShowPigGenHelp()
{
	WriteLine_I(
		"PigGen.exe is a tool that takes all the source files within a folder (recursive), "
		"opens them and searches for a a #if PIGGEN region, "
		"processes the contents and generates C code into an output folder, "
		"and adds a #else clause with a #include to the generated file if it doesn't already exist."
		"\n"
	);
	WriteLine_I("The main argument should be a path to a directory that you would like PigGen to recursively search to find source files");
	WriteLine_I("All other arguments are of the form -name=\"value\" (quotes not required) and are optional:");
	WriteLine_I("    help = Displays this help message and then exits (other arguments are ignored)");
	WriteLine_I("    version = Prints the version number of PigGen and the exits (other arguments are ignored)");
	WriteLine_I("    exclude = Exclude any files in the searched folder that match a given regular expression (multiple copies of this argument can be supplied)");
	WriteLine_I("    verbose = Enables a bunch of debug output lines that can help debug PigGen behavior");
	WriteLine_I("");
}
void ShowHelpHint()
{
	WriteLine_W("Pass -help for more information on usage");
}

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
#if 1
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#define USED_WIN_MAIN_ENTRY_POINT 1
#else
int main(int argc, char* argv[])
#define USED_WIN_MAIN_ENTRY_POINT 0
#endif
{
	PigGenState_t pigGenState;
	ClearStruct(pigGenState);
	pig = &pigGenState;
	
	LARGE_INTEGER perfCountFrequencyLarge;
	QueryPerformanceFrequency(&perfCountFrequencyLarge);
	pig->perfCountFrequency = (i64)perfCountFrequencyLarge.QuadPart;
	PerfTime_t pigGenStartTime = GetPerfTime();
	
	// Initialize memory arenas
	InitMemArena_StdHeap(&pig->stdHeap);
	InitMemArena_PagedHeapArena(&pig->mainHeap, Megabytes(1), &pig->stdHeap);
	mainHeap = &pig->mainHeap;
	u8* tempArenaMemory = AllocArray(&pig->stdHeap, u8, PIGGEN_TEMP_ARENA_SIZE);
	NotNull_(tempArenaMemory);
	InitMemArena_MarkedStack(&pig->tempArena, PIGGEN_TEMP_ARENA_SIZE, tempArenaMemory, PIGGEN_TEMP_ARENA_MAX_MARKS);
	TempArena = &pig->tempArena;
	
	pig->nextFileContentsId = 1;
	pig->nextOpenFileId = 1;
	
	TempPushMark();
	
	//Calculate the time
	bool doesTimezoneDoDst = false;
	pig->timestamp = GetCurrentTimestamp(true, nullptr, &doesTimezoneDoDst);
	ConvertTimestampToRealTime(pig->timestamp, &pig->realTime, doesTimezoneDoDst);
	
	// Process command line arguments
	#if USED_WIN_MAIN_ENTRY_POINT
	ProcessProgramArguments(lpCmdLine, nCmdShow);
	#else
	ProcessProgramArguments(argc, argv);
	#endif
	
	if (GetProgramArg(nullptr, NewStr("help"), nullptr) || GetProgramArg(nullptr, NewStr("h"), nullptr))
	{
		ShowPigGenHelp();
		exit(0);
	}
	if (GetProgramArg(nullptr, NewStr("version"), nullptr))
	{
		PrintLine_I("PigGen version: %u.%02u(%03u)", PIGGEN_VERSION_MAJOR, PIGGEN_VERSION_MINOR, PIGGEN_VERSION_BUILD);
		exit(0);
	}
	if (GetProgramArg(nullptr, NewStr("test"), nullptr))
	{
		if (false)
		{
			u64 testArenaSize = Kilobytes(2);
			void* testArenaPntr = malloc(testArenaSize);
			MemArena_t testArena = {};
			InitMemArena_FixedHeap(&testArena, testArenaSize, testArenaPntr);
			char* alloc1 = AllocArray(&testArena, char, 128);
			char* alloc2 = AllocArray(&testArena, char, 500);
			char* alloc3 = AllocArray(&testArena, char, 128);
			FreeMem(&testArena, alloc2);
			char* alloc4 = AllocArray(&testArena, char, 128);
			PrintLine_D("%p %p %p %p", alloc1, alloc2, alloc3, alloc4);
			GrowMemToken_t growthToken = {};
			u64 growthSpace = GrowMemQuery(&testArena, alloc4, 128, &growthToken);
			Assert(growthSpace >= 128);
			PrintLine_D("We can grow alloc4 by %llu", growthSpace);
			MyMemSet(alloc4, 0x00, 256);
			WriteLine_D("Cleared 256 bytes at alloc4");
			GrowMem(&testArena, alloc4, 128, 256, &growthToken);
			MemArenaVerify(&testArena, true);
			WriteLine_I("Successfully grew 128 byte section to 256 bytes!");
			growthSpace = GrowMemQuery(&testArena, alloc4, 256, &growthToken);
			GrowMem(&testArena, alloc4, 256, 500, &growthToken);
			MemArenaVerify(&testArena, true);
			WriteLine_I("Successfully grew 256 byte section to 500 bytes!");
		}
		
		if (false)
		{
			u64 stackArenaSize = Kilobytes(2);
			void* stackArenaPntr = malloc(stackArenaSize);
			MemArena_t stackArena = {};
			InitMemArena_MarkedStack(&stackArena, stackArenaSize, stackArenaPntr, 4);
			void* alloc1 = AllocArray(&stackArena, char, 128);
			void* alloc2 = AllocArray(&stackArena, char, 500);
			void* alloc3 = AllocArray(&stackArena, char, 128);
			GrowMemToken_t growthToken = {};
			u64 growthSpace = GrowMemQuery(&stackArena, alloc3, 128, &growthToken);
			PrintLine_D("We can grow alloc3 by %llu", growthSpace);
			GrowMem(&stackArena, alloc3, 128, 256, &growthToken);
			MemArenaVerify(&stackArena, true);
			WriteLine_I("Successfully grew 128 byte section to 256 bytes!");
			GrowMem(&stackArena, alloc3, 256, 500, &growthToken);
			MemArenaVerify(&stackArena, true);
			WriteLine_I("Successfully grew 256 byte section to 500 bytes!");
		}
		
		if (true)
		{
			u64 stackArenaSize = Kilobytes(2);
			void* stackArenaPntr = malloc(stackArenaSize);
			MemArena_t stackArena = {};
			InitMemArena_MarkedStack(&stackArena, stackArenaSize, stackArenaPntr, 4);
			
			u64 testArenaSize = Kilobytes(2);
			void* testArenaPntr = malloc(testArenaSize);
			MemArena_t testArena = {};
			InitMemArena_FixedHeap(&testArena, testArenaSize, testArenaPntr);
			
			StringBuilder_t builder;
			// NewStringBuilder(&builder, &stackArena);
			NewStringBuilder(&builder, &testArena);
			
			PrintLine_D("arena: %llu/%llu used", builder.allocArena->used, builder.allocArena->size);
			// StringBuilderAppend(&builder, NewStr("Build me a string!"));
			// MemArenaVerify(builder.allocArena, true);
			// StringBuilderAppend(&builder, NewStr("Build me a string!"));
			// MemArenaVerify(builder.allocArena, true);
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppend(&builder, NewStr("Build me a string!"));
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			char* charArray = AllocArray(builder.allocArena, char, 4);
			charArray[0] = 'A';
			charArray[1] = 'B';
			charArray[2] = 'C';
			charArray[3] = 'D';
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppend(&builder, NewStr("Build me a string!"));
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			StringBuilderAppendPrint(&builder, "Print me a %s! %llu", "string", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			
			StringBuilderPrint(&builder, "This is the final print %llu", builder.allocArena->used);
			MemArenaVerify(builder.allocArena, true);
			
			PrintLine_D("arena: %llu/%llu used", builder.allocArena->used, builder.allocArena->size);
			PrintLine_D("String: \"%s\"", ToStr(&builder));
			// for (u64 bIndex = 0; bIndex < builder.allocArena->used; bIndex++)
			// {
			// 	char c = ((char*)builder.allocArena->mainPntr)[bIndex];
			// 	PrintLine_D("[%llu]: (0x%02X) %c", bIndex, c, c);
			// }
		}
		
		exit(0);
	}
	
	pig->verboseEnabled = GetProgramArg(nullptr, NewStr("verbose"), nullptr);
	if (pig->verboseEnabled)
	{
		WriteLine_I("+==============================+");
		PrintLine_I("|      PigGen v%u.%02u(%03u)       |", PIGGEN_VERSION_MAJOR, PIGGEN_VERSION_MINOR, PIGGEN_VERSION_BUILD);
		WriteLine_I("+==============================+");
		PrintLine_D("Verbose output enabled!\n");
		
		#if 0
		PrintLine_D("We were passed %llu argument%s", pig->programArgs.count, (pig->programArgs.count == 1) ? "" : "s");
		for (u64 aIndex = 0; aIndex < pig->programArgs.count; aIndex++)
		{
			PrintLine_D("    [%llu]: \"%.*s\"", aIndex, pig->programArgs.args[aIndex].length, pig->programArgs.args[aIndex].chars);
		}
		#endif
	}
	
	// Determine our defaultPath and workingDirectory
	pig->defaultDirectory = NewStr(""); //TODO: Implement this!
	
	//Determine our outputPath
	pig->outputDirectory = MyStr_Empty;
	if (GetProgramArg(mainHeap, NewStr("output"), &pig->outputDirectory))
	{
		if (!IsRelativePath(pig->outputDirectory) && !IsEmptyStr(pig->defaultDirectory))
		{
			//TODO: It's about time we make a path join function...
			MyStr_t fullPath = PrintInArenaStr(mainHeap, "%.*s%s%.*s%s",
				pig->defaultDirectory.length, pig->defaultDirectory.chars,
				(StrEndsWithSlash(pig->defaultDirectory) ? "" : "/"),
				pig->outputDirectory.length, pig->outputDirectory.chars
			);
			FreeString(mainHeap, &pig->outputDirectory);
			pig->outputDirectory = fullPath;
		}
		else if (!IsEmptyStr(pig->outputDirectory))
		{
			MyStr_t pathWithEndingSlash = PrintInArenaStr(mainHeap, "%.*s", pig->outputDirectory.length, pig->outputDirectory.chars);
			FreeString(mainHeap, &pig->outputDirectory);
			pig->outputDirectory = pathWithEndingSlash;
		}
	}
	else if (!IsEmptyStr(pig->defaultDirectory))
	{
		pig->outputDirectory = AllocString(mainHeap, &pig->defaultDirectory);
	}
	if (!IsEmptyStr(pig->outputDirectory) && !StrEndsWithSlash(pig->outputDirectory)) { StrReallocAppend(&pig->outputDirectory, "/", mainHeap); }
	if (pig->verboseEnabled) { PrintLine_I("Outputting to \"%.*s\"", pig->outputDirectory.length, pig->outputDirectory.chars); }
	
	// Check the target path that was given to us
	bool targetPathGiven = GetProgramArg(mainHeap, MyStr_Empty, &pig->targetPath);
	if (!targetPathGiven || pig->targetPath.length == 0)
	{
		WriteLine_E("No target path was given. Please pass a directory path as the first unnamed argument");
		ShowHelpHint();
		exit(EXIT_CODE_STARTUP_FAILURE);
	}
	bool targetPathExists = false;
	bool targetPathIsFile = DoesFileExist(pig->targetPath, &targetPathExists);
	if (targetPathIsFile || !targetPathExists)
	{
		PrintLine_E("The target path %s! \"%.*s\"", (targetPathIsFile ? "is a file, not a directory" : "does not exist"), pig->targetPath.length, pig->targetPath.chars);
		ShowHelpHint();
		exit(EXIT_CODE_STARTUP_FAILURE);
	}
	
	CreateVarArray(&pig->exclusionPatterns, mainHeap, sizeof(ExclusionPattern_t));
	u64 excludeArgIndex = 0; 
	MyStr_t excludePatternStr;
	while (GetProgramArg(mainHeap, NewStr("exclude"), &excludePatternStr, excludeArgIndex))
	{
		ExclusionPattern_t* newPattern = VarArrayAdd(&pig->exclusionPatterns, ExclusionPattern_t);
		NotNull(newPattern);
		ClearPointer(newPattern);
		newPattern->patternStr = excludePatternStr;
		AssertNullTerm(&excludePatternStr);
		if (pig->verboseEnabled) { PrintLine_D("Exclude pattern \"%.*s\"", excludePatternStr.length, excludePatternStr.chars); }
		excludeArgIndex++;
	}
	
	// Find all files in the target directory that are .cpp or .h files
	VarArray_t allSourceCodeFiles;
	CreateVarArray(&allSourceCodeFiles, mainHeap, sizeof(FileToProcess_t));
	FindAllCodeFilesInFolder(&allSourceCodeFiles, pig->targetPath, &pig->exclusionPatterns, true);
	VarArraySort(&allSourceCodeFiles, CompareFuncFileToProcess, nullptr);
	if (pig->verboseEnabled)
	{
		PrintLine_I("Found %llu files to process:", allSourceCodeFiles.length);
		#if 0
		VarArrayLoop(&allSourceCodeFiles, fIndex)
		{
			VarArrayLoopGet(FileToProcess_t, file, &allSourceCodeFiles, fIndex);
			PrintLine_D("    [%llu]: \"%.*s\"", fIndex, file->path.length, file->path.chars);
		}
		#endif
	}
	
	VarArray_t genFilesWithPlaceholders;
	CreateVarArray(&genFilesWithPlaceholders, mainHeap, sizeof(FileToProcess_t));
	VarArray_t allSerializableStructs;
	CreateVarArray(&allSerializableStructs, mainHeap, sizeof(SerializableStruct_t));
	
	// Loop over and process each file
	u64 numPigGenRegions = 0;
	VarArrayLoop(&allSourceCodeFiles, fIndex)
	{
		VarArrayLoopGet(FileToProcess_t, fileToProcess, &allSourceCodeFiles, fIndex);
		FileContents_t fileContents;
		if (ReadFileContents(fileToProcess->path, &fileContents))
		{
			if (pig->verboseEnabled) { PrintLine_D("Processing %llu/%llu \"%.*s\" (%llu bytes)", fIndex+1, allSourceCodeFiles.length, fileToProcess->path.length, fileToProcess->path.chars, fileContents.size); }
			
			u64 outputFileCount = 0;
			
			MyStr_t newFileContents = NewStringInArena(mainHeap, fileContents.length, fileContents.chars);
			
			bool insidePigGenRegion = false;
			bool foundElseLine = false;
			u64 elseLineStartIndex = 0;
			u64 elseLineEndIndex = 0;
			u64 pigGenRegionStartIndex = 0;
			u64 pigGenRegionStartLineIndex = 0;
			
			MyStr_t originalFileContentsStr = NewStr(fileContents.size, fileContents.chars);
			LineParser_t lineParser = NewLineParser(originalFileContentsStr);
			MyStr_t line;
			while (LineParserGetLine(&lineParser, &line))
			{
				u64 startOfLineIndex = lineParser.lineBeginByteIndex;
				u64 endOfLineIndex = lineParser.byteIndex;
				TrimWhitespace(&line);
				if (!insidePigGenRegion)
				{
					if (StrStartsWith(line, "#if PIGGEN"))
					{
						if (pig->verboseEnabled) { PrintLine_D("Found #if PIGGEN on line %llu", lineParser.lineIndex); }
						insidePigGenRegion = true;
						foundElseLine = false;
						pigGenRegionStartIndex = endOfLineIndex;
						pigGenRegionStartLineIndex = lineParser.lineIndex;
					}
				}
				else
				{
					if (StrStartsWith(line, "#else"))
					{
						Assert(!foundElseLine);
						foundElseLine = true;
						elseLineStartIndex = startOfLineIndex;
						elseLineEndIndex = endOfLineIndex;
					}
					else if (StrStartsWith(line, "#endif"))
					{
						MyStr_t pigGenInput = StrSubstring(&originalFileContentsStr, pigGenRegionStartIndex, startOfLineIndex);
						if (foundElseLine)
						{
							Assert(elseLineStartIndex >= pigGenRegionStartIndex);
							Assert(elseLineStartIndex < startOfLineIndex);
							pigGenInput.length = elseLineStartIndex - pigGenRegionStartIndex;
						}
						if (pig->verboseEnabled) { PrintLine_D("Found #endif on line %llu (%llu bytes)", lineParser.lineIndex, pigGenInput.length); }
						insidePigGenRegion = false;
						
						// if (pig->verboseEnabled) { PrintLine_D("Parsing:\n%.*s", pigGenInput.length, pigGenInput.chars); }
						
						TempPushMark();
						
						MyStr_t outputFileName = GetOutputFileName(fileToProcess->path, outputFileCount, TempArena);
						MyStr_t outputFilePath = GetOutputFilePath(outputFileName, TempArena);
						outputFileCount++;
						
						OpenFile_t outputFile = {};
						bool openSuccess = OpenFile(outputFilePath, &outputFile);
						if (!openSuccess)
						{
							PrintLine_E("Failed to create file to hold generated code at \"%.*s\"", outputFilePath.length, outputFilePath.chars);
						}
						else
						{
							if (pig->verboseEnabled) { PrintLine_I("Generating code file: \"%.*s\"", outputFilePath.length, outputFilePath.chars); }
							
							ProcessLog_t parseLog;
							CreateProcessLog(&parseLog, Kilobytes(32), mainHeap, mainHeap, TempArena);
							SetProcessLogFilePath(&parseLog, fileToProcess->path);
							
							bool hadPlaceholders = false;
							bool parseSuccess = TryPigGenerate(pigGenInput, &outputFile, &allSerializableStructs, &hadPlaceholders, &parseLog, pigGenRegionStartLineIndex+1);
							if (pig->verboseEnabled) { PrintLineAt(parseSuccess ? DbgLevel_Info : DbgLevel_Error, "Region parse%s!", parseSuccess ? "d Successfully" : " Failure"); }
							if (parseLog.hadErrors || parseLog.hadWarnings) { DumpProcessLog(&parseLog, "PigGen Parse Log", DbgLevel_Warning); }
							FreeProcessLog(&parseLog);
							
							MyStr_t includeCode = TempPrintStr("%s#include \"%.*s\"" PIGGEN_NEW_LINE, (foundElseLine ? "" : PIGGEN_NEW_LINE "#else"), outputFileName.length, outputFileName.chars);
							
							u64 replaceRegionStart = (foundElseLine ? elseLineEndIndex : startOfLineIndex);
							u64 replaceRegionSize = (foundElseLine ? (startOfLineIndex - elseLineEndIndex) : 0);
							MyStr_t splicedFileContents = StrSplice(newFileContents, replaceRegionStart, replaceRegionStart + replaceRegionSize, includeCode, mainHeap);
							FreeString(mainHeap, &newFileContents);
							newFileContents = splicedFileContents;
							
							if (hadPlaceholders)
							{
								if (pig->verboseEnabled) { WriteLine_D("This file will need to be revisited to fill in placeholders..."); }
								FileToProcess_t* placeholderFileToProcess = VarArrayAdd(&genFilesWithPlaceholders, FileToProcess_t);
								ClearPointer(placeholderFileToProcess);
								placeholderFileToProcess->path = AllocString(mainHeap, &outputFilePath);
							}
							
							CloseFile(&outputFile);
						}
						
						TempPopMark();
						
						numPigGenRegions++;
					}
				}
			}
			
			if (!StrEquals(originalFileContentsStr, newFileContents))
			{
				if (pig->verboseEnabled) { PrintLine_D("Updating \"%.*s\" because we spliced in %llu #include%s", fileToProcess->path.length, fileToProcess->path.chars, outputFileCount, (outputFileCount == 1) ? "" : "s"); }
				bool writeSuccess = WriteEntireFile(fileToProcess->path, newFileContents.chars, newFileContents.length);
				Assert(writeSuccess);
			}
			else if (outputFileCount > 0)
			{
				if (pig->verboseEnabled) { PrintLine_D("No need to update \"%.*s\" because for %llu region%s", fileToProcess->path.length, fileToProcess->path.chars, outputFileCount, (outputFileCount == 1) ? "" : "s"); }
			}
			
			FreeString(mainHeap, &newFileContents);
			FreeFileContents(&fileContents);
		}
	}
	
	//TODO: Re-enable me once we fix the problems!
	#if 0
	if (genFilesWithPlaceholders.length > 0)
	{
		MyStr_t allSerializableStructsCode = PigGenGenerateAllSerializableStructsCode(mainHeap, &allSerializableStructs);
		
		VarArrayLoop(&genFilesWithPlaceholders, fIndex)
		{
			VarArrayLoopGet(FileToProcess_t, genFile, &genFilesWithPlaceholders, fIndex);
			FileContents_t fileContents;
			if (ReadFileContents(genFile->path, &fileContents))
			{
				MyStr_t fileContentsStr = NewStr(fileContents.length, fileContents.chars);
				MyStr_t newFileContents = StrReplace(fileContentsStr, NewStr(ALL_SERIALIZABLE_STRUCTS_PLACEHOLDER_STRING), allSerializableStructsCode, mainHeap);
				bool writeSuccess = WriteEntireFile(genFile->path, newFileContents.chars, newFileContents.length);
				Assert(writeSuccess);
				FreeString(mainHeap, &newFileContents);
				FreeFileContents(&fileContents);
			}
		}
		
		FreeString(mainHeap, &allSerializableStructsCode);
	}
	#endif
	
	// Free
	#if 0 //This isn't super necassary since we're shutting down. Maybe we want to do this work for some reason though? Might help us catch memory leaks?
	VarArrayLoop(&allSerializableStructs, sIndex)
	{
		VarArrayLoopGet(SerializableStruct_t, serializableStruct, &allSerializableStructs, sIndex);
		FreeSerializableStruct(serializableStruct);
	}
	FreeVarArray(&allSerializableStructs);
	#endif
	
	// Shutdown
	
	TempPopMark();
	AssertMsg(GetNumMarks(&pig->tempArena) == 0, "TempArena had missing Pop somewhere during PigGen execution!");
	PerfTime_t pigGenEndTime = GetPerfTime();
	r64 pigGenRunTime = GetPerfTimeDiff(&pigGenStartTime, &pigGenEndTime);
	if (pig->verboseEnabled) { WriteLine_I(""); }
	PrintLine_I("PigGen processed %llu file%s (%llu region%s) in %s",
		allSourceCodeFiles.length,
		((allSourceCodeFiles.length == 1) ? "" : "s"),
		numPigGenRegions,
		((numPigGenRegions == 1) ? "" : "s"),
		FormatMillisecondsNt((u64)pigGenRunTime, TempArena)
	);
	if (pig->verboseEnabled) { WriteLine_I(""); }
	return 0;
}

// +--------------------------------------------------------------+
// |                   Assert Failure Function                    |
// +--------------------------------------------------------------+
static bool insideAssertFailure = false;
void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	if (insideAssertFailure) { return; } //try to stop accidental recursions
	insideAssertFailure = true;
	
	if (messageStr != nullptr && messageStr[0] != '\0')
	{
		PrintLine_E("Assertion Failure! %s (Expression: %s) in %s %s:%d", messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
	}
	else
	{
		PrintLine_E("Assertion Failure! (%s) is not true in %s %s:%d", expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
	}
	
	if (IsDebuggerPresent() != 0) { MyBreak(); }
	exit(EXIT_CODE_ASSERTION_FAILED);
	insideAssertFailure = false;
}
