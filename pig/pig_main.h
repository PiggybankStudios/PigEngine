/*
File:   pig_main.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Holds the PigState_t structure which is the main place that we store all of our variables for the engine
*/

#ifndef _PIG_MAIN_H
#define _PIG_MAIN_H

struct PigWindowState_t
{
	u64 windowId;
	
	FrameBuffer_t frameBuffer;
	bool takeScreenshot;
	reci screenshotSubPartRec;
	bool screenshotKeyWasUsedForSelection;
	bool selectingScreenshotRec;
	v2i selectingScreenshotRecStart;
};

struct PigState_t
{
	bool initialized;
	bool dllReloaded;
	ThreadId_t mainThreadId;
	
	//Memory Arenas
	MemArena_t platHeap;
	MemArena_t mainHeap;
	MemArena_t stdHeap;
	MemArena_t tempArena;
	
	AppStateStructs_t appStates;
	
	AppState_t currentAppState;
	bool changeAppStateRequested;
	AppState_t newAppState;
	
	//Rendering
	RenderApi_t renderApi;
	RenderContext_t renderContext;
	MyPhysRenderer_c physRenderer;
	
	//IDs
	u64 nextShaderId;
	u64 nextVertBufferId;
	u64 nextTextureId;
	u64 nextFrameBufferId;
	u64 nextVectorImgId;
	u64 nextSpriteSheetId;
	u64 nextUiId;
	
	//Resources
	Resources_t resources;
	
	//Window Management
	const PlatWindow_t* currentWindow;
	PigWindowState_t* currentWindowState;
	LinkedList_t windowStates;
	
	//Input Handling
	MouseHitInfo_t prevMouseHit;
	MouseHitInfo_t mouseHit;
	u64 prevProgramTime;
	BtnHandlingInfo_t keyHandled[Key_NumKeys];
	BtnHandlingInfo_t mouseBtnHandled[MouseBtn_NumBtns];
	BtnHandlingInfo_t controllerBtnHandled[ControllerBtn_NumBtns];
	BtnHandlingInfo_t scrollXHandled;
	BtnHandlingInfo_t scrollYHandled;
	
	//Debug Menus
	DebugConsole_t debugConsole;
	PigNotificationQueue_t notificationsQueue;
	PigDebugOverlay_t debugOverlay;
	PigPerfGraph_t perfGraph;
	PigAudioOutGraph_t audioOutGraph;
	r64 physicsSimTimeLastFrame;
	
	//Audio
	PlatMutex_t soundInstancesMutex;
	u64 nextSoundInstanceId;
	SoundInstance_t soundInstances[PIG_MAX_SOUND_INSTANCES];
	PlatMutex_t audioOutSamplesMutex;
	u64 audioOutWriteIndex;
	r64 audioOutSamples[PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH];
	r64 audioMixerTime;
	u64 numAudioClips;
};

#endif //  _PIG_MAIN_H
