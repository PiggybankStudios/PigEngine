/*
File:   pig_imgui.h
Author: Taylor Robbins
Date:   03\01\2024
*/

#ifndef _PIG_IMGUI_H
#define _PIG_IMGUI_H

#define PIG_IMGUI_ARENA_PAGE_SIZE   Megabytes(1)

#define IMGUI_INI_FILE_NAME "debug_window_layout.ini"
#define IMGUI_LOG_FILE_NAME "debug_window_log.txt"

#define IMGUI_WINDOW_RENDER_FUNC_DEF(functionName) void functionName(struct PigRegisteredImguiWindow_t* window)
typedef IMGUI_WINDOW_RENDER_FUNC_DEF(ImguiWindowRenderFunc_f);

#define IMGUI_WINDOW_FREE_FUNC_DEF(functionName) void functionName(struct PigRegisteredImguiWindow_t* window)
typedef IMGUI_WINDOW_FREE_FUNC_DEF(ImguiWindowFreeFunc_f);

struct PigRegisteredImguiWindow_t
{
	MyStr_t name;
	Func_t renderFunc; //ImguiWindowRenderFunc_f
	Func_t freeFunc; //ImguiWindowCloseFunc_f
	void* contextPntr;
	u64 contextAllocSize; //if non-zero, we will allocate/deallocate contextPntr for the window automatically
	bool isOpen;
	bool wasOpen;
};

struct PigImguiState_t
{
	bool initialized;
	ImGuiContext* imguiContext;
	bool frameStarted;
	MyStr_t clipboardStr;
	
	bool isTypingIntoImgui;
	bool prevWantTextInput;
	bool isMouseOverImgui;
	MyStr_t mainShaderCode;
	Shader_t mainShader;
	Texture_t fontTexture;
	VertBuffer_t vertBuffer;
	
	bool launcherIsOpen;
	VarArray_t registeredWindows;
};

ImGuiKey GetImGuiKey(Key_t key)
{
	switch (key)
	{
		case Key_A: return ImGuiKey_A;
		case Key_B: return ImGuiKey_B;
		case Key_C: return ImGuiKey_C;
		case Key_D: return ImGuiKey_D;
		case Key_E: return ImGuiKey_E;
		case Key_F: return ImGuiKey_F;
		case Key_G: return ImGuiKey_G;
		case Key_H: return ImGuiKey_H;
		case Key_I: return ImGuiKey_I;
		case Key_J: return ImGuiKey_J;
		case Key_K: return ImGuiKey_K;
		case Key_L: return ImGuiKey_L;
		case Key_M: return ImGuiKey_M;
		case Key_N: return ImGuiKey_N;
		case Key_O: return ImGuiKey_O;
		case Key_P: return ImGuiKey_P;
		case Key_Q: return ImGuiKey_Q;
		case Key_R: return ImGuiKey_R;
		case Key_S: return ImGuiKey_S;
		case Key_T: return ImGuiKey_T;
		case Key_U: return ImGuiKey_U;
		case Key_V: return ImGuiKey_V;
		case Key_W: return ImGuiKey_W;
		case Key_X: return ImGuiKey_X;
		case Key_Y: return ImGuiKey_Y;
		case Key_Z: return ImGuiKey_Z;
		
		case Key_0: return ImGuiKey_0;
		case Key_1: return ImGuiKey_1;
		case Key_2: return ImGuiKey_2;
		case Key_3: return ImGuiKey_3;
		case Key_4: return ImGuiKey_4;
		case Key_5: return ImGuiKey_5;
		case Key_6: return ImGuiKey_6;
		case Key_7: return ImGuiKey_7;
		case Key_8: return ImGuiKey_8;
		case Key_9: return ImGuiKey_9;
		
		case Key_Num0: return ImGuiKey_Keypad0;
		case Key_Num1: return ImGuiKey_Keypad1;
		case Key_Num2: return ImGuiKey_Keypad2;
		case Key_Num3: return ImGuiKey_Keypad3;
		case Key_Num4: return ImGuiKey_Keypad4;
		case Key_Num5: return ImGuiKey_Keypad5;
		case Key_Num6: return ImGuiKey_Keypad6;
		case Key_Num7: return ImGuiKey_Keypad7;
		case Key_Num8: return ImGuiKey_Keypad8;
		case Key_Num9: return ImGuiKey_Keypad9;
		
		case Key_NumPeriod: return ImGuiKey_KeypadDecimal;
		case Key_NumDivide: return ImGuiKey_KeypadDivide;
		case Key_NumMultiply: return ImGuiKey_KeypadMultiply;
		case Key_NumSubtract: return ImGuiKey_KeypadSubtract;
		case Key_NumAdd: return ImGuiKey_KeypadAdd;
		
		case Key_F1: return ImGuiKey_F1;
		case Key_F2: return ImGuiKey_F2;
		case Key_F3: return ImGuiKey_F3;
		case Key_F4: return ImGuiKey_F4;
		case Key_F5: return ImGuiKey_F5;
		case Key_F6: return ImGuiKey_F6;
		case Key_F7: return ImGuiKey_F7;
		case Key_F8: return ImGuiKey_F8;
		case Key_F9: return ImGuiKey_F9;
		case Key_F10: return ImGuiKey_F10;
		case Key_F11: return ImGuiKey_F11;
		case Key_F12: return ImGuiKey_F12;
		
		case Key_Enter: return ImGuiKey_Enter;
		case Key_Backspace: return ImGuiKey_Backspace;
		case Key_Escape: return ImGuiKey_Escape;
		case Key_Insert: return ImGuiKey_Insert;
		case Key_Delete: return ImGuiKey_Delete;
		case Key_Home: return ImGuiKey_Home;
		case Key_End: return ImGuiKey_End;
		case Key_PageUp: return ImGuiKey_PageUp;
		case Key_PageDown: return ImGuiKey_PageDown;
		case Key_Tab: return ImGuiKey_Tab;
		case Key_CapsLock: return ImGuiKey_CapsLock;
		case Key_NumLock: return ImGuiKey_NumLock;
		
		case Key_Control: return ImGuiKey_LeftCtrl;
		case Key_Alt: return ImGuiKey_LeftAlt;
		case Key_Shift: return ImGuiKey_LeftShift;
		
		case Key_Right: return ImGuiKey_RightArrow;
		case Key_Left: return ImGuiKey_LeftArrow;
		case Key_Up: return ImGuiKey_UpArrow;
		case Key_Down: return ImGuiKey_DownArrow;
		
		case Key_Plus: return ImGuiKey_Equal;
		case Key_Minus: return ImGuiKey_Minus;
		case Key_Pipe: return ImGuiKey_Backslash;
		case Key_OpenBracket: return ImGuiKey_LeftBracket;
		case Key_CloseBracket: return ImGuiKey_RightBracket;
		case Key_Colon: return ImGuiKey_Semicolon;
		case Key_Quote: return ImGuiKey_Apostrophe;
		case Key_Comma: return ImGuiKey_Comma;
		case Key_Period: return ImGuiKey_Period;
		case Key_QuestionMark: return ImGuiKey_Slash;
		case Key_Tilde: return ImGuiKey_GraveAccent;
		case Key_Space: return ImGuiKey_Space;
		
		default: return ImGuiKey_None;
	}
}

