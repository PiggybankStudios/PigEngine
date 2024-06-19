/*
File:   pig_debug_console.h
Author: Taylor Robbins
Date:   01\01\2022
*/

#ifndef _PIG_DEBUG_CONSOLE_H
#define _PIG_DEBUG_CONSOLE_H

#define DBG_CONSOLE_BUFFER_SIZE       Kilobytes(128)
#define DBG_CONSOLE_BUILD_SPACE_SIZE  Kilobytes(4)

struct DebugConsoleTextPos_t
{
	u64 lineIndex; //index from lastLine (most recent) going backwards. Get's incremented whenever a new line is added
	u64 byteIndex;
};

//NOTE: Because our lineIndex goes from bottom (0) to top (+n) we actually flip the polarity of what it means
//      to be the "greater" value since text normally reads from top-down. The "greater" value is one that has
//      a lower lineIndex value.
i32 DebugConsoleTextPosCompare(DebugConsoleTextPos_t textPosLeft, DebugConsoleTextPos_t textPosRight)
{
	if (textPosLeft.lineIndex < textPosRight.lineIndex) { return 1; }
	else if (textPosLeft.lineIndex > textPosRight.lineIndex) { return -1; }
	else
	{
		if (textPosLeft.byteIndex < textPosRight.byteIndex) { return -1; }
		else if (textPosLeft.byteIndex > textPosRight.byteIndex) { return 1; }
		else { return 0; }
	}
}

struct DebugConsoleSelectionRec_t
{
	DbgLevel_t dbgLevel;
	DebugConsoleTextPos_t startPos;
	DebugConsoleTextPos_t endPos;
	rec mainRec;
};

#define DEBUG_COMMAND_MAX_NUM_ARGUMENTS 8

struct DebugConsoleRegisteredCommand_t
{
	u64 numArguments;
	MyStr_t command;
	MyStr_t description;
	MyStr_t arguments[DEBUG_COMMAND_MAX_NUM_ARGUMENTS];
};

struct DebugConsoleAutocompleteItem_t
{
	u64 index;
	MyStr_t command;
	MyStr_t displayText;
	TextMeasure_t displayTextMeasure;
	rec mainRec;
	v2 displayTextPos;
};

struct DebugConsoleLine_t
{
	u8 flags;
	u64 programTime;
	u64 preciseProgramTime;
	u64 timestamp;
	ThreadId_t thread;
	u64 fileLineNumber;
	DbgLevel_t dbgLevel;
	
	rec mainRec;
	v2 textPos;
	v2 textSize;
	v2 gutterNumSize;
	v2 fileNameSize;
	v2 fileLineNumSize;
	v2 funcNameSize;
};

enum DbgConsoleState_t
{
	DbgConsoleState_Closed = 0,
	DbgConsoleState_Open,
	DbgConsoleState_OpenLarge,
	DbgConsoleState_NumStates,
};

struct DebugConsole_t
{
	DbgConsoleState_t state;
	bool overlayMode;
	bool fullAlphaText;
	r32 openAmount;
	r32 alphaAmount;
	
	bool hasFifo;
	PlatMutex_t fifoMutex;
	StringFifo_t fifo;
	
	bool gutterEnabled;
	bool fileNameGutterEnabled;
	bool fileLineNumGutterEnabled;
	bool funcNameGutterEnabled;
	bool timeGutterEnabled;
	
	VarArray_t registeredCommands; //DebugConsoleRegisteredCommand_t
	
	u64 numLinesAdded;
	bool linesNeedResize;
	v2 totalLinesSize;
	
	r32 scroll; //+ values = view is higher in the lines stack
	r32 scrollTarget;
	r32 scrollMax;
	bool followingEnd;
	
	bool mouseHovering;
	DebugConsoleTextPos_t mouseHoverTextPos;
	v2 mouseHoverPos;
	bool mouseSelecting;
	
	bool selectionActive;
	bool selectionChanged;
	DebugConsoleTextPos_t selectionStart;
	DebugConsoleTextPos_t selectionEnd;
	VarArray_t selectionRecs; //DebugConsoleSelectionRec_t
	
	bool isInputValid;
	Textbox_t inputTextbox;
	
	VarArray_t inputHistory; //MyStr_t
	u64 recallIndex;
	MyStr_t suspendedInputStr;
	
	bool autocompleteActive;
	MyStr_t autocompleteFilterStr;
	r32 autocompleteOpenAmount;
	i64 autocompleteSelectionIndex;
	bool autocompleteScrollToSelection;
	VarArray_t autocompleteItems; //DebugConsoleAutocompleteItem_t
	v2 autocompleteItemsSize;
	r32 autocompleteScroll;
	r32 autocompleteScrollGoto;
	r32 autocompleteScrollMax;
	
	bool funcSigDisplayActive;
	r32 funcSigDisplayOpenAmount;
	ExpFuncDef_t funcSigDef;
	u64 funcSigCurrentArgIndex;
	
	rec mainRec;
	rec viewRec;
	rec viewUsableRec;
	rec scrollBarRec;
	rec scrollGutterRec;
	rec gutterRec;
	rec fileNameGutterRec;
	rec fileLineNumGutterRec;
	rec funcNameGutterRec;
	rec timeGutterRec;
	rec inputLabelRec;
	v2 inputLabelTextPos;
	rec jumpToEndBtnRec;
	v2 jumpToEndBtnTextPos;
	rec inputRec;
	rec autocompleteRec;
	rec funcSigDisplayRec;
	rec closeBtnRec;
	rec toggleGutterBtnRec;
	rec toggleFileNameBtnRec;
	rec toggleFileLineNumBtnRec;
	rec toggleFuncNameBtnRec;
	rec toggleTimeBtnRec;
	rec exportBtnRec;
	rec fullAlphaTextBtnRec;
};

#endif //  _PIG_DEBUG_CONSOLE_H
