/*
File:   pig_debug_console.cpp
Author: Taylor Robbins
Date:   01\01\2022
Description: 
	** Holds a bunch of code that helps us manage and render the debug console that stores debug
	** output from the app and platform layer in a StringFifo_t and displays the contents with
	** coloring based off DbgLevel_t of each line and the name of the file it came from.
*/

//TODO: Add support for hovering over a line to see the timestamp and/or programTime comparison
//TODO: Add pause and clear buttons

#define DBG_CONSOLE_BUFFER_SIZE       Kilobytes(128)
#define DBG_CONSOLE_BUILD_SPACE_SIZE  Kilobytes(4)

#define DBG_CONSOLE_OPEN_KEY                Key_Tilde
#define DBG_CONSOLE_INPUT_HISTORY_LENGTH    16 //items
#define DBG_CONSOLE_OPEN_TIME               200 //ms
#define DBG_CONSOLE_FADE_IN_TIME            300 //ms
#define DBG_CONSOLE_NORMAL_OPEN_AMOUNT      0.33f //percent
#define DBG_CONSOLE_LARGE_OPEN_AMOUNT       0.75f //percent
#define DBG_CONSOLE_OVERLAY_ALPHA           0.50f //alpha
#define DBG_CONSOLE_OPEN_AMOUNT_RATIO       (DBG_CONSOLE_NORMAL_OPEN_AMOUNT / DBG_CONSOLE_LARGE_OPEN_AMOUNT)
#define DBG_CONSOLE_SCROLL_SPEED            50 //multiplier
#define DBG_CONSOLE_SCROLL_LAG              4 //divisor

#define DBG_CONSOLE_INPUT_LABEL_STR            "\bInput:\b"
#define DBG_CONSOLE_JUMP_TO_END_STR            "\bJump to End\b"
#define DBG_CONSOLE_REFOCUS_STR                "\bRefocus\b"
#define DBG_CONSOLE_VIEW_PADDING               6 //px
#define DBG_CONSOLE_ELEM_MARGIN                5 //px
#define DBG_CONSOLE_INPUT_BOX_HEIGHT           24 //px
#define DBG_CONSOLE_GUTTER_WIDTH               50 //px
#define DBG_CONSOLE_GUTTER_PADDING             1 //px
#define DBG_CONSOLE_FILE_NAME_GUTTER_WIDTH     250 //px
#define DBG_CONSOLE_FILE_LINE_NUM_GUTTER_WIDTH 50 //px
#define DBG_CONSOLE_FUNC_NAME_GUTTER_WIDTH     300 //px
#define DBG_CONSOLE_TIME_GUTTER_WIDTH          200 //px
#define DBG_CONSOLE_SCROLL_BAR_WIDTH           4 //px
#define DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE    30 //px
#define DBG_CONSOLE_FULL_ALPHA_BTN_SIZE        32 //px
#define DBG_CONSOLE_ICON_SMALLER_AMOUNT        5 //px deflated
#define DBG_CONSOLE_AUTOCOMPLETE_OPEN_TIME     300 //ms
#define DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN   12 //px
#define DBG_CONSOLE_AUTOCOMPLETE_ITEM_PADDING  4 //px
#define DBG_CONSOLE_AUTOCOMPLETE_MAX_HEIGHT    400 //px
#define DBG_CONSOLE_AUTOCOMPLETE_SCROLL_PAST   50 //px

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void InitializeDebugConsole(DebugConsole_t* console, u64 fifoSize, u8* fifoSpace, u64 buildSize, u8* buildSpace)
{
	NotNull(console);
	Assert(fifoSpace != nullptr || fifoSize == 0);
	
	ClearPointer(console);
	console->state = DbgConsoleState_Closed;
	console->openAmount = 0.0f;
	console->alphaAmount = 0.0f;
	console->followingEnd = true;
	console->scroll = 0;
	console->scrollTarget = 0;
	console->fullAlphaText = true;
	
	console->gutterEnabled = true;
	console->fileNameGutterEnabled = false;
	console->fileLineNumGutterEnabled = true;
	console->funcNameGutterEnabled = false;
	console->timeGutterEnabled = false;
	
	CreateVarArray(&console->registeredCommands, mainHeap, sizeof(DebugConsoleRegisteredCommand_t));
	
	console->selectionActive = false;
	console->mouseHovering = false;
	CreateVarArray(&console->selectionRecs, mainHeap, sizeof(DebugConsoleSelectionRec_t));
	
	CreateVarArray(&console->autocompleteItems, mainHeap, sizeof(DebugConsoleAutocompleteItem_t));
	
	CreateTextbox(&console->inputTextbox, mainHeap, 0, false, false);
	SetTextboxHintText(&console->inputTextbox, NewStr("Enter Command... (or type \"help\")"));
	console->inputTextbox.autoSelectOnFocus = true;
	
	CreateVarArray(&console->inputHistory, mainHeap, sizeof(MyStr_t), DBG_CONSOLE_INPUT_HISTORY_LENGTH, false);
	
	plat->CreateMutex(&console->fifoMutex);
	
	if (fifoSize > 0)
	{
		console->hasFifo = true;
		CreateStringFifo(&console->fifo, fifoSize, fifoSpace);
		StringFifoAddBuildBuffer(&console->fifo, buildSize, buildSpace);
	}
	else
	{
		console->hasFifo = false;
	}
	
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void DebugConsoleRegisterCommand(DebugConsole_t* console, MyStr_t command, MyStr_t description, u64 numArguments, MyStr_t* arguments) //pre-declared in pig_func_defs.h
{
	NotNull(console);
	NotNullStr(&command);
	NotNullStr(&description);
	AssertIf(numArguments > 0, arguments != nullptr);
	Assert(numArguments <= DEBUG_COMMAND_MAX_NUM_ARGUMENTS);
	DebugConsoleRegisteredCommand_t* newCommand = VarArrayAdd(&console->registeredCommands, DebugConsoleRegisteredCommand_t);
	NotNull(newCommand);
	ClearPointer(newCommand);
	newCommand->command = AllocString(fixedHeap, &command);
	newCommand->description = AllocString(fixedHeap, &description);
	newCommand->numArguments = numArguments;
	for (u64 aIndex = 0; aIndex < numArguments; aIndex++)
	{
		NotNullStr(&arguments[aIndex]);
		newCommand->arguments[aIndex] = AllocString(mainHeap, &arguments[aIndex]);
	}
}

void DebugConsoleClearAutocompleteItems(DebugConsole_t* console, bool resetAnimation = true)
{
	NotNull(console);
	VarArrayLoop(&console->autocompleteItems, iIndex)
	{
		VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
		FreeString(mainHeap, &item->command);
		FreeString(mainHeap, &item->displayText);
	}
	VarArrayClear(&console->autocompleteItems);
	if (resetAnimation) { console->autocompleteOpenAmount = 0.0f; }
	console->autocompleteActive = false;
	console->autocompleteSelectionIndex = -1;
}

void DebugConsoleDismissAutocomplete(DebugConsole_t* console)
{
	NotNull(console);
	console->autocompleteActive = false;
}

void DebugConsoleClose(DebugConsole_t* console)
{
	NotNull(console);
	if (console->state == DbgConsoleState_Closed) { return; }
	
	if (console->state != DbgConsoleState_OpenLarge)
	{
		console->openAmount *= DBG_CONSOLE_OPEN_AMOUNT_RATIO;
		console->openAmount = InverseEaseQuadraticInOut(console->openAmount);
	}
	console->state = DbgConsoleState_Closed;
	console->overlayMode = false;
	if (IsFocused(&console->inputTextbox))
	{
		ClearFocus();
	}
	DebugConsoleClearAutocompleteItems(console);
}

void DebugConsoleClearInputHistory(DebugConsole_t* console)
{
	NotNull(console);
	VarArrayLoop(&console->inputHistory, hIndex)
	{
		VarArrayLoopGet(MyStr_t, pastCommandStr, &console->inputHistory, hIndex);
		FreeString(mainHeap, pastCommandStr);
	}
	VarArrayClear(&console->inputHistory);
}

bool DebugConsolePushInputHistory(DebugConsole_t* console, MyStr_t newInputStr)
{
	NotNull(console);
	
	bool isDuplicate = false;
	if (console->inputHistory.length > 0)
	{
		MyStr_t* previousInput = VarArrayGet(&console->inputHistory, 0, MyStr_t);
		if (StrEquals(newInputStr, *previousInput))
		{
			isDuplicate = true;
		}
	}
	
	if (!isDuplicate)
	{
		if (console->inputHistory.length >= DBG_CONSOLE_INPUT_HISTORY_LENGTH)
		{
			MyStr_t* lastItem = VarArrayGet(&console->inputHistory, console->inputHistory.length-1, MyStr_t);
			FreeString(mainHeap, lastItem);
			VarArrayPop(&console->inputHistory, MyStr_t);
		}
		
		MyStr_t* newStrPntr = VarArrayPushFront(&console->inputHistory, MyStr_t);
		*newStrPntr = AllocString(mainHeap, &newInputStr);
	}
	
	return !isDuplicate;
}

bool SplitDebugLineFileAndFuncStr(MyStr_t filePathAndFuncName, bool fullPath, MyStr_t* filePathOut, MyStr_t* funcNameOut)
{
	NotNull_(filePathOut);
	NotNull_(funcNameOut);
	for (u64 cIndex = 0; cIndex < filePathAndFuncName.length; cIndex++)
	{
		if (filePathAndFuncName.pntr[cIndex] == DBG_FILEPATH_AND_FUNCNAME_SEP_CHAR)
		{
			filePathOut->pntr = &filePathAndFuncName.pntr[0];
			filePathOut->length = cIndex;
			funcNameOut->pntr = &filePathAndFuncName.pntr[cIndex+1];
			funcNameOut->length = filePathAndFuncName.length - (cIndex+1);
			if (!fullPath)
			{
				*filePathOut = GetFileNamePart(*filePathOut);
			}
			return true;
		}
	}
	return false;
}

//NOTE: Be careful with how expensive this function is. It's running for each debug output
void DebugConsoleLineAdded(DebugConsole_t* console, StringFifoLine_t* newLine) //pre-declared in pig_func_defs.h
{
	NotNull_(console);
	NotNull_(newLine);
	DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(newLine, DebugConsoleLine_t);
	FlagSet(dbgLine->flags, DbgFlag_New);
	console->numLinesAdded++;
}

void DebugConsoleClampScroll(DebugConsole_t* console)
{
	console->scrollMax = console->totalLinesSize.height - console->viewUsableRec.height;
	if (console->scrollMax < 0) { console->scrollMax = 0; }
	console->scroll = ClampR32(console->scroll, 0, console->scrollMax);
	console->scrollTarget = ClampR32(console->scrollTarget, 0, console->scrollMax);
}

Color_t GetGutterBlockColorForFileName(MyStr_t fileName)
{
	if (StrStartsWith(fileName, "pig_")) { return NewColor(0xFFF27CB1); }
	if (StrStartsWith(fileName, "win32_")) { return MonokaiMagenta; }
	return MonokaiWhite;
}

MyStr_t GetDebugConsoleLinesAsCopyText(DebugConsole_t* console, DebugConsoleTextPos_t startPos, DebugConsoleTextPos_t endPos, MemArena_t* memArena, bool includeLinePrefixes)
{
	NotNull(console);
	DebugConsoleTextPos_t minPos = startPos;
	DebugConsoleTextPos_t maxPos = endPos;
	if (DebugConsoleTextPosCompare(minPos, maxPos) > 0)
	{
		minPos = endPos;
		maxPos = startPos;
	}
	if (minPos.lineIndex == maxPos.lineIndex && minPos.byteIndex == maxPos.byteIndex)
	{
		return (memArena != nullptr ? NewStringInArenaNt(memArena, "") : MyStr_Empty);
	}
	if (console->fifo.numLines == 0)
	{
		return (memArena != nullptr ? NewStringInArenaNt(memArena, "") : MyStr_Empty);
	}
	
	u64 longestFileName = 0;
	MyStr_t result = MyStr_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		u64 byteIndex = 0;
		
		if (pass == 0 && includeLinePrefixes)
		{
			StringFifoLine_t* fifoLine = console->fifo.firstLine;
			while (fifoLine != nullptr)
			{
				MyStr_t filePathAndFuncName = GetFifoLineMetaString(fifoLine);
				MyStr_t fileName = MyStr_Empty;
				MyStr_t funcName = MyStr_Empty;
				bool splitSuccess = SplitDebugLineFileAndFuncStr(filePathAndFuncName, false, &fileName, &funcName);
				DebugAssertAndUnused_(splitSuccess, splitSuccess);
				if (fileName.length > longestFileName) { longestFileName = fileName.length; }
				fifoLine = fifoLine->next;
			}
		}
		
		{
			u64 lineIndex = 0;
			StringFifoLine_t* fifoLine = console->fifo.lastLine;
			while (lineIndex < minPos.lineIndex && fifoLine->prev != nullptr)
			{
				fifoLine = fifoLine->prev;
				lineIndex++;
			}
			
			while (lineIndex >= maxPos.lineIndex)
			{
				NotNull_(fifoLine);
				DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
				MyStr_t text = GetFifoLineText(fifoLine);
				MyStr_t filePathAndFuncName = GetFifoLineMetaString(fifoLine);
				MyStr_t fileName = MyStr_Empty;
				MyStr_t funcName = MyStr_Empty;
				bool splitSuccess = SplitDebugLineFileAndFuncStr(filePathAndFuncName, false, &fileName, &funcName);
				DebugAssertAndUnused_(splitSuccess, splitSuccess);
				
				MyStr_t selectedText = text;
				if (lineIndex == maxPos.lineIndex)
				{
					Assert_(maxPos.byteIndex <= selectedText.length);
					selectedText.length = maxPos.byteIndex;
				}
				if (lineIndex == minPos.lineIndex && minPos.byteIndex > 0)
				{
					Assert_(minPos.byteIndex <= selectedText.length);
					selectedText.pntr += minPos.byteIndex;
					selectedText.length -= minPos.byteIndex;
				}
				
				if (includeLinePrefixes)
				{
					//TODO: Add timestamp and dbgLevel info to prefixes
					u64 fileNameIndent = (fileName.length <= longestFileName) ? (longestFileName - fileName.length) : 0;
					TwoPassPrint(result.pntr, result.length, &byteIndex, "[%*s%.*s:%4llu]: %.*s\n", fileNameIndent, "", fileName.length, fileName.pntr, dbgLine->fileLineNumber, selectedText.length, selectedText.pntr);
				}
				else
				{
					TwoPassPrint(result.pntr, result.length, &byteIndex, "%.*s\n", selectedText.length, selectedText.pntr);
				}
				
				if (lineIndex == maxPos.lineIndex) { break; } //to avoid an underflow when maxPos.lineIndex == 0
				Assert_(lineIndex > 0);
				fifoLine = fifoLine->next;
				lineIndex--;
			}
		}
		
		if (pass == 0)
		{
			result.length = byteIndex;
			if (memArena == nullptr) { return result; }
			result.pntr = AllocArray(memArena, char, result.length+1);
			if (result.pntr == nullptr) { return result; }
		}
		else
		{
			Assert(byteIndex == result.length);
			result.pntr[byteIndex] = '\0';
		}
	}
	return result;
}

