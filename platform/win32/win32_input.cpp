/*
File:   win32_input.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description: 
	** Holds functions that help us handle various kinds of input from the user and system
	** Generally this is keyboard, mouse, gamepad (joystick or controller), time, and other misc information
*/

#define TARGET_FRAMERATE                    60 //fps
#define TARGET_FRAME_TIME                   (1000.0 / 60) //ms
#define NUM_FRAMERATE_AVGS                  5  //frames
#define CONTROLLER_STICK_DEADZONE           0.2f //magnitude
#define CONTROLLER_STICK_DIR_DEADZONE       0.6f //magnitude
#define CONTROLLER_TRIGGER_ACTIVE_THRESHOLD 0.5f //triggerValue TODO: Check this value!
#define STABLE_FRAMERATE_MAX                66 //ms
#define STABLE_FRAMERATE_RUN_MIN            3 //frames

// +--------------------------------------------------------------+
// |                         Engine Input                         |
// +--------------------------------------------------------------+
void Win32_FillEngineInput(EngineInput_t* input)
{
	NotNull(input);
	CreateVarArray(&input->inputEvents, &Platform->mainHeap, sizeof(InputEvent_t));
	for (u64 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
	{
		input->controllerStates[cIndex].index = cIndex;
	}
}

void Win32_CopyEngineInput(EngineInput_t* dest, EngineInput_t* source)
{
	NotNull(dest);
	NotNull(source);
	
	//Free Stuff
	VarArrayLoop(&dest->inputEvents, eIndex)
	{
		VarArrayLoopGet(InputEvent_t, destEvent, &dest->inputEvents, eIndex);
		if (destEvent->type == InputEventType_FileDropped && destEvent->droppedFile.filePath.pntr != nullptr)
		{
			FreeString(&Platform->mainHeap, &destEvent->droppedFile.filePath);
		}
	}
	FreeVarArray(&dest->inputEvents);
	
	for (u64 cIndex = 0; cIndex < ArrayCount(dest->controllerStates); cIndex++)
	{
		PlatControllerState_t* controller = &dest->controllerStates[cIndex];
		FreeString(&Platform->mainHeap, &controller->name);
		FreeString(&Platform->mainHeap, &controller->typeIdStr);
	}
	
	//Copy the whole structure
	MyMemCopy(dest, source, sizeof(EngineInput_t));
	
	//Reallocate things so we don't conflict with allocations that got copied in the MemCopy
	CreateVarArray(&dest->inputEvents, &Platform->mainHeap, sizeof(InputEvent_t));
	VarArrayAddVarArray(&dest->inputEvents, &source->inputEvents, 0);
	VarArrayLoop(&dest->inputEvents, eIndex)
	{
		VarArrayLoopGet(InputEvent_t, destEvent, &dest->inputEvents, eIndex);
		if (destEvent->type == InputEventType_FileDropped && destEvent->droppedFile.filePath.pntr != nullptr)
		{
			destEvent->droppedFile.filePath = AllocString(&Platform->mainHeap, &destEvent->droppedFile.filePath);
		}
	}
	
	for (u64 cIndex = 0; cIndex < ArrayCount(dest->controllerStates); cIndex++)
	{
		PlatControllerState_t* controller = &dest->controllerStates[cIndex];
		if (controller->name.pntr != nullptr) { controller->name = AllocString(&Platform->mainHeap, &controller->name); }
		if (controller->typeIdStr.pntr != nullptr) { controller->typeIdStr = AllocString(&Platform->mainHeap, &controller->typeIdStr); }
	}
}

void Win32_RefreshButtonState(PlatBtnState_t* state)
{
	NotNull(state);
	state->wasDown = state->isDown;
	state->numPresses = 0;
	state->numReleases = 0;
	state->numTransitions = 0;
	state->numRepeats = 0;
}

void Win32_ResetEngineInput(EngineInput_t* input)
{
	NotNull(input);
	
	input->actualElapsedMsIgnored = false;
	input->scrollChangedX = false;
	input->scrollChangedY = false;
	input->scrollDelta = Vec2_Zero;
	
	for (u64 keyIndex = 0; keyIndex < ArrayCount(input->keyStates); keyIndex++)
	{
		Win32_RefreshButtonState(&input->keyStates[keyIndex]);
	}
	
	for (u64 bIndex = 0; bIndex < ArrayCount(input->mouseBtnStates); bIndex++)
	{
		Win32_RefreshButtonState(&input->mouseBtnStates[bIndex]);
	}
	
	for (u64 cIndex = 0; cIndex < ArrayCount(input->controllerStates); cIndex++)
	{
		PlatControllerState_t* controller = &input->controllerStates[cIndex];
		controller->connectedChanged = false;
		for (u64 bIndex = 0; bIndex < ControllerBtn_NumBtns; bIndex++)
		{
			Win32_RefreshButtonState(&controller->btnStates[bIndex]);
		}
	}
	
	VarArrayLoop(&input->inputEvents, eIndex)
	{
		VarArrayLoopGet(InputEvent_t, inputEvent, &input->inputEvents, eIndex);
		if (inputEvent->type == InputEventType_FileDropped && inputEvent->droppedFile.filePath.pntr != nullptr)
		{
			FreeString(&Platform->mainHeap, &inputEvent->droppedFile.filePath);
		}
	}
	VarArrayClear(&input->inputEvents);
}

void Win32_UpdateEngineInputTimeInfo(EngineInput_t* prevInput, EngineInput_t* newInput, bool windowInteractionOccurred)
{
	NotNull(newInput);
	
	if (Platform->engineOutput.fixedTimeScaleEnabled)
	{
		r64 lastTimeF = (prevInput != nullptr ? prevInput->programTimeF : 0.0);
		r64 scaledFrameTime = TARGET_FRAME_TIME * Platform->engineOutput.fixedTimeScale;
		r64 pretendProgramTimeF = lastTimeF + scaledFrameTime;
		u64 pretendProgramTime = (u64)pretendProgramTimeF;
		r64 actualProgramTimeF = 0;
		u64 actualProgramTime = Win32_GetProgramTime(&actualProgramTimeF, true);
		Platform->programTimeIsBehind = (pretendProgramTime < actualProgramTime);
		Platform->programTimeIsAhead = (pretendProgramTime > actualProgramTime);
		Platform->absProgramTimeDiffF = (Platform->programTimeIsBehind ? (actualProgramTimeF - pretendProgramTimeF) : (pretendProgramTimeF - actualProgramTimeF));
		Platform->absProgramTimeDiff  = (Platform->programTimeIsBehind ? (actualProgramTime  - pretendProgramTime)  : (pretendProgramTime  - actualProgramTime));
		newInput->programTimeF = pretendProgramTimeF;
		newInput->programTime = pretendProgramTime;
		newInput->elapsedMs = newInput->programTimeF - lastTimeF;
	}
	else
	{
		r64 lastTimeF = (prevInput != nullptr ? prevInput->programTimeF : 0.0);
		newInput->programTime = Win32_GetProgramTime(&newInput->programTimeF, true);
		if (Platform->programTimeIsBehind)
		{
			newInput->programTimeF -= Platform->absProgramTimeDiffF;
			newInput->programTime -= Platform->absProgramTimeDiff;
		}
		else if (Platform->programTimeIsAhead)
		{
			newInput->programTimeF += Platform->absProgramTimeDiffF;
			newInput->programTime += Platform->absProgramTimeDiff;
		}
		newInput->elapsedMs = newInput->programTimeF - lastTimeF;
	}
	
	newInput->lastUpdateElapsedMs = newInput->elapsedMs - Platform->timeSpentOnSwapBuffers;
	newInput->timeSpentWaitingLastFrame = Platform->timeSpentOnSwapBuffers;
	Platform->timeSpentOnSwapBuffers = 0.0;
	
	newInput->uncappedElapsedMs = newInput->elapsedMs;
	if (!Platform->engineOutput.fixedTimeScaleEnabled && (windowInteractionOccurred || !Platform->hasReachedStableFramerate))
	{
		//NOTE: Anything based off programTime will still jump far ahead but I think that should be fine.
		//      Anything that shouldn't be scaled arbitrarily by long frame hitches should be based off elapsedMs, not programTime
		// PrintLine_D("Clamping elapsedMs from %lf to %lf", newInput->elapsedMs, (1000.0 / 60.0));
		newInput->actualElapsedMsIgnored = true;
		r64 newElapsedMs = TARGET_FRAME_TIME;
		newInput->lastUpdateElapsedMs = (newInput->lastUpdateElapsedMs / newInput->elapsedMs) * newElapsedMs;
		newInput->timeSpentWaitingLastFrame = (newInput->timeSpentWaitingLastFrame / newInput->elapsedMs) * newElapsedMs;
		newInput->elapsedMs = newElapsedMs;
	}
	else
	{
		newInput->actualElapsedMsIgnored = false;
	}
	
	newInput->framerate = 1000.0 / newInput->elapsedMs;
	newInput->timeDelta = newInput->elapsedMs / (1000.0 / (r64)TARGET_FRAMERATE);
	newInput->avgElapsedMs = ((newInput->avgElapsedMs * (NUM_FRAMERATE_AVGS-1)) + newInput->elapsedMs) / NUM_FRAMERATE_AVGS;
	newInput->avgFramerate = ((newInput->avgFramerate * (NUM_FRAMERATE_AVGS-1)) + newInput->framerate) / NUM_FRAMERATE_AVGS;
	
	u64 unixTimestamp = Win32_GetCurrentTimestamp(false);
	ConvertTimestampToRealTime(unixTimestamp, &newInput->unixTime, false);
	
	MyStr_t timezoneName = MyStr_Empty;
	u64 localTimestamp = Win32_GetCurrentTimestamp(true, &newInput->localTimezoneOffset, &newInput->localTimezoneDoesDst, &timezoneName);
	ConvertTimestampToRealTime(localTimestamp, &newInput->localTime, newInput->localTimezoneDoesDst);
	if (StrCompareIgnoreCase(Platform->localTimezoneName, timezoneName) != 0)
	{
		if (!IsEmptyStr(Platform->localTimezoneName)) { FreeString(&Platform->mainHeap, &Platform->localTimezoneName); }
		Platform->localTimezoneName = AllocString(&Platform->mainHeap, &timezoneName);
	}
	newInput->localTimezoneName = Platform->localTimezoneName;
}

void Win32_CheckForStableFramerate(r64 elapsedMsLastFrame)
{
	if (!Platform->hasReachedStableFramerate)
	{
		if (elapsedMsLastFrame < STABLE_FRAMERATE_MAX)
		{
			Platform->numStableElapsedMsInARow++;
			if (Platform->numStableElapsedMsInARow >= STABLE_FRAMERATE_RUN_MIN)
			{
				Platform->hasReachedStableFramerate = true;
			}
		}
		else
		{
			Platform->numStableElapsedMsInARow = 0;
		}
	}
}

// +--------------------------------------------------------------+
// |                     Window Engine Input                      |
// +--------------------------------------------------------------+
void Win32_InitWindowEngineInput(PlatWindow_t* window, const PlatWindowCreateOptions_t* creationOptions, WindowEngineInput_t* input)
{
	NotNull(window);
	NotNull(input);
	
	ClearPointer(input);
	
	int windowWidth = 0;
	int windowHeight = 0;
	glfwGetWindowSize(window->handle, &windowWidth, &windowHeight);
	
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwGetFramebufferSize(window->handle, &framebufferWidth, &framebufferHeight);
	
	input->pixelResolution   = NewVec2i(framebufferWidth, framebufferHeight);
	input->windowResolution  = NewVec2i(     windowWidth,      windowHeight);
	input->contextResolution = NewVec2i(framebufferWidth, framebufferHeight); //TODO: When is this different? Only on OSX or High DPI screens?
	input->renderResolution  = NewVec2 ((r32)windowWidth, (r32)windowHeight); //TODO: When is this different? Only on OSX or High DPI screens?
	
	GLFWmonitor* fullscreenMonitorHandle = glfwGetWindowMonitor(window->handle); 	
	if (creationOptions->fullscreen)
	{
		input->fullscreen = true;
		NotNull(creationOptions->fullscreenMonitor);
		NotNull(creationOptions->fullscreenVideoMode);
		Assert(creationOptions->fullscreenFramerateIndex < creationOptions->fullscreenVideoMode->numFramerates);
		input->fullscreenMonitor = creationOptions->fullscreenMonitor;
		input->fullscreenVideoMode = creationOptions->fullscreenVideoMode;
		input->fullscreenFramerateIndex = creationOptions->fullscreenFramerateIndex;
		input->fullscreenFramerate = input->fullscreenVideoMode->framerates[input->fullscreenFramerateIndex];
		
		input->maximized = false;
		input->minimized = false;
		
		input->desktopRec = input->fullscreenMonitor->desktopSpaceRec;
		input->desktopInnerRec = input->fullscreenMonitor->desktopSpaceRec;
		input->unmaximizedWindowPos = NewVec2i(
			input->fullscreenMonitor->desktopSpaceRec.x + input->fullscreenMonitor->desktopSpaceRec.width/2,
			input->fullscreenMonitor->desktopSpaceRec.y + input->fullscreenMonitor->desktopSpaceRec.height/2
		);
		input->unmaximizedWindowSize = NewVec2i(
			input->fullscreenMonitor->desktopSpaceRec.width/2,
			input->fullscreenMonitor->desktopSpaceRec.height/2
		);
	}
	else 
	{
		input->fullscreen = false;
		input->maximized = (glfwGetWindowAttrib(window->handle, GLFW_MAXIMIZED) > 0);
		input->minimized = false;
		
		int windowPosX = 0;
		int windowPosY = 0;
		glfwGetWindowPos(window->handle, &windowPosX, &windowPosY);
		
		int windowLeftBorderSize = 0;
		int windowTopBorderSize = 0;
		int windowRightBorderSize = 0;
		int windowBottomBorderSize = 0;
		glfwGetWindowFrameSize(window->handle, &windowLeftBorderSize, &windowTopBorderSize, &windowRightBorderSize, &windowBottomBorderSize);
		
		//NOTE: On windows 10, this returns an extra 8 pixels of extra padding on all sides. We don't actually want that
		if (windowLeftBorderSize >= 8) { windowLeftBorderSize -= 8; }
		else { windowLeftBorderSize = 0; }
		if (windowTopBorderSize >= 8) { windowTopBorderSize -= 8; }
		else { windowTopBorderSize = 0; }
		if (windowRightBorderSize >= 8) { windowRightBorderSize -= 8; }
		else { windowRightBorderSize = 0; }
		if (windowBottomBorderSize >= 8) { windowBottomBorderSize -= 8; }
		else { windowBottomBorderSize = 0; }
		
		input->desktopRec = NewReci(
			windowPosX - windowLeftBorderSize, windowPosY - windowTopBorderSize,
			windowLeftBorderSize + windowWidth + windowRightBorderSize,
			windowTopBorderSize + windowHeight + windowBottomBorderSize
		);
		input->desktopInnerRec = NewReci(
			windowPosX, windowPosY,
			windowWidth, windowHeight
		);
		input->unmaximizedWindowPos = input->desktopInnerRec.topLeft;
		input->unmaximizedWindowSize = input->windowResolution;
	}
}

void Win32_CopyWindowEngineInput(WindowEngineInput_t* dest, WindowEngineInput_t* source)
{
	NotNull(dest);
	NotNull(source);
	
	//TODO: Free any members that are allocated in dest
	
	MyMemCopy(dest, source, sizeof(WindowEngineInput_t));
}

void Win32_ResetWindowEngineInput(WindowEngineInput_t* input)
{
	NotNull(input);
	input->windowInteractionOccurred = false;
	input->minimizedChanged = false;
	input->maximizedChanged = false;
	input->fullscreenChanged = false;
	input->resized = false;
	input->moved = false;
	input->isFocusedChanged = false;
	input->mouseInsideWindowChanged = false;
	input->mouseMoved = false;
	input->mouseDelta = Vec2_Zero;
}

// +--------------------------------------------------------------+
// |           Helper Functions used by win32_glfw.cpp            |
// +--------------------------------------------------------------+
ModifierKey_t Win32_GetCurrentModifierKeys(int glfwModifiers = -1)
{
	bool ctrl = false;
	bool alt = false;
	bool shift = false;
	bool super = false;
	bool capsLock = false;
	if (glfwModifiers >= 0)
	{
		ctrl     = IsFlagSet(glfwModifiers, GLFW_MOD_CONTROL);
		alt      = IsFlagSet(glfwModifiers, GLFW_MOD_ALT);
		shift    = IsFlagSet(glfwModifiers, GLFW_MOD_SHIFT);
		super    = IsFlagSet(glfwModifiers, GLFW_MOD_SUPER);
		capsLock = IsFlagSet(glfwModifiers, GLFW_MOD_CAPS_LOCK);
	}
	else
	{
		//TODO: Look up the key state through GLFW functions
	}
	
	//TODO: This is win32 platform specific, Was there a reason we opted for this? Was the GLFW one not reliable??
	SHORT capsKeyState = GetKeyState(VK_CAPITAL);
	capsLock = IsFlagSet(capsKeyState, 0x01);
	
	ModifierKey_t result = ModifierKey_None;
	if (ctrl || super) { FlagEnumSet(result, ModifierKey_Ctrl,     ModifierKey_t, u8); }
	if (alt)           { FlagEnumSet(result, ModifierKey_Alt,      ModifierKey_t, u8); }
	if (shift)         { FlagEnumSet(result, ModifierKey_Shift,    ModifierKey_t, u8); }
	if (capsLock)      { FlagEnumSet(result, ModifierKey_CapsLock, ModifierKey_t, u8); }
	return result;
}

InputEvent_t* Win32_CreateInputEvent(PlatWindow_t* window, EngineInput_t* currentInput, ModifierKey_t modifierKeys, InputEventType_t type)
{
	AssertSingleThreaded();
	//window can be nullptr
	NotNull(currentInput);
	Assert(type != InputEventType_None);
	
	if (InitPhase < Win32InitPhase_Initialized) { return nullptr; }
	
	InputEvent_t* result = VarArrayAdd(&currentInput->inputEvents, InputEvent_t);
	if (result == nullptr) { DebugAssert(false); return nullptr; }
	ClearPointer(result);
	result->index = currentInput->inputEvents.length-1;
	result->type = type;
	result->id = Platform->nextInputEventId;
	Platform->nextInputEventId++;
	result->handled = false;
	result->window = window;
	result->hadFocus = (window != nullptr ? window->activeInput.isFocused : true); //TODO: Replace "true" with some sort of application wide idea of whether any of our windows are focused
	result->mouseInsideWindow = (window != nullptr ? window->activeInput.mouseInsideWindow : true); //TODO: Replace "true" with some sort of application wide idea of whether the mouse is inside any of our windows
	result->mousePos = (window != nullptr ? window->activeInput.mousePos : Vec2_Zero);
	result->modifiers = modifierKeys;
	
	return result;
}
InputEvent_t* Win32_GetPairedInputEvent(EngineInput_t* input, InputEvent_t* event)
{
	NotNull(input);
	NotNull(event);
	InputEvent_t* previousEvent = VarArrayGetLastSoft(&input->inputEvents, InputEvent_t);
	if (previousEvent != nullptr && previousEvent->id == event->id)
	{
		previousEvent = VarArrayGetSoft(&input->inputEvents, input->inputEvents.length-2, InputEvent_t);
	}
	if (previousEvent == nullptr) { return nullptr; }
	Assert(previousEvent->id != event->id);
	if (previousEvent->pairedEventIndex != -1) { return nullptr; }
	
	//Pair keyboard events and character events together (a key press caused the character to be typed)
	if ((event->type == InputEventType_Key && previousEvent->type == InputEventType_Character) ||
		(event->type == InputEventType_Character && previousEvent->type == InputEventType_Key))
	{
		InputEvent_t* characterEvent = (event->type == InputEventType_Character) ? event : previousEvent;
		InputEvent_t* keyEvent       = (event->type == InputEventType_Key)       ? event : previousEvent;
		UNUSED(characterEvent); //TODO: Do we need these variables?
		UNUSED(keyEvent); //TODO: Do we need these variables?
		//TODO: Should we somehow verify that this character makes sense for this key??
		return previousEvent;
	}
	
	//Pair focus gain to previous focus lost events (focus moved from one window directly to another)
	if (event->type == InputEventType_WindowFocus && previousEvent->type == InputEventType_WindowFocus)
	{
		if (event->windowFocus.focused && !previousEvent->windowFocus.focused)
		{
			return previousEvent;
		}
	}
	
	//Pair mouse enter to previous mouse leave events (mouse moved from one window directly to another)
	if (event->type == InputEventType_MouseHover && previousEvent->type == InputEventType_MouseHover)
	{
		if (event->mouseHover.entered && !previousEvent->mouseHover.entered)
		{
			return previousEvent;
		}
	}
	
	return nullptr;
}
bool Win32_PairInputEvent(EngineInput_t* input, InputEvent_t* event)
{
	InputEvent_t* pairedEvent = Win32_GetPairedInputEvent(input, event);
	if (pairedEvent == nullptr) { event->pairedEventIndex = -1; return false; }
	if (pairedEvent->pairedEventIndex == -1) { pairedEvent->pairedEventIndex = (i64)event->index; }
	event->pairedEventIndex = (i64)pairedEvent->index;
	return true;
}

//returns whether the event actually had an effect on the btnState's isDown or numRepeats
bool Win32_HandleMouseEvent(PlatWindow_t* window, EngineInput_t* currentInput, int glfwButtonCode, int action, int glfwModifiers)
{
	NotNull(window);
	NotNull(window->handle);
	MouseBtn_t btn = GetMouseBtnForGlfwBtnCode(glfwButtonCode);
	if (btn >= MouseBtn_NumBtns) { return false; } //No button mapped to this glfw code
	if (window->closed) { return false; }
	
	PlatBtnState_t* btnState = &currentInput->mouseBtnStates[btn];
	
	bool pressed = (action == GLFW_PRESS);
	ModifierKey_t modifiers = Win32_GetCurrentModifierKeys(glfwModifiers);
	
	bool result = false;
	if (action == GLFW_REPEAT)
	{
		result = true;
		IncrementU8(btnState->numRepeats);
		
		InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_MouseBtn);
		if (newEvent != nullptr)
		{
			newEvent->mouseBtn.btn = btn;
			newEvent->mouseBtn.pressed = false;
			newEvent->mouseBtn.released = false;
			newEvent->mouseBtn.repeated = true;
		}
	}
	else if (btnState->isDown != pressed)
	{
		result = true;
		if (pressed)
		{
			IncrementU8(btnState->numPresses);
			IncrementU8(btnState->numTransitions);
			btnState->isDown = true;
			btnState->lastChangeTime = Win32_GetProgramTime(nullptr, false);
			
			//TODO: Move this logic to the calling code?
			// if (button == MouseButton_Left)
			// {
			// 	currentInput->mouseStartPos[MouseButton_Left] = currentInput->mousePos;
			// 	currentInput->mouseMaxDist[MouseButton_Left] = 0;
			// }
			// else if (button == MouseButton_Right)
			// {
			// 	currentInput->mouseStartPos[MouseButton_Right] = currentInput->mousePos;
			// 	currentInput->mouseMaxDist[MouseButton_Right] = 0;
			// }
			// else if (button == MouseButton_Middle)
			// {
			// 	currentInput->mouseStartPos[MouseButton_Middle] = currentInput->mousePos;
			// 	currentInput->mouseMaxDist[MouseButton_Middle] = 0;
			// }
			
			InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_MouseBtn);
			if (newEvent != nullptr)
			{
				newEvent->mouseBtn.btn = btn;
				newEvent->mouseBtn.pressed = true;
				newEvent->mouseBtn.released = false;
				newEvent->mouseBtn.repeated = false;
			}
		}
		else
		{
			//NOTE: If the button is being released we need to check for multiple button mappings
			//		and only trigger when the last one is released
			bool foundOtherHeldBtn = false;
			int btnCodes[MAX_GLFW_BTN_CODES_PER_BTN] = {};
			
			u8 numBtns = GetGlfwBtnCodesForMouseBtn(btn, btnCodes);
			for (u8 btnIndex = 0; btnIndex < numBtns; btnIndex++)
			{
				if (btnCodes[btnIndex] != glfwButtonCode && glfwGetMouseButton(window->handle, btnCodes[btnIndex]) == GLFW_PRESS)
				{
					WriteLine_D("Other mouse button held, ignoring button release");
					foundOtherHeldBtn = true;
					break;
				}
			}
			
			if (!foundOtherHeldBtn)
			{
				IncrementU8(btnState->numReleases);
				IncrementU8(btnState->numTransitions);
				btnState->isDown = false;
				btnState->lastChangeTime = Win32_GetProgramTime(nullptr, false);
				
				InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_MouseBtn);
				if (newEvent != nullptr)
				{
					newEvent->mouseBtn.btn = btn;
					newEvent->mouseBtn.pressed = false;
					newEvent->mouseBtn.released = true;
					newEvent->mouseBtn.repeated = false;
				}
			}
		}
	}
	
	return result;
}

