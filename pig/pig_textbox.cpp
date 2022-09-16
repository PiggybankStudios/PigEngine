/*
File:   pig_textbox.cpp
Author: Taylor Robbins
Date:   01\11\2022
Description: 
	** Holds the functions that help us update and render a Textbox_t
	** The Textbox_t handles text input from the user in single or multi-line format
*/

//TODO: Add support for scrolling the text left/right
//TODO: Add support for optional Undo Queue
//TODO: Add support for tab characters
//TODO: Add support for multi-line textbox (Up/Down arrows, scrolling up/down and autoSizeHeight too)
//TODO: Fix Textbox_MoveCursor(ByLine) so it looks for new-line characters and doesn't act identical to ByWholeText
//TODO: Add some sort of "operation invalid" indication to the user. Like when you try and type too many characters into a limited buffer textbox, or backspace while at the beginning
//TODO: Add better handling of InputEvent_t structures so our related Handled flags are set when working with InputEvents and vice-versa
//TODO: All our Key checks that care about modifier keys should check the modifier bit values on the relavant InputEvent_t
//TODO: Make copy while empty selection only copy a single line, rather than whole text
//TODO: Figure out how we should be able to change textbox colors
//TODO: Add support for editable flag restricting interaction when not editable
//TODO: Add support for Ctrl+Backspace and Ctrl+Delete to delete whole words (Also ctrl+shift+backspace?)
//TODO: Handle Shift being held when left clicking to move the selection

// +--------------------------------------------------------------+
// |                       Create and Free                        |
// +--------------------------------------------------------------+
void FreeTextbox(Textbox_t* tb)
{
	NotNull(tb);
	if (tb->hintText.pntr != nullptr)
	{
		NotNull(tb->allocArena);
		FreeString(tb->allocArena, &tb->hintText);
	}
	if (tb->text.pntr != nullptr)
	{
		NotNull(tb->allocArena);
		Assert(tb->textAllocLength > 0);
		FreeMem(tb->allocArena, tb->text.pntr, tb->textAllocLength);
	}
	ClearPointer(tb);
}

void CreateTextbox(Textbox_t* tb, MemArena_t* memArena, u64 textBufferSize, bool fixedTextBuffer, bool isMultiline)
{
	AssertSingleThreaded();
	NotNull(tb);
	NotNull(memArena);
	AssertIf(fixedTextBuffer, textBufferSize > 0);
	ClearPointer(tb);
	tb->allocArena = memArena;
	tb->isEditable = true;
	tb->isVisible = true;
	tb->isMultiline = isMultiline;
	tb->isFixedSizedBuffer = fixedTextBuffer;
	tb->hintText = MyStr_Empty;
	tb->font = GetFontHandle(&pig->resources.fonts->debug);
	tb->fontFaceSelector = SelectDefaultFontFace();
	tb->fontScale = 1.0f;
	tb->selectionActive = false;
	tb->mouseHovering = false;
	tb->isSelectDragging = false;
	tb->autoSizeHeight = !isMultiline;
	tb->autoSelectOnFocus = false;
	tb->innerMargin = NewVec2(4, 4);
	tb->hasTargetRec = false;
	tb->needToRemeasure = true;
	tb->textChanged = false;
	tb->targetRecMoved = false;
	tb->lastSelectionMoveTime = 0;
	
	tb->id = pig->nextUiId;
	pig->nextUiId++;
	
	tb->textAllocLength = textBufferSize;
	tb->text.length = 0;
	if (textBufferSize > 0)
	{
		tb->text.pntr = AllocArray(memArena, char, textBufferSize);
		tb->text.pntr[0] = '\0';
	}
	else
	{
		tb->text.pntr = nullptr;
	}
}

// +--------------------------------------------------------------+
// |                  Internal Helper Functions                   |
// +--------------------------------------------------------------+
bool Textbox_MakeSpaceForBytes(Textbox_t* tb, u64 numNewBytes)
{
	if (tb->text.length + numNewBytes < tb->textAllocLength) { return true; } //nothing to do, already enough space
	if (tb->isFixedSizedBuffer) { return false; }
	
	NotNull(tb->allocArena);
	u64 newSpaceSize = tb->textAllocLength;
	if (newSpaceSize == 0) { newSpaceSize = 2; }
	while (newSpaceSize <= tb->text.length + numNewBytes) { newSpaceSize *= 2; }
	if (tb->textAllocLength > 0)
	{
		tb->text.pntr = (char*)ReallocMem(tb->allocArena, tb->text.pntr, newSpaceSize, tb->textAllocLength);
	}
	else
	{
		tb->text.pntr = AllocArray(tb->allocArena, char, newSpaceSize);
	}
	NotNull(tb->text.pntr);
	tb->text.pntr[tb->text.length] = '\0';
	tb->textAllocLength = newSpaceSize;
	return true;
}