// +==========================================+
// | DebugConsoleAutocompleteSortingFunction  |
// +==========================================+
//i32 DebugConsoleAutocompleteSortingFunction(const void* left, const void* right, void* contextPntr)
COMPARE_FUNC_DEFINITION(DebugConsoleAutocompleteSortingFunction) 
{
	DebugConsole_t* console = (DebugConsole_t*)contextPntr;
	DebugConsoleAutocompleteItem_t* leftItem = (DebugConsoleAutocompleteItem_t*)left;
	DebugConsoleAutocompleteItem_t* rightItem = (DebugConsoleAutocompleteItem_t*)right;
	if (console->inputTextbox.text.length > 0)
	{
		u64 leftMatchIndex = 0;
		bool leftFoundSubstring = FindSubstring(leftItem->command, console->inputTextbox.text, &leftMatchIndex, true);
		DebugAssertAndUnused(leftFoundSubstring, leftFoundSubstring);
		u64 rightMatchIndex = 0;
		bool rightFoundSubstring = FindSubstring(rightItem->command, console->inputTextbox.text, &rightMatchIndex, true);
		DebugAssertAndUnused(rightFoundSubstring, rightFoundSubstring);
		if (leftMatchIndex < rightMatchIndex) { return -1; }
		if (rightMatchIndex < leftMatchIndex) { return 1; }
	}
	return CompareFuncMyStr(&leftItem->command, &rightItem->command, nullptr);
}

void DebugConsoleUpdateAutocompleteItems(DebugConsole_t* console, bool forceShowAllItems = false)
{
	NotNull(console);
	DebugConsoleClearAutocompleteItems(console, false);
	if (console->inputTextbox.text.length > 0 || forceShowAllItems)
	{
		VarArrayLoop(&console->registeredCommands, rIndex)
		{
			VarArrayLoopGet(DebugConsoleRegisteredCommand_t, registeredCommand, &console->registeredCommands, rIndex);
			if (forceShowAllItems || FindSubstring(registeredCommand->command, console->inputTextbox.text, nullptr, true))
			{
				u64 formattedCommandStrLength = registeredCommand->command.length+3; //+2 for bold toggle chars and +1 for a colon
				for (u64 aIndex = 0; aIndex < registeredCommand->numArguments; aIndex++)
				{
					formattedCommandStrLength += 1 + registeredCommand->arguments[aIndex].length;
				}
				u64 formattedCommandStrLengthBeforeTabs = formattedCommandStrLength-2; //-2 for bold chars not affecting column index
				if (formattedCommandStrLengthBeforeTabs <   4) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <   8) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  12) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  16) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  20) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  24) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  28) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs <  32) { formattedCommandStrLength++; }
				if (formattedCommandStrLengthBeforeTabs >= 32) { formattedCommandStrLength++; }
				char* formattedCommandStr = TempArray(char, formattedCommandStrLength+1);
				NotNull(formattedCommandStr);
				u64 formattedCommandStrIndex = 0;
				formattedCommandStr[formattedCommandStrIndex] = '\b'; formattedCommandStrIndex++;
				MyMemCopy(&formattedCommandStr[formattedCommandStrIndex], registeredCommand->command.pntr, registeredCommand->command.length); formattedCommandStrIndex += registeredCommand->command.length;
				formattedCommandStr[formattedCommandStrIndex] = '\b'; formattedCommandStrIndex++;
				for (u64 aIndex = 0; aIndex < registeredCommand->numArguments; aIndex++)
				{
					MyStr_t argument = registeredCommand->arguments[aIndex];
					formattedCommandStr[formattedCommandStrIndex] = ' '; formattedCommandStrIndex++;
					MyMemCopy(&formattedCommandStr[formattedCommandStrIndex], argument.pntr, argument.length); formattedCommandStrIndex += argument.length;
				}
				formattedCommandStr[formattedCommandStrIndex] = ':'; formattedCommandStrIndex++;
				if (formattedCommandStrLengthBeforeTabs <   4) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <   8) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  12) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  16) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  20) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  24) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  28) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs <  32) { formattedCommandStr[formattedCommandStrIndex] = '\t'; formattedCommandStrIndex++; }
				if (formattedCommandStrLengthBeforeTabs >= 32) { formattedCommandStr[formattedCommandStrIndex] = ' ';  formattedCommandStrIndex++; }
				Assert(formattedCommandStrIndex == formattedCommandStrLength);
				formattedCommandStr[formattedCommandStrLength] = '\0';
				
				DebugConsoleAutocompleteItem_t* newItem = VarArrayAdd(&console->autocompleteItems, DebugConsoleAutocompleteItem_t);
				NotNull(newItem);
				ClearPointer(newItem);
				newItem->index = console->autocompleteItems.length-1;
				newItem->command = AllocString(mainHeap, &registeredCommand->command);
				if (registeredCommand->description.length > 0)
				{
					newItem->displayText = PrintInArenaStr(mainHeap, "%s%.*s", formattedCommandStr, registeredCommand->description.length, registeredCommand->description.pntr);
				}
				else
				{
					newItem->displayText = PrintInArenaStr(mainHeap, "%s", formattedCommandStr);
				}
			}
		}
		
		VarArraySort(&console->autocompleteItems, DebugConsoleAutocompleteSortingFunction, console);
	}
	console->autocompleteActive = (console->autocompleteItems.length > 0);
	console->autocompleteScroll = 0;
	console->autocompleteScrollGoto = 0;
}

// +--------------------------------------------------------------+
// |                  Mouse Hover Pos Callbacks                   |
// +--------------------------------------------------------------+
//TODO: This needs a little more fixup to make it work truly nicely.
//      It really should prioritize positions on the line this is shares a y-value with.
//      Right now it does that when deciding when line to do the index check against but it doesn't do
//      the same logic for multiple lines in a single dbgLine (like ones caused by a line wrap)
struct DebugConsoleFindMouseContext_t
{
	DebugConsole_t* console;
	StringFifoLine_t* fifoLine;
	DebugConsoleLine_t* dbgLine;
	MyStr_t text;
	rec mainRec;
	v2 relativeMousePos;
	bool foundPosition;
	v2 closestPosition;
	u64 closestByteIndex;
	r32 closestDistance;
};
// | DebugConsoleFindMouseHoverIndexCallback  |
// void DebugConsoleFindMouseHoverIndexCallback(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
FFCB_BETWEEN_CHAR_DEFINITION(DebugConsoleFindMouseHoverIndexCallback)
{
	UNUSED(charIndex);
	UNUSED(state);
	NotNull_(state);
	NotNull_(context);
	DebugConsoleFindMouseContext_t* dbgContext = (DebugConsoleFindMouseContext_t*)context;
	r32 distanceToMouse = Vec2Length(dbgContext->relativeMousePos - position);
	if (!dbgContext->foundPosition || (distanceToMouse < dbgContext->closestDistance))
	{
		dbgContext->foundPosition = true;
		dbgContext->closestPosition = position;
		dbgContext->closestByteIndex = byteIndex;
		dbgContext->closestDistance = distanceToMouse;
	}
}

