/*
File:   pig_input.cpp
Author: Taylor Robbins
Date:   10\30\2021
Description: 
	** Holds a bunch of functions that help us quickly ask questions about the input information
	** coming from the platform layer (EngineInput_t and WindowEngineInput_t)
*/

// +--------------------------------------------------------------+
// |                         Constructors                         |
// +--------------------------------------------------------------+
void FreeMouseHitInfo(MouseHitInfo_t* info)
{
	NotNull(info);
	FreeString(fixedHeap, &info->name);
	ClearPointer(info);
}
void CreateMouseHitInfo(const PlatWindow_t* window, u64 priority, MyStr_t name, const void* pntr, u64 index, MouseHitInfo_t* infoOut)
{
	NotNull(infoOut);
	ClearPointer(infoOut);
	infoOut->window = window;
	infoOut->priority = priority;
	infoOut->name = (name.pntr != nullptr) ? AllocString(fixedHeap, &name) : MyStr_Empty;
	infoOut->pntr = pntr;
	infoOut->index = index;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Pig_InitializeInput()
{
	NotNull(pig);
	CreateMouseHitInfo(nullptr, 0, MyStr_Empty, nullptr, 0, &pig->mouseHit);
	CreateMouseHitInfo(nullptr, 0, MyStr_Empty, nullptr, 0, &pig->prevMouseHit);
	
	pig->focusedItemPntr = nullptr;
	pig->focusedItemName = MyStr_Empty;
	pig->isFocusedItemTyping = false;
	
	CreateVarArray(&pig->unfocusedItems, mainHeap, sizeof(const void*));
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Pig_UpdateBtnHandlingInfoBefore(BtnHandlingInfo_t* info, const PlatBtnState_t* btnState)
{
	NotNull(info);
	if (info->extendedHandled && btnState != nullptr && !btnState->isDown && btnState->numReleases == 0)
	{
		//backup in case we extended handle while the button isn't being held down
		info->extendedHandled = false;
	}
	if (!info->extendedHandled)
	{
		info->releaseHandled = false;
	}
	if (info->extendedHandled && btnState != nullptr && btnState->numReleases > 0)
	{
		// On the frame where a button releases we want to continue handling the release event (leave releaseHandled = true)
		// but any new presses on the same frame can be left unhandled
		info->extendedHandled = false;
	}
	if (!info->extendedHandled)
	{
		info->pressHandled = false;
		#if DEBUG_BUILD
		if (!IsEmptyStr(info->filePath))
		{
			FreeString(fixedHeap, &info->filePath);
		}
		#endif
	}
}
void Pig_UpdateInputBefore()
{
	NotNull(pig);
	NotNull(pigIn);
	NotNull(pigOut);
	
	// +==================================+
	// | Update cursorType and mouseMode  |
	// +==================================+
	// This makes it so if we don't set these to a value on any frame, they will default back to these values
	// Generally this makes our life easier since it's more of a constant opt-in and we never are at risk
	// of getting stuck with a non-default cursorType or mouseMode when some branch of update logic stops running
	pigOut->cursorType = PlatCursor_Default;
	pigOut->mouseMode = PlatMouseMode_Default;
	
	// +==============================+
	// |       Update MouseHit        |
	// +==============================+
	FreeMouseHitInfo(&pig->prevMouseHit);
	MyMemCopy(&pig->prevMouseHit, &pig->mouseHit, sizeof(MouseHitInfo_t));
	CreateMouseHitInfo(nullptr, 0, MyStr_Empty, nullptr, 0, &pig->mouseHit);
	
	// +==============================+
	// |     Clear Handled Arrays     |
	// +==============================+
	for (u64 keyIndex = 0; keyIndex < Key_NumKeys; keyIndex++)
	{
		Pig_UpdateBtnHandlingInfoBefore(&pig->keyHandled[keyIndex], &pigIn->keyStates[keyIndex]);
	}
	for (u64 btnIndex = 0; btnIndex < MouseBtn_NumBtns; btnIndex++)
	{
		Pig_UpdateBtnHandlingInfoBefore(&pig->mouseBtnHandled[btnIndex], &pigIn->mouseBtnStates[btnIndex]);
	}
	for (u64 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
	{
		for (u64 btnIndex = 0; btnIndex < ControllerBtn_NumBtns; btnIndex++)
		{
			Pig_UpdateBtnHandlingInfoBefore(&pig->controllerBtnHandled[btnIndex], &pigIn->controllerStates[cIndex].btnStates[btnIndex]);
		}
	}
	Pig_UpdateBtnHandlingInfoBefore(&pig->scrollXHandled, nullptr);
	Pig_UpdateBtnHandlingInfoBefore(&pig->scrollYHandled, nullptr);
	
	VarArrayClear(&pig->unfocusedItems);
}

void Pig_UpdateInputAfter()
{
	NotNull(pig);
	NotNull(pigIn);
	pig->prevProgramTime = ProgramTime;
}

void Pig_InputRenderDebugInfo()
{
	#if 0
	if (pig->mouseHit.priority > 0)
	{
		if (!IsEmptyStr(pig->mouseHit.name))
		{
			if (pig->mouseHit.index != 0)
			{
				plat->DebugReadout(TempPrintStr("MouseHover: %s[%llu]", pig->mouseHit.name.pntr, pig->mouseHit.index), White, 1.0f);
			}
			else
			{
				plat->DebugReadout(TempPrintStr("MouseHover: %s", pig->mouseHit.name.pntr), White, 1.0f);
			}
		}
		else if (pig->mouseHit.pntr != nullptr)
		{
			if (pig->mouseHit.index != 0)
			{
				plat->DebugReadout(TempPrintStr("MouseHover: %p[%llu]", pig->mouseHit.pntr, pig->mouseHit.index), White, 1.0f);
			}
			else
			{
				plat->DebugReadout(TempPrintStr("MouseHover: %p", pig->mouseHit.pntr), White, 1.0f);
			}
		}
		else
		{
			if (pig->mouseHit.index != 0)
			{
				plat->DebugReadout(TempPrintStr("MouseHover: ?[%llu]", pig->mouseHit.index), White, 1.0f);
			}
			else
			{
				plat->DebugReadout(NewStr("MouseHover: ?"), White, 1.0f);
			}
		}
	}
	// #if DEBUG_BUILD
	// if (pig->keyHandled[Key_Space].extendedHandled)
	// {
	// 	if (!IsEmptyStr(pig->keyHandled[Key_Space].filePath))
	// 	{
	// 		plat->DebugReadout(TempPrintStr("Space Handled: %s:%llu", pig->keyHandled[Key_Space].filePath.pntr, pig->keyHandled[Key_Space].lineNumber), White, 1.0f);
	// 	}
	// 	else
	// 	{
	// 		plat->DebugReadout(NewStr("Space Handled: ?"), White, 1.0f);
	// 	}
	// }
	// #endif
	#endif
}

// +--------------------------------------------------------------+
// |                        Focus Tracking                        |
// +--------------------------------------------------------------+
bool IsSomethingFocused()
{
	return (pig->focusedItemPntr != nullptr);
}
bool IsFocusedItemTyping()
{
	return (IsSomethingFocused() && pig->isFocusedItemTyping);
}
const void* GetFocusedItemPntr()
{
	return pig->focusedItemPntr;
}
bool IsFocused(const void* itemPntr)
{
	return (pig->focusedItemPntr == itemPntr);
}
bool WasUnfocused(const void* itemPntr)
{
	VarArrayLoop(&pig->unfocusedItems, iIndex)
	{
		VarArrayLoopGet(const void*, unfocusedItemPntrPntr, &pig->unfocusedItems, iIndex);
		if (*unfocusedItemPntrPntr == itemPntr)
		{
			return true;
		}
	}
	return false;
}

void ClearFocus()
{
	if (pig->focusedItemPntr != nullptr)
	{
		const void** unfocusedItemPntrPntr = VarArrayAdd(&pig->unfocusedItems, const void*);
		*unfocusedItemPntrPntr = pig->focusedItemPntr;
	}
	FreeString(mainHeap, &pig->focusedItemName);
	pig->focusedItemPntr = nullptr;
	pig->isFocusedItemTyping = false;
	pig->focusedItemName = MyStr_Empty;
}

void FocusItem(const void* itemPntr, MyStr_t itemName)
{
	NotNull(itemPntr);
	NotNullStr(&itemName);
	
	ClearFocus();
	
	pig->focusedItemPntr = itemPntr;
	if (!IsEmptyStr(itemName))
	{
		pig->focusedItemName = AllocString(mainHeap, &itemName);
	}
	else { pig->focusedItemName = MyStr_Empty; }
}
void FocusItem(const void* itemPntr, const char* itemName)
{
	return FocusItem(itemPntr, NewStr(itemName));
}
void FocusItemPrint(const void* itemPntr, const char* formatString, ...)
{
	TempPrintVa(printResult, printLength, formatString);
	DebugAssert(printResult != nullptr);
	if (printResult == nullptr) { printResult = (char*)formatString; printLength = (i32)MyStrLength32(formatString); }
	FocusItem(itemPntr, NewStr(printLength, printResult));
}

// +--------------------------------------------------------------+
// |                      Mouse Hit Testing                       |
// +--------------------------------------------------------------+
// +==============================+
// |    General Mouse Helpers     |
// +==============================+
bool IsMouseInsideRec(rec rectangle)
{
	NotNull(pig);
	NotNull(pig->currentWindow);
	if (!pig->currentWindow->input.mouseInsideWindow) { return false; }
	return (IsInsideRec(rectangle, MousePos));
}
bool IsMouseInsideWindow()
{
	NotNull(pig);
	NotNull(pig->currentWindow);
	return pig->currentWindow->input.mouseInsideWindow;
}
bool DidMouseMove()
{
	NotNull(pig);
	NotNull(pig->currentWindow);
	return pig->currentWindow->input.mouseMoved;
}

// +==============================+
// |    Mouse Capture Helpers     |
// +==============================+
bool MouseHitNamedIndex(const char* hitItemName, u64 index, u64 priority = 1)
{
	NotNull(pig);
	NotNull(hitItemName);
	if (pig->mouseHit.priority < priority)
	{
		CreateMouseHitInfo(pig->currentWindow, priority, NewStr(hitItemName), nullptr, index, &pig->mouseHit);
		return true;
	}
	return false;
}
bool MouseHitNamed(const char* hitItemName, u64 priority = 1)
{
	return MouseHitNamedIndex(hitItemName, 0, priority);
}
bool MouseHitPrintIndexPriority(u64 index, u64 priority, const char* formatString, ...)
{
	NotNull(pig);
	NotNull(formatString);
	TempPushMark();
	if (pig->mouseHit.priority < priority)
	{
		TempPrintVa(printResult, printLength, formatString);
		DebugAssert(printResult != nullptr);
		if (printResult == nullptr) { printResult = (char*)formatString; printLength = (i32)MyStrLength32(formatString); }
		CreateMouseHitInfo(pig->currentWindow, priority, NewStr(printLength, printResult), nullptr, index, &pig->mouseHit);
		TempPopMark();
		return true;
	}
	TempPopMark();
	return false;
}
#define MouseHitPrintIndex(index, formatString, ...) MouseHitPrintIndexPriority((index), 1, formatString, ##__VA_ARGS__)
#define MouseHitPrint(formatString, ...) MouseHitPrintIndexPriority(0, 1, formatString, ##__VA_ARGS__)
bool MouseHitPntrIndex(const void* hitItemPntr, u64 index, u64 priority = 1)
{
	NotNull(pig);
	NotNull(hitItemPntr);
	if (pig->mouseHit.priority < priority)
	{
		CreateMouseHitInfo(pig->currentWindow, priority, MyStr_Empty, hitItemPntr, index, &pig->mouseHit);
		return true;
	}
	return false;
}
bool MouseHitPntr(const void* hitItemPntr, u64 priority = 1)
{
	return MouseHitPntrIndex(hitItemPntr, 0, priority);
}

bool MouseHitRecNamedIndex(rec rectangle, const char* hitItemName, u64 index, u64 priority = 1)
{
	if (!IsMouseInsideRec(rectangle)) { return false; }
	return MouseHitNamedIndex(hitItemName, index, priority);
}
bool MouseHitRecNamed(rec rectangle, const char* hitItemName, u64 priority = 1)
{
	if (!IsMouseInsideRec(rectangle)) { return false; }
	return MouseHitNamed(hitItemName, priority);
}
bool MouseHitRecPrintIndexPriority(rec rectangle, u64 index, u64 priority, const char* formatString, ...)
{
	NotNull(pig);
	NotNull(formatString);
	if (!IsMouseInsideRec(rectangle)) { return false; }
	TempPushMark();
	if (pig->mouseHit.priority < priority)
	{
		TempPrintVa(printResult, printLength, formatString);
		DebugAssert(printResult != nullptr);
		if (printResult == nullptr) { printResult = (char*)formatString; printLength = (i32)MyStrLength32(formatString); }
		CreateMouseHitInfo(pig->currentWindow, priority, NewStr(printLength, printResult), nullptr, index, &pig->mouseHit);
		TempPopMark();
		return true;
	}
	TempPopMark();
	return false;
}
#define MouseHitRecPrintIndex(rectangle, index, formatString, ...) MouseHitRecPrintIndexPriority((rectangle), (index), 1, formatString, ##__VA_ARGS__)
#define MouseHitRecPrint(rectangle, formatString, ...) MouseHitRecPrintIndexPriority((rectangle), 0, 1, formatString, ##__VA_ARGS__)
bool MouseHitRecPntrIndex(rec rectangle, const void* hitItemPntr, u64 index, u64 priority = 1)
{
	if (!IsMouseInsideRec(rectangle)) { return false; }
	return MouseHitPntrIndex(hitItemPntr, index, priority);
}
bool MouseHitRecPntr(rec rectangle, const void* hitItemPntr, u64 priority = 1)
{
	if (!IsMouseInsideRec(rectangle)) { return false; }
	return MouseHitPntr(hitItemPntr, priority);
}

// +==============================+
// |    Mouse Query Functions     |
// +==============================+
bool IsMouseOverAnything()
{
	NotNull(pig);
	return (pig->mouseHit.priority > 0);
}
bool IsMouseOverNamed(const char* expectedName, bool mustBeIndex0 = false)
{
	NotNull(expectedName);
	if (pig->mouseHit.priority == 0) { return false; }
	if (mustBeIndex0 && pig->mouseHit.index != 0) { return false; }
	if (IsEmptyStr(pig->mouseHit.name)) { return false; }
	u64 expectedNameLength = MyStrLength64(expectedName);
	if (pig->mouseHit.name.length != expectedNameLength) { return false; }
	if (MyStrCompare(pig->mouseHit.name.pntr, expectedName, expectedNameLength) != 0) { return false; }
	return true;
}
bool IsMouseOverPrint(const char* formatString, ...)
{
	TempPushMark();
	TempPrintVa(nameStr, nameLength, formatString);
	DebugAssert(nameStr != nullptr);
	if (nameStr == nullptr) { TempPopMark(); return false; }
	bool result = IsMouseOverNamed(nameStr);
	TempPopMark();
	return result;
}
bool IsMouseOverNamedIndex(const char* expectedName, u64 index)
{
	if (!IsMouseOverNamed(expectedName, false)) { return false; }
	if (pig->mouseHit.index != index) { return false; }
	return true;
}
bool IsMouseOverNamedPartial(const char* expectedName, bool mustBeIndex0 = false)
{
	NotNull(expectedName);
	if (pig->mouseHit.priority == 0) { return false; }
	if (mustBeIndex0 && pig->mouseHit.index != 0) { return false; }
	if (IsEmptyStr(pig->mouseHit.name)) { return false; }
	u64 expectedNameLength = MyStrLength64(expectedName);
	if (pig->mouseHit.name.length < expectedNameLength) { return false; }
	if (MyStrCompare(pig->mouseHit.name.pntr, expectedName, expectedNameLength) != 0) { return false; }
	return true;
}

// +--------------------------------------------------------------+
// |                      Keyboard Handling                       |
// +--------------------------------------------------------------+
bool IsKeyHandled(Key_t key, bool checkPress = true, bool checkRelease = true)
{
	Assert(key < Key_NumKeys);
	return ((checkPress && pig->keyHandled[key].pressHandled) || (checkRelease && pig->keyHandled[key].releaseHandled));
}

// +==============================+
// |      No Handled Regard       |
// +==============================+
bool KeyDownRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (pigIn == nullptr) { return false; }
	return pigIn->keyStates[key].isDown;
}
bool KeyReleasedRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (pigIn == nullptr) { return false; }
	return (pigIn->keyStates[key].numReleases > 0);
}
bool KeyPressedRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (pigIn == nullptr) { return false; }
	return (pigIn->keyStates[key].numPresses > 0);
}
bool KeyPressedPlatRepeatingRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (pigIn == nullptr) { return false; }
	return (pigIn->keyStates[key].numPresses > 0 || pigIn->keyStates[key].numRepeats > 0);
}
//TODO: Does this work correctly? Seems like delay makes no difference?
bool KeyRepeatedRaw(Key_t key, u64 delay, u64 period)
{
	Assert(key < Key_NumKeys);
	Assert(period > 0);
	if (pigIn == nullptr) { return false; }
	if (pigIn->keyStates[key].numPresses > 0) { return true; }
	if (pig->prevProgramTime == ProgramTime) { return false; }
	if (!pigIn->keyStates[key].isDown) { return false; }
	u64 timeSince = TimeSince(pigIn->keyStates[key].lastChangeTime);
	u64 timeSincePrev = TimeSinceBy(pig->prevProgramTime, pigIn->keyStates[key].lastChangeTime);
	if (timeSince < delay) { return false; }
	if (timeSincePrev < delay) { return true; }
	timeSince -= delay;
	timeSincePrev -= delay;
	if ((timeSince / period) != (timeSincePrev / period)) { return true; }
	return false;
}

