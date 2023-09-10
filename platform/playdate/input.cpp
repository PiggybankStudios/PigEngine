/*
File:   input.cpp
Author: Taylor Robbins
Date:   09\08\2023
Description: 
	** Handles taking the raw input from the PlayDate's input API and converting it
	** into a highly usable format for the game to query.
*/

void InitializeAppInput()
{
	NotNull(input);
	ClearPointer(input);
}

void UpdateAppInputBtnState(Btn_t btn, bool isDown)
{
	NotNull(input);
	BtnState_t* state = &input->btnStates[btn];
	if (state->isDown != isDown)
	{
		state->isDown = isDown;
		if (isDown) { state->wasPressed = true; }
		else { state->wasReleased = true; }
		state->numTransitions++;
		// if (!IsPseudoBtn(btn)) { PrintLine_D("Btn_%s %s", GetBtnStr(btn), isDown ? "Pressed" : "Released"); }
	}
}

void UpdateAppInput()
{
	for (u64 bIndex = 0; bIndex < ArrayCount(input->btnStates); bIndex++)
	{
		BtnState_t* state = &input->btnStates[bIndex];
		state->wasDown = state->isDown;
		state->wasPressed = false;
		state->wasReleased = false;
		state->numTransitions = 0;
		state->handled = false;
	}
	input->crankDeltaHandled = false;
	input->crankAnglePrev = input->crankAngle;
	input->programTimePrev = input->programTime;
	input->realProgramTimePrev = input->realProgramTime;
	
	PDLanguage newLanguage = pd->system->getLanguage();
	if (input->language != newLanguage)
	{
		input->language = newLanguage;
		input->languageChanged = true;
	}
	
	input->programTime = pd->system->getCurrentTimeMilliseconds();
	input->elapsedMsU32 = (input->programTime >= input->programTimePrev) ? input->programTime - input->programTimePrev : 0;
	input->elapsedMs = (r32)input->elapsedMsU32;
	input->timeScale = (input->elapsedMs / TARGET_FRAME_TIME);
	if (AbsR32(input->timeScale - 1.0f) < TIME_SCALE_SNAP_PERCENT) { input->timeScale = 1.0f; }
	ProgramTime = input->programTime;
	ElapsedMs = (r32)input->elapsedMs;
	TimeScale = input->timeScale;
	
	input->timeSinceEpochSecs = pd->system->getSecondsSinceEpoch(&input->timeSinceEpochMs);
	input->timeSinceEpoch = (u64)input->timeSinceEpochSecs * 1000 + (u64)input->timeSinceEpochMs;
	input->realProgramTime = (u32)(input->timeSinceEpoch - pig->programStartTimeSinceEpoch);
	input->realElapsedMsU32 = (input->realProgramTime >= input->realProgramTimePrev) ? input->realProgramTime - input->realProgramTimePrev : 0;
	input->realElapsedMs = (r32)input->realElapsedMsU32;
	input->realTimeScale = (input->realElapsedMs / TARGET_FRAME_TIME);
	if (AbsR32(input->realTimeScale - 1.0f) < TIME_SCALE_SNAP_PERCENT) { input->realTimeScale = 1.0f; }
	
	PDButtons currentBtnBits = (PDButtons)0;
	PDButtons pressedBtnBits = (PDButtons)0;
	PDButtons releasedBtnBits = (PDButtons)0;
	pd->system->getButtonState(&currentBtnBits, &pressedBtnBits, &releasedBtnBits);
	UpdateAppInputBtnState(Btn_A,     IsFlagSet(currentBtnBits, kButtonA));
	UpdateAppInputBtnState(Btn_B,     IsFlagSet(currentBtnBits, kButtonB));
	UpdateAppInputBtnState(Btn_Right, IsFlagSet(currentBtnBits, kButtonRight));
	UpdateAppInputBtnState(Btn_Down,  IsFlagSet(currentBtnBits, kButtonDown));
	UpdateAppInputBtnState(Btn_Left,  IsFlagSet(currentBtnBits, kButtonLeft));
	UpdateAppInputBtnState(Btn_Up,    IsFlagSet(currentBtnBits, kButtonUp));
	
	input->crankAngle = pd->system->getCrankAngle();
	input->crankDelta = input->crankAngle - input->crankAnglePrev;
	input->crankMoved = BasicallyEqualR32(input->crankAngle, input->crankAnglePrev, CRANK_DELTA_DEADZONE);
	input->crankAngleRadians = ToRadians32(input->crankAngle);
	UpdateAppInputBtnState(Btn_CrankDock, pd->system->isCrankDocked());
	UpdateAppInputBtnState(Btn_CrankCw, (input->crankDelta >= CRANK_BTN_DEADZONE));
	UpdateAppInputBtnState(Btn_CrankCcw, (input->crankDelta <= -CRANK_BTN_DEADZONE));
	
	pd->system->getAccelerometer(&input->accelVec.x, &input->accelVec.y, &input->accelVec.z);
	
	for (u64 bIndex = 0; bIndex < ArrayCount(input->btnStates); bIndex++)
	{
		BtnState_t* state = &input->btnStates[bIndex];
		if (state->handledExtended)
		{
			if (state->isDown) { state->handled = true; }
			else
			{
				if (state->wasDown) { state->handled = true; }
				state->handledExtended = false;
			}
		}
	}
}

inline bool WasBtnHandled(Btn_t btn)     { return input->btnStates[btn].handled;                                               }
inline void HandleBtn(Btn_t btn)         { input->btnStates[btn].handled = true;                                               }
inline void HandleBtnExtended(Btn_t btn) { input->btnStates[btn].handled = true; input->btnStates[btn].handledExtended = true; }
inline bool WasCrankDeltaHandled()       { return input->crankDeltaHandled;                                                    }
inline void HandleCrankDelta()           { input->crankDeltaHandled = true;                                                    }
inline void HandleCrankDocked()          { HandleBtn(Btn_CrankDock);                                                           }
inline void HandleCrankDockedExtended()  { HandleBtnExtended(Btn_CrankDock);                                                   }

inline bool BtnDownRaw(Btn_t btn)     { return input->btnStates[btn].isDown;                                            }
inline bool BtnPressedRaw(Btn_t btn)  { return input->btnStates[btn].wasPressed;                                        }
inline bool BtnReleasedRaw(Btn_t btn) { return input->btnStates[btn].wasReleased;                                       }
inline bool BtnChangedRaw(Btn_t btn)  { return (input->btnStates[btn].wasPressed || input->btnStates[btn].wasReleased); }
inline bool CrankMovedRaw()           { return input->crankMoved;                                                       }
inline bool IsCrankDockedRaw()        { return BtnDownRaw(Btn_CrankDock);                                               }

inline bool BtnDown(Btn_t btn)     { return (!WasBtnHandled(btn) && BtnDownRaw(btn));     }
inline bool BtnPressed(Btn_t btn)  { return (!WasBtnHandled(btn) && BtnPressedRaw(btn));  }
inline bool BtnReleased(Btn_t btn) { return (!WasBtnHandled(btn) && BtnReleasedRaw(btn)); }
inline bool BtnChanged(Btn_t btn)  { return (!WasBtnHandled(btn) && BtnChangedRaw(btn));  }
inline bool CrankMoved()           { return (!WasCrankDeltaHandled() && CrankMovedRaw()); }
inline bool IsCrankUndocked()      { return !BtnDown(Btn_CrankDock);                      }
inline bool IsCrankDocked()        { return BtnDown(Btn_CrankDock);                       }