// +--------------------------------------------------------------+
// |                   Selection Rec Callbacks                    |
// +--------------------------------------------------------------+
struct DebugConsoleSelectionRecsContext_t
{
	DebugConsole_t* console;
	StringFifoLine_t* fifoLine;
	DebugConsoleLine_t* dbgLine;
	MyStr_t text;
	u64 lineIndex;
	DebugConsoleTextPos_t selectionMin;
	DebugConsoleTextPos_t selectionMax;
	bool selectionIsBackwards;
	u64 startByteIndex;
	u64 endByteIndex;
	bool foundStartPos;
	v2 startPos;
};
void DebugConsoleAddSelectionRec(DebugConsoleSelectionRecsContext_t* context, FontFlowState_t* state)
{
	DebugConsoleSelectionRec_t* newRec = VarArrayAdd(&context->console->selectionRecs, DebugConsoleSelectionRec_t);
	NotNull(newRec);
	ClearPointer(newRec);
	newRec->mainRec.x = context->startPos.x;
	newRec->mainRec.width = state->position.x - context->startPos.x;
	newRec->mainRec.y = state->lineRec.y;
	newRec->mainRec.height = state->lineRec.height;
	newRec->mainRec.topLeft += context->dbgLine->mainRec.topLeft + context->dbgLine->textPos;
	newRec->startPos.lineIndex = context->lineIndex;
	newRec->startPos.byteIndex = context->startByteIndex;
	newRec->endPos.lineIndex = context->lineIndex;
	newRec->endPos.byteIndex = context->endByteIndex;
	newRec->dbgLevel = context->dbgLine->dbgLevel;
	
	//TODO: Re-enable this if we want to do the work to loop from top selected line downwards.
	//      As it stands, the rectangles are actually out of order when line wraps are involved
	//To make the selection look connected we are going to expand the top of the previous selection rectangle to meet the bottom of the new one
	// if (context->console->selectionRecs.length >= 2)
	// {
	// 	DebugConsoleSelectionRec_t* prevRec = VarArrayGet(&context->console->selectionRecs, context->console->selectionRecs.length-2, DebugConsoleSelectionRec_t);
	// 	if (prevRec->mainRec.y > newRec->mainRec.y + newRec->mainRec.height)
	// 	{
	// 		prevRec->mainRec.height = prevRec->mainRec.y + prevRec->mainRec.height - (newRec->mainRec.y + newRec->mainRec.height);
	// 		prevRec->mainRec.y = newRec->mainRec.y + newRec->mainRec.height;
	// 	}
	// }
}
// | DebugConsoleSelectionRecsBeforeLineCallback  |
// void DebugConsoleSelectionRecsBeforeLineCallback(u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
FFCB_BEFORE_LINE_DEFINITION(DebugConsoleSelectionRecsBeforeLineCallback)
{
	UNUSED(lineIndex);
	NotNull_(state);
	NotNull_(context);
	DebugConsoleSelectionRecsContext_t* dbgContext = (DebugConsoleSelectionRecsContext_t*)context;
	if (byteIndex >= dbgContext->startByteIndex && byteIndex < dbgContext->endByteIndex)
	{
		dbgContext->foundStartPos = true;
		dbgContext->startPos = state->position;
	}
	else
	{
		dbgContext->foundStartPos = false;
	}
}
// | DebugConsoleSelectionRecsAfterLineCallback |
// void DebugConsoleSelectionRecsAfterLineCallback(bool isLineWrap, u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
FFCB_AFTER_LINE_DEFINITION(DebugConsoleSelectionRecsAfterLineCallback)
{
	UNUSED(isLineWrap);
	UNUSED(lineIndex);
	UNUSED(byteIndex);
	NotNull_(state);
	NotNull_(context);
	DebugConsoleSelectionRecsContext_t* dbgContext = (DebugConsoleSelectionRecsContext_t*)context;
	if (dbgContext->foundStartPos)
	{
		DebugAssert_(byteIndex <= dbgContext->endByteIndex);
		dbgContext->foundStartPos = false;
		DebugConsoleAddSelectionRec(dbgContext, state);
	}
}
// | DebugConsoleSelectionRecsBetweenCharCallback |
// void DebugConsoleSelectionRecsBetweenCharCallback(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
FFCB_BETWEEN_CHAR_DEFINITION(DebugConsoleSelectionRecsBetweenCharCallback)
{
	UNUSED(byteIndex);
	UNUSED(charIndex);
	UNUSED(state);
	NotNull_(state);
	NotNull_(context);
	DebugConsoleSelectionRecsContext_t* dbgContext = (DebugConsoleSelectionRecsContext_t*)context;
	if (byteIndex == dbgContext->startByteIndex)
	{
		//NOTE: This may happen twice if the startByteIndex happens to be right where a line wrap happens. This is fine.
		dbgContext->foundStartPos = true;
		dbgContext->startPos = position;
	}
	if (byteIndex == dbgContext->endByteIndex)
	{
		if (dbgContext->foundStartPos)
		{
			dbgContext->foundStartPos = false;
			DebugConsoleAddSelectionRec(dbgContext, state);
		}
		else { DebugAssert_(false); }
	}
}

// +--------------------------------------------------------------+
// |                       Render Callbacks                       |
// +--------------------------------------------------------------+
struct BeforeCharRenderContext_t
{
	DebugConsole_t* console;
	StringFifoLine_t* fifoLine;
	DebugConsoleLine_t* dbgLine;
	MyStr_t text;
	u64 lineIndex;
	DebugConsoleTextPos_t selectionMin;
	DebugConsoleTextPos_t selectionMax;
	Color_t originalColor;
	bool colorChanged;
};
// | DebugConsoleBeforeCharRenderCallback |
// bool DebugConsoleBeforeCharRenderCallback(u32 codepoint, const FontCharInfo_t* charInfo, rec logicalRec, rec renderRec, FontFlowState_t* state, void* context)
FFCB_BEFORE_CHAR_DEFINITION(DebugConsoleBeforeCharRenderCallback)
{
	UNUSED(codepoint);
	UNUSED(charInfo);
	UNUSED(logicalRec);
	UNUSED(renderRec);
	NotNull_(context);
	BeforeCharRenderContext_t* dbgContext = (BeforeCharRenderContext_t*)context;
	if (dbgContext->console->selectionActive)
	{
		if ((dbgContext->lineIndex < dbgContext->selectionMin.lineIndex || (dbgContext->lineIndex == dbgContext->selectionMin.lineIndex && state->byteIndex >= dbgContext->selectionMin.byteIndex)) &&
			(dbgContext->lineIndex > dbgContext->selectionMax.lineIndex || (dbgContext->lineIndex == dbgContext->selectionMax.lineIndex && state->byteIndex < dbgContext->selectionMax.byteIndex)))
		{
			if (!dbgContext->colorChanged)
			{
				dbgContext->colorChanged = true;
				dbgContext->originalColor = state->color;
				state->color = MonokaiDarkGray;
			}
		}
		else
		{
			if (dbgContext->colorChanged)
			{
				dbgContext->colorChanged = false;
				state->color = dbgContext->originalColor;
			}
		}
	}
	return true;
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void DebugConsoleLayout(DebugConsole_t* console)
{
	NotNull(console);
	
	console->mainRec = NewRec(0, 0, ScreenSize.width, ScreenSize.height);
	if (console->state == DbgConsoleState_Open)
	{
		console->mainRec.height *= DBG_CONSOLE_NORMAL_OPEN_AMOUNT;
	}
	else if (console->state == DbgConsoleState_OpenLarge || console->state == DbgConsoleState_Closed)
	{
		console->mainRec.height *= DBG_CONSOLE_LARGE_OPEN_AMOUNT;
	}
	console->mainRec.height *= EaseQuadraticInOut(console->openAmount);
	RecAlign(&console->mainRec);
	
	TextMeasure_t inputLabelMeasure = MeasureTextInFont(DBG_CONSOLE_INPUT_LABEL_STR, &pig->resources.fonts->debug, SelectDefaultFontFace());
	console->inputLabelRec.size = inputLabelMeasure.size;
	
	console->inputRec.height = console->inputTextbox.mainRec.height;
	RecLayoutTopOf(&console->inputRec, console->mainRec.y + console->mainRec.height, DBG_CONSOLE_ELEM_MARGIN);
	RecLayoutBetweenX(&console->inputRec,
		console->mainRec.x + DBG_CONSOLE_ELEM_MARGIN + console->inputLabelRec.width,
		console->mainRec.x + console->mainRec.width,
		DBG_CONSOLE_ELEM_MARGIN, DBG_CONSOLE_ELEM_MARGIN
	);
	RecAlign(&console->inputRec);
	TextboxMove(&console->inputTextbox, console->inputRec);
	
	RecLayoutLeftOf(&console->inputLabelRec, console->inputRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecLayoutVerticalCenter(&console->inputLabelRec, console->inputRec);
	RecAlign(&console->inputLabelRec);
	console->inputLabelTextPos = console->inputLabelRec.topLeft + inputLabelMeasure.offset;
	Vec2Align(&console->inputLabelTextPos);
	
	console->autocompleteItemsSize = Vec2_Zero;
	VarArrayLoop(&console->autocompleteItems, iIndex)
	{
		VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
		item->displayTextMeasure = MeasureTextInFont(item->displayText, &pig->resources.fonts->debug, SelectFontFace(18));
		item->mainRec.size = item->displayTextMeasure.size + NewVec2(DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN*2, DBG_CONSOLE_AUTOCOMPLETE_ITEM_PADDING);
		item->mainRec.y = console->autocompleteItemsSize.height + ((iIndex == 0) ? DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN : 0);
		item->mainRec.x = 0;
		RecAlign(&item->mainRec);
		item->displayTextPos.x = item->mainRec.x + item->mainRec.width/2 - item->displayTextMeasure.size.width/2 + item->displayTextMeasure.offset.x;
		item->displayTextPos.y = item->mainRec.y + item->mainRec.height/2 - item->displayTextMeasure.size.height/2 + item->displayTextMeasure.offset.y;
		Vec2Align(&item->displayTextPos);
		if (console->autocompleteItemsSize.width < item->mainRec.x + item->mainRec.width) { console->autocompleteItemsSize.width = item->mainRec.x + item->mainRec.width; }
		if (console->autocompleteItemsSize.height < item->mainRec.y + item->mainRec.height) { console->autocompleteItemsSize.height = item->mainRec.y + item->mainRec.height; }
	}
	console->autocompleteRec.size = console->autocompleteItemsSize + NewVec2(0, DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN);
	if (console->autocompleteRec.height > DBG_CONSOLE_AUTOCOMPLETE_MAX_HEIGHT) { console->autocompleteRec.height = DBG_CONSOLE_AUTOCOMPLETE_MAX_HEIGHT; }
	if (console->autocompleteRec.height > console->inputRec.y) { console->autocompleteRec.height = console->inputRec.y; }
	if (console->autocompleteRec.width > console->inputRec.width) { console->autocompleteRec.width = console->inputRec.width; }
	console->autocompleteRec.height = console->autocompleteRec.height * EaseQuadraticOut(console->autocompleteOpenAmount);
	console->autocompleteRec.x = console->inputRec.x;
	console->autocompleteRec.y = console->inputRec.y - console->autocompleteRec.height;
	RecAlign(&console->autocompleteRec);
	VarArrayLoop(&console->autocompleteItems, iIndex)
	{
		VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
		item->mainRec.width = console->autocompleteItemsSize.width;
	}
	if (console->autocompleteScrollToSelection)
	{
		console->autocompleteScrollToSelection = false;
		if (console->autocompleteSelectionIndex >= 0 && (u64)console->autocompleteSelectionIndex < console->autocompleteItems.length)
		{
			DebugConsoleAutocompleteItem_t* selectedItem = VarArrayGet(&console->autocompleteItems, console->autocompleteSelectionIndex, DebugConsoleAutocompleteItem_t);
			if (console->autocompleteScrollGoto < selectedItem->mainRec.y + selectedItem->mainRec.height - console->autocompleteRec.height)
			{
				console->autocompleteScrollGoto = selectedItem->mainRec.y + selectedItem->mainRec.height - console->autocompleteRec.height + DBG_CONSOLE_AUTOCOMPLETE_SCROLL_PAST;
			}
			if (console->autocompleteScrollGoto > selectedItem->mainRec.y)
			{
				console->autocompleteScrollGoto = selectedItem->mainRec.y - DBG_CONSOLE_AUTOCOMPLETE_SCROLL_PAST;
			}
		}
	}
	console->autocompleteScrollMax = (console->autocompleteItemsSize.height + DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN) - console->autocompleteRec.height;
	console->autocompleteScroll = ClampR32(console->autocompleteScroll, 0, console->autocompleteScrollMax);
	console->autocompleteScrollGoto = ClampR32(console->autocompleteScrollGoto, 0, console->autocompleteScrollMax);
	
	console->viewRec = console->mainRec;
	RecLayoutBetweenY(&console->viewRec, console->mainRec.y, console->inputRec.y, 0, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->viewRec);
	
	RecLayoutLeftPortionOf(&console->gutterRec,            &console->viewRec, console->gutterEnabled            ? DBG_CONSOLE_GUTTER_WIDTH               : 0.0f);
	RecAlign(&console->gutterRec);
	RecLayoutLeftPortionOf(&console->fileNameGutterRec,    &console->viewRec, console->fileNameGutterEnabled    ? DBG_CONSOLE_FILE_NAME_GUTTER_WIDTH     : 0.0f, false, console->gutterRec.width + DBG_CONSOLE_GUTTER_PADDING);
	RecAlign(&console->fileNameGutterRec);
	RecLayoutLeftPortionOf(&console->fileLineNumGutterRec, &console->viewRec, console->fileLineNumGutterEnabled ? DBG_CONSOLE_FILE_LINE_NUM_GUTTER_WIDTH : 0.0f, false, console->gutterRec.width + console->fileNameGutterRec.width + DBG_CONSOLE_GUTTER_PADDING);
	RecAlign(&console->fileLineNumGutterRec);
	RecLayoutLeftPortionOf(&console->funcNameGutterRec,    &console->viewRec, console->funcNameGutterEnabled    ? DBG_CONSOLE_FUNC_NAME_GUTTER_WIDTH     : 0.0f, false, console->gutterRec.width + console->fileNameGutterRec.width + console->fileLineNumGutterRec.width + DBG_CONSOLE_GUTTER_PADDING*2);
	RecAlign(&console->funcNameGutterRec);
	RecLayoutLeftPortionOf(&console->timeGutterRec,        &console->viewRec, console->timeGutterEnabled        ? DBG_CONSOLE_TIME_GUTTER_WIDTH          : 0.0f, false, console->gutterRec.width + console->fileNameGutterRec.width + console->fileLineNumGutterRec.width + console->funcNameGutterRec.width + DBG_CONSOLE_GUTTER_PADDING*3);
	RecAlign(&console->timeGutterRec);
	
	RecLayoutRightPortionOf(&console->scrollGutterRec, &console->viewRec, DBG_CONSOLE_SCROLL_BAR_WIDTH);
	RecAlign(&console->scrollGutterRec);
	
	console->viewUsableRec = console->viewRec;
	RecLayoutBetweenX(&console->viewUsableRec, console->timeGutterRec.x + console->timeGutterRec.width, console->scrollGutterRec.x);
	console->viewUsableRec = RecDeflate(console->viewUsableRec, DBG_CONSOLE_VIEW_PADDING, DBG_CONSOLE_VIEW_PADDING);
	RecAlign(&console->viewUsableRec);
	
	const char* currentJumpToEndStr = (console->overlayMode ? DBG_CONSOLE_REFOCUS_STR : DBG_CONSOLE_JUMP_TO_END_STR);
	TextMeasure_t jumpToEndBtnTextMeasure = MeasureTextInFont(currentJumpToEndStr, &pig->resources.fonts->debug, SelectDefaultFontFace());
	console->jumpToEndBtnRec.size = jumpToEndBtnTextMeasure.size;
	bool shouldBtnBeShiftedRight = (console->fileNameGutterEnabled && console->funcNameGutterEnabled);
	RecLayoutHorizontalCenter(&console->jumpToEndBtnRec, console->viewRec, (shouldBtnBeShiftedRight ? 0.75f : 0.5f));
	RecLayoutTopOf(&console->jumpToEndBtnRec, console->viewRec.y + console->viewRec.height, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->jumpToEndBtnRec);
	console->jumpToEndBtnTextPos = console->jumpToEndBtnRec.topLeft + jumpToEndBtnTextMeasure.offset;
	Vec2Align(&console->jumpToEndBtnTextPos);
	
	DebugConsoleClampScroll(console);
	
	r32 scrollBarSizePercentage = ClampR32(console->viewUsableRec.height / console->totalLinesSize.height, 0, 1);
	r32 scrollPercentage = 1.0f - ClampR32(console->scroll / console->scrollMax, 0, 1);
	
	console->scrollBarRec = console->scrollGutterRec;
	console->scrollBarRec.height *= scrollBarSizePercentage;
	if (console->scrollBarRec.height < 10) { console->scrollBarRec.height = 10; }
	r32 scrollBarMovableArea = console->scrollGutterRec.height - console->scrollBarRec.height;
	console->scrollBarRec.y += scrollBarMovableArea * scrollPercentage;
	RecAlign(&console->scrollBarRec);
	
	console->closeBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->closeBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->closeBtnRec, console->mainRec.x + console->mainRec.width, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->closeBtnRec);
	
	console->toggleTimeBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->toggleTimeBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->toggleTimeBtnRec, console->closeBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->toggleTimeBtnRec);
	
	console->toggleFuncNameBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->toggleFuncNameBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->toggleFuncNameBtnRec, console->toggleTimeBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->toggleFuncNameBtnRec);
	
	console->toggleFileLineNumBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->toggleFileLineNumBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->toggleFileLineNumBtnRec, console->toggleFuncNameBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->toggleFileLineNumBtnRec);
	
	console->toggleFileNameBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->toggleFileNameBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->toggleFileNameBtnRec, console->toggleFileLineNumBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->toggleFileNameBtnRec);
	
	console->toggleGutterBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->toggleGutterBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->toggleGutterBtnRec, console->toggleFileNameBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->toggleGutterBtnRec);
	
	console->exportBtnRec.y = console->mainRec.y + DBG_CONSOLE_ELEM_MARGIN;
	console->exportBtnRec.size = NewVec2(DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE, DBG_CONSOLE_TOGGLE_GUTTER_BTNS_SIZE);
	RecLayoutLeftOf(&console->exportBtnRec, console->toggleGutterBtnRec.x, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->exportBtnRec);
	
	console->fullAlphaTextBtnRec.size = Vec2Fill(DBG_CONSOLE_FULL_ALPHA_BTN_SIZE);
	RecLayoutLeftOf(&console->fullAlphaTextBtnRec, console->viewRec.x + console->viewRec.width, DBG_CONSOLE_ELEM_MARGIN);
	RecLayoutTopOf(&console->fullAlphaTextBtnRec, console->viewRec.y + console->viewRec.height, DBG_CONSOLE_ELEM_MARGIN);
	RecAlign(&console->fullAlphaTextBtnRec);
}

