/*
File:   pig_types.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Holds a bunch of typdefs, structs, and defines for use throughout the engine
*/

#ifndef _PIG_TYPES_H
#define _PIG_TYPES_H

enum PigEntryPoint_t
{
	PigEntryPoint_None = 0,
	PigEntryPoint_GetVersion,
	PigEntryPoint_GetStartupOptions,
	PigEntryPoint_Initialize,
	PigEntryPoint_Update,
	PigEntryPoint_ShouldWindowClose,
	PigEntryPoint_Closing,
	PigEntryPoint_PreReload,
	PigEntryPoint_PostReload,
	PigEntryPoint_PerformTask,
};

#define PERF_GRAPH_LENGTH     120 //frames
#define PERF_GRAPH_DATA_WIDTH 5 //px
#define PERF_GRAPH_HEIGHT     100 //px
#define PERF_GRAPH_MIN_SCALE  (PERF_GRAPH_HEIGHT / 1500) //px/ms
#define PERF_GRAPH_MAX_SCALE  (PERF_GRAPH_HEIGHT / 33) //px/ms

struct PigPerfGraph_t
{
	bool enabled;
	bool lockedScale;
	bool paused;
	bool hasAutoPaused;
	
	r64 msValues[PERF_GRAPH_LENGTH];
	r64 updateMsValues[PERF_GRAPH_LENGTH];
	r64 waitMsValues[PERF_GRAPH_LENGTH];
	u64 readoutProgramTimes[PERF_GRAPH_LENGTH];
	r64 maxValue;
	
	bool nextFrameMarked;
	bool isFrameMarked[PERF_GRAPH_LENGTH];
	
	r32 scale; //px/ms
	rec mainRec;
	rec lockScaleBtnRec;
	rec pauseBtnRec;
	v2 infoTextPos;
	rec audioOutRec;
};

struct PigMemGraphArenaPage_t
{
	u64 id;
	u64 size;
	u64 used;
	u64 numAllocations;
	r32 usedPercent;
	
	r32 usedPercentDisplay;
	u64 lastUsedChangeTime;
	i64 lastUsedChangeAmount;
	u64 lastAllocationsChangeTime;
	i64 lastAllocationsChangeAmount;
	r32 appearAnimTime;
	
	rec mainRec;
};
struct PigMemGraphArena_t
{
	u64 id;
	MemArena_t* pntr;
	MyStr_t name;
	Color_t fillColor;
	
	VarArray_t pages; //PigMemGraphArenaPage_t
	
	rec mainRec;
};

struct PigMemGraph_t
{
	bool enabled;
	u64 nextArenaId;
	u64 nextPageId;
	u64 selectedPageId;
	
	VarArray_t arenas; //PigMemGraphArena_t
	
	rec mainRec;
};

struct PigAudioOutGraph_t
{
	bool enabled;
	bool paused;
	
	bool mouseGrabbed;
	r64 mouseGrabPos;
	
	r64 viewCenter; //0-1
	r64 viewCenterGoto;
	r64 scale;
	r64 scaleGoto;
	r64 minScale;
	
	rec mainRec;
	rec pauseBtnRec;
	rec resetViewBtnRec;
};

struct PigDebugOverlay_t
{
	bool enabled;
	bool debugReadoutsEnabled;
	bool audioInstancesEnabled;
	bool pieChartsEnabled;
	bool easingFuncsEnabled;
	bool controllerDebugEnabled;
	bool perfGraphWasEnabled;
	bool memGraphWasEnabled;
	bool audioOutGraphWasEnabled;
	
	bool hotkeyPlusNumberPressed;
	u64 lastMouseCloseTime;
	r32 openAnimTime;
	
	r64 physicsSimTimeLastFrame;
	
	rec mainRec;
	rec initPieChartRec;
	rec audioGraphRec;
	
	rec totalToggleBtnsRec;
	union
	{
		rec toggleBtnRecs[8];
		struct
		{
			rec toggleDebugReadoutBtnRec;
			rec togglePerfGraphBtnRec;
			rec toggleAudioGraphBtnRec;
			rec toggleAudioInstancesBtnRec;
			rec toggleMemGraphBtnRec;
			rec togglePieChartsBtnRec;
			rec toggleEasingFuncsBtnRec;
			rec toggleContollerDebugBtnRec;
		};
	};
};

enum XmlParsingError_t
{
	XmlParsingError_None = 0,
	XmlParsingError_UnexpectedEol,
	XmlParsingError_ExpectedClosingAngleBracket,
	XmlParsingError_UnexpectedClosingToken,
	XmlParsingError_ClosingTokenMismatch,
	XmlParsingError_InvalidCharacterOutsideToken,
	XmlParsingError_InvalidUtf8,
	XmlParsingError_NoTypeFoundForToken,
	XmlParsingError_InvalidCharInIdentifier,
	XmlParsingError_NoEqualsForProperty,
	XmlParsingError_InvalidPropertyFirstChar,
	XmlParsingError_MissingPropertyValue,
	XmlParsingError_MissingClosingTokens,
	XmlParsingError_NumErrors,
};
const char* GetXmlParsingErrorStr(XmlParsingError_t error)
{
	switch (error)
	{
		case XmlParsingError_None:                         return "None";
		case XmlParsingError_UnexpectedEol:                return "UnexpectedEol";
		case XmlParsingError_ExpectedClosingAngleBracket:  return "ExpectedClosingAngleBracket";
		case XmlParsingError_UnexpectedClosingToken:       return "UnexpectedClosingToken";
		case XmlParsingError_ClosingTokenMismatch:         return "ClosingTokenMismatch";
		case XmlParsingError_InvalidCharacterOutsideToken: return "InvalidCharacterOutsideToken";
		case XmlParsingError_InvalidUtf8:                  return "InvalidUtf8";
		case XmlParsingError_NoTypeFoundForToken:          return "NoTypeFoundForToken";
		case XmlParsingError_InvalidCharInIdentifier:      return "InvalidCharInIdentifier";
		case XmlParsingError_NoEqualsForProperty:          return "NoEqualsForProperty";
		case XmlParsingError_InvalidPropertyFirstChar:     return "InvalidPropertyFirstChar";
		case XmlParsingError_MissingPropertyValue:         return "MissingPropertyValue";
		case XmlParsingError_MissingClosingTokens:         return "MissingClosingTokens";
		default: return "Unknown";
	}
}

