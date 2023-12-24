/*
File:   app_state.cpp
Author: Taylor Robbins
Date:   09\09\2023
Description: 
	** Holds functions that help us manage AppStateStack_t and call out to AppState functions
*/

AppState_t GetCurrentAppState()
{
	return (pig->appStates.stackSize > 0) ? pig->appStates.stack[pig->appStates.stackSize-1] : AppState_None;
}
bool IsCurrentAppState()
{
	Assert(pig->appStates.contextAppState != AppState_None);
	return (pig->appStates.contextAppState == GetCurrentAppState());
}
bool IsAppStateInitialized(AppState_t appState)
{
	return pig->appStates.infos[appState].initialized;
}
bool IsAppStateActive(AppState_t appState)
{
	return pig->appStates.infos[appState].isActive;
}

void StartAppState(AppState_t appState, bool initialize, AppState_t prevState, MyStr_t transitionStr)
{
	AppStateInfo_t* info = &pig->appStates.infos[appState];
	NotNull(info->Start);
	Assert(initialize == !info->initialized);
	pig->appStates.contextAppState = appState;
	PrintLine_D("%s AppState_%s", (initialize ? "Initializing" : "Starting"), GetAppStateStr(appState));
	if (info->Start) { info->Start(initialize, prevState, transitionStr); }
	if (initialize) { info->initialized = true; }
	info->isActive = true;
	pig->appStates.contextAppState = AppState_None;
}

void StopAppState(AppState_t appState, bool deinitialize, AppState_t nextState)
{
	AppStateInfo_t* info = &pig->appStates.infos[appState];
	NotNull(info->Stop);
	Assert(info->initialized);
	pig->appStates.contextAppState = appState;
	PrintLine_D("%s AppState_%s", (deinitialize ? "Deinitializing" : "Stopping"), GetAppStateStr(appState));
	if (info->Stop) { info->Stop(deinitialize, nextState); }
	if (deinitialize) { info->initialized = false; }
	info->isActive = false;
	pig->appStates.contextAppState = AppState_None;
}

void UpdateAppState(AppState_t appState)
{
	AppStateInfo_t* info = &pig->appStates.infos[appState];
	NotNull(info->Update);
	Assert(info->initialized);
	pig->appStates.contextAppState = appState;
	if (info->Update) { info->Update(); }
	pig->appStates.contextAppState = AppState_None;
}

void RenderAppState(AppState_t appState, bool isOnTop)
{
	AppStateInfo_t* info = &pig->appStates.infos[appState];
	NotNull(info->Render);
	Assert(info->initialized);
	pig->appStates.contextAppState = appState;
	if (info->Render) { info->Render(isOnTop); }
	pig->appStates.contextAppState = AppState_None;
}

bool IsFullAppState(AppState_t appState)
{
	bool result = true;
	AppStateInfo_t* info = &pig->appStates.infos[appState];
	Assert(info->initialized);
	if (info->IsFull != nullptr) { result = info->IsFull(); }
	return result;
}

void StartFirstAppState(AppState_t appState)
{
	pig->appStates.stack[0] = appState;
	pig->appStates.stackSize = 1;
	StartAppState(appState, true, AppState_None, MyStr_Empty);
}

void UpdateAppStateStack()
{
	for (u64 cIndex = 0; cIndex < pig->appStates.numChanges; cIndex++)
	{
		AppStateChange_t* change = &pig->appStates.changes[cIndex];
		switch (change->type)
		{
			case AppStateChangeType_Push:
			{
				Assert(pig->appStates.stackSize < ArrayCount(pig->appStates.stack));
				AppStateInfo_t* requestedInfo = &pig->appStates.infos[change->requestedState];
				AppState_t prevState = GetCurrentAppState();
				if (prevState != AppState_None) { StopAppState(prevState, false, change->requestedState); }
				StartAppState(change->requestedState, !requestedInfo->initialized, prevState, change->transitionStr);
				pig->appStates.stack[pig->appStates.stackSize] = change->requestedState;
				pig->appStates.stackSize++;
			} break;
			
			case AppStateChangeType_Pop:
			{
				Assert(pig->appStates.stackSize <= ArrayCount(pig->appStates.stack));
				AppState_t prevState = GetCurrentAppState();
				Assert(prevState == change->requestingState);
				AppState_t newState = (pig->appStates.stackSize > 1 ? pig->appStates.stack[pig->appStates.stackSize-2] : AppState_None);
				StopAppState(prevState, !change->stayInitialized, newState);
				pig->appStates.stackSize--;
				if (newState != AppState_None) { StartAppState(newState, false, prevState, change->transitionStr); }
			} break;
			
			case AppStateChangeType_Change:
			{
				Assert(pig->appStates.stackSize <= ArrayCount(pig->appStates.stack));
				AppStateInfo_t* requestedInfo = &pig->appStates.infos[change->requestedState];
				AppState_t prevState = GetCurrentAppState();
				if (prevState != AppState_None)
				{
					StopAppState(prevState, !change->stayInitialized, change->requestedState);
					pig->appStates.stackSize--;
				}
				StartAppState(change->requestedState, !requestedInfo->initialized, prevState, change->transitionStr);
				pig->appStates.stack[pig->appStates.stackSize] = change->requestedState;
				pig->appStates.stackSize++;
			} break;
			
			default: Assert(false); break;
		}
		if (!IsEmptyStr(change->transitionStr))
		{
			FreeString(mainHeap, &change->transitionStr);
		}
	}
	pig->appStates.numChanges = 0;
	
	if (pig->appStates.stackSize > 0)
	{
		UpdateAppState(pig->appStates.stack[pig->appStates.stackSize-1]);
	}
}