void DebugConsoleLineLayout(DebugConsole_t* console, StringFifoLine_t* fifoLine, DebugConsoleLine_t* dbgLine)
{
	TempPushMark();
	NotNull(console);
	NotNull(fifoLine);
	NotNull(dbgLine);
	MyStr_t text = GetFifoLineText(fifoLine);
	MyStr_t filePathAndFuncName = GetFifoLineMetaString(fifoLine);
	MyStr_t fileName = MyStr_Empty;
	MyStr_t funcName = MyStr_Empty;
	bool splitSuccess = SplitDebugLineFileAndFuncStr(filePathAndFuncName, false, &fileName, &funcName);
	DebugAssertAndUnused_(splitSuccess, splitSuccess);
	MyStr_t fileLineNumberStr = TempPrintStr("%llu", dbgLine->fileLineNumber);
	MyStr_t gutterNumberStr = TempPrintStr("%u", fifoLine->lineNumber);
	
	FontFace_t* fontFace = GetFontFace(&pig->resources.fonts->debug, SelectDefaultFontFace());
	NotNull_(fontFace);
	TextMeasure_t textMeasure = MeasureTextInFont(text, &pig->resources.fonts->debug, SelectDefaultFontFace(), 1.0f, console->viewUsableRec.width);
	if (textMeasure.size.height < fontFace->lineHeight)
	{
		r32 changeAmount = (fontFace->lineHeight - textMeasure.size.height);
		textMeasure.size.height += changeAmount;
		textMeasure.offset.y += RoundR32(changeAmount/2);
	}
	
	dbgLine->textSize        = textMeasure.size;
	dbgLine->fileNameSize    = MeasureTextInFont(fileName,          &pig->resources.fonts->debug, SelectDefaultFontFace()).size;
	dbgLine->funcNameSize    = MeasureTextInFont(funcName,          &pig->resources.fonts->debug, SelectDefaultFontFace()).size;
	dbgLine->fileLineNumSize = MeasureTextInFont(fileLineNumberStr, &pig->resources.fonts->debug, SelectDefaultFontFace()).size;
	dbgLine->gutterNumSize   = MeasureTextInFont(gutterNumberStr,   &pig->resources.fonts->debug, SelectDefaultFontFace()).size;
	
	dbgLine->textPos = NewVec2(0, textMeasure.offset.y);
	dbgLine->mainRec.size = dbgLine->textSize;
	
	TempPopMark();
}