bool Textbox_RemoveCharacters(Textbox_t* tb, bool forwardRemove)
{
	bool result = false;
	u64 removeStartIndex = 0;
	u64 removeNumBytes = 0;
	
	if (!tb->selectionActive)
	{
		//If there is no selection, set the cursor to be at the end of the text
		tb->selectionActive = true;
		tb->selectionStartIndex = tb->text.length;
		tb->selectionEndIndex = tb->selectionStartIndex;
		tb->needToRemeasure = true;
	}
	
	if (tb->selectionStartIndex != tb->selectionEndIndex)
	{
		removeStartIndex = MinU64(tb->selectionStartIndex, tb->selectionEndIndex);
		removeNumBytes = MaxU64(tb->selectionStartIndex, tb->selectionEndIndex) - removeStartIndex;
	}
	else if (forwardRemove && tb->selectionEndIndex < tb->text.length)
	{
		u32 nextCodepoint = 0;
		u8 nextCodepointByteSize = GetCodepointForUtf8Str(tb->text, tb->selectionEndIndex, &nextCodepoint);
		if (nextCodepointByteSize == 0) { nextCodepointByteSize = 1; }
		removeStartIndex = tb->selectionEndIndex;
		removeNumBytes = nextCodepointByteSize;
	}
	else if (!forwardRemove && tb->selectionEndIndex > 0)
	{
		u32 prevCodepoint = 0;
		u8 prevCodepointByteSize = GetCodepointBeforeIndex(tb->text.pntr, tb->selectionEndIndex, &prevCodepoint);
		if (prevCodepointByteSize == 0) { prevCodepointByteSize = 1; }
		removeStartIndex = tb->selectionEndIndex - prevCodepointByteSize;
		removeNumBytes = prevCodepointByteSize;
	}
	
	if (removeNumBytes > 0)
	{
		for (u64 bIndex = removeStartIndex + removeNumBytes; bIndex < tb->text.length; bIndex++)
		{
			tb->text.pntr[bIndex - removeNumBytes] = tb->text.pntr[bIndex];
		}
		tb->text.length -= removeNumBytes;
		tb->text.pntr[tb->text.length] = '\0';
		tb->selectionStartIndex = removeStartIndex;
		tb->selectionEndIndex = removeStartIndex;
		tb->needToRemeasure = true;
		tb->textChanged = true;
		result = true;
	}
	
	return result;
}

bool Textbox_CharactersTyped(Textbox_t* tb, MyStr_t newCharacters)
{
	NotNullStr(&newCharacters);
	AssertIf(tb->selectionActive, tb->selectionStartIndex >= 0 && tb->selectionStartIndex <= tb->text.length);
	AssertIf(tb->selectionActive, tb->selectionEndIndex >= 0 && tb->selectionEndIndex <= tb->text.length);
	if (!tb->selectionActive)
	{
		//if the selection isn't active. Make it active and at the end of the text
		tb->selectionActive = true;
		tb->selectionStartIndex = tb->text.length;
		tb->selectionEndIndex = tb->text.length;
	}
	if (tb->selectionStartIndex != tb->selectionEndIndex)
	{
		//if there is a selection, delete the items selected before inserting the typed character(s)
		Textbox_RemoveCharacters(tb, true);
	}
	
	bool result = false;
	if (newCharacters.length > 0)
	{
		if (Textbox_MakeSpaceForBytes(tb, newCharacters.length))
		{
			//Shift bytes after endIndex up by the size of the new characters coming in
			for (u64 bIndex = tb->text.length; bIndex > tb->selectionEndIndex; bIndex--)
			{
				tb->text.pntr[bIndex-1 + newCharacters.length] = tb->text.pntr[bIndex-1];
			}
			MyMemCopy(&tb->text.pntr[tb->selectionEndIndex], newCharacters.pntr, newCharacters.length);
			tb->text.length += newCharacters.length;
			tb->text.pntr[tb->text.length] = '\0';
			tb->selectionEndIndex += newCharacters.length;
			tb->selectionStartIndex = tb->selectionEndIndex;
			result = true;
		}
	}
	else { result = true; }
	
	tb->needToRemeasure = true;
	tb->textChanged = true;
	return result;
}

