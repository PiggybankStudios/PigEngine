/*
File:   oc_input_api.cpp
Author: Taylor Robbins
Date:   10\08\2024
Description: 
	** Holds all the functions that the application is expected to use when
	** querying for input information (like KeyPressed, MousePressed, etc.)
*/

//TODO: Add Mouse Scroll functions
//TODO: Add UI Focus Tracking functions
//TODO: Add Mouse Hit Testing functions

// +--------------------------------------------------------------+
// |                 ProgramTime Related Helpers                  |
// +--------------------------------------------------------------+
r32 Animate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return AnimateBy(ProgramTime, min, max, periodMs, offset);
}
u64 AnimateU64(u64 min, u64 max, u64 periodMs, u64 offset = 0)
{
	return AnimateByU64(ProgramTime, min, max, periodMs, offset);
}

v2i FrameIndexToSheetIndex(u64 frameIndex, v2i numFrames)
{
	v2i result;
	result.x = (i32)(frameIndex % numFrames.width);
	result.y = (i32)((frameIndex / numFrames.width) % numFrames.height);
	return result;
}

r32 Oscillate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateBy(ProgramTime, min, max, periodMs, offset);
}

r32 OscillatePhase(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillatePhaseBy(ProgramTime, min, max, periodMs, offset);
}

r32 OscillateSaw(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateSawBy(ProgramTime, min, max, periodMs, offset);
}

u64 TimeSince(u64 programTimeSnapshot)
{
	return TimeSinceBy(ProgramTime, programTimeSnapshot);
}

// +--------------------------------------------------------------+
// |                      Handling Functions                      |
// +--------------------------------------------------------------+
void HandleKey_(Key_t key, const char* filePath, u64 lineNumber)
{
	Assert(key < Key_NumKeys);
	BtnState_t* btnState = &appInput->keyStates[key];
	btnState->pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}
void HandleKeyRelease_(Key_t key, const char* filePath, u64 lineNumber)
{
	Assert(key < Key_NumKeys);
	BtnState_t* btnState = &appInput->keyStates[key];
	btnState->releaseHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}
void HandleKeyExtended_(Key_t key, const char* filePath, u64 lineNumber)
{
	Assert(key < Key_NumKeys);
	BtnState_t* btnState = &appInput->keyStates[key];
	btnState->pressHandled = true;
	btnState->releaseHandled = true;
	btnState->extendedHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}

void HandleMouse_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	BtnState_t* btnState = &appInput->mouseBtnStates[mouseBtn];
	btnState->pressHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}
void HandleMouseRelease_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	BtnState_t* btnState = &appInput->mouseBtnStates[mouseBtn];
	btnState->releaseHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}
void HandleMouseExtended_(MouseBtn_t mouseBtn, const char* filePath, u64 lineNumber)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	BtnState_t* btnState = &appInput->mouseBtnStates[mouseBtn];
	btnState->pressHandled = true;
	btnState->releaseHandled = true;
	btnState->extendedHandled = true;
	#if DEBUG_BUILD
	if (filePath != nullptr)
	{
		if (!IsEmptyStr(btnState->handlerFilePath)) { FreeString(mainHeap, &btnState->handlerFilePath); }
		btnState->handlerFilePath = NewStringInArenaNt(mainHeap, filePath);
		btnState->handlerLineNum = lineNumber;
	}
	#else
	UNUSED(filePath);
	UNUSED(lineNumber);
	#endif
}

#if DEBUG_BUILD
#define HandleKey(key)                  HandleKey_((key), __FILE__, __LINE__)
#define HandleKeyRelease(key)           HandleKeyRelease_((key), __FILE__, __LINE__)
#define HandleKeyExtended(key)          HandleKeyExtended_((key), __FILE__, __LINE__)
#define HandleMouse(mouseBtn)           HandleMouse_((mouseBtn), __FILE__, __LINE__)
#define HandleMouseRelease(mouseBtn)    HandleMouseRelease_((mouseBtn), __FILE__, __LINE__)
#define HandleMouseExtended(mouseBtn)   HandleMouseExtended_((mouseBtn), __FILE__, __LINE__)
#else
#define HandleKey(key)                  HandleKey_((key), nullptr, 0)
#define HandleKeyRelease(key)           HandleKeyRelease_((key), nullptr, 0)
#define HandleKeyExtended(key)          HandleKeyExtended_((key), nullptr, 0)
#define HandleMouse(mouseBtn)           HandleMouse_((mouseBtn), nullptr, 0)
#define HandleMouseRelease(mouseBtn)    HandleMouseRelease_((mouseBtn), nullptr, 0)
#define HandleMouseExtended(mouseBtn)   HandleMouseExtended_((mouseBtn), nullptr, 0)
#endif