void DebugConsoleCaptureMouse(DebugConsole_t* console)
{
	NotNull(console);
	DebugConsoleLayout(console);
	if (console->state != DbgConsoleState_Closed)
	{
		if (console->overlayMode || !console->followingEnd)
		{
			MouseHitRecNamed(console->jumpToEndBtnRec, "DebugConsoleJumpToEndBtn");
		}
		if (console->overlayMode)
		{
			MouseHitRecNamed(console->fullAlphaTextBtnRec, "DebugConsoleFullAlphaTextBtn");
		}
		if (!console->overlayMode)
		{
			if (console->autocompleteActive && IsMouseInsideRec(console->autocompleteRec))
			{
				v2 itemsOffset = console->autocompleteRec.topLeft + NewVec2(0, -console->autocompleteScroll);
				Vec2Align(&itemsOffset);
				VarArrayLoop(&console->autocompleteItems, iIndex)
				{
					VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
					rec itemRec = item->mainRec + itemsOffset;
					MouseHitRecPrint(itemRec, "DebugConsoleAutocompleteItem%llu", iIndex);
				}
				MouseHitNamed("DebugConsoleAutocomplete");
			}
			TextboxCaptureMouse(&console->inputTextbox);
			MouseHitRecNamed(console->closeBtnRec,             "DebugConsoleCloseBtn");
			MouseHitRecNamed(console->toggleGutterBtnRec,      "DebugConsoleToggleGutterBtn");
			MouseHitRecNamed(console->toggleFileNameBtnRec,    "DebugConsoleToggleFileNameBtn");
			MouseHitRecNamed(console->toggleFileLineNumBtnRec, "DebugConsoleToggleFileLineNumBtn");
			MouseHitRecNamed(console->toggleFuncNameBtnRec,    "DebugConsoleToggleFuncNameBtn");
			MouseHitRecNamed(console->toggleTimeBtnRec,        "DebugConsoleToggleTimeBtn");
			MouseHitRecNamed(console->exportBtnRec,            "DebugConsoleExportBtn");
			if (console->gutterEnabled)            { MouseHitRecNamed(console->gutterRec,            "DebugConsoleViewGutter");            }
			if (console->fileNameGutterEnabled)    { MouseHitRecNamed(console->fileNameGutterRec,    "DebugConsoleViewFileNameGutter");    }
			if (console->fileLineNumGutterEnabled) { MouseHitRecNamed(console->fileLineNumGutterRec, "DebugConsoleViewFileLineNumGutter"); }
			if (console->funcNameGutterEnabled)    { MouseHitRecNamed(console->funcNameGutterRec,    "DebugConsoleViewFuncNameGutter");    }
			MouseHitRecNamed(console->viewRec, "DebugConsoleView");
			MouseHitRecNamed(console->mainRec, "DebugConsole");
			MouseHitNamed("OutsideDebugConsole");
		}
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateDebugConsole(DebugConsole_t* console)
{
	NotNull(console);
	DebugConsoleLayout(console);
	
	if (platInfo->mainWindow->input.resized)
	{
		console->linesNeedResize = true;
		if (console->selectionActive) { console->selectionChanged = true; }
	}
	
	// +========================================+
	// | Handle Ctrl+Tilde to Run Last Command  |
	// +========================================+
	if (KeyPressed(DBG_CONSOLE_OPEN_KEY) && KeyDownRaw(Key_Control))
	{
		HandleKeyExtended(DBG_CONSOLE_OPEN_KEY);
		if (console->inputHistory.length > 0)
		{
			MyStr_t* previousInputPntr = VarArrayGet(&console->inputHistory, console->recallIndex, MyStr_t);
			bool wasValidCommand = PigParseDebugCommand(*previousInputPntr);
			UNUSED(wasValidCommand);
		}
	}
	
	// We need to handle this before the inputBox gets a chance to treat it as text input
	// (because it's bound to Tilde which is a valid character)
	bool openKeyWasPressed = KeyPressed(DBG_CONSOLE_OPEN_KEY);
	if (openKeyWasPressed) { HandleKeyExtended(DBG_CONSOLE_OPEN_KEY); }
	bool escapeKeyWasPressed = KeyPressed(Key_Escape);
	if (escapeKeyWasPressed && console->state != DbgConsoleState_Closed && !console->overlayMode) { HandleKeyExtended(Key_Escape); }
	bool tabKeyWasPressed = KeyPressedRepeating(Key_Tab, 400, 66);
	if (IsFocused(&console->inputTextbox)) { HandleKey(Key_Tab); }
	
	// +============================================+
	// | Handle Mouse Interaction With Autocomplete |
	// +============================================+
	if (console->autocompleteActive)
	{
		VarArrayLoop(&console->autocompleteItems, iIndex)
		{
			VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
			if (IsMouseOverPrint("DebugConsoleAutocompleteItem%llu", iIndex))
			{
				pigOut->cursorType = PlatCursor_Pointer;
				if (MousePressedAndHandleExtended(MouseBtn_Left))
				{
					console->autocompleteSelectionIndex = (i64)iIndex;
					TextboxSetText(&console->inputTextbox, item->command);
					console->inputTextbox.textChanged = false;
					console->inputTextbox.skipNextUnfocusClick = true;
					//don't scroll to selection because that feels bad for mouse selection
				}
			}
		}
	}
	
	// +==============================+
	// |     Update Input Textbox     |
	// +==============================+
	if (console->state != DbgConsoleState_Closed && !console->overlayMode)
	{
		UpdateTextbox(&console->inputTextbox);
		
		// +==============================+
		// |         Handle Enter         |
		// +==============================+
		if (KeyPressed(Key_Enter))
		{
			HandleKeyExtended(Key_Enter);
			if (IsFocused(&console->inputTextbox))
			{
				if (console->inputTextbox.text.length > 0)
				{
					bool wasValidCommand = PigParseDebugCommand(console->inputTextbox.text);
					UNUSED(wasValidCommand);
					DebugConsolePushInputHistory(console, console->inputTextbox.text);
					TextboxSetText(&console->inputTextbox, NewStr(""));
				}
				else
				{
					bool wasValidCommand = PigParseDebugCommand(NewStr("help"));
					UNUSED(wasValidCommand);
				}
				
				console->recallIndex = 0;
				FreeString(mainHeap, &console->suspendedInputStr);
				console->suspendedInputStr = MyStr_Empty;
				DebugConsoleDismissAutocomplete(console);
			}
			else
			{
				FocusTextbox(&console->inputTextbox);
			}
		}
		
		// +==============================+
		// |    Handle Up Arrow Recall    |
		// +==============================+
		if (KeyPressed(Key_Up) && IsFocused(&console->inputTextbox))
		{
			HandleKeyExtended(Key_Up);
			FocusTextbox(&console->inputTextbox);
			if (console->recallIndex < console->inputHistory.length)
			{
				if (console->recallIndex == 0)
				{
					// PrintLine_D("Storing in suspension: \"%*.s\"", console->inputTextbox.text.length, console->inputTextbox.text.pntr);
					console->suspendedInputStr = AllocString(mainHeap, &console->inputTextbox.text);
				}
				// PrintLine_D("Recalling previous input %llu", console->recallIndex);
				MyStr_t* previousInputPntr = VarArrayGet(&console->inputHistory, console->recallIndex, MyStr_t);
				TextboxSetText(&console->inputTextbox, *previousInputPntr);
				console->inputTextbox.textChanged = false;
				DebugConsoleDismissAutocomplete(console);
				console->recallIndex++;
			}
		}
		// +==============================+
		// |  Handle Down Arrow Unrecall  |
		// +==============================+
		if (KeyPressed(Key_Down) && IsFocused(&console->inputTextbox))
		{
			HandleKeyExtended(Key_Down);
			FocusTextbox(&console->inputTextbox);
			if (console->recallIndex > 0)
			{
				console->recallIndex--;
				if (console->recallIndex == 0)
				{
					// PrintLine_D("Recalling suspended text: \"%.*s\"", console->suspendedInputStr.length, console->suspendedInputStr.pntr);
					TextboxSetText(&console->inputTextbox, console->suspendedInputStr);
					FreeString(mainHeap, &console->suspendedInputStr);
					console->suspendedInputStr = MyStr_Empty;
				}
				else
				{
					// PrintLine_D("Recalling previous input %llu", console->recallIndex);
					MyStr_t* previousInputPntr = VarArrayGet(&console->inputHistory, console->recallIndex-1, MyStr_t);
					TextboxSetText(&console->inputTextbox, *previousInputPntr);
				}
				console->inputTextbox.textChanged = false;
				DebugConsoleDismissAutocomplete(console);
			}
		}
	}
	
	// +==============================+
	// |     Update Autocomplete      |
	// +==============================+
	if (console->inputTextbox.textChanged)
	{
		console->inputTextbox.textChanged = false;
		DebugConsoleUpdateAutocompleteItems(console);
	}
	
	// +========================================+
	// | Autocomplete Hides on InputBox Unfocus |
	// +========================================+
	if (!IsFocused(&console->inputTextbox) && console->autocompleteActive)
	{
		DebugConsoleDismissAutocomplete(console);
	}
	
	// +==============================================+
	// | Handle Key_Tab to Select Autocomplete Items  |
	// +==============================================+
	if (IsFocused(&console->inputTextbox) && tabKeyWasPressed)
	{
		if (!console->autocompleteActive && console->inputTextbox.text.length == 0)
		{
			DebugConsoleUpdateAutocompleteItems(console, true);
		}
		if (console->autocompleteActive && console->autocompleteItems.length > 0)
		{
			if (KeyDownRaw(Key_Shift))
			{
				if (console->autocompleteSelectionIndex < 0)
				{
					console->autocompleteSelectionIndex = console->autocompleteItems.length-1;
				}
				else if (console->autocompleteSelectionIndex > 0)
				{
					console->autocompleteSelectionIndex--;
				}
				else
				{
					console->autocompleteSelectionIndex = console->autocompleteItems.length-1;
				}
			}
			else
			{
				if (console->autocompleteSelectionIndex < 0)
				{
					console->autocompleteSelectionIndex = 0;
				}
				else
				{
					console->autocompleteSelectionIndex = (console->autocompleteSelectionIndex + 1) % console->autocompleteItems.length;
				}
			}
			
			DebugConsoleAutocompleteItem_t* selectedItem = VarArrayGet(&console->autocompleteItems, console->autocompleteSelectionIndex, DebugConsoleAutocompleteItem_t);
			TextboxSetText(&console->inputTextbox, selectedItem->command);
			console->inputTextbox.textChanged = false;
			console->autocompleteScrollToSelection = true;
		}
	}
	
	// +======================================+
	// | Handle Scroll Wheel on Autocomplete  |
	// +======================================+
	if (IsMouseOverNamedPartial("DebugConsoleAutocomplete") && pigIn->scrollChangedY)
	{
		HandleMouseScrollY();
		console->autocompleteScrollGoto += -pigIn->scrollDelta.y * DBG_CONSOLE_SCROLL_SPEED;
	}
	
	// +==============================+
	// | Handle New Lines and Resize  |
	// +==============================+
	r32 newLinesTotalHeight = 0;
	{
		StringFifoLine_t* fifoLine = console->fifo.firstLine;
		while (fifoLine != nullptr)
		{
			DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
			if (IsFlagSet(dbgLine->flags, DbgFlag_New))
			{
				FlagUnset(dbgLine->flags, DbgFlag_New);
				DebugConsoleLineLayout(console, fifoLine, dbgLine);
				newLinesTotalHeight += dbgLine->mainRec.height;
			}
			else if (console->linesNeedResize)
			{
				DebugConsoleLineLayout(console, fifoLine, dbgLine);
			}
			fifoLine = fifoLine->next;
		}
		if (console->linesNeedResize || newLinesTotalHeight > 0)
		{
			console->totalLinesSize = Vec2_Zero;
			fifoLine = console->fifo.lastLine;
			while (fifoLine != nullptr)
			{
				DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
				dbgLine->mainRec.x = 0;
				dbgLine->mainRec.y = -console->totalLinesSize.height - dbgLine->mainRec.height;
				console->totalLinesSize.height += dbgLine->mainRec.height;
				if (dbgLine->mainRec.width > console->totalLinesSize.width) { console->totalLinesSize.width = dbgLine->mainRec.width; }
				fifoLine = fifoLine->prev;
			}
		}
		console->linesNeedResize = false;
	}
	
	// +==============================+
	// |   Position and Index Fixup   |
	// +==============================+
	if (console->followingEnd) { console->scrollTarget = 0; }
	if (newLinesTotalHeight > 0 || console->numLinesAdded > 0)
	{
		console->scroll += newLinesTotalHeight;
		console->scrollTarget += newLinesTotalHeight;
		console->mouseHoverPos.y -= newLinesTotalHeight;
		console->mouseHoverTextPos.lineIndex += console->numLinesAdded;
		if (console->selectionActive)
		{
			VarArrayLoop(&console->selectionRecs, rIndex)
			{
				VarArrayLoopGet(DebugConsoleSelectionRec_t, selRec, &console->selectionRecs, rIndex);
				selRec->mainRec.y -= newLinesTotalHeight;
				selRec->startPos.lineIndex += console->numLinesAdded;
				//TODO: This is technically broken but it's not a big enough deal yet that I want to fix it
				if (selRec->startPos.lineIndex >= console->fifo.numLines)
				{
					if (console->fifo.numLines > 0)
					{
						selRec->startPos.lineIndex = console->fifo.numLines-1;
						selRec->startPos.byteIndex = 0;
					}
					else { console->selectionActive = false; console->selectionChanged = true; }
				}
				if (selRec->endPos.lineIndex >= console->fifo.numLines)
				{
					console->selectionActive = false;
					console->selectionChanged = true;
				}
				selRec->endPos.lineIndex += console->numLinesAdded;
			}
		}
		if (console->selectionActive)
		{
			console->selectionStart.lineIndex += console->numLinesAdded;
			console->selectionEnd.lineIndex += console->numLinesAdded;
		}
		DebugConsoleClampScroll(console);
	}
	console->numLinesAdded = 0;
	
	// +==============================+
	// |    Handle Refocus Button     |
	// +==============================+
	if (IsMouseOverNamed("DebugConsoleJumpToEndBtn") && console->overlayMode)
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			console->overlayMode = false;
			FocusTextbox(&console->inputTextbox);
		}
	}
	
	// +===============================+
	// | Handle Full Alpha Text Button |
	// +===============================+
	if (IsMouseOverNamed("DebugConsoleFullAlphaTextBtn") && console->overlayMode)
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			console->fullAlphaText = !console->fullAlphaText;
		}
	}
	
	// +==============================+
	// |    Handle Toggle Buttons     |
	// +==============================+
	{
		if (IsMouseOverNamed("DebugConsoleCloseBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				DebugConsoleClose(console);
			}
		}
		if (IsMouseOverNamed("DebugConsoleToggleGutterBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				console->gutterEnabled = !console->gutterEnabled;
				console->linesNeedResize = true;
			}
		}
		if (IsMouseOverNamed("DebugConsoleToggleFileNameBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				console->fileNameGutterEnabled = !console->fileNameGutterEnabled;
				console->linesNeedResize = true;
			}
		}
		if (IsMouseOverNamed("DebugConsoleToggleFileLineNumBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				console->fileLineNumGutterEnabled = !console->fileLineNumGutterEnabled;
				console->linesNeedResize = true;
			}
		}
		if (IsMouseOverNamed("DebugConsoleToggleFuncNameBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				console->funcNameGutterEnabled = !console->funcNameGutterEnabled;
				console->linesNeedResize = true;
			}
		}
		if (IsMouseOverNamed("DebugConsoleToggleTimeBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				console->timeGutterEnabled = !console->timeGutterEnabled;
				console->linesNeedResize = true;
			}
		}
		if (IsMouseOverNamed("DebugConsoleExportBtn"))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				DebugConsoleTextPos_t firstPos;
				firstPos.lineIndex = console->fifo.numLines-1;
				firstPos.byteIndex = 0;
				DebugConsoleTextPos_t lastPos;
				lastPos.lineIndex = 0;
				lastPos.byteIndex = (console->fifo.lastLine != nullptr) ? console->fifo.lastLine->textLength : 0;
				TempPushMark();
				MyStr_t selectedText = GetDebugConsoleLinesAsCopyText(console, firstPos, lastPos, TempArena, true);
				if (selectedText.length > 0)
				{
					MyStr_t outputFilePath = TempPrintStr("%llu.txt", ProgramTime); //TODO: Change this to an actual date time format
					if (plat->WriteEntireFile(outputFilePath, selectedText.pntr, selectedText.length))
					{
						if (!plat->ShowFile(outputFilePath))
						{
							PrintLine_E("Failed to show output file at \"%.*s\"", outputFilePath.length, outputFilePath.pntr);
						}
					}
					else
					{
						PrintLine_E("Failed to write %llu chars to file \"%.*s\"", selectedText.length, outputFilePath.length, outputFilePath.pntr);
					}
				}
				TempPopMark();
			}
		}
	}
	
	// +==============================+
	// |     Handle Scroll Wheel      |
	// +==============================+
	if (pigIn->scrollChangedY && (IsMouseOverNamedPartial("DebugConsoleView") || IsMouseOverNamed("DebugConsoleJumpToEndBtn")))
	{
		HandleMouseScrollY();
		console->scrollTarget += pigIn->scrollDelta.y * DBG_CONSOLE_SCROLL_SPEED;
		if (pigIn->scrollDelta.y < 0 && console->scrollTarget <= 0)
		{
			console->followingEnd = true;
		}
		else if (pigIn->scrollDelta.y > 0 && console->scrollTarget > 0)
		{
			console->followingEnd = false;
		}
	}
	
	// +==============================+
	// |        Handle End Key        |
	// +==============================+
	if (console->state != DbgConsoleState_Closed && !console->overlayMode && KeyPressed(Key_End))
	{
		HandleKeyExtended(Key_End);
		console->followingEnd = true;
		console->scrollTarget = 0;
	}
	// +==============================+
	// |       Handle Home Key        |
	// +==============================+
	if (console->state != DbgConsoleState_Closed && !console->overlayMode && KeyPressed(Key_Home))
	{
		HandleKeyExtended(Key_Home);
		console->followingEnd = false;
		console->scrollTarget = console->scrollMax;
	}
	
	DebugConsoleClampScroll(console);
	
	// +==============================+
	// |        Update Scroll         |
	// +==============================+
	if (AbsR32(console->scrollTarget - console->scroll) > 1)
	{
		console->scroll += (console->scrollTarget - console->scroll) / DBG_CONSOLE_SCROLL_LAG;
	}
	else
	{
		console->scroll = console->scrollTarget;
	}
	
	// +==============================+
	// |     Find Mouse Hover Pos     |
	// +==============================+
	console->mouseHovering = false;
	if (IsMouseOverNamed("DebugConsoleView") || console->mouseSelecting)
	{
		StringFifoLine_t* fifoLine = console->fifo.lastLine;
		v2 lineBasePos = NewVec2(console->viewUsableRec.x, console->viewUsableRec.y + console->viewUsableRec.height + console->scroll);
		u64 lineIndex = 0;
		while (fifoLine != nullptr)
		{
			DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
			rec mainRec = dbgLine->mainRec + lineBasePos;
			v2 textPos = mainRec.topLeft + dbgLine->textPos;
			//going from bottom up, once we pass over the MousePos.y that means the mouse is hovering over this line
			if (mainRec.y <= MousePos.y)
			{
				MyStr_t text = GetFifoLineText(fifoLine);
				DebugConsoleFindMouseContext_t context = {};
				context.console = console;
				context.fifoLine = fifoLine;
				context.dbgLine = dbgLine;
				context.text = text;
				context.mainRec = mainRec;
				context.relativeMousePos = MousePos - textPos;
				FontFlowCallbacks_t flowCallbacks = {};
				flowCallbacks.context = (void*)&context;
				flowCallbacks.betweenChar = DebugConsoleFindMouseHoverIndexCallback;
				
				FontFlowInfo_t flowInfo = {};
				TextMeasure_t measure = MeasureTextInFont(text, &pig->resources.fonts->debug, SelectDefaultFontFace(), 1.0f, console->viewUsableRec.width, &flowInfo, &flowCallbacks);
				
				if (context.foundPosition)
				{
					console->mouseHovering = true;
					console->mouseHoverTextPos.lineIndex = lineIndex;
					console->mouseHoverTextPos.byteIndex = context.closestByteIndex;
					console->mouseHoverPos = context.closestPosition + textPos - lineBasePos;
				}
				else
				{
					MyDebugBreak();
				}
				
				break;
			}
			lineIndex++;
			fifoLine = fifoLine->prev;
		}
	}
	if (console->mouseHovering)
	{
		pigOut->cursorType = PlatCursor_TextIBeam;
	}
	
	// +==============================+
	// | Handle Mouse Drag to Select  |
	// +==============================+
	{
		if (console->mouseHovering && MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			console->mouseSelecting = true;
			if (!KeyDownRaw(Key_Shift) || !console->selectionActive)
			{
				console->selectionActive = false;
				console->selectionStart = console->mouseHoverTextPos;
			}
			console->selectionEnd = console->mouseHoverTextPos;
			console->selectionChanged = true;
		}
		else if (MouseDown(MouseBtn_Left) && console->mouseSelecting)
		{
			HandleMouse(MouseBtn_Left);
			if (console->mouseHovering &&
				(console->selectionEnd.lineIndex != console->mouseHoverTextPos.lineIndex || console->selectionEnd.byteIndex != console->mouseHoverTextPos.byteIndex))
			{
				console->selectionEnd = console->mouseHoverTextPos;
				if (console->selectionStart.lineIndex == console->selectionEnd.lineIndex &&
					console->selectionStart.byteIndex == console->selectionEnd.byteIndex)
				{
					console->selectionActive = false;
				}
				else
				{
					console->selectionActive = true;
				}
				console->selectionChanged = true;
			}
		}
		if (!MouseDownRaw(MouseBtn_Left) && console->mouseSelecting)
		{
			HandleMouse(MouseBtn_Left);
			console->mouseSelecting = false;
			if (console->selectionStart.lineIndex == console->selectionEnd.lineIndex &&
				console->selectionStart.byteIndex == console->selectionEnd.byteIndex)
			{
				console->selectionActive = false;
				console->selectionChanged = true;
			}
		}
	}
	
	// +==================================+
	// | Handle Select All Hotkey Ctrl+A  |
	// +==================================+
	if (KeyPressed(Key_A) && KeyDownRaw(Key_Control))
	{
		HandleKeyExtended(Key_A);
		if (console->selectionActive)
		{
			console->selectionActive = false;
			console->selectionChanged = true;
		}
		else if (console->fifo.numLines > 0)
		{
			console->selectionActive = true;
			console->selectionStart.lineIndex = console->fifo.numLines-1;
			console->selectionStart.byteIndex = 0;
			console->selectionEnd.lineIndex = 0;
			console->selectionEnd.byteIndex = console->fifo.lastLine->textLength;
			console->selectionChanged = true;
		}
	}
	
	// +==============================+
	// |   Handle JumpToEnd Button    |
	// +==============================+
	if (IsMouseOverNamed("DebugConsoleJumpToEndBtn") && !console->overlayMode && !console->followingEnd)
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			console->followingEnd = true;
		}
	}
	
	// +==============================+
	// |   Handle Selection Changed   |
	// +==============================+
	if (console->selectionChanged)
	{
		console->selectionChanged = false;
		VarArrayClear(&console->selectionRecs);
		if (console->selectionActive)
		{
			bool selectionIsBackwards = false;
			DebugConsoleTextPos_t selectionMin = console->selectionStart;
			DebugConsoleTextPos_t selectionMax = console->selectionEnd;
			if (DebugConsoleTextPosCompare(selectionMin, selectionMax) > 0)
			{
				selectionMin = console->selectionEnd;
				selectionMax = console->selectionStart;
				selectionIsBackwards = true;
			}
			StringFifoLine_t* fifoLine = console->fifo.lastLine;
			u64 lineIndex = 0;
			while (fifoLine != nullptr)
			{
				//NOTE: This logic may look like < and > are flipped but read the note next to DebugConsoleTextPosCompare for why it's this way
				if (lineIndex <= selectionMin.lineIndex && lineIndex >= selectionMax.lineIndex)
				{
					DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
					MyStr_t text = GetFifoLineText(fifoLine);
					u64 startByteIndex = (lineIndex == selectionMin.lineIndex) ? selectionMin.byteIndex : 0;
					u64 endByteIndex = (lineIndex == selectionMax.lineIndex) ? selectionMax.byteIndex : text.length;
					Assert_(endByteIndex >= startByteIndex);
					
					DebugConsoleSelectionRecsContext_t context = {};
					context.console = console;
					context.fifoLine = fifoLine;
					context.dbgLine = dbgLine;
					context.text = text;
					context.lineIndex = lineIndex;
					context.selectionMin = selectionMin;
					context.selectionMax = selectionMax;
					context.selectionIsBackwards = selectionIsBackwards;
					context.startByteIndex = startByteIndex;
					context.endByteIndex = endByteIndex;
					context.foundStartPos = false;
					context.startPos = Vec2_Zero;
					FontFlowCallbacks_t flowCallbacks = {};
					flowCallbacks.context = (void*)&context;
					flowCallbacks.betweenChar = DebugConsoleSelectionRecsBetweenCharCallback;
					flowCallbacks.beforeLine = DebugConsoleSelectionRecsBeforeLineCallback;
					flowCallbacks.afterLine = DebugConsoleSelectionRecsAfterLineCallback;
					
					FontFlowInfo_t flowInfo = {};
					TextMeasure_t measure = MeasureTextInFont(text, &pig->resources.fonts->debug, SelectDefaultFontFace(), 1.0f, console->viewUsableRec.width, &flowInfo, &flowCallbacks);
					DebugAssert_(console->selectionRecs.length > 0);
				}
				
				fifoLine = fifoLine->prev;
				lineIndex++;
			}
		}
	}
	
	// +==============================+
	// | Handle Gutter Block Clicked  |
	// +==============================+
	if (!console->overlayMode && IsMouseOverNamed("DebugConsoleViewFileLineNumGutter"))
	{
		v2 lineBasePos = NewVec2(console->viewUsableRec.x, console->viewUsableRec.y + console->viewUsableRec.height + console->scroll);
		StringFifoLine_t* fifoLine = console->fifo.firstLine;
		while (fifoLine != nullptr)
		{
			DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
			MyStr_t filePathAndFuncName = GetFifoLineMetaString(fifoLine);
			rec mainRec = dbgLine->mainRec + lineBasePos;
			if (MousePos.y >= mainRec.y && MousePos.y < mainRec.y + mainRec.height)
			{
				MyStr_t filePath = MyStr_Empty;
				MyStr_t funcName = MyStr_Empty;
				if (SplitDebugLineFileAndFuncStr(filePathAndFuncName, true, &filePath, &funcName))
				{
					pigOut->cursorType = PlatCursor_Pointer;
					if (MousePressedAndHandleExtended(MouseBtn_Left))
					{
						// PrintLine_I("Clicked on file \"%.*s\" line %u", filePath.length, filePath.pntr, dbgLine->fileLineNumber);
						if (!plat->ShowSourceFile(filePath, dbgLine->fileLineNumber))
						{
							DebugAssert_(false);
						}
					}
				}
			}
			fifoLine = fifoLine->next;
		}
	}
	
	// +==============================+
	// |       Handle Open Key        |
	// +==============================+
	if (openKeyWasPressed)
	{
		if (console->state == DbgConsoleState_Closed)
		{
			console->state = DbgConsoleState_Open;
			FocusTextbox(&console->inputTextbox);
		}
		else if (console->overlayMode)
		{
			console->overlayMode = false;
			FocusTextbox(&console->inputTextbox);
		}
		else if (console->state == DbgConsoleState_Open)
		{
			console->state = DbgConsoleState_OpenLarge;
			console->openAmount *= DBG_CONSOLE_OPEN_AMOUNT_RATIO;
			console->openAmount = InverseEaseQuadraticInOut(console->openAmount);
		}
		else
		{
			DebugConsoleClose(console);
		}
	}
	
	// +==============================+
	// |       Handle Close Key       |
	// +==============================+
	if (escapeKeyWasPressed && console->state != DbgConsoleState_Closed && !console->overlayMode)
	{
		if (console->autocompleteActive)
		{
			DebugConsoleDismissAutocomplete(console);
		}
		else
		{
			DebugConsoleClose(console);
		}
	}
	
	// +==============================+
	// |       Update Open Anim       |
	// +==============================+
	if (console->state != DbgConsoleState_Closed)
	{
		UpdateAnimationUp(&console->openAmount, DBG_CONSOLE_OPEN_TIME);
	}
	else
	{
		UpdateAnimationDown(&console->openAmount, DBG_CONSOLE_OPEN_TIME);
	}
	// +==============================+
	// |    Update Alpha Open Anim    |
	// +==============================+
	if (console->state != DbgConsoleState_Closed)
	{
		if (console->overlayMode)
		{
			UpdateAnimationDownTo(&console->alphaAmount, DBG_CONSOLE_FADE_IN_TIME, DBG_CONSOLE_OVERLAY_ALPHA);
		}
		else
		{
			UpdateAnimationUp(&console->alphaAmount, DBG_CONSOLE_FADE_IN_TIME);
		}
	}
	else
	{
		UpdateAnimationDown(&console->alphaAmount, DBG_CONSOLE_FADE_IN_TIME);
	}
	
	// +==============================+
	// |  Update Autocomplete Scroll  |
	// +==============================+
	if (console->autocompleteActive)
	{
		r32 scrollDelta = console->autocompleteScrollGoto - console->autocompleteScroll;
		if (AbsR32(scrollDelta) > 1.0f)
		{
			console->autocompleteScroll += (scrollDelta / DBG_CONSOLE_SCROLL_LAG);
		}
		else
		{
			console->autocompleteScroll = console->autocompleteScrollGoto;
		}
	}
	
	// +==============================+
	// |   Update Autocomplete Anim   |
	// +==============================+
	if (console->autocompleteActive && console->autocompleteOpenAmount < 1.0f)
	{
		UpdateAnimationUp(&console->autocompleteOpenAmount, DBG_CONSOLE_AUTOCOMPLETE_OPEN_TIME);
	}
	else if (!console->autocompleteActive && console->autocompleteOpenAmount > 0.0f)
	{
		UpdateAnimationDown(&console->autocompleteOpenAmount, DBG_CONSOLE_AUTOCOMPLETE_OPEN_TIME);
	}
	if (!console->autocompleteActive && console->autocompleteOpenAmount <= 0.0f && console->autocompleteItems.length > 0)
	{
		DebugConsoleClearAutocompleteItems(console);
	}
	
	// +==============================+
	// |  Handle Overlay Mode Clicks  |
	// +==============================+
	if (console->state != DbgConsoleState_Closed && !console->overlayMode && IsMouseOverNamed("OutsideDebugConsole"))
	{
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			console->overlayMode = true;
			console->followingEnd = true;
			if (console->alphaAmount < DBG_CONSOLE_OVERLAY_ALPHA) { console->alphaAmount = DBG_CONSOLE_OVERLAY_ALPHA; }
		}
	}
	
	// +==================================+
	// | Handle Ctrl+C Copy to Clipboard  |
	// +==================================+
	if (console->state != DbgConsoleState_Closed && KeyPressed(Key_C) && KeyDownRaw(Key_Control) && console->selectionActive && !console->overlayMode)
	{
		HandleKeyExtended(Key_C);
		TempPushMark();
		bool moreThanOneLineSelected = (console->selectionStart.lineIndex != console->selectionEnd.lineIndex);
		MyStr_t selectedText = GetDebugConsoleLinesAsCopyText(console, console->selectionStart, console->selectionEnd, TempArena, moreThanOneLineSelected);
		if (selectedText.length > 0)
		{
			if (!plat->CopyTextToClipboard(selectedText))
			{
				PrintLine_E("Failed to copy selection of %llu chars to clipboard", selectedText.length);
			}
		}
		TempPopMark();
	}
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void DebugConsoleRenderGutterBlock(rec gutterRec, MyStr_t text, rec nextLineMainRec, r32 blockStartY, rec viewRec, Color_t gutterColor, Color_t textColor, r32 textOffsetY)
{
	rec blockRec = NewRec(gutterRec.x, 0, gutterRec.width, 0);
	RecLayoutBetweenY(&blockRec, nextLineMainRec.y + nextLineMainRec.height, blockStartY);
	blockRec.height -= DBG_CONSOLE_GUTTER_PADDING;
	blockRec = RecOverlap(blockRec, viewRec);
	if (blockRec.width > 0 && blockRec.height > 0)
	{
		v2 textPos = NewVec2(
			blockRec.x + blockRec.width/2,
			blockRec.y + blockRec.height/2 + textOffsetY
		);
		Vec2Align(&textPos);
		if (IsMouseOverNamed("DebugConsoleViewFileLineNumGutter") && MousePos.y >= blockRec.y && MousePos.y < blockRec.y + blockRec.height && MousePos.x >= blockRec.x)
		{
			gutterColor = ColorDarken(gutterColor, 20);
		}
		rec oldViewport = rc->state.viewportRec;
		RcSetViewport(blockRec);
		RcDrawRectangle(blockRec, gutterColor);
		RcDrawText(text, textPos, textColor, TextAlignment_Center);
		RcSetViewport(oldViewport);
	}
}

