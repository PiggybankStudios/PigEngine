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
			Pig_UpdateBtnHandlingInfoBefore(&pig->controllerBtnHandled[cIndex][btnIndex], &pigIn->controllerStates[cIndex].btnStates[btnIndex]);
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
	pig->isMouseFullyCaptured = pig->wasMouseCapturedLastFrame;
	pig->wasMouseCapturedLastFrame = (pigOut->mouseMode == PlatMouseMode_FirstPersonCamera);
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

// +--------------------------------------------------------------+
// |                     Controller Handling                      |
// +--------------------------------------------------------------+
#define CONTROLLER_INDEX_ANY -1
bool IsControllerBtnHandled(i32 controllerIndex, ControllerBtn_t btn, bool checkPress = true, bool checkRelease = true)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if ((checkPress && pig->controllerBtnHandled[cIndex][btn].pressHandled) || (checkRelease && pig->controllerBtnHandled[cIndex][btn].releaseHandled)) { return true; }
		}
		return false;
	}
	else
	{
		return ((checkPress && pig->controllerBtnHandled[controllerIndex][btn].pressHandled) || (checkRelease && pig->controllerBtnHandled[controllerIndex][btn].releaseHandled));
	}
}

bool IsControllerConnected(i32 controllerIndex)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if (controllerIndex == CONTROLLER_INDEX_ANY)
			{
				if (pigIn->controllerStates[cIndex].connected) { return true; }
			}
		}
		return false;
	}
	else
	{
		return pigIn->controllerStates[controllerIndex].connected;
	}
}

// +==============================+
// |      No Handled Regard       |
// +==============================+
bool ControllerBtnDownRaw(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if (pigIn->controllerStates[cIndex].connected && pigIn->controllerStates[cIndex].btnStates[btn].isDown) { return true; }
		}
		return false;
	}
	else
	{
		return (pigIn->controllerStates[controllerIndex].connected && pigIn->controllerStates[controllerIndex].btnStates[btn].isDown);
	}
}
bool ControllerBtnReleasedRaw(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if (pigIn->controllerStates[cIndex].connected && pigIn->controllerStates[cIndex].btnStates[btn].numReleases > 0) { return true; }
		}
		return false;
	}
	else
	{
		return (pigIn->controllerStates[controllerIndex].connected && pigIn->controllerStates[controllerIndex].btnStates[btn].numReleases > 0);
	}
}
bool ControllerBtnPressedRaw(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if (pigIn->controllerStates[cIndex].connected && pigIn->controllerStates[cIndex].btnStates[btn].numPresses > 0) { return true; }
		}
		return false;
	}
	else
	{
		return (pigIn->controllerStates[controllerIndex].connected && pigIn->controllerStates[controllerIndex].btnStates[btn].numPresses > 0);
	}
}
bool ControllerBtnPressedPlatRepeatingRaw(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (pigIn == nullptr) { return false; }
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			if (pigIn->controllerStates[cIndex].connected && (pigIn->controllerStates[cIndex].btnStates[btn].numPresses > 0 || pigIn->controllerStates[controllerIndex].btnStates[btn].numRepeats > 0)) { return true; }
		}
		return false;
	}
	else
	{
		return (pigIn->controllerStates[controllerIndex].connected && (pigIn->controllerStates[controllerIndex].btnStates[btn].numPresses > 0 || pigIn->controllerStates[controllerIndex].btnStates[btn].numRepeats > 0));
	}
}
//TODO: Does this work correctly? Seems like delay makes no difference?
bool ControllerBtnRepeatedRaw(i32 controllerIndex, ControllerBtn_t btn, u64 delay, u64 period)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	Assert(period > 0);
	if (pigIn == nullptr) { return false; }
	if (ControllerBtnPressedRaw(controllerIndex, btn)) { return true; }
	if (pig->prevProgramTime == ProgramTime) { return false; }
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			PlatControllerState_t* controller = &pigIn->controllerStates[cIndex];
			if (controller->connected && controller->btnStates[btn].isDown)
			{
				u64 timeSince = TimeSince(controller->btnStates[btn].lastChangeTime);
				u64 timeSincePrev = TimeSinceBy(pig->prevProgramTime, controller->btnStates[btn].lastChangeTime);
				if (timeSince < delay) { continue; }
				if (timeSincePrev < delay) { return true; }
				timeSince -= delay;
				timeSincePrev -= delay;
				if ((timeSince / period) != (timeSincePrev / period)) { return true; }
			}
		}
		return false;
	}
	else
	{
		if (!pigIn->controllerStates[controllerIndex].connected) { return false; }
		if (!pigIn->controllerStates[controllerIndex].btnStates[btn].isDown) { return false; }
		u64 timeSince = TimeSince(pigIn->controllerStates[controllerIndex].btnStates[btn].lastChangeTime);
		u64 timeSincePrev = TimeSinceBy(pig->prevProgramTime, pigIn->controllerStates[controllerIndex].btnStates[btn].lastChangeTime);
		if (timeSince < delay) { return false; }
		if (timeSincePrev < delay) { return true; }
		timeSince -= delay;
		timeSincePrev -= delay;
		if ((timeSince / period) != (timeSincePrev / period)) { return true; }
		return false;
	}
	
}

