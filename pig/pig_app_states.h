/*
File:   pig_app_states.h
Author: Taylor Robbins
Date:   07\09\2022
*/

#ifndef _PIG_APP_STATES_H
#define _PIG_APP_STATES_H

#define MAX_APP_STATE_STACK_SIZE 16

enum AppStateChange_t
{
	AppStateChange_None = 0,
	AppStateChange_Change,
	AppStateChange_Push,
	AppStateChange_Pop,
	AppStateChange_NumTypes,
};
const char* GetAppStateChangeStr(AppStateChange_t stateChange)
{
	switch (stateChange)
	{
		case AppStateChange_None:   return "None";
		case AppStateChange_Change: return "Change";
		case AppStateChange_Push:   return "Push";
		case AppStateChange_Pop:    return "Pop";
		default: return "Unknown";
	}
}

//Implemented in app_state_list.cpp which is provided by the player
void StartAppState(AppState_t appState, AppState_t oldAppState, bool initialize);
void StopAppState(AppState_t appState, AppState_t newAppState, bool deinitialize, bool shuttingDown);
void UpdateAppState(AppState_t appState);
void RenderAppState(AppState_t appState, FrameBuffer_t* renderBuffer, bool bottomLayer);

#endif //  _PIG_APP_STATES_H
