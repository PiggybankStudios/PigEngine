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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetKeyStr(Key_t key);
#else
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
#endif

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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetControllerBtnStr(ControllerBtn_t controllerBtn);
#else
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
#endif

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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetMouseBtnStr(MouseBtn_t mouseBtn);
#else
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
#endif

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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetModifierKeyStr(ModifierKey_t modifierKey);
#else
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
#endif

#ifdef PIG_COMMON_HEADER_ONLY
ModifierKey_t GetModifierKeyForKey(Key_t key);
#else
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
#endif

#ifdef PIG_COMMON_HEADER_ONLY
Key_t GetKeyForModifierKey(ModifierKey_t modifierKey);
#else
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
#endif

#endif //  _COMMON_BUTTON_ENUMS_H
