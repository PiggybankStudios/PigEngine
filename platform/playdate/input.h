/*
File:   input.h
Author: Taylor Robbins
Date:   09\08\2023
*/

#ifndef _INPUT_H
#define _INPUT_H

#define CRANK_DELTA_DEADZONE  0.1f
#define CRANK_BTN_DEADZONE    0.1f

enum Btn_t
{
	Btn_None = 0,
	Btn_A,
	Btn_B,
	Btn_Right,
	Btn_Down,
	Btn_Left,
	Btn_Up,
	Btn_CrankDock,
	Btn_CrankCw,
	Btn_CrankCcw,
	Btn_NumButtons,
};
const char* GetBtnStr(Btn_t enumValue)
{
	switch (enumValue)
	{
		case Btn_None:      return "None";
		case Btn_A:         return "A";
		case Btn_B:         return "B";
		case Btn_Right:     return "Right";
		case Btn_Down:      return "Down";
		case Btn_Left:      return "Left";
		case Btn_Up:        return "Up";
		case Btn_CrankDock: return "CrankDock";
		case Btn_CrankCw:   return "CrankCw";
		case Btn_CrankCcw:  return "CrankCcw";
		default: return "Unknown";
	}
}

inline bool IsLetterBtn(Btn_t btn) { return (btn == Btn_A || btn == Btn_B); }
inline bool IsDpadBtn(Btn_t btn) { return (btn == Btn_Right || btn == Btn_Down || btn == Btn_Left || btn == Btn_Up); }
inline bool IsFaceBtn(Btn_t btn) { return (IsDpadBtn(btn) || IsLetterBtn(btn)); }
inline bool IsPseudoBtn(Btn_t btn) { return (!IsDpadBtn(btn) && !IsLetterBtn(btn) && btn != Btn_CrankDock); } //pseudo-buttons are ones derived from analog values
inline Btn_t GetCrankBtn(bool clockwise) { return (clockwise ? Btn_CrankCw : Btn_CrankCcw); }
inline Dir2_t ToDir2(Btn_t btn)
{
	switch (btn)
	{
		case Btn_Right: return Dir2_Right;
		case Btn_Down:  return Dir2_Down;
		case Btn_Left:  return Dir2_Left;
		case Btn_Up:    return Dir2_Up;
		default: return Dir2_None;
	}
}
inline Btn_t ToBtn(Dir2_t direction)
{
	switch (direction)
	{
		case Dir2_Right: return Btn_Right;
		case Dir2_Down:  return Btn_Down;
		case Dir2_Left:  return Btn_Left;
		case Dir2_Up:    return Btn_Up;
		default: return Btn_None;
	}
}

struct BtnState_t
{
	bool isDown;
	bool wasDown;
	bool wasPressed;
	bool wasReleased;
	u8 numTransitions;
	bool handled;
	bool handledExtended;
};
struct AppInput_t
{
	PDLanguage language;
	bool languageChanged;
	
	u32 programTime;
	u32 programTimePrev;
	u32 elapsedMsU32;
	r32 elapsedMs;
	r32 timeScale;
	
	u32 timeSinceEpochSecs;
	u32 timeSinceEpochMs;
	u64 timeSinceEpoch; //total ms
	
	u32 realProgramTime;
	u32 realProgramTimePrev;
	u32 realElapsedMsU32;
	r32 realElapsedMs;
	r32 realTimeScale;
	
	r32 crankAngle;
	r32 crankAnglePrev;
	r32 crankDelta;
	r32 crankMoved;
	r32 crankDeltaHandled;
	r32 crankAngleRadians;
	
	v3 accelVec; //don't forget to call pd->system->setPeripheralsEnabled(kAccelerometer)
	
	BtnState_t btnStates[Btn_NumButtons];
};

#endif //  _INPUT_H
