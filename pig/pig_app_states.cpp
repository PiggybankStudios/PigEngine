/*
File:   pig_app_states.cpp
Author: Taylor Robbins
Date:   07\09\2022
Description: 
	** Holds functions that allow the application to easily make requests to change AppState
*/

void Pig_InitializeAppStateStack()
{
	pig->appStateStackSize = 0;
	ClearArray(pig->appStateStack);
	ClearArray(pig->appStateInitialized);
	pig->currentAppState = AppState_None;
	pig->thisAppState = AppState_None;
}

void Pig_HandleAppStateChanges(bool initialAppState)
{
	UNUSED(initialAppState);
	if (pig->appStateChange == AppStateChange_Push || (pig->appStateChange == AppStateChange_Change && pig->appStateStackSize == 0))
	{
		Assert(pig->newAppState != AppState_None && pig->newAppState < AppState_NumStates);
		Assert(pig->appStateStackSize < MAX_APP_STATE_STACK_SIZE);
		if (pig->currentAppState != AppState_None)
		{
			StopAppState(pig->currentAppState, pig->newAppState, false);
		}
		pig->appStateStack[pig->appStateStackSize] = pig->newAppState;
		pig->appStateStackSize++;
		AppState_t oldAppState = pig->currentAppState;
		pig->currentAppState = pig->newAppState;
		StartAppState(pig->newAppState, oldAppState, !pig->appStateInitialized[pig->newAppState]);
		pig->appStateInitialized[pig->newAppState] = true;
	}
	else if (pig->appStateChange == AppStateChange_Pop)
	{
		Assert(pig->appStateStackSize > 0);
		AppState_t newAppState = ((pig->appStateStackSize > 1) ? pig->appStateStack[pig->appStateStackSize-2] : AppState_None);
		if (pig->appStateInitialized[pig->currentAppState])
		{
			StopAppState(pig->currentAppState, newAppState, true);
			pig->appStateInitialized[pig->currentAppState] = false;
		}
		pig->appStateStackSize--;
		AppState_t oldAppState = pig->currentAppState;
		pig->currentAppState = newAppState;
		StartAppState(newAppState, oldAppState, !pig->appStateInitialized[newAppState]);
	}
	else if (pig->appStateChange == AppStateChange_Change)
	{
		Assert(pig->appStateStackSize > 0);
		if (pig->appStateInitialized[pig->currentAppState])
		{
			StopAppState(pig->currentAppState, pig->newAppState, true);
			pig->appStateInitialized[pig->currentAppState] = false;
		}
		pig->appStateStack[pig->appStateStackSize-1] = pig->newAppState;
		AppState_t oldAppState = pig->currentAppState;
		pig->currentAppState = pig->newAppState;
		StartAppState(pig->newAppState, oldAppState, !pig->appStateInitialized[pig->newAppState]);
	}
	else if (pig->appStateChange != AppState_None) { Unimplemented(); }
	
	pig->appStateChange = AppStateChange_None;
	pig->newAppState = AppState_None;
}

bool IsAppStateInitialized(AppState_t appState)
{
	Assert(appState != AppState_None && appState < AppState_NumStates);
	for (u64 sIndex = 0; sIndex < pig->appStateStackSize; sIndex++)
	{
		if (pig->appStateStack[sIndex] == appState) { return true; }
	}
	return false;
}

void ChangeAppState(AppState_t newState)
{
	AssertSingleThreaded();
	pig->appStateChange = AppStateChange_Change;
	pig->newAppState = newState;
}

void PushAppState(AppState_t newState)
{
	AssertSingleThreaded();
	pig->appStateChange = AppStateChange_Push;
	pig->newAppState = newState;
}
void PopAppState()
{
	AssertSingleThreaded();
	pig->appStateChange = AppStateChange_Pop;
}