// +==============================+
// |    With Regard to Handled    |
// +==============================+
bool KeyDown(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyDownRaw(key);
}
bool KeyReleased(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (IsKeyHandled(key, false, true)) { return false; }
	return KeyReleasedRaw(key);
}
bool KeyPressed(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyPressedRaw(key);
}
bool KeyPressedPlatRepeating(Key_t key)
{
	Assert(key < Key_NumKeys);
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyPressedPlatRepeatingRaw(key);
}
bool KeyPressedRepeating(Key_t key, u64 repeatDelay, u64 repeatPeriod)
{
	Assert(key < Key_NumKeys);
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyRepeatedRaw(key, repeatDelay, repeatPeriod);
}

// +==============================+
// |       Handle Functions       |
// +==============================+
//TODO: Set handled flags on related Key and Character events
void HandleKey_(Key_t key, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->keyHandled[key].pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->keyHandled[key].filePath)) { FreeString(fixedHeap, &pig->keyHandled[key].filePath); }
		pig->keyHandled[key].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->keyHandled[key].lineNumber = lineNumber;
	}
	#endif
}
void HandleKeyRelease_(Key_t key, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->keyHandled[key].releaseHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->keyHandled[key].filePath)) { FreeString(fixedHeap, &pig->keyHandled[key].filePath); }
		pig->keyHandled[key].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->keyHandled[key].lineNumber = lineNumber;
	}
	#endif
}
void HandleKeyExtended_(Key_t key, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->keyHandled[key].pressHandled = true;
	pig->keyHandled[key].releaseHandled = true;
	pig->keyHandled[key].extendedHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->keyHandled[key].filePath)) { FreeString(fixedHeap, &pig->keyHandled[key].filePath); }
		pig->keyHandled[key].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->keyHandled[key].lineNumber = lineNumber;
	}
	#endif
}