enum Textbox_MoveCursorType_t
{
	Textbox_MoveCursorType_ByCharacter = 0,
	Textbox_MoveCursorType_ByWord,
	Textbox_MoveCursorType_BySubword,
	Textbox_MoveCursorType_ByLine,
	Textbox_MoveCursorType_ByWholeText,
	Textbox_MoveCursorType_NumTypes,
};
bool Textbox_MoveCursor(Textbox_t* tb, bool forward, bool extendSelection, Textbox_MoveCursorType_t type)
{
	bool result = false;
	switch (type)
	{
		case Textbox_MoveCursorType_ByCharacter:
		{
			if (!tb->selectionActive)
			{
				tb->selectionActive = true;
				tb->selectionEndIndex = (forward ? 0 : tb->text.length);
				tb->selectionStartIndex = tb->selectionEndIndex;
				result = true;
			}
			else if (tb->selectionStartIndex != tb->selectionEndIndex && !extendSelection)
			{
				tb->selectionEndIndex = (forward ? MaxI64(tb->selectionStartIndex, tb->selectionEndIndex) : MinI64(tb->selectionStartIndex, tb->selectionEndIndex));
				result = true;
			}
			else if (forward && tb->selectionEndIndex < tb->text.length)
			{
				u32 nextCodepoint = 0;
				u8 nextCodepointByteSize = GetCodepointForUtf8Str(tb->text, tb->selectionEndIndex, &nextCodepoint);
				if (nextCodepointByteSize == 0) { nextCodepointByteSize = 1; }
				tb->selectionEndIndex += nextCodepointByteSize;
				result = true;
			}
			else if (!forward && tb->selectionEndIndex > 0)
			{
				u32 prevCodepoint = 0;
				u8 prevCodepointByteSize = GetCodepointBeforeIndex(tb->text.pntr, tb->selectionEndIndex, &prevCodepoint);
				if (prevCodepointByteSize == 0) { prevCodepointByteSize = 1; }
				tb->selectionEndIndex -= prevCodepointByteSize;
				result = true;
			}
		} break;
		case Textbox_MoveCursorType_ByWord:
		case Textbox_MoveCursorType_BySubword:
		{
			if (!tb->selectionActive)
			{
				tb->selectionEndIndex = (forward ? 0 : tb->text.length);
				tb->selectionStartIndex = tb->selectionEndIndex;
				result = true;
			}
			u64 nextWordBreak = FindNextWordBreakInString(tb->text, tb->selectionEndIndex, forward, (type == Textbox_MoveCursorType_BySubword));
			if (nextWordBreak != tb->selectionEndIndex || (!extendSelection && tb->selectionStartIndex != tb->selectionEndIndex))
			{
				tb->selectionEndIndex = nextWordBreak;
				result = true;
			}
		} break;
		case Textbox_MoveCursorType_ByLine:
		{
			if (!tb->selectionActive)
			{
				tb->selectionActive = true;
				tb->selectionEndIndex = (forward ? 0 : tb->text.length);
				tb->selectionStartIndex = tb->selectionEndIndex;
				result = true;
			}
			else if (forward && (tb->selectionEndIndex < tb->text.length || (!extendSelection && tb->selectionStartIndex != tb->selectionEndIndex)))
			{
				tb->selectionEndIndex = tb->text.length; //TODO: This should look forward or back for the next occurance of a line break character (or line wrap?)
				result = true;
			}
			else if (!forward && (tb->selectionEndIndex > 0 || (!extendSelection && tb->selectionStartIndex != tb->selectionEndIndex)))
			{
				tb->selectionEndIndex = 0; //TODO: This should look forward or back for the next occurance of a line break character (or line wrap?)
				result = true;
			}
		} break;
		case Textbox_MoveCursorType_ByWholeText:
		{
			if (!tb->selectionActive)
			{
				tb->selectionActive = true;
				tb->selectionEndIndex = (forward ? 0 : tb->text.length);
				tb->selectionStartIndex = tb->selectionEndIndex;
				result = true;
			}
			else if (forward && (tb->selectionEndIndex < tb->text.length || (!extendSelection && tb->selectionStartIndex != tb->selectionEndIndex)))
			{
				tb->selectionEndIndex = tb->text.length;
				result = true;
			}
			else if (!forward && (tb->selectionEndIndex > 0 || (!extendSelection && tb->selectionStartIndex != tb->selectionEndIndex)))
			{
				tb->selectionEndIndex = 0;
				result = true;
			}
		} break;
		default: Unimplemented(); break;
	}
	
	if (result)
	{
		tb->needToRemeasure = true;
		if (!extendSelection) { tb->selectionStartIndex = tb->selectionEndIndex; }
	}
	return result;
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void SetTextboxHintText(Textbox_t* tb, MyStr_t hintText)
{
	NotNull(tb);
	NotNull(tb->allocArena);
	FreeString(tb->allocArena, &tb->hintText);
	tb->hintText = AllocString(tb->allocArena, &hintText);
}

void SetTextboxFont(Textbox_t* tb, Font_t* font, FontFaceSelector_t faceSelector)
{
	NotNull(tb);
	NotNull(tb->allocArena);
	NotNull(font);
	tb->font = GetFontHandle(font);
	tb->fontFaceSelector = faceSelector;
	tb->needToRemeasure = true;
}

void TextboxMove(Textbox_t* tb, rec targetRec)
{
	NotNull(tb);
	tb->hasTargetRec = true;
	if (!RecBasicallyEqual(tb->targetRec, targetRec))
	{
		tb->targetRec = targetRec;
		tb->targetRecMoved = true;
	}
}

bool IsMouseOverTextbox(Textbox_t* tb)
{
	NotNull(tb);
	return IsMouseOverPrint("Textbox%llu", tb->id);
}

r32 TextboxGetAutoHeight(Textbox_t* tb)
{
	NotNull(tb);
	const FontFace_t* fontFace = GetFontFace(GetPointer(&tb->font), tb->fontFaceSelector);
	r32 fontLineHeight = (fontFace != nullptr) ? (fontFace->lineHeight * tb->fontScale) : 0;
	return (fontLineHeight + tb->innerMargin.y*2);
}

MyStr_t TextboxGetSelectedText(Textbox_t* tb, bool giveWholeTextIfEmptySelection = false)
{
	NotNull(tb);
	MyStr_t result = MyStr_Empty;
	if (tb->selectionActive)
	{
		if (tb->selectionStartIndex != tb->selectionEndIndex)
		{
			u64 selectionMinIndex = MinU64(tb->selectionStartIndex, tb->selectionEndIndex);
			u64 selectionMaxIndex = MaxU64(tb->selectionStartIndex, tb->selectionEndIndex);
			result = StrSubstring(&tb->text, selectionMinIndex, selectionMaxIndex);
		}
		else if (giveWholeTextIfEmptySelection)
		{
			result = tb->text;
		}
	}
	return result;
}

void FocusTextbox(Textbox_t* tb)
{
	NotNull(tb);
	if (!IsFocused(tb))
	{
		FocusItemPrint(tb, "Textbox%llu", tb->id);
		pig->isFocusedItemTyping = true;
		if (tb->autoSelectOnFocus)
		{
			tb->selectionActive = true;
			tb->selectionStartIndex = 0;
			tb->selectionEndIndex = tb->text.length;
			tb->needToRemeasure = true;
		}
	}
}

void TextboxClearSelection(Textbox_t* tb)
{
	NotNull(tb);
	tb->selectionActive = false;
	tb->needToRemeasure = true;
	tb->textChanged = true;
}

void TextboxSetText(Textbox_t* tb, MyStr_t newText)
{
	NotNull(tb);
	NotNullStr(&newText);
	if (!StrEquals(tb->text, newText))
	{
		if (newText.length > tb->text.length)
		{
			Textbox_MakeSpaceForBytes(tb, newText.length - tb->text.length);
		}
		if (newText.length > 0)
		{
			MyMemCopy(tb->text.pntr, newText.pntr, newText.length);
		}
		tb->text.length = newText.length;
		if (tb->textAllocLength > 0) { tb->text.pntr[tb->text.length] = '\0'; }
		
		if (tb->selectionActive)
		{
			tb->selectionStartIndex = tb->text.length;
			tb->selectionEndIndex = tb->selectionStartIndex;
		}
		
		tb->needToRemeasure = true;
		tb->textChanged = true;
	}
}

// +--------------------------------------------------------------+
// |                     Text Flow Callbacks                      |
// +--------------------------------------------------------------+
struct Textbox_FontFlowContext_t
{
	Textbox_t* tb;
	bool foundStartPos;
	bool foundEndPos;
};
//void TextBox_FontFlowBetweenCharCallback(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
FFCB_BETWEEN_CHAR_DEFINITION(TextBox_FontFlowBetweenCharCallback) // | TextBox_FontFlowBetweenCharCallback |
{
	UNUSED(charIndex);
	UNUSED(state);
	NotNull(context);
	Textbox_FontFlowContext_t* contextPntr = (Textbox_FontFlowContext_t*)context;
	Textbox_t* tb = contextPntr->tb;
	NotNull(tb);
	if (tb->selectionActive)
	{
		if (!contextPntr->foundStartPos && byteIndex == tb->selectionStartIndex)
		{
			tb->selectionStartPos = position;
			contextPntr->foundStartPos = true;
		}
		if (!contextPntr->foundEndPos && byteIndex == tb->selectionEndIndex)
		{
			tb->selectionEndPos = position;
			contextPntr->foundEndPos = true;
		}
	}
}

// +--------------------------------------------------------------+
// |                   Layout and Mouse Capture                   |
// +--------------------------------------------------------------+
void TextboxLayout(Textbox_t* tb)
{
	NotNull(tb);
	AssertFilledHandle(&tb->font);
	
	if (tb->needToRemeasure)
	{
		AssertIf(tb->selectionActive, tb->selectionStartIndex >= 0 && tb->selectionStartIndex <= tb->text.length);
		AssertIf(tb->selectionActive, tb->selectionEndIndex >= 0 && tb->selectionEndIndex <= tb->text.length);
		Textbox_FontFlowContext_t context = {};
		context.tb = tb;
		context.foundStartPos = false;
		context.foundEndPos = false;
		FontFlowCallbacks_t flowCallbacks = {};
		flowCallbacks.context = &context;
		flowCallbacks.betweenChar = TextBox_FontFlowBetweenCharCallback;
		tb->textMeasure = MeasureTextInFont(tb->text, GetPointer(&tb->font), tb->fontFaceSelector, tb->fontScale, 0.0f, &tb->flowInfo, &flowCallbacks);
		if (tb->text.length == 0) { tb->selectionEndPos = Vec2_Zero; tb->selectionStartPos = Vec2_Zero; }
		else if (tb->selectionActive) { Assert(context.foundStartPos); Assert(context.foundEndPos); }
		tb->needToRemeasure = false;
	}
	
	if (tb->autoSizeHeight)
	{
		tb->mainRec.height = TextboxGetAutoHeight(tb);
	}
	
	if (tb->hasTargetRec)
	{
		tb->mainRec.topLeft = tb->targetRec.topLeft;
		tb->mainRec.width = tb->targetRec.width;
		if (!tb->autoSizeHeight)
		{
			tb->mainRec.height = tb->targetRec.height;
		}
	}
	
	tb->usableRec = RecDeflate(tb->mainRec, tb->innerMargin);
	
	const FontFace_t* fontFace = GetFontFace(GetPointer(&tb->font), tb->fontFaceSelector);
	r32 fontMaxAscend = (fontFace != nullptr) ? (fontFace->maxAscend * tb->fontScale) : 0;
	tb->textPos = tb->mainRec.topLeft + tb->innerMargin + NewVec2(0, fontMaxAscend);
	Vec2Align(&tb->textPos);
}

void TextboxCaptureMouse(Textbox_t* tb)
{
	NotNull(tb);
	TextboxLayout(tb);
	if (tb->isVisible)
	{
		MouseHitRecPrint(tb->mainRec, "Textbox%llu", tb->id);
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateTextbox(Textbox_t* tb)
{
	NotNull(tb);
	TextboxLayout(tb);
	bool oldSelectionActive = tb->selectionActive;
	u64 oldSelectionStartIndex = tb->selectionStartIndex;
	u64 oldSelectionEndIndex = tb->selectionEndIndex;
	
	// +==============================+
	// | Handle Outside Click Unfocus |
	// +==============================+
	if (IsFocused(tb) && MousePressedRaw(MouseBtn_Left) && !IsMouseOverTextbox(tb))
	{
		if (tb->skipNextUnfocusClick)
		{
			tb->skipNextUnfocusClick = false;
		}
		else
		{
			ClearFocus();
		}
	}
	
	// +==============================+
	// |        Handle Unfocus        |
	// +==============================+
	if (WasUnfocused(tb))
	{
		tb->isSelectDragging = false;
		if (tb->autoSelectOnFocus) { TextboxClearSelection(tb); }
	}
	
	// +==============================+
	// |  Handle Mouse Interactions   |
	// +==============================+
	//TODO: Once we have a focusing system, redirect some mouse interactions to focus/unfocus the textbox
	if (IsMouseOverTextbox(tb) || tb->isSelectDragging)
	{
		pigOut->cursorType = PlatCursor_TextIBeam;
		
		tb->mouseHovering = true;
		i64 findCursorResult = FindCursorIndexInFlowedText(tb->text, GetPointer(&tb->font), tb->fontFaceSelector, MousePos - tb->textPos, tb->fontScale, 0.0f, &tb->mouseHoverPos);
		if (findCursorResult >= 0)
		{
			tb->mouseHoverIndex = (u64)findCursorResult;
			//mouseHoverPos was filled by FindCursorIndexInFlowedText
		}
		else
		{
			MyDebugBreak(); //really this shouldn't happen
			tb->mouseHoverIndex = 0;
			tb->mouseHoverPos = Vec2_Zero;
		}
		
		// +==============================+
		// |   Left Click Interactions    |
		// +==============================+
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			
			bool mouseClickHandled = false;
			if (!IsFocused(tb))
			{
				FocusTextbox(tb);
				if (tb->autoSelectOnFocus)
				{
					HandleMouseExtended(MouseBtn_Left);
					mouseClickHandled = true;
				}
			}
			
			if (!mouseClickHandled)
			{
				bool isQuickClick = (TimeSince(tb->lastMouseLeftClickTime) <= PIG_DOUBLE_CLICK_TIME);
				bool isClickInSamePos = (Vec2Length(tb->lastMouseLeftClickPos - MousePos) <= PIG_DOUBLE_CLICK_MAX_DIST);
				tb->lastMouseLeftClickTime = ProgramTime;
				tb->lastMouseLeftClickPos = MousePos;
				if (!isQuickClick || !isClickInSamePos) { tb->mouseQuickLeftClickCount = 0; }
				tb->mouseQuickLeftClickCount++;
				
				// +==============================+
				// |      Triple Left Click       |
				// +==============================+
				if (tb->mouseQuickLeftClickCount >= 3)
				{
					if (!tb->selectionActive || tb->selectionStartIndex != 0 || tb->selectionEndIndex != tb->text.length)
					{
						tb->selectionStartIndex = 0;
						tb->selectionEndIndex = tb->text.length;
						tb->selectionActive = true;
						tb->needToRemeasure = true;
					}
					tb->mouseQuickLeftClickCount = 0;
				}
				// +==============================+
				// |      Double Left Click       |
				// +==============================+
				else if (tb->mouseQuickLeftClickCount >= 2)
				{
					bool subwords = (KeyDownRaw(Key_Alt));
					u64 wordStartIndex = FindNextWordBreakInString(tb->text, tb->mouseHoverIndex, false, subwords, true);
					u64 wordEndIndex = FindNextWordBreakInString(tb->text, tb->mouseHoverIndex, true, subwords, true);
					if (!tb->selectionActive || tb->selectionStartIndex != wordStartIndex || tb->selectionEndIndex != wordEndIndex)
					{
						tb->selectionStartIndex = wordStartIndex;
						tb->selectionEndIndex = wordEndIndex;
						tb->selectionActive = true;
						tb->needToRemeasure = true;
					}
				}
				// +==============================+
				// |      Single Left Click       |
				// +==============================+
				else
				{
					tb->selectionStartIndex = tb->mouseHoverIndex;
					tb->selectionEndIndex = tb->mouseHoverIndex;
					tb->selectionActive = true;
					tb->isSelectDragging = true;
					tb->needToRemeasure = true;
				}
			}
		}
		
		// +==============================+
		// |       Left Mouse Drag        |
		// +==============================+
		if (MouseDownRaw(MouseBtn_Left) && IsFocused(tb))
		{
			HandleMouse(MouseBtn_Left);
			if (tb->isSelectDragging && tb->selectionEndIndex != tb->mouseHoverIndex)
			{
				tb->selectionEndIndex = tb->mouseHoverIndex;
				tb->needToRemeasure = true;
			}
		}
		else if (tb->isSelectDragging && IsFocused(tb))
		{
			if (MouseReleasedRaw(MouseBtn_Left)) { HandleMouse(MouseBtn_Left); }
			tb->isSelectDragging = false;
		}
		
		#if 0
		// +==============================+
		// |   Right Click Interactions   |
		// +==============================+
		if (MousePressed(MouseBtn_Right) && IsFocused(tb))
		{
			HandleMouseExtended(MouseBtn_Right);
			bool isQuickClick = (TimeSince(tb->lastMouseRightClickTime) <= PIG_DOUBLE_CLICK_TIME);
			bool isClickInSamePos = (Vec2Length(tb->lastMouseRightClickPos - MousePos) <= PIG_DOUBLE_CLICK_MAX_DIST);
			tb->lastMouseRightClickTime = ProgramTime;
			tb->lastMouseRightClickPos = MousePos;
			if (!isQuickClick || !isClickInSamePos) { tb->mouseQuickRightClickCount = 0; }
			tb->mouseQuickRightClickCount++;
			
			//TODO: Do we need all this triple, double, single click stuff for right click?
			// +==============================+
			// |      Triple Right Click      |
			// +==============================+
			if (tb->mouseQuickRightClickCount >= 3)
			{
				
			}
			// +==============================+
			// |      Double Right Click      |
			// +==============================+
			else if (tb->mouseQuickRightClickCount >= 2)
			{
				
			}
			// +==============================+
			// |      Single Right Click      |
			// +==============================+
			else
			{
				
			}
		}
		#endif
	}
	
	// +==============================+
	// |   Handle Character Inputs    |
	// +==============================+
	if (IsFocused(tb))
	{
		VarArrayLoop(&pigIn->inputEvents, eIndex)
		{
			VarArrayLoopGet(InputEvent_t, inputEvent, &pigIn->inputEvents, eIndex);
			if (!CheckIfInputEventWasHandled(inputEvent))
			{
				if (inputEvent->type == InputEventType_Character)
				{
					u8 codepointBytes[UTF8_MAX_CHAR_SIZE];
					u8 codepointSize = GetUtf8BytesForCode(inputEvent->character.codepoint, &codepointBytes[0]);
					if (codepointSize > 0)
					{
						MyStr_t codepointStr = NewStr(codepointSize, (char*)&codepointBytes[0]);
						if (!Textbox_CharactersTyped(tb, codepointStr))
						{
							//TODO: Should we somehow let the user know? Maybe a sound effect?
						}
						
						HandleInputEvent(inputEvent);
					}
					else
					{
						//TODO: Should we somehow let the user know that they tried to type a character that can't be encoded in UTF-8? Is that even possible?
					}
				}
			}
		}
	}
	
	// +==============================+
	// |      Handle Other Keys       |
	// +==============================+
	if (IsFocused(tb))
	{
		// +==============================+
		// |        Handle Escape         |
		// +==============================+
		if (KeyPressedPlatRepeating(Key_Escape))
		{
			ClearFocus();
		}
		HandleKey(Key_Escape);
		
		// +==============================+
		// |       Handle Backspace       |
		// +==============================+
		if (KeyPressedPlatRepeating(Key_Backspace))
		{
			Textbox_RemoveCharacters(tb, false);
		}
		HandleKey(Key_Backspace);
		
		// +==============================+
		// |        Handle Delete         |
		// +==============================+
		if (KeyPressedPlatRepeating(Key_Delete))
		{
			Textbox_RemoveCharacters(tb, true);
		}
		HandleKey(Key_Delete);
		
		// +==============================+
		// |      Handle Left Arrow       |
		// +==============================+
		if (KeyPressedPlatRepeating(Key_Left))
		{
			//TODO: We should take into account the modifier keys that were held when the arrow key was pressed if we want to be super accurate (important when you have barcode scanners or other software that dumps a bunch of key events in a single frame)
			bool extendSelection = KeyDownRaw(Key_Shift);
			bool moved = false;
			if (KeyDownRaw(Key_Control)) { moved = Textbox_MoveCursor(tb, false, extendSelection, Textbox_MoveCursorType_ByWord); }
			else if (KeyDownRaw(Key_Alt)) { moved = Textbox_MoveCursor(tb, false, extendSelection, Textbox_MoveCursorType_BySubword); }
			else { moved = Textbox_MoveCursor(tb, false, extendSelection, Textbox_MoveCursorType_ByCharacter); }
			if (!moved)
			{
				//TODO: Should we let the user know? Maybe a sound effect?
			}
		}
		HandleKey(Key_Left);
		
		// +==============================+
		// |      Handle Right Arrow      |
		// +==============================+
		if (KeyPressedPlatRepeating(Key_Right))
		{
			//TODO: We should take into account the modifier keys that were held when the arrow key was pressed if we want to be super accurate (important when you have barcode scanners or other software that dumps a bunch of key events in a single frame)
			bool extendSelection = KeyDownRaw(Key_Shift);
			bool moved = false;
			if (KeyDownRaw(Key_Control)) { moved = Textbox_MoveCursor(tb, true, extendSelection, Textbox_MoveCursorType_ByWord); }
			else if (KeyDownRaw(Key_Alt)) { moved = Textbox_MoveCursor(tb, true, extendSelection, Textbox_MoveCursorType_BySubword); }
			else { moved = Textbox_MoveCursor(tb, true, extendSelection, Textbox_MoveCursorType_ByCharacter); }
			if (!moved)
			{
				//TODO: Should we let the user know? Maybe a sound effect?
			}
		}
		HandleKey(Key_Left);
		
		//TODO: We should handle Up/Down arrows for multi-line textboxes
		
		// +==============================+
		// |         Handle Home          |
		// +==============================+
		if (KeyPressed(Key_Home))
		{
			HandleKeyExtended(Key_Home);
			//TODO: We should take into account the modifier keys that were held when the arrow key was pressed if we want to be super accurate (important when you have barcode scanners or other software that dumps a bunch of key events in a single frame)
			bool extendSelection = KeyDownRaw(Key_Shift);
			bool moved = false;
			if (KeyDownRaw(Key_Control)) { moved = Textbox_MoveCursor(tb, false, extendSelection, Textbox_MoveCursorType_ByWholeText); }
			else { moved = Textbox_MoveCursor(tb, false, extendSelection, Textbox_MoveCursorType_ByLine); }
			if (!moved)
			{
				//TODO: Should we let the user know? Maybe a sound effect?
			}
		}
		
		// +==============================+
		// |          Handle End          |
		// +==============================+
		if (KeyPressed(Key_End))
		{
			HandleKeyExtended(Key_End);
			//TODO: We should take into account the modifier keys that were held when the arrow key was pressed if we want to be super accurate (important when you have barcode scanners or other software that dumps a bunch of key events in a single frame)
			bool extendSelection = KeyDownRaw(Key_Shift);
			bool moved = false;
			if (KeyDownRaw(Key_Control)) { moved = Textbox_MoveCursor(tb, true, extendSelection, Textbox_MoveCursorType_ByWholeText); }
			else { moved = Textbox_MoveCursor(tb, true, extendSelection, Textbox_MoveCursorType_ByLine); }
			if (!moved)
			{
				//TODO: Should we let the user know? Maybe a sound effect?
			}
		}
		
		// +==============================+
		// |        Handle Ctrl+A         |
		// +==============================+
		if (KeyPressed(Key_A) && KeyDownRaw(Key_Control))
		{
			HandleKeyExtended(Key_A);
			if (!tb->selectionActive || tb->selectionStartIndex != 0 || tb->selectionEndIndex != tb->text.length)
			{
				tb->selectionActive = true;
				tb->selectionStartIndex = 0;
				tb->selectionEndIndex = tb->text.length;
				tb->needToRemeasure = true;
			}
			else
			{
				tb->selectionActive = false;
			}
		}
		
		// +==============================+
		// |      Handle Ctrl+C Copy      |
		// +==============================+
		if (KeyPressed(Key_C) && KeyDownRaw(Key_Control))
		{
			HandleKeyExtended(Key_C);
			if (tb->selectionActive)
			{
				MyStr_t selectedText = TextboxGetSelectedText(tb, true);
				if (!IsEmptyStr(selectedText))
				{
					PrintLine_D("Putting \"%.*s\" in the clipboard", selectedText.length, selectedText.pntr);
					if (!plat->CopyTextToClipboard(selectedText))
					{
						NotifyPrint_E("Failed to copy %llu byte selected text to clipboard", selectedText.length);
					}
				}
			}
		}
		
		// +==============================+
		// |     Handle Ctrl+V Paste      |
		// +==============================+
		if (KeyPressed(Key_V) && KeyDownRaw(Key_Control))
		{
			HandleKeyExtended(Key_V);
			if (!tb->selectionActive)
			{
				tb->selectionActive = true;
				tb->selectionStartIndex = tb->text.length;
				tb->selectionEndIndex = tb->selectionStartIndex;
				tb->needToRemeasure = true;
			}
			MyStr_t pastedText = plat->PasteTextFromClipboard(TempArena);
			if (tb->isMultiline)
			{
				pastedText = StrReplace(pastedText, "\r\n", "\n", TempArena);
			}
			else
			{
				pastedText = StrReplace(pastedText, "\r", "", TempArena);
				pastedText = StrReplace(pastedText, "\n", "", TempArena);
			}
			if (pastedText.length > 0)
			{
				Textbox_CharactersTyped(tb, pastedText);
			}
		}
	}
	
	// +==============================+
	// |     End of Update Checks     |
	// +==============================+
	tb->targetRecMoved = false;
	if (oldSelectionActive != tb->selectionActive ||
		oldSelectionStartIndex != tb->selectionStartIndex ||
		oldSelectionEndIndex != tb->selectionEndIndex)
	{
		tb->lastSelectionMoveTime = ProgramTime;
	}
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderTextbox(Textbox_t* tb)
{
	NotNull(tb);
	TextboxLayout(tb);
	
	const FontFace_t* fontFace = GetFontFace(GetPointer(&tb->font), tb->fontFaceSelector);
	r32 fontMaxAscend = (fontFace != nullptr) ? (fontFace->maxAscend * tb->fontScale) : 0;
	r32 fontMaxDescend = (fontFace != nullptr) ? (fontFace->maxDescend * tb->fontScale) : 0;
	
	// +==============================+
	// |          Render Box          |
	// +==============================+
	Color_t outlineColor = IsFocused(tb) ? MonokaiWhite : MonokaiDarkGray;
	Color_t backColor = MonokaiDarkGray;
	RcDrawRoundedRectangle(RecInflate(tb->mainRec, 1, 1), tb->innerMargin.y, outlineColor);
	RcDrawRoundedRectangle(tb->mainRec, tb->innerMargin.y, backColor);
	
	// +==============================+
	// |         Render Text          |
	// +==============================+
	rec oldViewport = rc->state.viewportRec;
	RcSetViewport(tb->mainRec);
	if (!IsEmptyStr(tb->text))
	{
		RcBindFont(GetPointer(&tb->font), tb->fontFaceSelector, tb->fontScale);
		Color_t textColor = MonokaiWhite;
		Color_t selectionColor = IsFocused(tb) ? MonokaiWhite : MonokaiGray1;
		Color_t selectionTextColor = MonokaiDarkGray;
		if (tb->selectionActive)
		{
			u64 selectionMinIndex = MinU64(tb->selectionStartIndex, tb->selectionEndIndex);
			u64 selectionMaxIndex = MaxU64(tb->selectionStartIndex, tb->selectionEndIndex);
			RcDrawTextWithSelection(tb->text, tb->textPos, textColor, selectionTextColor, selectionColor, selectionMinIndex, (selectionMaxIndex - selectionMinIndex));
		}
		else
		{
			RcDrawText(tb->text, tb->textPos, textColor);
		}
	}
	else if (!IsEmptyStr(tb->hintText))
	{
		Color_t hintTextColor = MonokaiGray2;
		RcDrawText(tb->hintText, tb->textPos, hintTextColor);
	}
	RcSetViewport(oldViewport);
	
	// +==============================+
	// |        Render Cursor         |
	// +==============================+
	if (tb->selectionActive && IsFocused(tb))
	{
		r32 cursorAlpha = ClampR32(Oscillate(2, 0, 1000, 1000 - (tb->lastSelectionMoveTime%1000)), 0, 1);
		if (TimeSince(tb->lastSelectionMoveTime) < 500) { cursorAlpha = 1; }
		Color_t cursorColor = ColorTransparent(MonokaiWhite, cursorAlpha);
		v2 selectionEndPos = tb->textPos + tb->selectionEndPos;
		rec cursorRec = NewRec(selectionEndPos.x, selectionEndPos.y - fontMaxAscend, 1, fontMaxAscend + fontMaxDescend);
		RecAlign(&cursorRec);
		RcDrawRectangle(cursorRec, cursorColor);
	}
}
