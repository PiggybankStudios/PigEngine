/*
File:   common_button_enums.h
Author: Taylor Robbins
Date:   10\19\2021
Description:
	** Just holds the Key_t, ControllerBtn_t, MouseBtn_t, and ModifierKey_t enumerations
	** We put them in their own file since they are kind of long
*/

#ifndef _COMMON_BUTTON_ENUMS_H
#define _COMMON_BUTTON_ENUMS_H

// +--------------------------------------------------------------+
// |                            Key_t                             |
// +--------------------------------------------------------------+
enum Key_t
{
	Key_None = 0,
	
	Key_A,
	Key_B,
	Key_C,
	Key_D,
	Key_E,
	Key_F,
	Key_G,
	Key_H,
	Key_I,
	Key_J,
	Key_K,
	Key_L,
	Key_M,
	Key_N,
	Key_O,
	Key_P,
	Key_Q,
	Key_R,
	Key_S,
	Key_T,
	Key_U,
	Key_V,
	Key_W,
	Key_X,
	Key_Y,
	Key_Z,
	
	Key_0,
	Key_1,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
	Key_6,
	Key_7,
	Key_8,
	Key_9,
	
	Key_Num0,
	Key_Num1,
	Key_Num2,
	Key_Num3,
	Key_Num4,
	Key_Num5,
	Key_Num6,
	Key_Num7,
	Key_Num8,
	Key_Num9,
	
	Key_NumPeriod,
	Key_NumDivide,
	Key_NumMultiply,
	Key_NumSubtract,
	Key_NumAdd,
	
	Key_F1,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,
	
	Key_Enter,
	Key_Backspace,
	Key_Escape,
	Key_Insert,
	Key_Delete,
	Key_Home,
	Key_End,
	Key_PageUp,
	Key_PageDown,
	Key_Tab,
	Key_CapsLock,
	Key_NumLock,
	
	Key_Control,
	Key_Alt,
	Key_Shift,
	
	Key_Right,
	Key_Left,
	Key_Up,
	Key_Down,
	
	Key_Plus,
	Key_Minus,
	Key_Pipe,
	Key_OpenBracket,
	Key_CloseBracket,
	Key_Colon,
	Key_Quote,
	Key_Comma,
	Key_Period,
	Key_QuestionMark,
	Key_Tilde,
	Key_Space,
	