#if DEBUG_BUILD
#define HandleKey(key)                  HandleKey_((key), __FILE__, __LINE__)
#define HandleKeyRelease(key)           HandleKeyRelease_((key), __FILE__, __LINE__)
#define HandleKeyExtended(key)          HandleKeyExtended_((key), __FILE__, __LINE__)
#else
#define HandleKey(key)                  HandleKey_((key), nullptr, 0)
#define HandleKeyRelease(key)           HandleKeyRelease_((key), nullptr, 0)
#define HandleKeyExtended(key)          HandleKeyExtended_((key), nullptr, 0)
#endif

// +==============================+
// |  Check and Handle Functions  |
// +==============================+
//NOTE: Extended button presses are a way for a piece of logic who only cares about the press or holding of a specific button to still
//      handle the entire sequence of events (press, hold, release) for a button so another check doesn't errantly handle the hold or release of the button
bool KeyDownAndHandle_(Key_t key, bool extended, const char* filePath, u64 lineNumber)
{
	if (KeyDown(key))
	{
		if (extended) { HandleKeyExtended_(key, filePath, lineNumber); }
		else { HandleKey_(key, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool KeyPressedAndHandle_(Key_t key, bool extended, const char* filePath, u64 lineNumber)
{
	if (KeyPressed(key))
	{
		if (extended) { HandleKeyExtended_(key, filePath, lineNumber); }
		else { HandleKey_(key, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool KeyReleasedAndHandle_(Key_t key, const char* filePath, u64 lineNumber)
{
	if (KeyReleased(key))
	{
		HandleKeyRelease_(key, filePath, lineNumber);
		return true;
	}
	return false;
}

#if DEBUG_BUILD
#define KeyDownAndHandle(key)            KeyDownAndHandle_((key), false, __FILE__, __LINE__)
#define KeyDownAndHandleExtended(key)    KeyDownAndHandle_((key), true, __FILE__, __LINE__)
#define KeyPressedAndHandle(key)         KeyPressedAndHandle_((key), false, __FILE__, __LINE__)
#define KeyPressedAndHandleExtended(key) KeyPressedAndHandle_((key), true, __FILE__, __LINE__)
#define KeyReleasedAndHandle(key)        KeyReleasedAndHandle_((key), __FILE__, __LINE__)
#else
#define KeyDownAndHandle(key)            KeyDownAndHandle_((key), false, nullptr, 0)
#define KeyDownAndHandleExtended(key)    KeyDownAndHandle_((key), true, nullptr, 0)
#define KeyPressedAndHandle(key)         KeyPressedAndHandle_((key), false, nullptr, 0)
#define KeyPressedAndHandleExtended(key) KeyPressedAndHandle_((key), true, nullptr, 0)
#define KeyReleasedAndHandle(key)        KeyReleasedAndHandle_((key), nullptr, 0)
#endif

// +--------------------------------------------------------------+
// |                    Mouse Button Handling                     |
// +--------------------------------------------------------------+
bool IsMouseHandled(MouseBtn_t mouseBtn, bool checkPress = true, bool checkRelease = true)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	return ((checkPress && pig->mouseBtnHandled[mouseBtn].pressHandled) || (checkRelease && pig->mouseBtnHandled[mouseBtn].releaseHandled));
}

// +==============================+
// |      No Handled Regard       |
// +==============================+
bool MouseDownRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	return pigIn->mouseBtnStates[mouseBtn].isDown;
}
bool MouseReleasedRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	return (pigIn->mouseBtnStates[mouseBtn].numReleases > 0);
}
bool MousePressedRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	return (pigIn->mouseBtnStates[mouseBtn].numPresses > 0);
}
bool MouseRepeatedRaw(MouseBtn_t mouseBtn, u64 delay, u64 period)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	if (pigIn->mouseBtnStates[mouseBtn].numPresses > 0) { return true; }
	if (pig->prevProgramTime == ProgramTime) { return false; }
	if (!pigIn->mouseBtnStates[mouseBtn].isDown) { return false; }
	u64 timeSince = TimeSince(pigIn->mouseBtnStates[mouseBtn].lastChangeTime);
	u64 timeSincePrev = TimeSinceBy(pig->prevProgramTime, pigIn->mouseBtnStates[mouseBtn].lastChangeTime);
	if (timeSince < delay) { return false; }
	if (timeSincePrev < delay) { return true; }
	timeSince -= delay;
	timeSincePrev -= delay;
	if ((timeSince / period) != (timeSincePrev / period)) { return true; }
	return false;
}

// +==============================+
// |    With Regard to Handled    |
// +==============================+
bool MouseDown(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (IsMouseHandled(mouseBtn, true, false)) { return false; }
	return MouseDownRaw(mouseBtn);
}
bool MouseReleased(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (IsMouseHandled(mouseBtn, false, true)) { return false; }
	return MouseReleasedRaw(mouseBtn);
}
bool MousePressed(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (IsMouseHandled(mouseBtn, true, false)) { return false; }
	return MousePressedRaw(mouseBtn);
}
bool MousePressedRepeating(MouseBtn_t mouseBtn, u64 repeatDelay, u64 repeatPeriod)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	if (IsMouseHandled(mouseBtn, true, false)) { return false; }
	return MouseRepeatedRaw(mouseBtn, repeatDelay, repeatPeriod);
}

// +==============================+
// |       Handle Functions       |
// +==============================+
//TODO: Set handled flags on related MouseBtn events
void HandleMouse_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->mouseBtnHandled[mouseBtn].pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->mouseBtnHandled[mouseBtn].filePath)) { FreeString(fixedHeap, &pig->mouseBtnHandled[mouseBtn].filePath); }
		pig->mouseBtnHandled[mouseBtn].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->mouseBtnHandled[mouseBtn].lineNumber = lineNumber;
	}
	#endif
}
void HandleMouseRelease_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->mouseBtnHandled[mouseBtn].releaseHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->mouseBtnHandled[mouseBtn].filePath)) { FreeString(fixedHeap, &pig->mouseBtnHandled[mouseBtn].filePath); }
		pig->mouseBtnHandled[mouseBtn].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->mouseBtnHandled[mouseBtn].lineNumber = lineNumber;
	}
	#endif
}
void HandleMouseExtended_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->mouseBtnHandled[mouseBtn].pressHandled = true;
	pig->mouseBtnHandled[mouseBtn].releaseHandled = true;
	pig->mouseBtnHandled[mouseBtn].extendedHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->mouseBtnHandled[mouseBtn].filePath)) { FreeString(fixedHeap, &pig->mouseBtnHandled[mouseBtn].filePath); }
		pig->mouseBtnHandled[mouseBtn].filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->mouseBtnHandled[mouseBtn].lineNumber = lineNumber;
	}
	#endif
}

