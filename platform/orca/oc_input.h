/*
File:   oc_input.h
Author: Taylor Robbins
Date:   10\08\2024
*/

#ifndef _OC_INPUT_H
#define _OC_INPUT_H

struct BtnState_t
{
	u64 lastChangeTime;
	bool isDown;
	bool wasDown;
	u8 numPresses;
	u8 numReleases;
	u8 numTransitions;
	u8 numRepeats;
	bool pressHandled;
	bool releaseHandled;
	bool extendedHandled;
	#if DEBUG_BUILD
	MyStr_t handlerFilePath;
	u64 handlerLineNum;
	#endif
};

//NOTE: All the commented out stuff comes from EngineInput_t and WindowEngineInput_t
//      for regular Pig Engine platform layers. But in Orca we either don't use
//      the value or we haven't implemented it yet
struct AppInput_t
{
	r64 programStartTime;
	
	// VarArray_t inputEvents; //InputEvent_t
	
	// bool actualElapsedMsIgnored;
	r64 programTimeF; //ms (since start of program)
	u64 programTime; //ms (since start of program)
	u64 prevProgramTime; //ms (since start of program)
	r64 elapsedMs; //ms
	// r64 uncappedElapsedMs; //ms
	// r64 avgElapsedMs; //ms
	// r64 framerate; //frames/sec
	// r64 avgFramerate; //frames/sec
	r64 timeDelta; //stays near 1.0f if framerate is close to targetFramerate
	// r64 lastUpdateElapsedMs; //ms
	// r64 timeSpentWaitingLastFrame; //ms
	
	// RealTime_t localTime;
	// RealTime_t unixTime;
	// MyStr_t localTimezoneName;
	// i64 localTimezoneOffset; //in seconds
	// bool localTimezoneDoesDst;
	
	BtnState_t keyStates[Key_NumKeys];
	BtnState_t mouseBtnStates[MouseBtn_NumBtns];
	// ControllerState_t controllerStates[MAX_NUM_CONTROLLERS];
	
	bool scrollChangedX;
	bool scrollChangedY;
	v2 scrollValue;
	v2 scrollDelta;
	
	// bool isFocusedChanged;
	// bool isFocused;
	
	// bool fullscreen;
	// bool fullscreenChanged;
	// const PlatMonitorInfo_t* fullscreenMonitor;
	// const PlatMonitorVideoMode_t* fullscreenVideoMode;
	// u64 fullscreenFramerateIndex;
	// i64 fullscreenFramerate;
	
	// bool windowMoved;
	// reci desktopRec; //This can be used in conjunction with PlatMonitorList_t desktopRec to get an idea of where our window is across all monitors
	// reci desktopInnerRec; //Similar to desktopRec but doesn't include the title bar or borders. i.e. it's only the render portion of the window
	// i64 windowedFramerate;
	
	// bool minimizedChanged;
	// bool minimized;
	// bool maximizedChanged;
	// bool maximized;
	
	bool windowResized;
	v2i pixelResolution;       //the actual number of pixels we think we have to work with
	v2i prevPixelResolution;
	// v2i windowResolution;      //the size reported to us by GLFW for the window (in screen space coordinates)
	// v2i prevWindowResolution;
	// v2i contextResolution;     //the size that OpenGL or whatever wants us to talk in when doing stuff like glViewport
	v2  renderResolution;      //the size used for all of our render logic. The "effective" resolution
	
	// v2i prevUnmaximizedWindowPos; //NOTE: window position callback comes as (0, 0) BEFORE we get the maximized callback. And also comes before we get the unmaximized callback. So we need to revert the value sometimes
	// v2i unmaximizedWindowPos;  //the last value of desktopInnerRec.topLeft when we weren't maximized, minimized, or full screen
	// v2i unmaximizedWindowSize; //the last value of windowResolution when we weren't maximized, minimized, or full screen
	
	bool mouseInsideWindowChanged;
	bool mouseInsideWindow;
	bool mouseMoved;
	v2i mousePosi;
	v2 mousePos;
	v2 mouseDelta; //useful for PlatMouseMode_FirstPersonCamera
};

