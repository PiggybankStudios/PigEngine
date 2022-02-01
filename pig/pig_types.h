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
	bool pieChartsEnabled;
	bool easingFuncsEnabled;
	bool controllerDebugEnabled;
	bool perfGraphWasEnabled;
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
		rec toggleBtnRecs[6];
		struct
		{
			rec toggleDebugReadoutBtnRec;
			rec togglePerfGraphBtnRec;
			rec toggleAudioGraphBtnRec;
			rec togglePieChartsBtnRec;
			rec toggleEasingFuncsBtnRec;
			rec toggleContollerDebugBtnRec;
		};
	};
};

// +==============================+
// |       MyPhysRenderer_c       |
// +==============================+
class MyPhysRenderer_c : q3Render
{
	private:
	Color_t drawColor;
	v3 drawPos;
	v3 drawScale;
	v3 drawNormal;
	
	public:
	void SetPenColor(f32 r, f32 g, f32 b, f32 a = 1.0f) override;
	void SetPenPosition(f32 x, f32 y, f32 z) override;
	void SetScale(f32 sx, f32 sy, f32 sz) override;
	void SetTriNormal(f32 x, f32 y, f32 z) override;
	
	void Point() override;
	void Line(f32 x, f32 y, f32 z) override;
	void Triangle(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 x3, f32 y3, f32 z3) override;
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
	
	bool repeating;
	r32 volume;
	u64 numFrames; //frames
	u64 falloffTime; //ms
	EasingStyle_t falloffCurve;
	u64 attackTime; //ms
	EasingStyle_t attackCurve;
	
	r64 frequency; //Hz
	
	PlatAudioFormat_t format;
	//TODO: Once we add support for keeping a file open, let's have an OpenFile_t struct in here
	
	bool playing;
	u64 playOnAlignmentSize;
	u64 numLoops;
	u64 frameIndex; //frames
	const void* samples;
};

struct SoundInstanceHandle_t
{
	u64 id;
	SoundInstance_t* instancePntr;
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