// +==============================+
// |    With Regard to Handled    |
// +==============================+
bool ControllerBtnDown(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (IsControllerBtnHandled(controllerIndex, btn, true, false)) { return false; }
	return ControllerBtnDownRaw(controllerIndex, btn);
}
bool ControllerBtnReleased(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (IsControllerBtnHandled(controllerIndex, btn, false, true)) { return false; }
	return ControllerBtnReleasedRaw(controllerIndex, btn);
}
bool ControllerBtnPressed(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (IsControllerBtnHandled(controllerIndex, btn, true, false)) { return false; }
	return ControllerBtnPressedRaw(controllerIndex, btn);
}
bool ControllerBtnPressedPlatRepeating(i32 controllerIndex, ControllerBtn_t btn)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (IsControllerBtnHandled(controllerIndex, btn, true, false)) { return false; }
	return ControllerBtnPressedPlatRepeatingRaw(controllerIndex, btn);
}
bool ControllerBtnPressedRepeating(i32 controllerIndex, ControllerBtn_t btn, u64 repeatDelay, u64 repeatPeriod)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (IsControllerBtnHandled(controllerIndex, btn, true, false)) { return false; }
	return ControllerBtnRepeatedRaw(controllerIndex, btn, repeatDelay, repeatPeriod);
}