void ClearAppStateStackBeforeClose()
{
	for (u64 stackIndex = pig->appStates.stackSize; stackIndex > 0; stackIndex--)
	{
		StopAppState(pig->appStates.stack[stackIndex-1], true, AppState_None);
	}
	pig->appStates.stackSize = 0;
}

void RenderAppStateStack()
{
	u64 firstFullAppStateIndex = 0;
	for (u64 sIndex = pig->appStates.stackSize; sIndex > 0; sIndex--)
	{
		if (IsFullAppState(pig->appStates.stack[sIndex-1]))
		{
			firstFullAppStateIndex = sIndex-1;
			break;
		}
	}
	
	if (pig->appStates.stackSize == 0)
	{
		pd->graphics->clear(kColorBlack);
	}
	
	for (u64 sIndex = firstFullAppStateIndex; sIndex < pig->appStates.stackSize; sIndex++)
	{
		RenderAppState(pig->appStates.stack[sIndex], (sIndex == pig->appStates.stackSize-1));
	}
}

// +--------------------------------------------------------------+
// |                             API                              |
// +--------------------------------------------------------------+
void* RegisterAppState(AppState_t state, u64 dataSize, AppStateStart_f* StartFunc, AppStateStop_f* StopFunc, AppStateUpdate_f* UpdateFunc, AppStateRender_f* RenderFunc, AppStateIsFull_f* IsFullFunc = nullptr)
{
	Assert(state < AppState_NumStates);
	NotNull4(StartFunc, StopFunc, UpdateFunc, RenderFunc);
	AppStateInfo_t* info = &pig->appStates.infos[state];
	Assert(info->dataPntr == nullptr);
	ClearPointer(info);
	info->dataSize = dataSize;
	info->dataPntr = AllocMem(fixedHeap, dataSize);
	NotNull(info->dataPntr);
	MyMemSet(info->dataPntr, 0x00, dataSize);
	info->Start = StartFunc;
	info->Stop = StopFunc;
	info->Update = UpdateFunc;
	info->Render = RenderFunc;
	info->IsFull = IsFullFunc;
	return info->dataPntr;
}

void PushAppState(AppState_t newAppState, MyStr_t transitionStr = MyStr_Empty_Const)
{
	Assert(pig->appStates.numChanges < MAX_NUM_APP_STATE_CHANGE_REQUESTS);
	Assert(pig->appStates.contextAppState != AppState_None);
	Assert(newAppState != AppState_None);
	Assert(!IsAppStateActive(newAppState));
	AppStateChange_t* change = &pig->appStates.changes[pig->appStates.numChanges];
	ClearPointer(change);
	change->type = AppStateChangeType_Push;
	change->requestingState = pig->appStates.contextAppState;
	change->requestedState = newAppState;
	if (!IsEmptyStr(transitionStr)) { change->transitionStr = AllocString(mainHeap, &transitionStr); }
	pig->appStates.numChanges++;
}

void PopAppState(MyStr_t transitionStr = MyStr_Empty, bool stayInitialized = false)
{
	Assert(pig->appStates.numChanges < MAX_NUM_APP_STATE_CHANGE_REQUESTS);
	Assert(pig->appStates.contextAppState != AppState_None);
	AppStateChange_t* change = &pig->appStates.changes[pig->appStates.numChanges];
	ClearPointer(change);
	change->type = AppStateChangeType_Pop;
	change->requestingState = pig->appStates.contextAppState;
	if (!IsEmptyStr(transitionStr)) { change->transitionStr = AllocString(mainHeap, &transitionStr); }
	change->stayInitialized = stayInitialized;
	pig->appStates.numChanges++;
}

void ChangeAppState(AppState_t newAppState, MyStr_t transitionStr = MyStr_Empty, bool stayInitialized = false)
{
	Assert(pig->appStates.numChanges < MAX_NUM_APP_STATE_CHANGE_REQUESTS);
	Assert(pig->appStates.contextAppState != AppState_None);
	Assert(newAppState != AppState_None);
	Assert(!IsAppStateActive(newAppState));
	AppStateChange_t* change = &pig->appStates.changes[pig->appStates.numChanges];
	ClearPointer(change);
	change->type = AppStateChangeType_Change;
	change->requestingState = pig->appStates.contextAppState;
	change->requestedState = newAppState;
	if (!IsEmptyStr(transitionStr)) { change->transitionStr = AllocString(mainHeap, &transitionStr); }
	change->stayInitialized = stayInitialized;
	pig->appStates.numChanges++;
}