//returns whether the event actually had an effect on the btnState's isDown or numRepeats
bool Win32_HandleKeyEvent(PlatWindow_t* window, EngineInput_t* currentInput, int glfwKeyCode, int action, int glfwModifiers)
{
	NotNull(window);
	NotNull(window->handle);
	Key_t key = GetKeyForGlfwKeyCode(glfwKeyCode);
	if (key >= Key_NumKeys) { return false; } //No key mapped to this glfw code
	if (window->closed) { return false; }
	
	PlatBtnState_t* btnState = &currentInput->keyStates[key];
	
	bool pressed = (action == GLFW_PRESS);
	ModifierKey_t modifiers = Win32_GetCurrentModifierKeys(glfwModifiers);
	
	bool result = false;
	if (action == GLFW_REPEAT)
	{
		result = true;
		IncrementU8(btnState->numRepeats);
		
		InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_Key);
		if (newEvent != nullptr)
		{
			newEvent->key.key = key;
			newEvent->key.pressed = false;
			newEvent->key.released = false;
			newEvent->key.repeated = true;
			Win32_PairInputEvent(currentInput, newEvent);
		}
	}
	else if (btnState->isDown != pressed)
	{
		result = true;
		if (pressed)
		{
			IncrementU8(btnState->numPresses);
			IncrementU8(btnState->numTransitions);
			btnState->isDown = true;
			btnState->lastChangeTime = Win32_GetProgramTime(nullptr, false);
			
			InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_Key);
			if (newEvent != nullptr)
			{
				newEvent->key.key = key;
				newEvent->key.pressed = true;
				newEvent->key.released = false;
				newEvent->key.repeated = false;
				Win32_PairInputEvent(currentInput, newEvent);
			}
		}
		else
		{
			//NOTE: If the key is being released we need to check for multiple key code mappings
			//		and only trigger when the last one is released
			bool foundOtherHeldKey = false;
			int keyCodes[MAX_GLFW_KEY_CODES_PER_KEY] = {};
			
			u8 numKeyCodes = GetGlfwKeyCodesForKey(key, keyCodes);
			for (u8 keyIndex = 0; keyIndex < numKeyCodes; keyIndex++)
			{
				if (keyCodes[keyIndex] != glfwKeyCode && glfwGetKey(window->handle, keyCodes[keyIndex]) == GLFW_PRESS)
				{
					WriteLine_D("Other glfw key held, ignoring key release");
					foundOtherHeldKey = true;
					break;
				}
			}
			
			if (!foundOtherHeldKey)
			{
				IncrementU8(btnState->numReleases);
				IncrementU8(btnState->numTransitions);
				btnState->isDown = false;
				btnState->lastChangeTime = Win32_GetProgramTime(nullptr, false);
				
				InputEvent_t* newEvent = Win32_CreateInputEvent(window, currentInput, modifiers, InputEventType_Key);
				if (newEvent != nullptr)
				{
					newEvent->key.key = key;
					newEvent->key.pressed = false;
					newEvent->key.released = true;
					newEvent->key.repeated = false;
					Win32_PairInputEvent(currentInput, newEvent);
				}
			}
		}
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                 Controller Helper Functions                  |
// +--------------------------------------------------------------+
bool Win32_UpdateControllerButton(EngineInput_t* currentInput, PlatControllerState_t* controller, ControllerBtn_t controllerBtn, bool isDown)
{
	NotNull(controller);
	Assert(controllerBtn < ControllerBtn_NumBtns);
	PlatBtnState_t* btnState = &controller->btnStates[controllerBtn];
	bool result = false;
	
	if (btnState->isDown != isDown)
	{
		result = true;
		btnState->isDown = isDown;
		IncrementU8(btnState->numTransitions);
		btnState->lastChangeTime = Win32_GetProgramTime(nullptr, false);
		if (isDown) { IncrementU8(btnState->numPresses); }
		else { IncrementU8(btnState->numReleases); }
		
		InputEvent_t* newEvent = Win32_CreateInputEvent(nullptr, currentInput, ModifierKey_None, InputEventType_ControllerBtn);
		if (newEvent != nullptr)
		{
			newEvent->controllerBtn.btn = controllerBtn;
			newEvent->controllerBtn.pressed = isDown;
			newEvent->controllerBtn.released = !isDown;
			newEvent->controllerBtn.repeated = false;
			Win32_PairInputEvent(currentInput, newEvent);
		}
	}
	
	if (controllerBtn == ControllerBtn_Left || controllerBtn == ControllerBtn_Right ||
		controllerBtn == ControllerBtn_Up   || controllerBtn == ControllerBtn_Down)
	{
		v2i newDpadValue = Vec2i_Zero;
		if (controller->btnStates[ControllerBtn_Left].isDown)  { newDpadValue += Vec2i_Left;  }
		if (controller->btnStates[ControllerBtn_Right].isDown) { newDpadValue += Vec2i_Right; }
		if (controller->btnStates[ControllerBtn_Up].isDown)    { newDpadValue += Vec2i_Up;    }
		if (controller->btnStates[ControllerBtn_Down].isDown)  { newDpadValue += Vec2i_Down;  }
		if (newDpadValue != controller->dpad)
		{
			controller->dpad = newDpadValue;
			//TODO: Should we make an event? Or set a changed flag?
		}
	}
	
	return result;
}