MouseBtn_t GetMouseBtnForOrcaMouseButton(OC_MouseButton_t orcaMouseButton)
{
	switch (orcaMouseButton)
	{
		case OC_MOUSE_LEFT:   return MouseBtn_Left;
		case OC_MOUSE_RIGHT:  return MouseBtn_Right;
		case OC_MOUSE_MIDDLE: return MouseBtn_Middle;
		// TODO: OC_MOUSE_EXT1, OC_MOUSE_EXT2
		default: return MouseBtn_None;
	}
}
Key_t GetKeyForOrcaKeyCode(OC_KeyCode_t orcaKeyCode)
{
	switch (orcaKeyCode)
	{
		case OC_KEY_A: return Key_A;
		case OC_KEY_B: return Key_B;
		case OC_KEY_C: return Key_C;
		case OC_KEY_D: return Key_D;
		case OC_KEY_E: return Key_E;
		case OC_KEY_F: return Key_F;
		case OC_KEY_G: return Key_G;
		case OC_KEY_H: return Key_H;
		case OC_KEY_I: return Key_I;
		case OC_KEY_J: return Key_J;
		case OC_KEY_K: return Key_K;
		case OC_KEY_L: return Key_L;
		case OC_KEY_M: return Key_M;
		case OC_KEY_N: return Key_N;
		case OC_KEY_O: return Key_O;
		case OC_KEY_P: return Key_P;
		case OC_KEY_Q: return Key_Q;
		case OC_KEY_R: return Key_R;
		case OC_KEY_S: return Key_S;
		case OC_KEY_T: return Key_T;
		case OC_KEY_U: return Key_U;
		case OC_KEY_V: return Key_V;
		case OC_KEY_W: return Key_W;
		case OC_KEY_X: return Key_X;
		case OC_KEY_Y: return Key_Y;
		case OC_KEY_Z: return Key_Z;
		
		case OC_KEY_0: return Key_0;
		case OC_KEY_1: return Key_1;
		case OC_KEY_2: return Key_2;
		case OC_KEY_3: return Key_3;
		case OC_KEY_4: return Key_4;
		case OC_KEY_5: return Key_5;
		case OC_KEY_6: return Key_6;
		case OC_KEY_7: return Key_7;
		case OC_KEY_8: return Key_8;
		case OC_KEY_9: return Key_9;
		
		case OC_KEY_KP_0: return Key_Num0;
		case OC_KEY_KP_1: return Key_Num1;
		case OC_KEY_KP_2: return Key_Num2;
		case OC_KEY_KP_3: return Key_Num3;
		case OC_KEY_KP_4: return Key_Num4;
		case OC_KEY_KP_5: return Key_Num5;
		case OC_KEY_KP_6: return Key_Num6;
		case OC_KEY_KP_7: return Key_Num7;
		case OC_KEY_KP_8: return Key_Num8;
		case OC_KEY_KP_9: return Key_Num9;
		
		case OC_KEY_F1:  return Key_F1;
		case OC_KEY_F2:  return Key_F2;
		case OC_KEY_F3:  return Key_F3;
		case OC_KEY_F4:  return Key_F4;
		case OC_KEY_F5:  return Key_F5;
		case OC_KEY_F6:  return Key_F6;
		case OC_KEY_F7:  return Key_F7;
		case OC_KEY_F8:  return Key_F8;
		case OC_KEY_F9:  return Key_F9;
		case OC_KEY_F10: return Key_F10;
		case OC_KEY_F11: return Key_F11;
		case OC_KEY_F12: return Key_F12;
		//TODO: OC_KEY_F13, OC_KEY_F14, OC_KEY_F15, OC_KEY_F16, OC_KEY_F17, OC_KEY_F18, OC_KEY_F19, OC_KEY_F20, OC_KEY_F21, OC_KEY_F22, OC_KEY_F23, OC_KEY_F24, OC_KEY_F25
		
		case OC_KEY_KP_DECIMAL:  return Key_NumPeriod;
		case OC_KEY_KP_DIVIDE:   return Key_NumDivide;
		case OC_KEY_KP_MULTIPLY: return Key_NumMultiply;
		case OC_KEY_KP_SUBTRACT: return Key_NumSubtract;
		case OC_KEY_KP_ADD:      return Key_NumAdd;
		case OC_KEY_KP_ENTER:    return Key_Enter;
		//TODO: OC_KEY_KP_EQUAL
		
		case OC_KEY_ENTER:     return Key_Enter;
		case OC_KEY_BACKSPACE: return Key_Backspace;
		case OC_KEY_ESCAPE:    return Key_Escape;
		case OC_KEY_INSERT:    return Key_Insert;
		case OC_KEY_DELETE:    return Key_Delete;
		case OC_KEY_HOME:      return Key_Home;
		case OC_KEY_END:       return Key_End;
		case OC_KEY_PAGE_UP:   return Key_PageUp;
		case OC_KEY_PAGE_DOWN: return Key_PageDown;
		case OC_KEY_TAB:       return Key_Tab;
		case OC_KEY_CAPS_LOCK: return Key_CapsLock;
		case OC_KEY_NUM_LOCK:  return Key_NumLock;
			
		case OC_KEY_LEFT_CONTROL:  return Key_Control;
		case OC_KEY_RIGHT_CONTROL: return Key_Control;
		case OC_KEY_LEFT_ALT:      return Key_Alt;
		case OC_KEY_RIGHT_ALT:     return Key_Alt;
		case OC_KEY_LEFT_SHIFT:    return Key_Shift;
		case OC_KEY_RIGHT_SHIFT:   return Key_Shift;
		
		case OC_KEY_RIGHT: return Key_Right;
		case OC_KEY_LEFT:  return Key_Left;
		case OC_KEY_UP:    return Key_Up;
		case OC_KEY_DOWN:  return Key_Down;
		
		case OC_KEY_EQUAL:          return Key_Plus;
		case OC_KEY_MINUS:          return Key_Minus;
		case OC_KEY_BACKSLASH:      return Key_Pipe;
		case OC_KEY_LEFT_BRACKET:   return Key_OpenBracket;
		case OC_KEY_RIGHT_BRACKET:  return Key_CloseBracket;
		case OC_KEY_SEMICOLON:      return Key_Colon;
		case OC_KEY_APOSTROPHE:     return Key_Quote;
		case OC_KEY_COMMA:          return Key_Comma;
		case OC_KEY_PERIOD:         return Key_Period;
		case OC_KEY_SLASH:          return Key_QuestionMark;
		case OC_KEY_GRAVE_ACCENT:   return Key_Tilde;
		case OC_KEY_SPACE:          return Key_Space;
		
		//TODO: OC_KEY_WORLD_1, OC_KEY_WORLD_2, OC_KEY_SCROLL_LOCK, OC_KEY_PRINT_SCREEN, OC_KEY_PAUSE, OC_KEY_LEFT_SUPER, OC_KEY_RIGHT_SUPER, OC_KEY_MENU
		
		default: return Key_None;
	}
}

#endif //  _OC_INPUT_H
