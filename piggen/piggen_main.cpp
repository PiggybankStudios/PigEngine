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
	
	// Initialize memory arenas
	InitMemArena_StdHeap(&pig->stdHeap);
	InitMemArena_PagedHeapArena(&pig->mainHeap, Megabytes(1), &pig->stdHeap);
	mainHeap = &pig->mainHeap;
	u8* tempArenaMemory = AllocArray(&pig->stdHeap, u8, PIGGEN_TEMP_ARENA_SIZE);
	NotNull_(tempArenaMemory);
	InitMemArena_MarkedStack(&pig->tempArena, PIGGEN_TEMP_ARENA_SIZE, tempArenaMemory, PIGGEN_TEMP_ARENA_MAX_MARKS);
	TempArena = &pig->tempArena;
	
	pig->nextFileContentsId = 1;
	
	TempPushMark();
	
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
		
		if (true)
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
	VarArray_t allSourceCodePaths;
	CreateVarArray(&allSourceCodePaths, mainHeap, sizeof(FileToProcess_t));
	FindAllCodeFilesInFolder(&allSourceCodePaths, pig->targetPath, &pig->exclusionPatterns, true);
	VarArraySort(&allSourceCodePaths, CompareFuncFileToProcess, nullptr);
	if (pig->verboseEnabled)
	{
		PrintLine_I("Found %llu files to process:", allSourceCodePaths.length);
		#if 0
		VarArrayLoop(&allSourceCodePaths, fIndex)
		{
			VarArrayLoopGet(FileToProcess_t, file, &allSourceCodePaths, fIndex);
			PrintLine_D("    [%llu]: \"%.*s\"", fIndex, file->path.length, file->path.chars);
		}
		#endif
	}
	
	// Loop over and process each file
	VarArrayLoop(&allSourceCodePaths, fIndex)
	{
		VarArrayLoopGet(FileToProcess_t, fileToProcess, &allSourceCodePaths, fIndex);
		FileContents_t fileContents;
		if (ReadFileContents(fileToProcess->path, &fileContents))
		{
			if (pig->verboseEnabled) { PrintLine_D("Processing %llu/%llu \"%.*s\" (%llu bytes)", fIndex+1, allSourceCodePaths.length, fileToProcess->path.length, fileToProcess->path.chars, fileContents.size); }
			
			bool insidePigGenRegion = false;
			u64 pigGenRegionStartIndex = 0;
			u64 pigGenRegionStartLineIndex = 0;
			
			MyStr_t fileString = NewStr(fileContents.size, fileContents.chars);
			LineParser_t lineParser = NewLineParser(fileString);
			u64 lineIndex = 0;
			MyStr_t line;
			while (LineParserGetLine(&lineParser, &line))
			{
				TrimWhitespace(&line);
				if (!insidePigGenRegion)
				{
					if (StrStartsWith(line, "#if PIGGEN"))
					{
						if (pig->verboseEnabled) { PrintLine_D("Found #if PIGGEN on line %llu", lineIndex+1); }
						insidePigGenRegion = true;
						pigGenRegionStartIndex = lineParser.byteIndex;
						pigGenRegionStartLineIndex = lineIndex+1;
					}
				}
				else
				{
					if (StrStartsWith(line, "#endif"))
					{
						MyStr_t regionContents = StrSubstring(&fileString, pigGenRegionStartIndex, lineParser.byteIndex);
						if (pig->verboseEnabled) { PrintLine_D("Found #endif on line %llu (%llu bytes)", lineIndex+1, regionContents.length); }
						insidePigGenRegion = false;
						
						ProcessLog_t parseLog;
						CreateProcessLog(&parseLog, Kilobytes(32), mainHeap, mainHeap, TempArena);
						bool parseSuccess = TryPigGenerate(regionContents, &parseLog, pigGenRegionStartLineIndex+1);
						PrintLineAt(parseSuccess ? DbgLevel_Info : DbgLevel_Error, "Region parse%s!", parseSuccess ? "d Successfully" : " Failure");
						DumpProcessLog(&parseLog, "PigGen Parse Log", DbgLevel_Debug);
						FreeProcessLog(&parseLog);
					}
				}
				lineIndex++;
			}
			
			FreeFileContents(&fileContents);
		}
	}
	
	// Shutdown
	TempPopMark();
	AssertMsg(GetNumMarks(&pig->tempArena) == 0, "TempArena had missing Pop somewhere during PigGen execution!");
	WriteLine_I("\nPigGen succeeded!\n");
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