void Win32_UpdateControllerStick(EngineInput_t* currentInput, PlatControllerState_t* controller, bool rightStick, v2 analogStickValue)
{
	NotNull(controller);
	v2* stickValuePntr = (rightStick ? &controller->rightStick : &controller->leftStick);
	v2* stickRawValuePntr = (rightStick ? &controller->rightStickRaw : &controller->leftStickRaw);
	
	*stickRawValuePntr = analogStickValue;
	
	r32 analogMagnitude = Vec2Length(analogStickValue);
	bool outOfDeadzone = (analogMagnitude >= CONTROLLER_STICK_DEADZONE);
	bool outOfDirDeadzone = (analogMagnitude >= CONTROLLER_STICK_DIR_DEADZONE);
	Dir2_t stickDir = ToDir2(analogStickValue);
	if (analogMagnitude > 1.0f)
	{
		analogStickValue = analogStickValue / analogMagnitude;
		analogMagnitude = 1.0f;
	}
	*stickValuePntr = (outOfDeadzone ? analogStickValue : Vec2_Zero);
	
	Win32_UpdateControllerButton(currentInput, controller, (rightStick ? ControllerBtn_rsRight : ControllerBtn_lsRight), (outOfDirDeadzone && (stickDir == Dir2_Right)));
	Win32_UpdateControllerButton(currentInput, controller, (rightStick ? ControllerBtn_rsLeft  : ControllerBtn_lsLeft),  (outOfDirDeadzone && (stickDir == Dir2_Left)));
	Win32_UpdateControllerButton(currentInput, controller, (rightStick ? ControllerBtn_rsUp    : ControllerBtn_lsUp),    (outOfDirDeadzone && (stickDir == Dir2_Up)));
	Win32_UpdateControllerButton(currentInput, controller, (rightStick ? ControllerBtn_rsDown  : ControllerBtn_lsDown),  (outOfDirDeadzone && (stickDir == Dir2_Down)));
}

bool Win32_UpdateControllerTrigger(EngineInput_t* currentInput, PlatControllerState_t* controller, bool rightTrigger, r32 triggerValue)
{
	NotNull(controller);
	r32* triggerValuePntr = (rightTrigger ? &controller->rightTrigger : &controller->leftTrigger);
	
	*triggerValuePntr = triggerValue;
	
	return Win32_UpdateControllerButton(currentInput, controller, (rightTrigger ? ControllerBtn_RightTrigger : ControllerBtn_LeftTrigger), (triggerValue >= CONTROLLER_TRIGGER_ACTIVE_THRESHOLD));
}