ImGuiKey GetImGuiKey(ControllerBtn_t btn)
{
	switch (btn)
	{
		case ControllerBtn_A:            return ImGuiKey_GamepadFaceDown;
		case ControllerBtn_B:            return ImGuiKey_GamepadFaceRight;
		case ControllerBtn_X:            return ImGuiKey_GamepadFaceLeft;
		case ControllerBtn_Y:            return ImGuiKey_GamepadFaceUp;
		case ControllerBtn_Right:        return ImGuiKey_GamepadDpadRight;
		case ControllerBtn_Left:         return ImGuiKey_GamepadDpadLeft;
		case ControllerBtn_Up:           return ImGuiKey_GamepadDpadUp;
		case ControllerBtn_Down:         return ImGuiKey_GamepadDpadDown;
		case ControllerBtn_LeftBumper:   return ImGuiKey_GamepadL1;
		case ControllerBtn_RightBumper:  return ImGuiKey_GamepadR1;
		case ControllerBtn_LeftTrigger:  return ImGuiKey_GamepadL2;
		case ControllerBtn_RightTrigger: return ImGuiKey_GamepadR2;
		case ControllerBtn_LeftStick:    return ImGuiKey_GamepadL3;
		case ControllerBtn_RightStick:   return ImGuiKey_GamepadR3;
		case ControllerBtn_Start:        return ImGuiKey_GamepadStart;
		case ControllerBtn_Back:         return ImGuiKey_GamepadBack;
		case ControllerBtn_lsRight:      return ImGuiKey_GamepadLStickRight;
		case ControllerBtn_lsLeft:       return ImGuiKey_GamepadLStickLeft;
		case ControllerBtn_lsUp:         return ImGuiKey_GamepadLStickUp;
		case ControllerBtn_lsDown:       return ImGuiKey_GamepadLStickDown;
		case ControllerBtn_rsRight:      return ImGuiKey_GamepadRStickRight;
		case ControllerBtn_rsLeft:       return ImGuiKey_GamepadRStickLeft;
		case ControllerBtn_rsUp:         return ImGuiKey_GamepadRStickUp;
		case ControllerBtn_rsDown:       return ImGuiKey_GamepadRStickDown;
		
		default: return ImGuiKey_None;
	}
}

#endif //  _PIG_IMGUI_H
