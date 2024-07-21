/*
File:   pig_main.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Holds the PigState_t structure which is the main place that we store all of our variables for the engine
*/

#ifndef _PIG_MAIN_H
#define _PIG_MAIN_H

struct PigGifFrame_t
{
	PlatImageData_t imageData;
};

struct PigWindowState_t
{
	u64 windowId;
	
	bool selectingSubPart;
	v2i subPartStartPos;
	reci screenSubPart;
	
	FrameBuffer_t frameBuffer;
	bool takeScreenshot;
	bool screenshotKeyWasUsedForSelection;
	
	bool recordingGif;
	bool finishGif;
	bool gifKeyWasUsedForSelection;
	LinkedList_t gifFrames; //PigGifFrame_t
};

struct PigState_t
{
	bool initialized;
	bool firstUpdate;
	bool dllReloaded;
	u64 reloadIndex;
	ThreadId_t mainThreadId;
	bool dontExitOnAssert;
	
	PigSettings_t settings;
	PigDebugBindings_t debugBindings;
	PigDebugBindings_t sessionDebugBindings;
	RandomSeries_t random;
	
	//Memory Arenas
	MemArena_t fixedHeap;
	MemArena_t mainHeap;
	MemArena_t threadSafeHeap;
	MemArena_t imguiHeap;
	MemArena_t largeAllocHeap;
	MemArena_t tempArena;
	MemArena_t audioHeap;
	#if PIG_MAIN_ARENA_DEBUG
	MemArena_t mainHeapDebug;
	#endif
	#if PIG_MEM_ARENA_TEST_SET
	MemArenaTestSet_t arenaTestSet;
	#endif
	
	//AppStates
	AppGlobals_t appGlobals;
	AppStateStructs_t appStateStructs;
	bool appStateInitialized[AppState_NumStates];
	
	//AppState Stack
	u64 appStateStackSize;
	AppState_t appStateStack[MAX_APP_STATE_STACK_SIZE];
	AppState_t currentAppState; //the one at the top of appStateStack
	AppState_t thisAppState; //the one we are currently calling update/render/etc on
	AppStateChange_t appStateChange;
	AppState_t newAppState;
	
	//Rendering
	RenderApi_t renderApi;
	RenderContext_t renderContext;
	
	//IDs
	u64 nextSoundId;
	u64 nextShaderId;
	u64 nextVertBufferId;
	u64 nextTextureId;
	u64 nextFrameBufferId;
	u64 nextVectorImgId;
	u64 nextSpriteSheetId;
	u64 nextUiId;
	u64 nextWavOggAudioDataId;
	u64 nextConfirmDialogId;
	
	//Resources
	Resources_t resources;
	
	//Window Management
	const PlatWindow_t* currentWindow;
	PigWindowState_t* currentWindowState;
	LinkedList_t windowStates;
	
	//Input Handling
	bool isMouseFullyCaptured;
	bool wasMouseCapturedLastFrame;
	MouseHitInfo_t prevMouseHit;
	MouseHitInfo_t mouseHit;
	u64 prevProgramTime;
	BtnHandlingInfo_t keyHandled[Key_NumKeys];
	BtnHandlingInfo_t mouseBtnHandled[MouseBtn_NumBtns];
	BtnHandlingInfo_t controllerBtnHandled[MAX_NUM_CONTROLLERS][ControllerBtn_NumBtns];
	BtnHandlingInfo_t scrollXHandled;
	BtnHandlingInfo_t scrollYHandled;
	//For Debug Purposes:
	BtnHandlingInfo_t prevKeyHandled[Key_NumKeys];
	BtnHandlingInfo_t prevMouseBtnHandled[MouseBtn_NumBtns];
	BtnHandlingInfo_t prevControllerBtnHandled[MAX_NUM_CONTROLLERS][ControllerBtn_NumBtns];
	BtnHandlingInfo_t prevScrollXHandled;
	BtnHandlingInfo_t prevScrollYHandled;
	
	//Focus Tracking
	VarArray_t unfocusedItems; //const void*
	const void* focusedItemPntr;
	MyStr_t focusedItemName;
	bool isFocusedItemTyping;
	
	//Debug Menus
	PigImguiState_t imgui;
	DebugConsole_t debugConsole;
	PigNotificationQueue_t notificationsQueue;
	PigDebugOverlay_t debugOverlay;
	LogGlobals_t logGlobals;
	PigPerfGraph_t perfGraph;
	PigMemGraph_t memGraph;
	PigAudioOutGraph_t audioOutGraph;
	r64 physicsSimTimeLastFrame;
	bool cyclicFuncsDebug;
	CyclicFunc_t cyclicFunc;
	bool monitorsDebug;
	
	//Audio
	PlatMutex_t volumeMutex;
	bool musicEnabled;
	bool soundsEnabled;
	r32 masterVolumeRestoreAfterMute;
	r32 masterVolume;
	r32 musicVolume;
	r32 soundsVolume;
	PlatMutex_t soundInstancesMutex;
	u64 nextSoundInstanceId;
	SoundInstance_t soundInstances[PIG_MAX_SOUND_INSTANCES];
	PlatMutex_t audioOutSamplesMutex;
	u64 audioOutWriteIndex;
	r64* audioOutSamples;
	r64 audioMixerTime;
	u64 numAudioClips;
	
	MusicSystemState_t musicSystem;
	
	VarArray_t confirmDialogs; //ConfirmDialog_t
	
	#if STEAM_BUILD
	bool debugRenderSteamFriendsList;
	VarArray_t steamAvatars; //SteamAvatar_t
	#endif
	
	//Lua
	#if LUA_SUPPORTED
	MemArena_t luaHeap;
	LuaState_t lua;
	#endif
	
	//Python
	#if PYTHON_SUPPORTED
	MemArena_t pythonHeap;
	PythonState_t python;
	#endif
	
	#if PROCMON_SUPPORTED
	ThreadId_t procmonThreadId;
	MemArena_t* procmonTempArena;
	#endif
};

#endif //  _PIG_MAIN_H
