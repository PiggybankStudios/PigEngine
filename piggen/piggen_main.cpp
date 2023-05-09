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
	WriteLine_I("    exclude = Exclude any files in the searched folder that match a given regular expression (multiple copies of this argument can be supplied)");
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
	
	WriteLine_I("+==============================+");
	PrintLine_I("|      PigGen v%u.%02u(%03u)       |", PIGGEN_VERSION_MAJOR, PIGGEN_VERSION_MINOR, PIGGEN_VERSION_BUILD);
	WriteLine_I("+==============================+");
	TempPushMark();
	
	// Process command line arguments
	#if USED_WIN_MAIN_ENTRY_POINT
	ProcessProgramArguments(lpCmdLine, nCmdShow);
	#else
	ProcessProgramArguments(argc, argv);
	#endif
	#if 1
	PrintLine_D("We were passed %llu argument%s", pig->programArgs.count, (pig->programArgs.count == 1) ? "" : "s");
	for (u64 aIndex = 0; aIndex < pig->programArgs.count; aIndex++)
	{
		PrintLine_D("    [%llu]: \"%.*s\"", aIndex, pig->programArgs.args[aIndex].length, pig->programArgs.args[aIndex].chars);
	}
	#endif
	
	// Determine our defaultPath and workingDirectory
	pig->defaultDirectory = NewStr(""); //TODO: Implement this!
	
	// Check the target path that was given to us
	bool targetPathGiven = GetProgramArg(mainHeap, MyStr_Empty, &pig->targetPath);
	if (!targetPathGiven || pig->targetPath.length == 0)
	{
		WriteLine_E("No target path was given. Please pass a directory path as the first unnamed argument");
		exit(EXIT_CODE_STARTUP_FAILURE);
	}
	if (StrEqualsIgnoreCase(pig->targetPath, "help"))
	{
		ShowPigGenHelp();
		exit(0);
	}
	bool targetPathExists = false;
	bool targetPathIsFile = DoesFileExist(pig->targetPath, &targetPathExists);
	if (targetPathIsFile || !targetPathExists)
	{
		PrintLine_E("The target path %s! \"%.*s\"", (targetPathIsFile ? "is a file, not a directory" : "does not exist"), pig->targetPath.length, pig->targetPath.chars);
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
		newPattern->pattern = re_compile(excludePatternStr.chars);
		PrintLine_D("Exclude pattern \"%.*s\"", excludePatternStr.length, excludePatternStr.chars);
		excludeArgIndex++;
	}
	
	// Find all files in the target directory that are .cpp or .h files
	VarArray_t allSourceCodePaths;
	CreateVarArray(&allSourceCodePaths, mainHeap, sizeof(FileToProcess_t));
	FindAllCodeFilesInFolder(&allSourceCodePaths, pig->targetPath, &pig->exclusionPatterns, true);
	VarArraySort(&allSourceCodePaths, CompareFuncFileToProcess, nullptr);
	PrintLine_I("Found %llu files to process:", allSourceCodePaths.length);
	VarArrayLoop(&allSourceCodePaths, fIndex)
	{
		VarArrayLoopGet(FileToProcess_t, file, &allSourceCodePaths, fIndex);
		PrintLine_D("    [%llu]: \"%.*s\"", fIndex, file->path.length, file->path.chars);
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