struct ProcessLog_t
{
	bool hadErrors;
	bool hadWarnings;
	u32 errorCode;
	TryParseFailureReason_t parseFailureReason;
	XmlParsingError_t xmlParsingError;
	
	MemArena_t* allocArena;
	MyStr_t processName;
	MyStr_t filePath;
	StringFifo_t fifo;
};

struct Sound_t
{
	MemArena_t* allocArena;
	u64 id;
	PlatAudioFormat_t format;
	u64 numFrames;
	u64 dataSize;
	union
	{
		void* data;
		i8*   dataI8;
		i16*  dataI16;
		i32*  dataI32;
		r32*  dataF32;
		r64*  dataF64;
	};
};

enum SoundInstanceType_t
{
	SoundInstanceType_None = 0,
	SoundInstanceType_SineWave,
	SoundInstanceType_SquareWave,
	SoundInstanceType_SawWave,
	SoundInstanceType_Samples,
	SoundInstanceType_StreamingSamples,
	SoundInstanceType_NumTypes,
};
const char* GetSoundInstanceTypeStr(SoundInstanceType_t instanceType)
{
	switch (instanceType)
	{
		case SoundInstanceType_None:             return "None";
		case SoundInstanceType_SineWave:         return "SineWave";
		case SoundInstanceType_SquareWave:       return "SquareWave";
		case SoundInstanceType_SawWave:          return "SawWave";
		case SoundInstanceType_Samples:          return "Samples";
		case SoundInstanceType_StreamingSamples: return "StreamingSamples";
		default: return "Unknown";
	}
}
bool IsSoundInstanceTypeGenerated(SoundInstanceType_t instanceType)
{
	switch (instanceType)
	{
		case SoundInstanceType_SineWave: return true;
		case SoundInstanceType_SquareWave: return true;
		case SoundInstanceType_SawWave: return true;
		default: return false;
	}
}

struct SoundInstance_t
{
	u64 id;
	SoundInstanceType_t type;
	SoundInstance_t* nextInstanceToStart;
	
	bool isMusic;
	bool repeating;
	r32 volume;
	u64 numFrames; //frames
	PlatAudioFormat_t format;
	u64 falloffTime; //ms
	EasingStyle_t falloffCurve;
	u64 attackTime; //ms
	EasingStyle_t attackCurve;
	
	//Generated
	r64 frequency; //Hz
	
	//Samples
	const Sound_t* sound;
	
	//StreamingSamples
	PlatOpenFile_t openFile;
	
	bool playing;
	u64 playOnAlignmentSize;
	u64 numLoops;
	u64 frameIndex; //frames
};

struct SoundInstanceHandle_t
{
	u64 id;
	SoundInstance_t* instancePntr;
};

enum MusicFade_t
{
	MusicFade_None = 0,
	MusicFade_Instant,
	MusicFade_Step,
	MusicFade_LinearCrossfade,
	MusicFade_FadeOutThenIn,
	MusicFade_FadeInThenOut,
	MusicFade_PartialCrossFade,
	MusicFade_NumTypes,
};
const char* GetMusicFadeStr(MusicFade_t musicFade)
{
	switch (musicFade)
	{
		case MusicFade_None:             return "None";
		case MusicFade_Instant:          return "Instant";
		case MusicFade_Step:             return "Step";
		case MusicFade_LinearCrossfade:  return "LinearCrossfade";
		case MusicFade_FadeOutThenIn:    return "FadeOutThenIn";
		case MusicFade_FadeInThenOut:    return "FadeInThenOut";
		case MusicFade_PartialCrossFade: return "PartialCrossFade";
		default: return "Unknown";
	}
}

struct MusicSystemState_t
{
	SoundInstanceHandle_t currentMusic;
	SoundInstanceHandle_t previousMusic;
	MusicFade_t sndQueuedFade;
	r32 sndQueuedFadeDuration; //ms
	SoundInstanceHandle_t sndQueuedMusic;
	MusicFade_t currentFade;
	r32 currentFadeDuration; //ms
	r32 currentFadeProgress; //percent
};

struct PigNotification_t
{
	MemArena_t* allocArena;
	u64 id;
	bool alive;
	bool showing;
	
	MyStr_t message;
	MyStr_t filePath;
	u64 fileLineNumber;
	MyStr_t funcName;
	u64 repeatCount;
	u64 appearTime;
	DbgLevel_t dbgLevel;
	
	r32 showAnimTime;
	u64 lifespan;
	
	rec mainRec;
	v2 textSize;
	v2 textOffset;
	rec repeatCountRec;
	v2 repeatCountTextSize;
	v2 repeatCountTextOffset;
};

struct PigNotificationQueue_t
{
	bool initialized;
	u64 nextId;
	PigNotification_t notifications[PIG_MAX_NUM_NOTIFICATIONS];
	
	rec mainRec;
	v2 notificationsSize;
};

#endif //  _PIG_TYPES_H
