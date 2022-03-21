/*
File:   win32_glfw.h
Author: Taylor Robbins
Date:   03\13\2022
*/

#ifndef _WIN_32_GLFW_H
#define _WIN_32_GLFW_H

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

#endif //  _WIN_32_GLFW_H
