/*
File:   win32_assert.cpp
Author: Taylor Robbins
Date:   02\28\2022
Description: 
	** Holds the AssertFailure function that gets called if any Assertions in the platform layer fail
*/

// +==============================+
// |    Win32_HandleAssertion     |
// +==============================+
// void HandleAssertion(bool shouldExit, const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
PLAT_API_HANDLE_ASSERTION_DEFINITION(Win32_HandleAssertion)
{
	ThreadId_t threadId = Win32_GetThisThreadId();
	bool isMainThread = (threadId == MainThreadId);
	
	if (isMainThread)
	{
		if (GetTempArena() != nullptr)
		{
			if (expressionStr != nullptr)
			{
				if (filePath != nullptr && funcName != nullptr)
				{
					if (messageStr)
					{
						Win32_ShowMessageBox("Assertion failed", TempPrint("%s\n(%s) is not true!\nin %s:%d\nin function %s", messageStr, expressionStr, GetFileNamePartNt(filePath), lineNumber, funcName));
					}
					else
					{
						Win32_ShowMessageBox("Assertion failed", TempPrint("(%s) is not true!\nin %s:%d\nin function %s", expressionStr, GetFileNamePartNt(filePath), lineNumber, funcName));
					}
				}
				else
				{
					if (messageStr)
					{
						Win32_ShowMessageBox("Assertion failed", TempPrint("%s\n(%s) is not true!", messageStr, expressionStr));
					}
					else
					{
						Win32_ShowMessageBox("Assertion failed", TempPrint("(%s) is not true!", expressionStr));
					}
				}
			}
			else
			{
				Win32_ShowMessageBox("Assertion failed", "An assertion failed!\n(No further info is available)");
			}
		}
		else
		{
			Win32_ShowMessageBox("Assertion failed", "An assertion failed!\n(No TempArena available for formatting this message box. Please check the file and line number in the debugger)");
		}
		
		if (IsDebuggerPresent() != 0) { MyBreak(); }
		
		if (shouldExit) { exit(EXIT_CODE_ASSERTION_FAILED); }
	}
	else
	{
		PlatThread_t* thread = Win32_GetThreadById(threadId);
		if (thread != nullptr)
		{
			thread->assertionFailedLineNum = lineNumber;
			thread->assertionFailedFilepath = filePath;
			thread->assertionFailedFuncName = funcName;
			thread->assertionFailedMessage = messageStr;
			thread->assertionFailedExpression = expressionStr;
			thread->assertionFailed = true;
		}
		
		if (IsDebuggerPresent() != 0) { MyBreak(); }
		
		ThreadingWriteBarrier();
		if (shouldExit) { ExitThread(EXIT_CODE_ASSERTION_FAILED); }
	}
}

// +==============================+
// |            Update            |
// +==============================+
void Win32_CheckForThreadAssertions()
{
	for (u64 tIndex = 0; tIndex < PLAT_MAX_NUM_THREADS; tIndex++)
	{
		PlatThread_t* thread = &Platform->threads[tIndex];
		if (thread->assertionFailed)
		{
			Win32_HandleAssertion(true, thread->assertionFailedFilepath, thread->assertionFailedLineNum, thread->assertionFailedFuncName, thread->assertionFailedExpression, thread->assertionFailedMessage);
		}
	}
}

// +==============================+
// |      GyLibAssertFailure      |
// +==============================+
static bool insideAssertFailure = false;
void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	if (insideAssertFailure) { return; } //try to stop accidental recursions
	insideAssertFailure = true;
	ThreadId_t threadId = Win32_GetThisThreadId();
	bool isMainThread = (threadId == MainThreadId);
	
	if (InitPhase < Win32InitPhase_DebugOutputInitialized)
	{
		MyBreak();
		if (!DEBUG_BUILD) { Win32_FatalError(((messageStr != nullptr) ? messageStr : expressionStr), "Assertion failed during initialization"); }
		insideAssertFailure = false;
		return;
	}
	if (isMainThread)
	{
		if (messageStr != nullptr && messageStr[0] != '\0')
		{
			PrintLine_E("Assertion Failure! %s (Expression: %s) in %s %s:%d", messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
		else
		{
			PrintLine_E("Assertion Failure! (%s) is not true in %s %s:%d", expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
	}
	else
	{
		if (messageStr != nullptr && messageStr[0] != '\0')
		{
			PrintLine_E("Assertion Failure on thread %u! %s (Expression: %s) in %s %s:%d", threadId, messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
		else
		{
			PrintLine_E("Assertion Failure on thread %u! (%s) is not true in %s %s:%d", threadId, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
	}
	
	Win32_HandleAssertion(true, filePath, lineNumber, funcName, expressionStr, messageStr);
	
	#if DEBUG_BUILD
	insideAssertFailure = false;
	#endif
}