#if DEBUG_BUILD
#define HandleMouse(mouseBtn)                  HandleMouse_((mouseBtn), __FILE__, __LINE__)
#define HandleMouseRelease(mouseBtn)           HandleMouseRelease_((mouseBtn), __FILE__, __LINE__)
#define HandleMouseExtended(mouseBtn)          HandleMouseExtended_((mouseBtn), __FILE__, __LINE__)
#else
#define HandleMouse(mouseBtn)                  HandleMouse_((mouseBtn), nullptr, 0)
#define HandleMouseRelease(mouseBtn)           HandleMouseRelease_((mouseBtn), nullptr, 0)
#define HandleMouseExtended(mouseBtn)          HandleMouseExtended_((mouseBtn), nullptr, 0)
#endif

// +==============================+
// |  Check and Handle Functions  |
// +==============================+
//NOTE: Extended button presses are a way for a piece of logic who only cares about the press or holding of a specific button to still
//      handle the entire sequence of events (press, hold, release) for a button so another check doesn't errantly handle the hold or release of the button
bool MouseDownAndHandle_(MouseBtn_t mouseBtn, bool extended, const char* filePath, u64 lineNumber)
{
	if (MouseDown(mouseBtn))
	{
		if (extended) { HandleMouseExtended_(mouseBtn, filePath, lineNumber); }
		else { HandleMouse_(mouseBtn, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool MousePressedAndHandle_(MouseBtn_t mouseBtn, bool extended, const char* filePath, u64 lineNumber)
{
	if (MousePressed(mouseBtn))
	{
		if (extended) { HandleMouseExtended_(mouseBtn, filePath, lineNumber); }
		else { HandleMouse_(mouseBtn, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool MouseReleasedAndHandle_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	if (MouseReleased(mouseBtn))
	{
		HandleMouseRelease_(mouseBtn, filePath, lineNumber);
		return true;
	}
	return false;
}

#if DEBUG_BUILD
#define MouseDownAndHandle(mouseBtn)            MouseDownAndHandle_((mouseBtn), false, __FILE__, __LINE__)
#define MouseDownAndHandleExtended(mouseBtn)    MouseDownAndHandle_((mouseBtn), true, __FILE__, __LINE__)
#define MousePressedAndHandle(mouseBtn)         MousePressedAndHandle_((mouseBtn), false, __FILE__, __LINE__)
#define MousePressedAndHandleExtended(mouseBtn) MousePressedAndHandle_((mouseBtn), true, __FILE__, __LINE__)
#define MouseReleasedAndHandle(mouseBtn)        MouseReleasedAndHandle_((mouseBtn), __FILE__, __LINE__)
#else
#define MouseDownAndHandle(mouseBtn)            MouseDownAndHandle_((mouseBtn), false, nullptr, 0)
#define MouseDownAndHandleExtended(mouseBtn)    MouseDownAndHandle_((mouseBtn), true, nullptr, 0)
#define MousePressedAndHandle(mouseBtn)         MousePressedAndHandle_((mouseBtn), false, nullptr, 0)
#define MousePressedAndHandleExtended(mouseBtn) MousePressedAndHandle_((mouseBtn), true, nullptr, 0)
#define MouseReleasedAndHandle(mouseBtn)        MouseReleasedAndHandle_((mouseBtn), nullptr, 0)
#endif

// +--------------------------------------------------------------+
// |                         Mouse Wheel                          |
// +--------------------------------------------------------------+
bool MouseScrolledXRaw()
{
	if (pigIn == nullptr) { return false; }
	return pigIn->scrollChangedX;
}
bool MouseScrolledYRaw()
{
	if (pigIn == nullptr) { return false; }
	return pigIn->scrollChangedY;
}
bool MouseScrolledRaw()
{
	if (pigIn == nullptr) { return false; }
	return (pigIn->scrollChangedX || pigIn->scrollChangedY);
}

bool MouseScrolledX()
{
	if (pig->scrollXHandled.pressHandled) { return false; }
	return MouseScrolledXRaw();
}
bool MouseScrolledY()
{
	if (pig->scrollYHandled.pressHandled) { return false; }
	return MouseScrolledYRaw();
}
bool MouseScrolled()
{
	return (MouseScrolledX() || MouseScrolledY());
}

r32 GetMouseScrollDeltaX()
{
	if (pigIn == nullptr) { return 0.0f; }
	return pigIn->scrollDelta.x;
}
r32 GetMouseScrollDeltaY()
{
	if (pigIn == nullptr) { return 0.0f; }
	return pigIn->scrollDelta.y;
}
v2 GetMouseScrollDelta()
{
	if (pigIn == nullptr) { return Vec2_Zero; }
	return pigIn->scrollDelta;
}

//TODO: Set handled flags on MouseScroll events
void HandleMouseScrollX_(const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->scrollXHandled.pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->scrollXHandled.filePath)) { FreeString(fixedHeap, &pig->scrollXHandled.filePath); }
		pig->scrollXHandled.filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->scrollXHandled.lineNumber = lineNumber;
	}
	#endif
}
void HandleMouseScrollY_(const char* filePath, u64 lineNumber)
{
	UNUSED(filePath);
	UNUSED(lineNumber);
	pig->scrollYHandled.pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(pig->scrollYHandled.filePath)) { FreeString(fixedHeap, &pig->scrollYHandled.filePath); }
		pig->scrollYHandled.filePath = NewStringInArenaNt(fixedHeap, filePath);
		pig->scrollYHandled.lineNumber = lineNumber;
	}
	#endif
}
void HandleMouseScroll_(const char* filePath, u64 lineNumber)
{
	HandleMouseScrollX_(filePath, lineNumber);
	HandleMouseScrollY_(filePath, lineNumber);
}

#if DEBUG_BUILD
#define HandleMouseScrollX()  HandleMouseScrollX_(__FILE__, __LINE__)
#define HandleMouseScrollY()  HandleMouseScrollY_(__FILE__, __LINE__)
#define HandleMouseScroll()   HandleMouseScroll_(__FILE__, __LINE__)
#else
#define HandleMouseScrollX()  HandleMouseScrollX_(nullptr, 0)
#define HandleMouseScrollY()  HandleMouseScrollY_(nullptr, 0)
#define HandleMouseScroll()   HandleMouseScroll_(nullptr, 0)
#endif

bool MouseScrolledXAndHandle_(const char* filePath, u64 lineNumber)
{
	if (MouseScrolledX())
	{
		HandleMouseScrollX_(filePath, lineNumber);
		return true;
	}
	return false;
}
bool MouseScrolledYAndHandle_(const char* filePath, u64 lineNumber)
{
	if (MouseScrolledY())
	{
		HandleMouseScrollY_(filePath, lineNumber);
		return true;
	}
	return false;
}
bool MouseScrolledAndHandle_(const char* filePath, u64 lineNumber)
{
	if (MouseScrolled())
	{
		HandleMouseScroll_(filePath, lineNumber);
		return true;
	}
	return false;
}

#if DEBUG_BUILD
#define MouseScrolledXAndHandle()  MouseScrolledXAndHandle_(__FILE__, __LINE__)
#define MouseScrolledYAndHandle()  MouseScrolledYAndHandle_(__FILE__, __LINE__)
#define MouseScrolledAndHandle()   MouseScrolledAndHandle_(__FILE__, __LINE__)
#else
#define MouseScrolledXAndHandle()  MouseScrolledXAndHandle_(nullptr, 0)
#define MouseScrolledYAndHandle()  MouseScrolledYAndHandle_(nullptr, 0)
#define MouseScrolledAndHandle()   MouseScrolledAndHandle_(nullptr, 0)
#endif

//TODO: Add controller support for handling!

// +--------------------------------------------------------------+
// |                     Input Event Handling                     |
// +--------------------------------------------------------------+
bool CheckIfInputEventWasHandled(InputEvent_t* inputEvent)
{
	NotNull(inputEvent);
	bool isHandled = false;
	
	if (inputEvent->handled) { isHandled = true; }
	if (inputEvent->type == InputEventType_Key && IsKeyHandled(inputEvent->key.key, (inputEvent->key.pressed || inputEvent->key.repeated), inputEvent->key.released)) { isHandled = true; }
	if (inputEvent->type == InputEventType_MouseBtn && IsMouseHandled(inputEvent->mouseBtn.btn, (inputEvent->mouseBtn.pressed || inputEvent->mouseBtn.repeated), inputEvent->mouseBtn.released)) { isHandled = true; }
	//TODO: Add support for InputEventType_ControllerBtn
	
	if (inputEvent->pairedEventIndex >= 0 && (u64)inputEvent->pairedEventIndex < pigIn->inputEvents.length)
	{
		InputEvent_t* pairedEvent = VarArrayGet(&pigIn->inputEvents, inputEvent->pairedEventIndex, InputEvent_t);
		if (pairedEvent->handled) { isHandled = true; }
		if (pairedEvent->type == InputEventType_Key && IsKeyHandled(pairedEvent->key.key, (pairedEvent->key.pressed || pairedEvent->key.repeated), pairedEvent->key.released)) { isHandled = true; }
		if (pairedEvent->type == InputEventType_MouseBtn && IsMouseHandled(pairedEvent->mouseBtn.btn, (pairedEvent->mouseBtn.pressed || pairedEvent->mouseBtn.repeated), pairedEvent->mouseBtn.released)) { isHandled = true; }
		//TODO: Add support for InputEventType_ControllerBtn
		
		if (isHandled) { pairedEvent->handled = true; }
	}
	
	if (isHandled) { inputEvent->handled = true; }
	return isHandled;
}

void HandleInputEvent(InputEvent_t* inputEvent, bool extended = false)
{
	NotNull(inputEvent);
	inputEvent->handled = true;
	if (inputEvent->type == InputEventType_Key)
	{
		if (extended) { HandleKeyExtended(inputEvent->key.key); }
		else if (inputEvent->key.pressed || inputEvent->key.repeated) { HandleKey(inputEvent->key.key); }
		else { HandleKeyRelease(inputEvent->key.key); }
	}
	if (inputEvent->type == InputEventType_MouseBtn)
	{
		if (extended) { HandleMouseExtended(inputEvent->mouseBtn.btn); }
		else if (inputEvent->mouseBtn.pressed || inputEvent->mouseBtn.repeated) { HandleMouse(inputEvent->mouseBtn.btn); }
		else { HandleMouseRelease(inputEvent->mouseBtn.btn); }
	}
	//TODO: Add support for InputEventType_ControllerBtn
	
	if (inputEvent->pairedEventIndex >= 0 && (u64)inputEvent->pairedEventIndex < pigIn->inputEvents.length)
	{
		InputEvent_t* pairedEvent = VarArrayGet(&pigIn->inputEvents, inputEvent->pairedEventIndex, InputEvent_t);
		pairedEvent->handled = true;
		if (pairedEvent->type == InputEventType_Key)
		{
			if (extended) { HandleKeyExtended(pairedEvent->key.key); }
			else if (pairedEvent->key.pressed || pairedEvent->key.repeated) { HandleKey(pairedEvent->key.key); }
			else { HandleKeyRelease(pairedEvent->key.key); }
		}
		if (pairedEvent->type == InputEventType_MouseBtn)
		{
			if (extended) { HandleMouseExtended(pairedEvent->mouseBtn.btn); }
			else if (pairedEvent->mouseBtn.pressed || pairedEvent->mouseBtn.repeated) { HandleMouse(pairedEvent->mouseBtn.btn); }
			else { HandleMouseRelease(pairedEvent->mouseBtn.btn); }
		}
		//TODO: Add support for InputEventType_ControllerBtn
	}
}

// +--------------------------------------------------------------+
// |                         Text Editing                         |
// +--------------------------------------------------------------+
bool HandleBasicTypingToEditString(MemArena_t* allocAndFreeArena, MyStr_t* editString)
{
	NotNull(allocAndFreeArena);
	NotNullStr(editString);
	
	bool result = false;
	VarArrayLoop(&pigIn->inputEvents, eIndex)
	{
		VarArrayLoopGet(InputEvent_t, event, &pigIn->inputEvents, eIndex);
		if (!event->handled)
		{
			if (event->type == InputEventType_Character)
			{
				//TODO: Handle the key that this character input is associated with?
				event->handled = true;
				u32 codepoint = event->character.codepoint;
				Assert(codepoint != '\b'); //I don't think we get backspace character events?
				u8 encodedBuffer[UTF8_MAX_CHAR_SIZE];
				u8 encodedSize = GetUtf8BytesForCode(codepoint, &encodedBuffer[0]);
				Assert(encodedSize > 0);
				u64 newStrLength = editString->length + encodedSize;
				char* newStrSpace = AllocArray(allocAndFreeArena, char, newStrLength + 1);
				if (editString->length > 0) { MyMemCopy(&newStrSpace[0], editString->pntr, editString->length); }
				MyMemCopy(&newStrSpace[editString->length], &encodedBuffer[0], encodedSize);
				newStrSpace[newStrLength] = '\0';
				FreeString(allocAndFreeArena, editString);
				editString->pntr = newStrSpace;
				editString->length = newStrLength;
			}
			else if (event->type == InputEventType_Key && event->key.key == Key_Backspace && (event->key.pressed || event->key.repeated))
			{
				HandleKeyExtended(Key_Backspace); //TODO: Handle this event more properly!
				event->handled = true;
				if (editString->length > 0)
				{
					if (KeyDownRaw(Key_Control))
					{
						FreeString(allocAndFreeArena, editString);
					}
					else
					{
						u32 deletedCodepoint = 0;
						u8 previousCharSize = GetCodepointBeforeIndex(editString->pntr, editString->length, &deletedCodepoint);
						if (previousCharSize == 0)
						{
							DebugAssertMsg(false, "Some sort of invalid encoding has snuck into our name?");
							previousCharSize = 1;
							deletedCodepoint = CharToU32(editString->pntr[editString->length-1]);
						}
						u64 newStrSize = (editString->length - previousCharSize);
						char* newStrSpace = AllocArray(allocAndFreeArena, char, newStrSize+1);
						NotNull(newStrSpace);
						MyMemCopy(newStrSpace, editString->pntr, newStrSize);
						newStrSpace[newStrSize] = '\0';
						FreeString(allocAndFreeArena, editString);
						editString->pntr = newStrSpace;
						editString->length = newStrSize;
					}
				}
			}
		}
	}
	return result;
}