// +--------------------------------------------------------------+
// |                    KeyPressed and Friends                    |
// +--------------------------------------------------------------+
bool IsKeyHandled(Key_t key, bool checkPress = true, bool checkRelease = true)
{
	Assert(key < Key_NumKeys);
	return ((checkPress && appInput->keyStates[key].pressHandled) || (checkRelease && appInput->keyStates[key].releaseHandled));
}

bool KeyDownRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	return appInput->keyStates[key].isDown;
}
bool KeyReleasedRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	return (appInput->keyStates[key].numReleases > 0);
}
bool KeyPressedRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	return (appInput->keyStates[key].numPresses > 0);
}
bool KeyPressedPlatRepeatingRaw(Key_t key)
{
	Assert(key < Key_NumKeys);
	return (appInput->keyStates[key].numPresses > 0 || appInput->keyStates[key].numRepeats > 0);
}
//TODO: Does this work correctly? Seems like delay makes no difference?
bool KeyRepeatedRaw(Key_t key, u64 delay, u64 period)
{
	Assert(key < Key_NumKeys);
	Assert(period > 0);
	BtnState_t* btnState = &appInput->keyStates[key];
	if (btnState->numPresses > 0) { return true; }
	if (appInput->prevProgramTime == ProgramTime) { return false; }
	if (!btnState->isDown) { return false; }
	u64 timeSince = TimeSince(btnState->lastChangeTime);
	u64 timeSincePrev = TimeSinceBy(appInput->prevProgramTime, btnState->lastChangeTime);
	if (timeSince < delay) { return false; }
	if (timeSincePrev < delay) { return true; }
	timeSince -= delay;
	timeSincePrev -= delay;
	if ((timeSince / period) != (timeSincePrev / period)) { return true; }
	return false;
}

bool KeyDown(Key_t key)
{
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyDownRaw(key);
}
bool KeyReleased(Key_t key)
{
	if (IsKeyHandled(key, false, true)) { return false; }
	return KeyReleasedRaw(key);
}
bool KeyPressed(Key_t key)
{
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyPressedRaw(key);
}
bool KeyPressedPlatRepeating(Key_t key)
{
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyPressedPlatRepeatingRaw(key);
}
//TODO: Does this work correctly? Seems like delay makes no difference?
bool KeyRepeated(Key_t key, u64 delay, u64 period)
{
	if (IsKeyHandled(key, true, false)) { return false; }
	return KeyRepeatedRaw(key, delay, period);
}

// +--------------------------------------------------------------+
// |                   MousePressed and Friends                   |
// +--------------------------------------------------------------+
bool IsMouseHandled(MouseBtn_t mouseBtn, bool checkPress = true, bool checkRelease = true)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	return ((checkPress && appInput->mouseBtnStates[mouseBtn].pressHandled) || (checkRelease && appInput->mouseBtnStates[mouseBtn].releaseHandled));
}

bool MouseDownRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	return appInput->mouseBtnStates[mouseBtn].isDown;
}
bool MouseReleasedRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	return (appInput->mouseBtnStates[mouseBtn].numReleases > 0);
}
bool MousePressedRaw(MouseBtn_t mouseBtn)
{
	Assert(mouseBtn < MouseBtn_NumBtns);
	return (appInput->mouseBtnStates[mouseBtn].numPresses > 0);
}

bool MouseDown(MouseBtn_t mouseBtn)
{
	if (IsMouseHandled(mouseBtn, true, false)) { return false; }
	return MouseDownRaw(mouseBtn);
}
bool MouseReleased(MouseBtn_t mouseBtn)
{
	if (IsMouseHandled(mouseBtn, false, true)) { return false; }
	return MouseReleasedRaw(mouseBtn);
}
bool MousePressed(MouseBtn_t mouseBtn)
{
	if (IsMouseHandled(mouseBtn, true, false)) { return false; }
	return MousePressedRaw(mouseBtn);
}
