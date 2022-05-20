/*
File:   web_input.cpp
Author: Taylor Robbins
Date:   03\20\2022
Description: 
	** Holds functions that help us manage the EngineInput_t structure that is fed to the application
*/

#define TARGET_FRAMERATE                    60 //fps TODO: Move this into a proper configurable
#define NUM_FRAMERATE_AVGS                  5  //frames
// #define CONTROLLER_STICK_DEADZONE           0.2f //magnitude
// #define CONTROLLER_STICK_DIR_DEADZONE       0.6f //magnitude
// #define CONTROLLER_TRIGGER_ACTIVE_THRESHOLD 0.5f //triggerValue TODO: Check this value!
// #define STABLE_FRAMERATE_MAX                66 //ms
// #define STABLE_FRAMERATE_RUN_MIN            3 //frames

void Web_FillEngineInput(EngineInput_t* input)
{
	NotNull(input);
	CreateVarArray(&input->inputEvents, &Platform->mainHeap, sizeof(InputEvent_t));
	for (u64 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
	{
		input->controllerStates[cIndex].index = cIndex;
	}
}

void Web_CopyEngineInput(EngineInput_t* dest, EngineInput_t* source)
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

void Web_RefreshButtonState(PlatBtnState_t* state)
{
	NotNull(state);
	state->wasDown = state->isDown;
	state->numPresses = 0;
	state->numReleases = 0;
	state->numTransitions = 0;
	state->numRepeats = 0;
}

void Web_ResetEngineInput(EngineInput_t* input)
{
	NotNull(input);
	
	input->actualElapsedMsIgnored = false;
	input->scrollChangedX = false;
	input->scrollChangedY = false;
	input->scrollDelta = Vec2_Zero;
	
	for (u64 keyIndex = 0; keyIndex < ArrayCount(input->keyStates); keyIndex++)
	{
		Web_RefreshButtonState(&input->keyStates[keyIndex]);
	}
	
	for (u64 bIndex = 0; bIndex < ArrayCount(input->mouseBtnStates); bIndex++)
	{
		Web_RefreshButtonState(&input->mouseBtnStates[bIndex]);
	}
	
	for (u64 cIndex = 0; cIndex < ArrayCount(input->controllerStates); cIndex++)
	{
		PlatControllerState_t* controller = &input->controllerStates[cIndex];
		controller->connectedChanged = false;
		for (u64 bIndex = 0; bIndex < ControllerBtn_NumBtns; bIndex++)
		{
			Web_RefreshButtonState(&controller->btnStates[bIndex]);
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

void Web_UpdateEngineInputTimeInfo(EngineInput_t* prevInput, EngineInput_t* newInput)
{
	NotNull(newInput);
	
	r64 lastTimeF = (prevInput != nullptr ? prevInput->programTimeF : 0.0);
	newInput->programTimeF = js_GetTime();
	newInput->programTime = (u64)newInput->programTimeF;
	newInput->elapsedMs = newInput->programTimeF - lastTimeF;
	// newInput->lastUpdateElapsedMs = newInput->elapsedMs - Platform->timeSpentOnSwapBuffers;
	// newInput->timeSpentWaitingLastFrame = Platform->timeSpentOnSwapBuffers;
	// Platform->timeSpentOnSwapBuffers = 0.0;
	newInput->uncappedElapsedMs = newInput->elapsedMs;
	// if (windowInteractionOccurred || !Platform->hasReachedStableFramerate)
	// {
	// 	//NOTE: Anything based off programTime will still jump far ahead but I think that should be fine.
	// 	//      Anything that shouldn't be scaled arbitrarily by long frame hitches should be based off elapsedMs, not programTime
	// 	// PrintLine_D("Clamping elapsedMs from %lf to %lf", newInput->elapsedMs, (1000.0 / 60.0));
	// 	newInput->actualElapsedMsIgnored = true;
	// 	r64 newElapsedMs = (1000.0 / TARGET_FRAMERATE);
	// 	newInput->lastUpdateElapsedMs = (newInput->lastUpdateElapsedMs / newInput->elapsedMs) * newElapsedMs;
	// 	newInput->timeSpentWaitingLastFrame = (newInput->timeSpentWaitingLastFrame / newInput->elapsedMs) * newElapsedMs;
	// 	newInput->elapsedMs = newElapsedMs;
	// }
	// else
	{
		newInput->actualElapsedMsIgnored = false;
	}
	newInput->framerate = 1000.0 / newInput->elapsedMs;
	newInput->timeDelta = newInput->elapsedMs / (1000.0 / (r64)TARGET_FRAMERATE);
	newInput->avgElapsedMs = ((newInput->avgElapsedMs * (NUM_FRAMERATE_AVGS-1)) + newInput->elapsedMs) / NUM_FRAMERATE_AVGS;
	newInput->avgFramerate = ((newInput->avgFramerate * (NUM_FRAMERATE_AVGS-1)) + newInput->framerate) / NUM_FRAMERATE_AVGS;
	
	//TODO: We need to find a way to fill this stuff out on web
	// u64 unixTimestamp = 0;
	// FILETIME systemFileTime = {};
	// GetSystemTimeAsFileTime(&systemFileTime);
	// ULARGE_INTEGER systemLargeIntegerTime = {};
	// systemLargeIntegerTime.HighPart = systemFileTime.dwHighDateTime;
	// systemLargeIntegerTime.LowPart = systemFileTime.dwLowDateTime;
	// unixTimestamp = (u64)(systemLargeIntegerTime.QuadPart/10000000ULL);
	// if (unixTimestamp >= WIN32_FILETIME_SEC_OFFSET) { unixTimestamp -= WIN32_FILETIME_SEC_OFFSET; }
	// ConvertTimestampToRealTime(unixTimestamp, &newInput->unixTime, false);
	
	// u64 localTimestamp = 0;
	// TIME_ZONE_INFORMATION timezoneInfo = {};
	// DWORD timezoneResult = GetTimeZoneInformation(&timezoneInfo);
	// DebugAssertAndUnusedMsg(timezoneResult != TIME_ZONE_ID_INVALID, timezoneResult, "GetTimeZoneInformation failed and gave TIME_ZONE_ID_INVALID");
	// newInput->localTimezoneOffset = -((i64)timezoneInfo.Bias * NUM_SEC_PER_MINUTE);
	// localTimestamp = unixTimestamp + newInput->localTimezoneOffset;
	// newInput->localTimezoneDoesDst = (timezoneInfo.DaylightBias != 0);
	// MyStr_t timezoneName = ConvertUcs2StrToUtf8Nt(TempArena, &timezoneInfo.StandardName[0]);
	// if (StrCompareIgnoreCase(Platform->localTimezoneName, timezoneName) != 0)
	// {
	// 	if (!IsStrEmpty(Platform->localTimezoneName)) { FreeString(&Platform->mainHeap, &Platform->localTimezoneName); }
	// 	Platform->localTimezoneName = AllocString(&Platform->mainHeap, &timezoneName);
	// }
	// newInput->localTimezoneName = Platform->localTimezoneName;
	// ConvertTimestampToRealTime(localTimestamp, &newInput->localTime, newInput->localTimezoneDoesDst);
}

