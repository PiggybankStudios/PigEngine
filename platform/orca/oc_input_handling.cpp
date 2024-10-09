/*
File:   oc_input_handling.cpp
Author: Taylor Robbins
Date:   10\08\2024
Description: 
	** Holds the logic for processing raw input from the Orca runtime and transforming
	** it into a format that is easy for the application to query and use (i.e. AppInput_t)
*/

void Orca_InitAppInput(AppInput_t* input)
{
	NotNull(input);
	ClearPointer(input);
	input->programStartTime = OC_ClockTime(OC_CLOCK_MONOTONIC);
}

void Orca_AppInputBeforeFrame(AppInput_t* input)
{
	r64 clockTime = OC_ClockTime(OC_CLOCK_MONOTONIC);
	r64 prevProgramTimeF = input->programTimeF;
	input->programTimeF = (clockTime - input->programStartTime) * 1000.0;
	input->prevProgramTime = input->programTime;
	input->programTime = (u64)input->programTimeF;
	input->elapsedMs = input->programTimeF - prevProgramTimeF;
	
	//TODO: Implement unixTime using OC_ClockTime(OC_CLOCK_DATE)
	//TODO: Can we implement localTime?
	
	ProgramTime = input->programTime;
	ElapsedMs = (r32)input->elapsedMs;
	TimeScale = (r32)input->timeDelta;
}

void Orca_AppInputAfterFrame(AppInput_t* input)
{
	NotNull(input);
	
	input->scrollChangedX = false;
	input->scrollChangedY = false;
	input->windowResized = false;
	input->mouseInsideWindowChanged = false;
	input->mouseMoved = false;
	
	for (u64 keyIndex = 0; keyIndex < Key_NumKeys; keyIndex++)
	{
		BtnState_t* btnState = &input->keyStates[keyIndex];
		btnState->wasDown = btnState->isDown;
		btnState->numPresses = 0;
		btnState->numReleases = 0;
		btnState->numTransitions = 0;
		btnState->numRepeats = 0;
		btnState->pressHandled = false;
		btnState->releaseHandled = false;
		btnState->extendedHandled = false;
		#if DEBUG_BUILD
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		#endif
	}
	for (u64 btnIndex = 0; btnIndex < MouseBtn_NumBtns; btnIndex++)
	{
		BtnState_t* btnState = &input->mouseBtnStates[btnIndex];
		btnState->wasDown = btnState->isDown;
		btnState->numPresses = 0;
		btnState->numReleases = 0;
		btnState->numTransitions = 0;
		btnState->numRepeats = 0;
		btnState->pressHandled = false;
		btnState->releaseHandled = false;
		btnState->extendedHandled = false;
		#if DEBUG_BUILD
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		#endif
	}
}

void Orca_AppInputHandleRawEvent(AppInput_t* input, OC_Event_t* event)
{
	NotNull(input);
	//TODO: Do we want to handle any of these?
	//  OC_EVENT_KEYBOARD_MODS,  OC_EVENT_KEYBOARD_KEY, OC_EVENT_KEYBOARD_CHAR,
	//  OC_EVENT_MOUSE_BUTTON,   OC_EVENT_MOUSE_MOVE,   OC_EVENT_MOUSE_WHEEL,
	//  OC_EVENT_MOUSE_ENTER,    OC_EVENT_MOUSE_LEAVE,  OC_EVENT_CLIPBOARD_PASTE,
	//  OC_EVENT_WINDOW_RESIZE,  OC_EVENT_WINDOW_MOVE,  OC_EVENT_WINDOW_FOCUS,
	//  OC_EVENT_WINDOW_UNFOCUS, OC_EVENT_WINDOW_HIDE,  OC_EVENT_WINDOW_SHOW,
	//  OC_EVENT_WINDOW_CLOSE,   OC_EVENT_PATHDROP,     OC_EVENT_FRAME,
	//  OC_EVENT_QUIT
}

void Orca_AppInputHandleResize(AppInput_t* input, v2i newSize)
{
	NotNull(input);
	if (newSize != input->pixelResolution)
	{
		input->prevPixelResolution = input->pixelResolution;
		input->pixelResolution = newSize;
		input->renderResolution = ToVec2(input->pixelResolution);
		input->windowResized = true;
	}
	ScreenSize = input->renderResolution;
	ScreenSizei = input->pixelResolution;
	ScreenRec = NewRec(0, 0, ScreenSize);
}

void Orca_AppInputHandleMouseBtnEvent(AppInput_t* input, OC_MouseButton_t orcaMouseButton, bool isDown)
{
	NotNull(input);
	MouseBtn_t mouseBtn = GetMouseBtnForOrcaMouseButton(orcaMouseButton);
	if (mouseBtn != MouseBtn_None && mouseBtn < MouseBtn_NumBtns)
	{
		BtnState_t* btnState = &input->mouseBtnStates[mouseBtn];
		if (btnState->isDown != isDown)
		{
			btnState->isDown = isDown;
			IncrementU8(btnState->numTransitions);
			if (isDown) { IncrementU8(btnState->numPresses); }
			else { IncrementU8(btnState->numReleases); }
			btnState->lastChangeTime = input->programTime; //TODO: Should we take a more accurate time?
		}
	}
}

//TODO: Does this always have the correct initial value? i.e. will we always get a mouse entered event even if the mouse was inside us on startup?
void Orca_AppInputHandleMouseHoverEvent(AppInput_t* input, bool isOverWindow)
{
	NotNull(input);
	if (input->mouseInsideWindow != isOverWindow)
	{
		input->mouseInsideWindow = isOverWindow;
		input->mouseInsideWindowChanged = true;
	}
}

void Orca_AppInputHandleMouseMove(AppInput_t* input, v2 newPos, v2 deltaVec)
{
	NotNull(input);
	if (newPos != input->mousePos)
	{
		input->mouseMoved = true;
		input->mouseDelta += (newPos - input->mousePos);
		input->mousePos = newPos;
		input->mousePosi = Vec2Roundi(input->mousePos);
	}
	MousePos = input->mousePos;
}

void Orca_AppInputHandleMouseWheel(AppInput_t* input, v2 deltaVec)
{
	NotNull(input);
	if (deltaVec.x != 0.0f)
	{
		input->scrollValue.x += deltaVec.x;
		input->scrollDelta.x += deltaVec.x;
		input->scrollChangedX = true;
	}
	if (deltaVec.y != 0.0f)
	{
		input->scrollValue.y += deltaVec.y;
		input->scrollDelta.y += deltaVec.y;
		input->scrollChangedY = true;
	}
}

//TODO: Does this notify us when a key is held and OS-level repetition kicks in?
void Orca_AppInputHandleKeyEvent(AppInput_t* input, OC_ScanCode_t orcaScanCode, OC_KeyCode_t orcaKeyCode, bool isDown)
{
	NotNull(input);
	Key_t key = GetKeyForOrcaKeyCode(orcaKeyCode);
	if (key != Key_None && key < Key_NumKeys)
	{
		BtnState_t* btnState = &input->keyStates[key];
		if (btnState->isDown != isDown)
		{
			btnState->isDown = isDown;
			IncrementU8(btnState->numTransitions);
			if (isDown) { IncrementU8(btnState->numPresses); }
			else { IncrementU8(btnState->numReleases); }
			btnState->lastChangeTime = input->programTime; //TODO: Should we take a more accurate time?
		}
	}
}