	Key_NumKeys,
};
const char* GetKeyStr(Key_t key)
{
	switch (key)
	{
		case Key_None:         return "None";
		case Key_A:            return "A";
		case Key_B:            return "B";
		case Key_C:            return "C";
		case Key_D:            return "D";
		case Key_E:            return "E";
		case Key_F:            return "F";
		case Key_G:            return "G";
		case Key_H:            return "H";
		case Key_I:            return "I";
		case Key_J:            return "J";
		case Key_K:            return "K";
		case Key_L:            return "L";
		case Key_M:            return "M";
		case Key_N:            return "N";
		case Key_O:            return "O";
		case Key_P:            return "P";
		case Key_Q:            return "Q";
		case Key_R:            return "R";
		case Key_S:            return "S";
		case Key_T:            return "T";
		case Key_U:            return "U";
		case Key_V:            return "V";
		case Key_W:            return "W";
		case Key_X:            return "X";
		case Key_Y:            return "Y";
		case Key_Z:            return "Z";
		case Key_0:            return "0";
		case Key_1:            return "1";
		case Key_2:            return "2";
		case Key_3:            return "3";
		case Key_4:            return "4";
		case Key_5:            return "5";
		case Key_6:            return "6";
		case Key_7:            return "7";
		case Key_8:            return "8";
		case Key_9:            return "9";
		case Key_Num0:         return "Num0";
		case Key_Num1:         return "Num1";
		case Key_Num2:         return "Num2";
		case Key_Num3:         return "Num3";
		case Key_Num4:         return "Num4";
		case Key_Num5:         return "Num5";
		case Key_Num6:         return "Num6";
		case Key_Num7:         return "Num7";
		case Key_Num8:         return "Num8";
		case Key_Num9:         return "Num9";
		case Key_NumPeriod:    return "NumPeriod";
		case Key_NumDivide:    return "NumDivide";
		case Key_NumMultiply:  return "NumMultiply";
		case Key_NumSubtract:  return "NumSubtract";
		case Key_NumAdd:       return "NumAdd";
		case Key_F1:           return "F1";
		case Key_F2:           return "F2";
		case Key_F3:           return "F3";
		case Key_F4:           return "F4";
		case Key_F5:           return "F5";
		case Key_F6:           return "F6";
		case Key_F7:           return "F7";
		case Key_F8:           return "F8";
		case Key_F9:           return "F9";
		case Key_F10:          return "F10";
		case Key_F11:          return "F11";
		case Key_F12:          return "F12";
		case Key_Enter:        return "Enter";
		case Key_Backspace:    return "Backspace";
		case Key_Escape:       return "Escape";
		case Key_Insert:       return "Insert";
		case Key_Delete:       return "Delete";
		case Key_Home:         return "Home";
		case Key_End:          return "End";
		case Key_PageUp:       return "PageUp";
		case Key_PageDown:     return "PageDown";
		case Key_Tab:          return "Tab";
		case Key_CapsLock:     return "CapsLock";
		case Key_NumLock:      return "NumLock";
		case Key_Control:      return "Control";
		case Key_Alt:          return "Alt";
		case Key_Shift:        return "Shift";
		case Key_Right:        return "Right";
		case Key_Left:         return "Left";
		case Key_Up:           return "Up";
		case Key_Down:         return "Down";
		case Key_Plus:         return "Plus";
		case Key_Minus:        return "Minus";
		case Key_Pipe:         return "Pipe";
		case Key_OpenBracket:  return "OpenBracket";
		case Key_CloseBracket: return "CloseBracket";
		case Key_Colon:        return "Colon";
		case Key_Quote:        return "Quote";
		case Key_Comma:        return "Comma";
		case Key_Period:       return "Period";
		case Key_QuestionMark: return "QuestionMark";
		case Key_Tilde:        return "Tilde";
		case Key_Space:        return "Space";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                       ControllerBtn_t                        |
// +--------------------------------------------------------------+
enum ControllerBtn_t
{
	ControllerBtn_None = 0,
	
	ControllerBtn_A, //Aka Bottom Face Btn
	ControllerBtn_B, //Aka Right Face Btn
	ControllerBtn_X, //Aka Left Face Btn
	ControllerBtn_Y, //Aka Top Face Btn
	
	ControllerBtn_Right,
	ControllerBtn_Left,
	ControllerBtn_Up,
	ControllerBtn_Down,
	
	ControllerBtn_LeftBumper,
	ControllerBtn_RightBumper,
	ControllerBtn_LeftTrigger,
	ControllerBtn_RightTrigger,
	ControllerBtn_LeftStick,
	ControllerBtn_RightStick,
	
	ControllerBtn_Start,
	ControllerBtn_Back,
	
	ControllerBtn_lsRight,
	ControllerBtn_lsLeft,
	ControllerBtn_lsUp,
	ControllerBtn_lsDown,
	ControllerBtn_rsRight,
	ControllerBtn_rsLeft,
	ControllerBtn_rsUp,
	ControllerBtn_rsDown,
	
	ControllerBtn_NumBtns,
};
const char* GetControllerBtnStr(ControllerBtn_t controllerBtn)
{
	switch (controllerBtn)
	{
		case ControllerBtn_None:         return "None";
		case ControllerBtn_A:            return "A";
		case ControllerBtn_B:            return "B";
		case ControllerBtn_X:            return "X";
		case ControllerBtn_Y:            return "Y";
		case ControllerBtn_Right:        return "Right";
		case ControllerBtn_Left:         return "Left";
		case ControllerBtn_Up:           return "Up";
		case ControllerBtn_Down:         return "Down";
		case ControllerBtn_LeftBumper:   return "LeftBumper";
		case ControllerBtn_RightBumper:  return "RightBumper";
		case ControllerBtn_LeftTrigger:  return "LeftTrigger";
		case ControllerBtn_RightTrigger: return "RightTrigger";
		case ControllerBtn_LeftStick:    return "LeftStick";
		case ControllerBtn_RightStick:   return "RightStick";
		case ControllerBtn_Start:        return "Start";
		case ControllerBtn_Back:         return "Back";
		case ControllerBtn_lsRight:      return "lsRight";
		case ControllerBtn_lsLeft:       return "lsLeft";
		case ControllerBtn_lsUp:         return "lsUp";
		case ControllerBtn_lsDown:       return "lsDown";
		case ControllerBtn_rsRight:      return "rsRight";
		case ControllerBtn_rsLeft:       return "rsLeft";
		case ControllerBtn_rsUp:         return "rsUp";
		case ControllerBtn_rsDown:       return "rsDown";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                          MouseBtn_t                          |
// +--------------------------------------------------------------+
enum MouseBtn_t
{
	MouseBtn_None = 0,
	
	MouseBtn_Left,
	MouseBtn_Right,
	MouseBtn_Middle,
	
	MouseBtn_NumBtns,
};
const char* GetMouseBtnStr(MouseBtn_t mouseBtn)
{
	switch (mouseBtn)
	{
		case MouseBtn_None:   return "None";
		case MouseBtn_Left:   return "Left";
		case MouseBtn_Right:  return "Right";
		case MouseBtn_Middle: return "Middle";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                        ModifierKey_t                         |
// +--------------------------------------------------------------+
enum ModifierKey_t
{
	ModifierKey_None = 0x00,
	
	ModifierKey_Shift    = 0x01,
	ModifierKey_Alt      = 0x02, //options on OSX
	ModifierKey_Ctrl     = 0x04, //command on OSX
	ModifierKey_CapsLock = 0x08,
	
	ModifierKey_CtrlShift    = (ModifierKey_Ctrl|ModifierKey_Shift),
	ModifierKey_CtrlAlt      = (ModifierKey_Ctrl|ModifierKey_Alt),
	ModifierKey_AltShift     = (ModifierKey_Alt|ModifierKey_Shift),
	ModifierKey_CtrlAltShift = (ModifierKey_Ctrl|ModifierKey_Alt|ModifierKey_Shift),
	
	ModifierKey_All = 0x0F,
	ModifierKey_NumKeys = 4,
};
const char* GetModifierKeyStr(ModifierKey_t modifierKey)
{
	switch (modifierKey)
	{
		case ModifierKey_None:      return "None";
		case ModifierKey_Shift:     return "Shift";
		case ModifierKey_Alt:       return "Alt";
		case ModifierKey_Ctrl:      return "Ctrl";
		case ModifierKey_CapsLock:  return "CapsLock";
		case ModifierKey_CtrlShift: return "CtrlShift";
		case ModifierKey_CtrlAlt:   return "CtrlAlt";
		case ModifierKey_AltShift:  return "AltShift";
		case ModifierKey_All:       return "All";
		default: return "Unknown";
	}
}
ModifierKey_t GetModifierKeyForKey(Key_t key)
{
	switch (key)
	{
		case Key_Shift:   return ModifierKey_Shift;
		case Key_Alt:     return ModifierKey_Alt;
		case Key_Control: return ModifierKey_Ctrl;
		default: return ModifierKey_None;
	}
}
Key_t GetKeyForModifierKey(ModifierKey_t modifierKey)
{
	switch (modifierKey)
	{
		case ModifierKey_Shift: return Key_Shift;
		case ModifierKey_Alt:   return Key_Alt;
		case ModifierKey_Ctrl:  return Key_Control;
		default: return Key_None;
	}
}

// +--------------------------------------------------------------+
// |                   GLFW Key/Button Mappings                   |
// +--------------------------------------------------------------+
Key_t GetKeyForGlfwKeyCode(int glfwKeyCode, bool upsideDownModeEnabled = false)
{
	switch (glfwKeyCode)
	{
		case GLFW_KEY_A:             return Key_A;
		case GLFW_KEY_B:             return Key_B;
		case GLFW_KEY_C:             return Key_C;
		case GLFW_KEY_D:             return Key_D;
		case GLFW_KEY_E:             return Key_E;
		case GLFW_KEY_F:             return Key_F;
		case GLFW_KEY_G:             return Key_G;
		case GLFW_KEY_H:             return Key_H;
		case GLFW_KEY_I:             return Key_I;
		case GLFW_KEY_J:             return Key_J;
		case GLFW_KEY_K:             return Key_K;
		case GLFW_KEY_L:             return Key_L;
		case GLFW_KEY_M:             return Key_M;
		case GLFW_KEY_N:             return Key_N;
		case GLFW_KEY_O:             return Key_O;
		case GLFW_KEY_P:             return Key_P;
		case GLFW_KEY_Q:             return Key_Q;
		case GLFW_KEY_R:             return Key_R;
		case GLFW_KEY_S:             return Key_S;
		case GLFW_KEY_T:             return Key_T;
		case GLFW_KEY_U:             return Key_U;
		case GLFW_KEY_V:             return Key_V;
		case GLFW_KEY_W:             return Key_W;
		case GLFW_KEY_X:             return Key_X;
		case GLFW_KEY_Y:             return Key_Y;
		case GLFW_KEY_Z:             return Key_Z;
		
		case GLFW_KEY_0:             return Key_0;
		case GLFW_KEY_1:             return Key_1;
		case GLFW_KEY_2:             return Key_2;
		case GLFW_KEY_3:             return Key_3;
		case GLFW_KEY_4:             return Key_4;
		case GLFW_KEY_5:             return Key_5;
		case GLFW_KEY_6:             return Key_6;
		case GLFW_KEY_7:             return Key_7;
		case GLFW_KEY_8:             return Key_8;
		case GLFW_KEY_9:             return Key_9;
		
		case GLFW_KEY_KP_0:          return Key_Num0;
		case GLFW_KEY_KP_1:          return Key_Num1;
		case GLFW_KEY_KP_2:          return Key_Num2;
		case GLFW_KEY_KP_3:          return Key_Num3;
		case GLFW_KEY_KP_4:          return Key_Num4;
		case GLFW_KEY_KP_5:          return Key_Num5;
		case GLFW_KEY_KP_6:          return Key_Num6;
		case GLFW_KEY_KP_7:          return Key_Num7;
		case GLFW_KEY_KP_8:          return Key_Num8;
		case GLFW_KEY_KP_9:          return Key_Num9;
		
		case GLFW_KEY_KP_DECIMAL:    return Key_NumPeriod;
		case GLFW_KEY_KP_DIVIDE:     return Key_NumDivide;
		case GLFW_KEY_KP_MULTIPLY:   return Key_NumMultiply;
		case GLFW_KEY_KP_SUBTRACT:   return Key_NumSubtract;
		case GLFW_KEY_KP_ADD:        return Key_NumAdd;
		case GLFW_KEY_KP_ENTER:      return Key_Enter;
		
		case GLFW_KEY_F1:            return Key_F1;
		case GLFW_KEY_F2:            return Key_F2;
		case GLFW_KEY_F3:            return Key_F3;
		case GLFW_KEY_F4:            return Key_F4;
		case GLFW_KEY_F5:            return Key_F5;
		case GLFW_KEY_F6:            return Key_F6;
		case GLFW_KEY_F7:            return Key_F7;
		case GLFW_KEY_F8:            return Key_F8;
		case GLFW_KEY_F9:            return Key_F9;
		case GLFW_KEY_F10:           return Key_F10;
		case GLFW_KEY_F11:           return Key_F11;
		case GLFW_KEY_F12:           return Key_F12;
		
		case GLFW_KEY_ENTER:         return Key_Enter;
		case GLFW_KEY_BACKSPACE:     return Key_Backspace;
		case GLFW_KEY_ESCAPE:        return Key_Escape;
		case GLFW_KEY_INSERT:        return Key_Insert;
		case GLFW_KEY_DELETE:        return Key_Delete;
		case GLFW_KEY_HOME:          return Key_Home;
		case GLFW_KEY_END:           return Key_End;
		case GLFW_KEY_PAGE_UP:       return Key_PageUp;
		case GLFW_KEY_PAGE_DOWN:     return Key_PageDown;
		case GLFW_KEY_TAB:           return Key_Tab;
		case GLFW_KEY_CAPS_LOCK:     return Key_CapsLock;
		case GLFW_KEY_NUM_LOCK:      return Key_NumLock;
		
		#if WINDOWS_COMPILATION
		
		case GLFW_KEY_LEFT_CONTROL:
		case GLFW_KEY_RIGHT_CONTROL: return Key_Control;
		case GLFW_KEY_LEFT_ALT:
		case GLFW_KEY_RIGHT_ALT:     return Key_Alt;
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:   return Key_Shift;
		
		#elif OSX_COMPILATION
		
		case GLFW_KEY_LEFT_CONTROL:
		case GLFW_KEY_RIGHT_CONTROL: return Key_Alt;
		case GLFW_KEY_LEFT_SUPER:
		case GLFW_KEY_RIGHT_SUPER:   return Key_Control;
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:   return Key_Shift;
		
		#else
		
		#warning This platform hasn't been taken into account in GetKeyForGlfwKeyCode
		
		#endif
		
		case GLFW_KEY_RIGHT:         return Key_Right;
		case GLFW_KEY_LEFT:          return Key_Left;
		case GLFW_KEY_UP:            return upsideDownModeEnabled ? Key_Down : Key_Up;
		case GLFW_KEY_DOWN:          return upsideDownModeEnabled ? Key_Up : Key_Down;
		
		case GLFW_KEY_EQUAL:         return Key_Plus;
		case GLFW_KEY_MINUS:         return Key_Minus;
		case GLFW_KEY_BACKSLASH:     return Key_Pipe;
		case GLFW_KEY_LEFT_BRACKET:  return Key_OpenBracket;
		case GLFW_KEY_RIGHT_BRACKET: return Key_CloseBracket;
		case GLFW_KEY_SEMICOLON:     return Key_Colon;
		case GLFW_KEY_APOSTROPHE:    return Key_Quote;
		case GLFW_KEY_COMMA:         return Key_Comma;
		case GLFW_KEY_PERIOD:        return Key_Period;
		case GLFW_KEY_SLASH:         return Key_QuestionMark;
		case GLFW_KEY_GRAVE_ACCENT:  return Key_Tilde;
		case GLFW_KEY_SPACE:         return Key_Space;
		
		
		default:                     return Key_NumKeys;
	};
}

#define MAX_GLFW_KEY_CODES_PER_KEY   2
//Returns a count, and stores each code in the output buffer
u8 GetGlfwKeyCodesForKey(Key_t key, int* keyCodeBuffer)
{
	//NOTE: Don't return anything larger than MAX_GLFW_KEY_CODES_PER_KEY
	switch (key)
	{
		case Key_A:            keyCodeBuffer[0] = GLFW_KEY_A; return 1;
		case Key_B:            keyCodeBuffer[0] = GLFW_KEY_B; return 1;
		case Key_C:            keyCodeBuffer[0] = GLFW_KEY_C; return 1;
		case Key_D:            keyCodeBuffer[0] = GLFW_KEY_D; return 1;
		case Key_E:            keyCodeBuffer[0] = GLFW_KEY_E; return 1;
		case Key_F:            keyCodeBuffer[0] = GLFW_KEY_F; return 1;
		case Key_G:            keyCodeBuffer[0] = GLFW_KEY_G; return 1;
		case Key_H:            keyCodeBuffer[0] = GLFW_KEY_H; return 1;
		case Key_I:            keyCodeBuffer[0] = GLFW_KEY_I; return 1;
		case Key_J:            keyCodeBuffer[0] = GLFW_KEY_J; return 1;
		case Key_K:            keyCodeBuffer[0] = GLFW_KEY_K; return 1;
		case Key_L:            keyCodeBuffer[0] = GLFW_KEY_L; return 1;
		case Key_M:            keyCodeBuffer[0] = GLFW_KEY_M; return 1;
		case Key_N:            keyCodeBuffer[0] = GLFW_KEY_N; return 1;
		case Key_O:            keyCodeBuffer[0] = GLFW_KEY_O; return 1;
		case Key_P:            keyCodeBuffer[0] = GLFW_KEY_P; return 1;
		case Key_Q:            keyCodeBuffer[0] = GLFW_KEY_Q; return 1;
		case Key_R:            keyCodeBuffer[0] = GLFW_KEY_R; return 1;
		case Key_S:            keyCodeBuffer[0] = GLFW_KEY_S; return 1;
		case Key_T:            keyCodeBuffer[0] = GLFW_KEY_T; return 1;
		case Key_U:            keyCodeBuffer[0] = GLFW_KEY_U; return 1;
		case Key_V:            keyCodeBuffer[0] = GLFW_KEY_V; return 1;
		case Key_W:            keyCodeBuffer[0] = GLFW_KEY_W; return 1;
		case Key_X:            keyCodeBuffer[0] = GLFW_KEY_X; return 1;
		case Key_Y:            keyCodeBuffer[0] = GLFW_KEY_Y; return 1;
		case Key_Z:            keyCodeBuffer[0] = GLFW_KEY_Z; return 1;
		
		case Key_0:            keyCodeBuffer[0] = GLFW_KEY_0; return 1;
		case Key_1:            keyCodeBuffer[0] = GLFW_KEY_1; return 1;
		case Key_2:            keyCodeBuffer[0] = GLFW_KEY_2; return 1;
		case Key_3:            keyCodeBuffer[0] = GLFW_KEY_3; return 1;
		case Key_4:            keyCodeBuffer[0] = GLFW_KEY_4; return 1;
		case Key_5:            keyCodeBuffer[0] = GLFW_KEY_5; return 1;
		case Key_6:            keyCodeBuffer[0] = GLFW_KEY_6; return 1;
		case Key_7:            keyCodeBuffer[0] = GLFW_KEY_7; return 1;
		case Key_8:            keyCodeBuffer[0] = GLFW_KEY_8; return 1;
		case Key_9:            keyCodeBuffer[0] = GLFW_KEY_9; return 1;
		
		case Key_Num0:         keyCodeBuffer[0] = GLFW_KEY_KP_0; return 1;
		case Key_Num1:         keyCodeBuffer[0] = GLFW_KEY_KP_1; return 1;
		case Key_Num2:         keyCodeBuffer[0] = GLFW_KEY_KP_2; return 1;
		case Key_Num3:         keyCodeBuffer[0] = GLFW_KEY_KP_3; return 1;
		case Key_Num4:         keyCodeBuffer[0] = GLFW_KEY_KP_4; return 1;
		case Key_Num5:         keyCodeBuffer[0] = GLFW_KEY_KP_5; return 1;
		case Key_Num6:         keyCodeBuffer[0] = GLFW_KEY_KP_6; return 1;
		case Key_Num7:         keyCodeBuffer[0] = GLFW_KEY_KP_7; return 1;
		case Key_Num8:         keyCodeBuffer[0] = GLFW_KEY_KP_8; return 1;
		case Key_Num9:         keyCodeBuffer[0] = GLFW_KEY_KP_9; return 1;
		
		case Key_NumPeriod:    keyCodeBuffer[0] = GLFW_KEY_KP_DECIMAL;  return 1;
		case Key_NumDivide:    keyCodeBuffer[0] = GLFW_KEY_KP_DIVIDE;   return 1;
		case Key_NumMultiply:  keyCodeBuffer[0] = GLFW_KEY_KP_MULTIPLY; return 1;
		case Key_NumSubtract:  keyCodeBuffer[0] = GLFW_KEY_KP_SUBTRACT; return 1;
		case Key_NumAdd:       keyCodeBuffer[0] = GLFW_KEY_KP_ADD;      return 1;
		
		case Key_F1:           keyCodeBuffer[0] = GLFW_KEY_F1; return 1;
		case Key_F2:           keyCodeBuffer[0] = GLFW_KEY_F2; return 1;
		case Key_F3:           keyCodeBuffer[0] = GLFW_KEY_F3; return 1;
		case Key_F4:           keyCodeBuffer[0] = GLFW_KEY_F4; return 1;
		case Key_F5:           keyCodeBuffer[0] = GLFW_KEY_F5; return 1;
		case Key_F6:           keyCodeBuffer[0] = GLFW_KEY_F6; return 1;
		case Key_F7:           keyCodeBuffer[0] = GLFW_KEY_F7; return 1;
		case Key_F8:           keyCodeBuffer[0] = GLFW_KEY_F8; return 1;
		case Key_F9:           keyCodeBuffer[0] = GLFW_KEY_F9; return 1;
		case Key_F10:          keyCodeBuffer[0] = GLFW_KEY_F10; return 1;
		case Key_F11:          keyCodeBuffer[0] = GLFW_KEY_F11; return 1;
		case Key_F12:          keyCodeBuffer[0] = GLFW_KEY_F12; return 1;
		
		case Key_Enter:        keyCodeBuffer[0] = GLFW_KEY_ENTER; keyCodeBuffer[1] = GLFW_KEY_KP_ENTER; return 2;
		case Key_Backspace:    keyCodeBuffer[0] = GLFW_KEY_BACKSPACE; return 1;
		case Key_Escape:       keyCodeBuffer[0] = GLFW_KEY_ESCAPE; return 1;
		case Key_Insert:       keyCodeBuffer[0] = GLFW_KEY_INSERT; return 1;
		case Key_Delete:       keyCodeBuffer[0] = GLFW_KEY_DELETE; return 1;
		case Key_Home:         keyCodeBuffer[0] = GLFW_KEY_HOME; return 1;
		case Key_End:          keyCodeBuffer[0] = GLFW_KEY_END; return 1;
		case Key_PageUp:       keyCodeBuffer[0] = GLFW_KEY_PAGE_UP; return 1;
		case Key_PageDown:     keyCodeBuffer[0] = GLFW_KEY_PAGE_DOWN; return 1;
		case Key_Tab:          keyCodeBuffer[0] = GLFW_KEY_TAB; return 1;
		case Key_CapsLock:     keyCodeBuffer[0] = GLFW_KEY_CAPS_LOCK; return 1;
		case Key_NumLock:      keyCodeBuffer[0] = GLFW_KEY_NUM_LOCK;  return 1;
		
		case Key_Control:      keyCodeBuffer[0] = GLFW_KEY_LEFT_CONTROL; keyCodeBuffer[1] = GLFW_KEY_RIGHT_CONTROL; return 2;
		case Key_Alt:          keyCodeBuffer[0] = GLFW_KEY_LEFT_ALT;     keyCodeBuffer[1] = GLFW_KEY_RIGHT_ALT; return 2;  
		case Key_Shift:        keyCodeBuffer[0] = GLFW_KEY_LEFT_SHIFT;   keyCodeBuffer[1] = GLFW_KEY_RIGHT_SHIFT; return 2;
		
		case Key_Right:        keyCodeBuffer[0] = GLFW_KEY_RIGHT; return 1;
		case Key_Left:         keyCodeBuffer[0] = GLFW_KEY_LEFT; return 1;
		case Key_Up:           keyCodeBuffer[0] = GLFW_KEY_UP; return 1;
		case Key_Down:         keyCodeBuffer[0] = GLFW_KEY_DOWN; return 1;
		
		case Key_Plus:         keyCodeBuffer[0] = GLFW_KEY_EQUAL; return 1;
		case Key_Minus:        keyCodeBuffer[0] = GLFW_KEY_MINUS; return 1;
		case Key_Pipe:         keyCodeBuffer[0] = GLFW_KEY_BACKSLASH; return 1;
		case Key_OpenBracket:  keyCodeBuffer[0] = GLFW_KEY_LEFT_BRACKET; return 1;
		case Key_CloseBracket: keyCodeBuffer[0] = GLFW_KEY_RIGHT_BRACKET; return 1;
		case Key_Colon:        keyCodeBuffer[0] = GLFW_KEY_SEMICOLON; return 1;
		case Key_Quote:        keyCodeBuffer[0] = GLFW_KEY_APOSTROPHE; return 1;
		case Key_Comma:        keyCodeBuffer[0] = GLFW_KEY_COMMA; return 1;
		case Key_Period:       keyCodeBuffer[0] = GLFW_KEY_PERIOD; return 1;
		case Key_QuestionMark: keyCodeBuffer[0] = GLFW_KEY_SLASH; return 1;
		case Key_Tilde:        keyCodeBuffer[0] = GLFW_KEY_GRAVE_ACCENT; return 1;
		case Key_Space:        keyCodeBuffer[0] = GLFW_KEY_SPACE; return 1;
		
		
		default: return 0;
	};
}

MouseBtn_t GetMouseBtnForGlfwBtnCode(int glfwButtonCode)
{
	switch (glfwButtonCode)
	{
		case GLFW_MOUSE_BUTTON_LEFT:   return MouseBtn_Left;
		case GLFW_MOUSE_BUTTON_RIGHT:  return MouseBtn_Right;
		case GLFW_MOUSE_BUTTON_MIDDLE: return MouseBtn_Middle;
		default: return MouseBtn_NumBtns;
	};
}

#define MAX_GLFW_BTN_CODES_PER_BTN   2
//Returns a count, and stores each code in the output buffer
u8 GetGlfwBtnCodesForMouseBtn(MouseBtn_t button, int* btnCodeBuffer)
{
	//NOTE: Don't return anything larger than MAX_GLFW_BTN_CODES_PER_BTN
	switch (button)
	{
		case MouseBtn_Left:   btnCodeBuffer[0] = GLFW_MOUSE_BUTTON_LEFT; return 1;
		case MouseBtn_Right:  btnCodeBuffer[0] = GLFW_MOUSE_BUTTON_RIGHT; return 1;
		case MouseBtn_Middle: btnCodeBuffer[0] = GLFW_MOUSE_BUTTON_MIDDLE; return 1;
		default: return 0;
	};
}

#endif //  _COMMON_BUTTON_ENUMS_H