// +==============================+
// |       Handle Functions       |
// +==============================+
//TODO: Set handled flags on related Key and Character events
void HandleControllerBtn_(i32 controllerIndex, ControllerBtn_t btn, const char* filePath, u64 lineNumber)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			pig->controllerBtnHandled[cIndex][btn].pressHandled = true;
			#if DEBUG_BUILD
			if (filePath != nullptr)
			{
				if (!IsEmptyStr(pig->controllerBtnHandled[cIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[cIndex][btn].filePath); }
				pig->controllerBtnHandled[cIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
				pig->controllerBtnHandled[cIndex][btn].lineNumber = lineNumber;
			}
			#else
			UNUSED(filePath);
			UNUSED(lineNumber);
			#endif
		}
	}
	else
	{
		pig->controllerBtnHandled[controllerIndex][btn].pressHandled = true;
		#if DEBUG_BUILD
		if (filePath != nullptr)
		{
			if (!IsEmptyStr(pig->controllerBtnHandled[controllerIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[controllerIndex][btn].filePath); }
			pig->controllerBtnHandled[controllerIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
			pig->controllerBtnHandled[controllerIndex][btn].lineNumber = lineNumber;
		}
		#else
		UNUSED(filePath);
		UNUSED(lineNumber);
		#endif
	}
	
}
void HandleControllerBtnRelease_(i32 controllerIndex, ControllerBtn_t btn, const char* filePath, u64 lineNumber)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			pig->controllerBtnHandled[cIndex][btn].releaseHandled = true;
			#if DEBUG_BUILD
			if (filePath != nullptr)
			{
				if (!IsEmptyStr(pig->controllerBtnHandled[cIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[cIndex][btn].filePath); }
				pig->controllerBtnHandled[cIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
				pig->controllerBtnHandled[cIndex][btn].lineNumber = lineNumber;
			}
			#else
			UNUSED(filePath);
			UNUSED(lineNumber);
			#endif
		}
	}
	else
	{
		pig->controllerBtnHandled[controllerIndex][btn].releaseHandled = true;
		#if DEBUG_BUILD
		if (filePath != nullptr)
		{
			if (!IsEmptyStr(pig->controllerBtnHandled[controllerIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[controllerIndex][btn].filePath); }
			pig->controllerBtnHandled[controllerIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
			pig->controllerBtnHandled[controllerIndex][btn].lineNumber = lineNumber;
		}
		#else
		UNUSED(filePath);
		UNUSED(lineNumber);
		#endif
	}
}
void HandleControllerBtnExtended_(i32 controllerIndex, ControllerBtn_t btn, const char* filePath, u64 lineNumber)
{
	Assert(controllerIndex == CONTROLLER_INDEX_ANY || (controllerIndex >= 0 && controllerIndex < MAX_NUM_CONTROLLERS));
	Assert(btn < ControllerBtn_NumBtns);
	UNUSED(filePath);
	UNUSED(lineNumber);
	if (controllerIndex == CONTROLLER_INDEX_ANY)
	{
		for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
		{
			pig->controllerBtnHandled[cIndex][btn].pressHandled = true;
			pig->controllerBtnHandled[cIndex][btn].releaseHandled = true;
			pig->controllerBtnHandled[cIndex][btn].extendedHandled = true;
			#if DEBUG_BUILD
			if (filePath != nullptr)
			{
				if (!IsEmptyStr(pig->controllerBtnHandled[cIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[cIndex][btn].filePath); }
				pig->controllerBtnHandled[cIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
				pig->controllerBtnHandled[cIndex][btn].lineNumber = lineNumber;
			}
			#else
			UNUSED(filePath);
			UNUSED(lineNumber);
			#endif
		}
	}
	else
	{
		pig->controllerBtnHandled[controllerIndex][btn].pressHandled = true;
		pig->controllerBtnHandled[controllerIndex][btn].releaseHandled = true;
		pig->controllerBtnHandled[controllerIndex][btn].extendedHandled = true;
		#if DEBUG_BUILD
		if (filePath != nullptr)
		{
			if (!IsEmptyStr(pig->controllerBtnHandled[controllerIndex][btn].filePath)) { FreeString(fixedHeap, &pig->controllerBtnHandled[controllerIndex][btn].filePath); }
			pig->controllerBtnHandled[controllerIndex][btn].filePath = NewStringInArenaNt(fixedHeap, filePath);
			pig->controllerBtnHandled[controllerIndex][btn].lineNumber = lineNumber;
		}
		#else
		UNUSED(filePath);
		UNUSED(lineNumber);
		#endif
	}
}

#if DEBUG_BUILD
#define HandleControllerBtn(controllerIndex, btn)                  HandleControllerBtn_((controllerIndex), (btn), __FILE__, __LINE__)
#define HandleControllerBtnRelease(controllerIndex, btn)           HandleControllerBtnRelease_((controllerIndex), (btn), __FILE__, __LINE__)
#define HandleControllerBtnExtended(controllerIndex, btn)          HandleControllerBtnExtended_((controllerIndex), (btn), __FILE__, __LINE__)
#else
#define HandleControllerBtn(controllerIndex, btn)                  HandleControllerBtn_((controllerIndex), (btn), nullptr, 0)
#define HandleControllerBtnRelease(controllerIndex, btn)           HandleControllerBtnRelease_((controllerIndex), (btn), nullptr, 0)
#define HandleControllerBtnExtended(controllerIndex, btn)          HandleControllerBtnExtended_((controllerIndex), (btn), nullptr, 0)
#endif

// +==============================+
// |  Check and Handle Functions  |
// +==============================+
//NOTE: Extended button presses are a way for a piece of logic who only cares about the press or holding of a specific button to still
//      handle the entire sequence of events (press, hold, release) for a button so another check doesn't errantly handle the hold or release of the button
bool ControllerBtnDownAndHandle_(i32 controllerIndex, ControllerBtn_t btn, bool extended, const char* filePath, u64 lineNumber)
{
	if (ControllerBtnDown(controllerIndex, btn))
	{
		if (extended) { HandleControllerBtnExtended_(controllerIndex, btn, filePath, lineNumber); }
		else { HandleControllerBtn_(controllerIndex, btn, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool ControllerBtnPressedAndHandle_(i32 controllerIndex, ControllerBtn_t btn, bool extended, const char* filePath, u64 lineNumber)
{
	if (ControllerBtnPressed(controllerIndex, btn))
	{
		if (extended) { HandleControllerBtnExtended_(controllerIndex, btn, filePath, lineNumber); }
		else { HandleControllerBtn_(controllerIndex, btn, filePath, lineNumber); }
		return true;
	}
	return false;
}
bool ControllerBtnReleasedAndHandle_(i32 controllerIndex, ControllerBtn_t btn, const char* filePath, u64 lineNumber)
{
	if (ControllerBtnReleased(controllerIndex, btn))
	{
		HandleControllerBtnRelease_(controllerIndex, btn, filePath, lineNumber);
		return true;
	}
	return false;
}

#if DEBUG_BUILD
#define ControllerBtnDownAndHandle(controllerIndex, btn)            ControllerBtnDownAndHandle_((controllerIndex), (btn), false, __FILE__, __LINE__)
#define ControllerBtnDownAndHandleExtended(controllerIndex, btn)    ControllerBtnDownAndHandle_((controllerIndex), (btn), true, __FILE__, __LINE__)
#define ControllerBtnPressedAndHandle(controllerIndex, btn)         ControllerBtnPressedAndHandle_((controllerIndex), (btn), false, __FILE__, __LINE__)
#define ControllerBtnPressedAndHandleExtended(controllerIndex, btn) ControllerBtnPressedAndHandle_((controllerIndex), (btn), true, __FILE__, __LINE__)
#define ControllerBtnReleasedAndHandle(controllerIndex, btn)        ControllerBtnReleasedAndHandle_((controllerIndex), (btn), __FILE__, __LINE__)
#else
#define ControllerBtnDownAndHandle(controllerIndex, btn)            ControllerBtnDownAndHandle_((controllerIndex), (btn), false, nullptr, 0)
#define ControllerBtnDownAndHandleExtended(controllerIndex, btn)    ControllerBtnDownAndHandle_((controllerIndex), (btn), true, nullptr, 0)
#define ControllerBtnPressedAndHandle(controllerIndex, btn)         ControllerBtnPressedAndHandle_((controllerIndex), (btn), false, nullptr, 0)
#define ControllerBtnPressedAndHandleExtended(controllerIndex, btn) ControllerBtnPressedAndHandle_((controllerIndex), (btn), true, nullptr, 0)
#define ControllerBtnReleasedAndHandle(controllerIndex, btn)        ControllerBtnReleasedAndHandle_((controllerIndex), (btn), nullptr, 0)
#endif

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
	if (inputEvent->type == InputEventType_ControllerBtn && IsControllerBtnHandled(inputEvent->controllerBtn.controllerIndex, inputEvent->controllerBtn.btn, (inputEvent->controllerBtn.pressed || inputEvent->controllerBtn.repeated), inputEvent->controllerBtn.released)) { isHandled = true; }
	
	if (inputEvent->pairedEventIndex >= 0 && (u64)inputEvent->pairedEventIndex < pigIn->inputEvents.length)
	{
		InputEvent_t* pairedEvent = VarArrayGet(&pigIn->inputEvents, inputEvent->pairedEventIndex, InputEvent_t);
		if (pairedEvent->handled) { isHandled = true; }
		if (pairedEvent->type == InputEventType_Key && IsKeyHandled(pairedEvent->key.key, (pairedEvent->key.pressed || pairedEvent->key.repeated), pairedEvent->key.released)) { isHandled = true; }
		if (pairedEvent->type == InputEventType_MouseBtn && IsMouseHandled(pairedEvent->mouseBtn.btn, (pairedEvent->mouseBtn.pressed || pairedEvent->mouseBtn.repeated), pairedEvent->mouseBtn.released)) { isHandled = true; }
		if (pairedEvent->type == InputEventType_ControllerBtn && IsControllerBtnHandled(pairedEvent->controllerBtn.controllerIndex, pairedEvent->controllerBtn.btn, (pairedEvent->controllerBtn.pressed || pairedEvent->controllerBtn.repeated), pairedEvent->controllerBtn.released)) { isHandled = true; }
		
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
	if (inputEvent->type == InputEventType_ControllerBtn)
	{
		if (extended) { HandleControllerBtnExtended(inputEvent->controllerBtn.controllerIndex, inputEvent->controllerBtn.btn); }
		else if (inputEvent->controllerBtn.pressed || inputEvent->controllerBtn.repeated) { HandleControllerBtn(inputEvent->controllerBtn.controllerIndex, inputEvent->controllerBtn.btn); }
		else { HandleControllerBtnRelease(inputEvent->controllerBtn.controllerIndex, inputEvent->controllerBtn.btn); }
	}
	
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
		if (pairedEvent->type == InputEventType_ControllerBtn)
		{
			if (extended) { HandleControllerBtnExtended(pairedEvent->controllerBtn.controllerIndex, pairedEvent->controllerBtn.btn); }
			else if (pairedEvent->controllerBtn.pressed || pairedEvent->controllerBtn.repeated) { HandleControllerBtn(pairedEvent->controllerBtn.controllerIndex, pairedEvent->controllerBtn.btn); }
			else { HandleControllerBtnRelease(pairedEvent->controllerBtn.controllerIndex, pairedEvent->controllerBtn.btn); }
		}
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

// +--------------------------------------------------------------+
// |                         Monitor Info                         |
// +--------------------------------------------------------------+
const PlatMonitorInfo_t* GetCurrentMonitorInfoForDesktopRec(reci desktopRec, u64* monitorIndexOut = nullptr)
{
	const PlatMonitorInfo_t* result = nullptr;
	u64 resultIndex = 0;
	i32 resultOverlap = 0;
	
	const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
	for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
	{
		reci monitorRec = monitorInfo->desktopSpaceRec;
		reci overlapWithMonitorRec = ReciOverlap(monitorRec, desktopRec);
		i32 overlapAmount = (overlapWithMonitorRec.width * overlapWithMonitorRec.height);
		if (result == nullptr || overlapAmount >= resultOverlap)
		{
			result = monitorInfo;
			resultOverlap = overlapAmount;
			resultIndex = mIndex;
		}
		
		monitorInfo = LinkedListNext(&platInfo->monitors->list, PlatMonitorInfo_t, monitorInfo);
	}
	
	if (monitorIndexOut != nullptr) { *monitorIndexOut = resultIndex; }
	return result;
}
const PlatMonitorInfo_t* GetCurrentMonitorInfoForWindow(const PlatWindow_t* window, u64* monitorIndexOut = nullptr)
{
	NotNull(window);
	return GetCurrentMonitorInfoForDesktopRec(window->input.desktopRec, monitorIndexOut);
}
const PlatMonitorInfo_t* GetCurrentMonitorInfo(u64* monitorIndexOut = nullptr)
{
	return GetCurrentMonitorInfoForWindow(pig->currentWindow, monitorIndexOut);
}

const PlatMonitorInfo_t* GetPrimaryMonitorInfo(u64* monitorIndexOut = nullptr)
{
	NotNull(platInfo);
	NotNull(platInfo->monitors);
	Assert(platInfo->monitors->primaryIndex < platInfo->monitors->list.count);
	if (monitorIndexOut != nullptr) { *monitorIndexOut = platInfo->monitors->primaryIndex; }
	return LinkedListGet(&platInfo->monitors->list, PlatMonitorInfo_t, platInfo->monitors->primaryIndex);
}
const PlatMonitorInfo_t* GetMonitorInfoByNumber(u64 designatedNumber)
{
	const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
	for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
	{
		if (monitorInfo->designatedNumber == designatedNumber) { return monitorInfo; }
		monitorInfo = LinkedListNext(&platInfo->monitors->list, PlatMonitorInfo_t, monitorInfo);
	}
	return nullptr;
}

const PlatMonitorVideoMode_t* GetVideoModeWithResolution(const PlatMonitorInfo_t* monitorInfo, v2i resolution, bool giveClosestFit = false)
{
	NotNull(monitorInfo);
	const PlatMonitorVideoMode_t* closestFit = nullptr;
	i64 closestFitDifference = 0;
	VarArrayLoop(&monitorInfo->videoModes, vIndex)
	{
		VarArrayLoopGet(PlatMonitorVideoMode_t, videoMode, &monitorInfo->videoModes, vIndex);
		if (videoMode->resolution == resolution)
		{
			return videoMode;
		}
		else if (giveClosestFit)
		{
			i64 differenceInArea = ((i64)(videoMode->resolution.width * videoMode->resolution.height) - (i64)(resolution.width * resolution.height));
			if (closestFit == nullptr || (AbsI64(differenceInArea) < closestFitDifference))
			{
				closestFit = videoMode;
				closestFitDifference = differenceInArea;
			}
		}
	}
	return closestFit;
}
i64 FindVideoModeFramerateIndex(const PlatMonitorVideoMode_t* videoMode, i64 framerate, bool giveClosestFit = false)
{
	NotNull(videoMode);
	i64 closestFitIndex = -1;
	i64 closestFitDifference = 0;
	for (u64 fIndex = 0; fIndex < videoMode->numFramerates; fIndex++)
	{
		if (videoMode->framerates[fIndex] == framerate)
		{
			return (i64)fIndex;
		}
		else if (giveClosestFit)
		{
			i64 difference = (framerate - videoMode->framerates[fIndex]);
			if (closestFitIndex == -1 || difference < closestFitDifference)
			{
				closestFitIndex = (i64)fIndex;
				closestFitDifference = difference;
			}
		}
	}
	return closestFitIndex;
}

bool IsDesktopRecLostBetweenMonitors(reci desktopRec)
{
	const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
	for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
	{
		if (RecsIntersect(ToRec(monitorInfo->desktopSpaceRec), ToRec(desktopRec)))
		{
			return false;
		}
		monitorInfo = LinkedListNext(&platInfo->monitors->list, PlatMonitorInfo_t, monitorInfo);
	}
	return true;
}
bool IsWindowLostBetweenMonitors(const PlatWindow_t* window)
{
	NotNull(window);
	if (window->input.minimized) { return false; }
	if (window->input.maximized) { return false; }
	//TODO: Add a check for fullscreen
	return IsDesktopRecLostBetweenMonitors(window->input.desktopRec);
}
bool IsCurrentlyLostBetweenMonitors()
{
	return IsWindowLostBetweenMonitors(pig->currentWindow);
}

void UpdateSettingsWithWindowInfo(PigSettings_t* settings, const PlatWindow_t* window, MemArena_t* printArena)
{
	NotNull3(settings, window, printArena);
	const bool ignoreCase = true;
	if (window->input.fullscreen)
	{
		const PlatMonitorInfo_t* fullscreenMonitor = window->input.fullscreenMonitor;
		NotNull(fullscreenMonitor);
		const PlatMonitorVideoMode_t* fullscreenVideoMode = window->input.fullscreenVideoMode;
		NotNull(fullscreenVideoMode);
		Assert(window->input.fullscreenFramerateIndex < fullscreenVideoMode->numFramerates);
		i64 fullscreenFramerate = fullscreenVideoMode->framerates[window->input.fullscreenFramerateIndex];
		Assert(fullscreenFramerate > 0 && fullscreenFramerate < 300);
		
		PigSetSettingBool(settings, "Fullscreen", true, ignoreCase, printArena);
		PigSetSettingV2i(settings, "Resolution", fullscreenVideoMode->resolution, ignoreCase, printArena);
		
		PigSetSettingStr(settings, "Monitor", (fullscreenMonitor->name.length > 0) ? fullscreenMonitor->name : NewStr("[Unnamed]"));
		PigSetSettingU64(settings, "MonitorNumber", fullscreenMonitor->designatedNumber, ignoreCase, printArena);
		PigSetSettingU64(settings, "Framerate", (u64)fullscreenFramerate, ignoreCase, printArena);
		
		PigUnsetSetting(settings, "WindowedPosition");
		PigUnsetSetting(settings, "MaximizedWindow");
	}
	else
	{
		const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
		NotNull(currentMonitor);
		NotNullStr(&currentMonitor->name);
		v2i windowOffsetInMonitor = window->input.desktopInnerRec.topLeft - currentMonitor->desktopSpaceRec.topLeft;
		
		PigSetSettingBool(settings, "Fullscreen", false, ignoreCase, printArena);
		PigSetSettingV2i(settings, "Resolution", window->input.unmaximizedWindowSize, ignoreCase, printArena);
		
		PigSetSettingStr(settings, "Monitor", (currentMonitor->name.length > 0) ? currentMonitor->name : NewStr("[Unnamed]"));
		PigSetSettingU64(settings, "MonitorNumber", currentMonitor->designatedNumber, ignoreCase, printArena);
		PigUnsetSetting(settings, "Framerate");
		
		PigSetSettingV2i(settings, "WindowedPosition", window->input.unmaximizedWindowPos - currentMonitor->desktopSpaceRec.topLeft, ignoreCase, printArena);
		PigSetSettingBool(settings, "MaximizedWindow", window->input.maximized, ignoreCase, printArena);
	}
}

// +--------------------------------------------------------------+
// |                 Fullscreen Related Functions                 |
// +--------------------------------------------------------------+
void ToggleFullscreen(bool doDebugOutput = false)
{
	NotNull(platInfo);
	NotNull(pigOut);
	
	const PlatWindow_t* window = platInfo->mainWindow;
	NotNull(window);
	
	if (window->input.fullscreen)
	{
		pigOut->changeFullscreen = true;
		pigOut->changeFullscreenWindowId = window->id;
		pigOut->fullscreenEnabled = false;
		pigOut->windowedResolution = NewVec2i(
			window->input.fullscreenVideoMode->resolution.width/2,
			window->input.fullscreenVideoMode->resolution.height/2
		);
		Assert(window->input.fullscreenFramerateIndex < window->input.fullscreenVideoMode->numFramerates);
		pigOut->windowedFramerate = window->input.fullscreenVideoMode->framerates[window->input.fullscreenFramerateIndex];
		if (doDebugOutput)
		{
			PrintLine_I("Coming out of fullscreen on monitor %llu \"%.*s\". Going to windowed at %dx%d %lldHz",
				window->input.fullscreenMonitor->designatedNumber,
				window->input.fullscreenMonitor->name.length, window->input.fullscreenMonitor->name.pntr,
				pigOut->windowedResolution.width, pigOut->windowedResolution.height,
				pigOut->windowedFramerate
			);
		}
	}
	else
	{
		const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
		NotNull(currentMonitor);
		const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
		
		pigOut->changeFullscreen = true;
		pigOut->changeFullscreenWindowId = window->id;
		pigOut->fullscreenEnabled = true;
		pigOut->fullscreenMonitorId = currentMonitor->id;
		pigOut->fullscreenVideoModeIndex = currentVideoMode->index;
		Assert(currentVideoMode->currentFramerateIndex < currentVideoMode->numFramerates);
		pigOut->fullscreenFramerateIndex = currentVideoMode->currentFramerateIndex;
		if (doDebugOutput)
		{
			PrintLine_I("Enabling fullscreen on monitor %llu \"%.*s\" at %dx%d %lldHz",
				currentMonitor->designatedNumber,
				currentMonitor->name.length, currentMonitor->name.pntr,
				currentVideoMode->resolution.width, currentVideoMode->resolution.height,
				currentVideoMode->framerates[currentVideoMode->currentFramerateIndex]
			);
		}
	}
}

void DoFullscreenOnMonitor(const PlatWindow_t* window, const PlatMonitorInfo_t* monitor, v2i resolution, i64 framerate, bool doDebugOutput = false)
{
	NotNull(platInfo);
	NotNull(pigOut);
	NotNull(window);
	NotNull(monitor);
	const PlatMonitorVideoMode_t* targetVideoMode = GetVideoModeWithResolution(monitor, resolution);
	Assert(targetVideoMode != nullptr);
	i64 targetFramerateIndex = FindVideoModeFramerateIndex(targetVideoMode, framerate);
	Assert(targetFramerateIndex >= 0);
	
	if (doDebugOutput)
	{
		PrintLine_I("Changing to fullscreen mode %dx%d at %lldHz on monitor %d \"%.*s\"", resolution.width, resolution.height, framerate, monitor->designatedNumber, monitor->name.length, monitor->name.pntr);
	}
	pigOut->changeFullscreen = true;
	pigOut->changeFullscreenWindowId = window->id;
	pigOut->fullscreenEnabled = true;
	pigOut->fullscreenMonitorId = monitor->id;
	pigOut->fullscreenVideoModeIndex = targetVideoMode->index;
	pigOut->fullscreenFramerateIndex = (u64)targetFramerateIndex;
}

void StopFullscreen(const PlatWindow_t* window, v2i resolution, i64 framerate, bool doDebugOutput = false)
{
	NotNull(platInfo);
	NotNull(pigOut);
	NotNull(window);
	const PlatMonitorInfo_t* currentMonitor = GetCurrentMonitorInfoForWindow(window);
	const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&currentMonitor->videoModes, currentMonitor->currentVideoModeIndex, PlatMonitorVideoMode_t);
	NotNull2(currentMonitor, currentVideoMode);
	
	if (resolution.width > currentVideoMode->resolution.width)
	{
		if (doDebugOutput) { PrintLine_W("Limiting width to %d", currentVideoMode->resolution.width); }
		resolution.width = currentVideoMode->resolution.width;
	}
	if (resolution.height > currentVideoMode->resolution.height)
	{
		if (doDebugOutput) { PrintLine_W("Limiting height to %d", currentVideoMode->resolution.height); }
		resolution.height = currentVideoMode->resolution.height;
	}
				
	if (doDebugOutput)
	{
		PrintLine_I("Changing to windowed mode %dx%d at %lldHz", resolution.width, resolution.height, framerate);
	}
	pigOut->changeFullscreen = true;
	pigOut->changeFullscreenWindowId = window->id;
	pigOut->fullscreenEnabled = false;
	pigOut->windowedResolution = resolution;
	pigOut->windowedFramerate = framerate;
}

// +--------------------------------------------------------------+
// |                        Debug Bindings                        |
// +--------------------------------------------------------------+
void Pig_HandleDebugBindings(PigDebugBindings_t* bindings)
{
	NotNull(bindings);
	VarArrayLoop(&bindings->entries, bIndex)
	{
		VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &bindings->entries, bIndex);
		switch (binding->type)
		{
			case PigDebugBindingType_Keyboard:
			{
				bool modifiersHeld = true;
				for (u64 mIndex = 0; mIndex < ModifierKey_NumKeys; mIndex++)
				{
					ModifierKey_t modifierKey = (ModifierKey_t)((u8)1 << mIndex);
					if (IsFlagSet(binding->modifiers, modifierKey))
					{
						if (!KeyDownRaw(GetKeyForModifierKey(modifierKey)))
						{
							modifiersHeld = false;
							break;
						}
					}
				}
				
				if (modifiersHeld && KeyPressed(binding->key))
				{
					HandleKeyExtended(binding->key);
					PigParseDebugCommand(binding->commandStr);
				}
			} break;
			case PigDebugBindingType_Mouse:
			{
				if (MousePressed(binding->mouseBtn))
				{
					HandleMouseExtended(binding->mouseBtn);
					PigParseDebugCommand(binding->commandStr);
				}
			} break;
			case PigDebugBindingType_Controller:
			{
				for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
				{
					if (ControllerBtnPressed(cIndex, binding->controllerBtn))
					{
						HandleControllerBtnExtended(cIndex, binding->controllerBtn);
						PigParseDebugCommand(binding->commandStr);
					}
				}
			} break;
			default: DebugAssert(false); break;
		}
	}
}