void RenderDebugConsole(DebugConsole_t* console)
{
	NotNull(console);
	DebugConsoleLayout(console);
	
	if (console->openAmount > 0)
	{
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
		
		// +==============================+
		// |      Render Window Back      |
		// +==============================+
		RcBindShader(&pig->resources.shaders->main2D);
		if (!console->overlayMode)
		{
			RcDrawRectangle(console->mainRec, ColorTransparent(MonokaiBack, console->alphaAmount));
			RcDrawRectangle(NewRec(console->mainRec.x, console->mainRec.y + console->mainRec.height, console->mainRec.width, 1), ColorTransparent(Black, console->alphaAmount));
		}
		
		// +==============================+
		// |        Render ViewRec        |
		// +==============================+
		r32 viewRecAlpha = ((console->fullAlphaText && console->overlayMode) ? 0.8f : (console->alphaAmount * console->alphaAmount));
		RcDrawGradientRectangle(console->viewRec, false, ColorTransparent(MonokaiDarkGray, viewRecAlpha), ColorTransparent(Black, viewRecAlpha));
		
		// +==============================+
		// |        Render Gutter         |
		// +==============================+
		if (!console->overlayMode)
		{
			RcDrawGradientRectangle(console->gutterRec, true, ColorTransparent(MonokaiDarkGray, console->alphaAmount), ColorTransparent(MonokaiBack, console->alphaAmount));
		}
		
		// +==============================+
		// |       Render Input Box       |
		// +==============================+
		if (!console->overlayMode)
		{
			RenderTextbox(&console->inputTextbox);
			// RcDrawRoundedRectangle(RecInflate(console->inputRec, 1, 1), 4, ColorTransparent(MonokaiWhite, console->alphaAmount));
			// RcDrawRoundedRectangle(console->inputRec, 4, ColorTransparent(MonokaiDarkGray, console->alphaAmount));
		}
		
		// +==============================+
		// |      Render Input Label      |
		// +==============================+
		RcBindShader(&pig->resources.shaders->main2D);
		if (!console->overlayMode)
		{
			RcDrawText(DBG_CONSOLE_INPUT_LABEL_STR, console->inputLabelTextPos, MonokaiWhite);
			// RcDrawRectangle(RecInflate(console->inputLabelRec, 100, 100), MonokaiWhite);
		}
		
		RcSetViewport(console->viewRec);
		v2 lineBasePos = NewVec2(console->viewUsableRec.x, console->viewUsableRec.y + console->viewUsableRec.height + console->scroll);
		
		// +==============================+
		// |       Render Selection       |
		// +==============================+
		if (console->selectionActive)
		{
			VarArrayLoop(&console->selectionRecs, rIndex)
			{
				VarArrayLoopGet(DebugConsoleSelectionRec_t, selRec, &console->selectionRecs, rIndex);
				rec mainRec = selRec->mainRec + lineBasePos;
				Color_t selectionColor = GetDbgLevelTextColor(selRec->dbgLevel);
				selectionColor = ColorDarken(selectionColor, 20);
				RcDrawRectangle(mainRec, ColorTransparent(selectionColor, console->alphaAmount));
			}
		}
		
		// +================================+
		// | Render Lines and Gutter Blocks |
		// +================================+
		{
			bool selectionIsBackwards = false;
			DebugConsoleTextPos_t selectionMin = console->selectionStart;
			DebugConsoleTextPos_t selectionMax = console->selectionEnd;
			if (DebugConsoleTextPosCompare(selectionMin, selectionMax) > 0)
			{
				selectionMin = console->selectionEnd;
				selectionMax = console->selectionStart;
				selectionIsBackwards = true;
			}
			
			bool fileNameBlockStarted = false;
			r32 fileNameBlockStartY = 0;
			MyStr_t fileNameBlockStr = MyStr_Empty;
			
			bool fileLineNumBlockStarted = false;
			r32 fileLineNumBlockStartY = 0;
			u64 fileLineNumBlockValue = 0;
			
			bool funcNameBlockStarted = false;
			r32 funcNameBlockStartY = 0;
			MyStr_t funcNameBlockStr = MyStr_Empty;
			
			bool timeBlockStarted = false;
			r32 timeBlockStartY = 0;
			u64 timeBlockValue = 0;
			
			r32 gutterTextOffsetY = 0; //TODO: Remove the need for me!
			Color_t gutterColor = White;
			Color_t gutterTextColor = White;
			
			StringFifoLine_t* fifoLine = console->fifo.lastLine;
			u64 lineIndex = 0;
			while (fifoLine != nullptr)
			{
				DebugConsoleLine_t* dbgLine = GetFifoLineMetaStruct(fifoLine, DebugConsoleLine_t);
				rec mainRec = dbgLine->mainRec + lineBasePos;
				MyStr_t filePathAndFuncName = GetFifoLineMetaString(fifoLine);
				MyStr_t fileName = MyStr_Empty;
				MyStr_t funcName = MyStr_Empty;
				bool splitSuccess = SplitDebugLineFileAndFuncStr(filePathAndFuncName, false, &fileName, &funcName);
				DebugAssertAndUnused_(splitSuccess, splitSuccess);
				// TODO: the use of dbgLine->textPos.y is a dumb hack just tuned because out maxAscend and maxDescend are kind of wack
				gutterTextOffsetY = -RcGetLineHeight()/2 + dbgLine->textPos.y;
				gutterColor = ColorTransparent(GetGutterBlockColorForFileName(fileName), console->alphaAmount);
				if (console->overlayMode) { gutterColor.a = 10; }
				gutterTextColor = ColorTransparent(MonokaiDarkGray, console->alphaAmount);
				
				bool fileNameChanged = false;
				if (!fileNameBlockStarted || StrCompareIgnoreCase(fileName, fileNameBlockStr) != 0)
				{
					if (fileNameBlockStarted)
					{
						gutterColor = ColorTransparent(GetGutterBlockColorForFileName(fileNameBlockStr), console->alphaAmount);
						DebugConsoleRenderGutterBlock(console->fileNameGutterRec, fileNameBlockStr, mainRec, fileNameBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
					}
					fileNameBlockStarted = true;
					fileNameBlockStartY = mainRec.y + mainRec.height;
					fileNameBlockStr = AllocString(TempArena, &fileName);
					fileNameChanged = true;
				}
				if (!fileLineNumBlockStarted || dbgLine->fileLineNumber != fileLineNumBlockValue || fileNameChanged)
				{
					if (fileLineNumBlockStarted)
					{
						DebugConsoleRenderGutterBlock(console->fileLineNumGutterRec, TempPrintStr(":%llu", fileLineNumBlockValue), mainRec, fileLineNumBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
					}
					fileLineNumBlockStarted = true;
					fileLineNumBlockStartY = mainRec.y + mainRec.height;
					fileLineNumBlockValue = dbgLine->fileLineNumber;
				}
				if (!funcNameBlockStarted || StrCompareIgnoreCase(funcName, funcNameBlockStr) != 0 || fileNameChanged)
				{
					if (funcNameBlockStarted)
					{
						DebugConsoleRenderGutterBlock(console->funcNameGutterRec, funcNameBlockStr, mainRec, funcNameBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
					}
					funcNameBlockStarted = true;
					funcNameBlockStartY = mainRec.y + mainRec.height;
					funcNameBlockStr = AllocString(TempArena, &funcName);
				}
				if (!timeBlockStarted || timeBlockValue != dbgLine->timestamp)
				{
					RealTime_t lineRealTime = {};
					ConvertTimestampToRealTime(timeBlockValue, &lineRealTime, pigIn->localTimezoneDoesDst);
					MyStr_t timeBlockStr = FormatRealTime(&lineRealTime, TempArena, false, true, false);
					if (timeBlockStarted)
					{
						DebugConsoleRenderGutterBlock(console->timeGutterRec, timeBlockStr, mainRec, timeBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
					}
					timeBlockStarted = true;
					timeBlockStartY = mainRec.y + mainRec.height;
					timeBlockValue = dbgLine->timestamp;
				}
				
				if (!IsFlagSet(dbgLine->flags, DbgFlag_New) && RecsIntersect(mainRec, console->viewRec))
				{
					TempPushMark();
					MyStr_t text = GetFifoLineText(fifoLine);
					Color_t textColor = GetDbgLevelTextColor(dbgLine->dbgLevel);
					textColor = ColorTransparent(textColor, console->fullAlphaText ? 1.0f : console->alphaAmount);
					v2 textPos = mainRec.topLeft + dbgLine->textPos;
					v2 gutterDrawPos = NewVec2(console->gutterRec.x + console->gutterRec.width - 3, textPos.y);
					
					BeforeCharRenderContext_t context = {};
					context.console = console;
					context.fifoLine = fifoLine;
					context.dbgLine = dbgLine;
					context.text = text;
					context.lineIndex = lineIndex;
					context.selectionMin = selectionMin;
					context.selectionMax = selectionMax;
					context.originalColor = textColor;
					context.colorChanged = false;
					FontFlowCallbacks_t flowCallbacks = {};
					flowCallbacks.context = &context;
					flowCallbacks.beforeChar = DebugConsoleBeforeCharRenderCallback;
					
					if (IsFlagSet(dbgLine->flags, DbgFlag_Inverted))
					{
						rec backgroundRec = mainRec;
						RecLayoutBetweenX(&backgroundRec, console->viewUsableRec.x, console->viewUsableRec.x + console->viewUsableRec.width, -DBG_CONSOLE_VIEW_PADDING, -DBG_CONSOLE_VIEW_PADDING);
						RecAlign(&backgroundRec);
						RcDrawRectangle(backgroundRec, textColor);
						textColor = MonokaiDarkGray;
					}
					
					// RcDrawRectangle(mainRec, ColorTransparent(MonokaiWhite, 0.15f));
					RcSetFontFlowCallbacks(&flowCallbacks);
					RcDrawText(text, textPos, textColor, TextAlignment_Left, console->viewUsableRec.width);
					RcClearFontFlowCallbacks();
					
					RcSetViewport(console->gutterRec);
					RcDrawText(TempPrint("%u", fifoLine->lineNumber), gutterDrawPos, ColorTransparent(MonokaiWhite, console->alphaAmount), TextAlignment_Right);
					RcSetViewport(console->viewRec);
					
					TempPopMark();
				}
				fifoLine = fifoLine->prev;
				lineIndex++;
			}
			
			if (fileNameBlockStarted)
			{
				rec pretendNextRec = NewRec(0, lineBasePos.y - console->totalLinesSize.height, 0, 0);
				DebugConsoleRenderGutterBlock(console->fileNameGutterRec, fileNameBlockStr, pretendNextRec, fileNameBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
				fileNameBlockStarted = false;
			}
			if (fileLineNumBlockStarted)
			{
				rec pretendNextRec = NewRec(0, lineBasePos.y - console->totalLinesSize.height, 0, 0);
				DebugConsoleRenderGutterBlock(console->fileLineNumGutterRec, TempPrintStr(":%llu", fileLineNumBlockValue), pretendNextRec, fileLineNumBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
				fileLineNumBlockStarted = false;
			}
			if (funcNameBlockStarted)
			{
				rec pretendNextRec = NewRec(0, lineBasePos.y - console->totalLinesSize.height, 0, 0);
				DebugConsoleRenderGutterBlock(console->funcNameGutterRec, funcNameBlockStr, pretendNextRec, funcNameBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
				funcNameBlockStarted = false;
			}
			if (timeBlockStarted)
			{
				RealTime_t lineRealTime = {};
				ConvertTimestampToRealTime(timeBlockValue, &lineRealTime, pigIn->localTimezoneDoesDst);
				MyStr_t timeBlockStr = FormatRealTime(&lineRealTime, TempArena, false, true, false);
				rec pretendNextRec = NewRec(0, lineBasePos.y - console->totalLinesSize.height, 0, 0);
				DebugConsoleRenderGutterBlock(console->timeGutterRec, timeBlockStr, pretendNextRec, timeBlockStartY, console->viewRec, gutterColor, gutterTextColor, gutterTextOffsetY);
				timeBlockStarted = false;
			}
		}
		
		// +==============================+
		// |       Render Scrollbar       |
		// +==============================+
		if (!console->overlayMode)
		{
			RcDrawRectangle(RecDeflateY(console->scrollBarRec, 1), ColorTransparent(MonokaiWhite, console->alphaAmount*0.7f));
			RcDrawRectangle(RecDeflateX(console->scrollBarRec, 1), ColorTransparent(MonokaiWhite, console->alphaAmount*0.7f));
		}
		
		// +==============================+
		// |    Render Jump To End Btn    |
		// +==============================+
		if (console->overlayMode)
		{
			RcDrawText(DBG_CONSOLE_REFOCUS_STR, console->jumpToEndBtnTextPos, ColorTransparent(MonokaiWhite, IsMouseOverNamed("DebugConsoleJumpToEndBtn") ? 1.0f : console->alphaAmount));
		}
		else if (!console->followingEnd)
		{
			r32 gradientAlpha = IsMouseOverNamed("DebugConsoleJumpToEndBtn") ? 0.5f : 0.3f;
			rec whiteGradientRec;
			RecLayoutBottomPortionOf(&whiteGradientRec, &console->viewRec, console->jumpToEndBtnRec.height*1.5f);
			RcDrawGradientRectangle(whiteGradientRec, false, TransparentWhite, ColorTransparent(MonokaiWhite, gradientAlpha));
			
			RcDrawText(DBG_CONSOLE_JUMP_TO_END_STR, console->jumpToEndBtnTextPos, ColorTransparent(MonokaiWhite, console->alphaAmount));
		}
		
		RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
		
		// +==============================+
		// |  Render Autocomplete Window  |
		// +==============================+
		if (console->autocompleteOpenAmount > 0.0f && !console->overlayMode)
		{
			RcDrawRectangle(console->autocompleteRec, ColorTransparent(Black, 0.75f));
			
			RcSetViewport(RecDeflateY(console->autocompleteRec, DBG_CONSOLE_AUTOCOMPLETE_ITEM_MARGIN));
			RcBindFont(&pig->resources.fonts->debug, SelectFontFace(18));
			v2 itemsOffset = console->autocompleteRec.topLeft + NewVec2(0, -console->autocompleteScroll);
			Vec2Align(&itemsOffset);
			VarArrayLoop(&console->autocompleteItems, iIndex)
			{
				VarArrayLoopGet(DebugConsoleAutocompleteItem_t, item, &console->autocompleteItems, iIndex);
				rec mainRec = item->mainRec + itemsOffset;
				v2 displayTextPos = item->displayTextPos + itemsOffset;
				if (RecsIntersect(mainRec, console->autocompleteRec))
				{
					Color_t textColor = MonokaiWhite;
					Color_t backColor = Transparent;
					bool isSelected = (console->autocompleteSelectionIndex >= 0 && (u64)console->autocompleteSelectionIndex == iIndex);
					bool isHovered = IsMouseOverPrint("DebugConsoleAutocompleteItem%llu", iIndex);
					if (isSelected) { textColor = MonokaiBack; backColor = ColorTransparent(MonokaiWhite, 0.7f); }
					else if (isHovered) { backColor = ColorTransparent(MonokaiWhite, 0.25f); }
					if (backColor.a > 0) { RcDrawRectangle(mainRec, backColor); }
					RcDrawText(item->displayText, displayTextPos, textColor);
				}
			}
			RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
		}
		
		// +==============================+
		// |    Render Toggle Buttons     |
		// +==============================+
		if (!console->overlayMode)
		{
			RcBindShader(&pig->resources.shaders->roundedCorners);
			RcDrawRoundedRectangle(console->closeBtnRec,            10, ColorTransparent(MonokaiMagenta, IsMouseOverNamed("DebugConsoleCloseBtn") ? console->alphaAmount : console->alphaAmount*0.6f), false);
			RcDrawRoundedRectangle(console->toggleGutterBtnRec,      7, ColorTransparent(console->gutterEnabled            ? MonokaiWhite : MonokaiGray1, console->alphaAmount), false);
			RcDrawRoundedRectangle(console->toggleFileNameBtnRec,    7, ColorTransparent(console->fileNameGutterEnabled    ? MonokaiWhite : MonokaiGray1, console->alphaAmount), false);
			RcDrawRoundedRectangle(console->toggleFileLineNumBtnRec, 7, ColorTransparent(console->fileLineNumGutterEnabled ? MonokaiWhite : MonokaiGray1, console->alphaAmount), false);
			RcDrawRoundedRectangle(console->toggleFuncNameBtnRec,    7, ColorTransparent(console->funcNameGutterEnabled    ? MonokaiWhite : MonokaiGray1, console->alphaAmount), false);
			RcDrawRoundedRectangle(console->toggleTimeBtnRec,        7, ColorTransparent(console->timeGutterEnabled        ? MonokaiWhite : MonokaiGray1, console->alphaAmount), false);
			RcDrawRoundedRectangle(console->exportBtnRec,            7, ColorTransparent(MonokaiGreen, IsMouseOverNamed("DebugConsoleExportBtn") ? console->alphaAmount : console->alphaAmount*0.6f), false);
			
			RcBindShader(&pig->resources.shaders->main2D);
			RcBindSpriteSheet(&pig->resources.sheets->vectorIcons64);
			RcDrawSheetFrame(NewStr("CrossIcon"), RecDeflate(console->closeBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("PoundIcon"), RecDeflate(console->toggleGutterBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("FileIcon"), RecDeflate(console->toggleFileNameBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("ColonPoundIcon"), RecDeflate(console->toggleFileLineNumBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("ParensIcon"), RecDeflate(console->toggleFuncNameBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("ClockIcon"), RecDeflate(console->toggleTimeBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
			RcDrawSheetFrame(NewStr("ExportIcon"), RecDeflate(console->exportBtnRec, DBG_CONSOLE_ICON_SMALLER_AMOUNT, DBG_CONSOLE_ICON_SMALLER_AMOUNT), MonokaiDarkGray);
		}
		
		// +===============================+
		// | Render Full Alpha Text Button |
		// +===============================+
		if (console->state != DbgConsoleState_Closed && console->overlayMode)
		{
			Color_t iconColor = ColorTransparent(White, IsMouseOverNamed("DebugConsoleFullAlphaTextBtn") ? 1.0f : 0.5f);
			RcBindSpriteSheet(&pig->resources.sheets->vectorIcons64);
			RcDrawSheetFrame(NewStr("EyeIcon"), console->fullAlphaTextBtnRec, iconColor);
		}
	}
}
