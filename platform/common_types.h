/*
File:   common_types.h
Author: Taylor Robbins
Date:   09\23\2021
Description:
	** Contains common typedefs and structs used by both the engine and
	** the platform layer that don't need any platform specific members
*/

#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

enum PlatType_t
{
	PlatType_Windows,
	PlatType_OSX,
	PlatType_Linux,
};
const char* GetPlatTypeStr(PlatType_t platType)
{
	switch (platType)
	{
		case PlatType_Windows: return "Windows";
		case PlatType_OSX:     return "OSX";
		case PlatType_Linux:   return "Linux";
		default: return "Unknown";
	}
}
const char* GetKeyStrOnPlatform(PlatType_t platType, Key_t key)
{
	switch (key)
	{
		case Key_Control: return ((platType == PlatType_OSX) ? "Command" : GetKeyStr(key));
		case Key_Alt:     return ((platType == PlatType_OSX) ? "Options" : GetKeyStr(key));
		default: return GetKeyStr(key);
	}
}
const char* GetModifierKeyStrOnPlatform(PlatType_t platType, ModifierKey_t modifierKey)
{
	switch (modifierKey)
	{
		case ModifierKey_Alt:       return ((platType == PlatType_OSX) ? "Options"      : GetModifierKeyStr(modifierKey));
		case ModifierKey_Ctrl:      return ((platType == PlatType_OSX) ? "Cmd"          : GetModifierKeyStr(modifierKey));
		case ModifierKey_CtrlShift: return ((platType == PlatType_OSX) ? "CmdShift"     : GetModifierKeyStr(modifierKey));
		case ModifierKey_CtrlAlt:   return ((platType == PlatType_OSX) ? "CmdOptions"   : GetModifierKeyStr(modifierKey));
		case ModifierKey_AltShift:  return ((platType == PlatType_OSX) ? "OptionsShift" : GetModifierKeyStr(modifierKey));
		default: return GetModifierKeyStr(modifierKey);
	}
}

enum RenderApi_t
{
	RenderApi_None = 0,
	RenderApi_OpenGL,
	RenderApi_DirectX,
	RenderApi_Vulkan,
	RenderApi_NumModes,
};
const char* GetRenderApiStr(RenderApi_t renderApi)
{
	switch (renderApi)
	{
		case RenderApi_None:    return "None";
		case RenderApi_OpenGL:  return "OpenGL";
		case RenderApi_DirectX: return "DirectX";
		case RenderApi_Vulkan:  return "Vulkan";
		default: return "Unknown";
	}
}

struct Version_t
{
	u32 major;
	u32 minor;
	u32 build;
};

PACKED(struct VersionPacked_t
{
	u8 major;
	u8 minor;
	u16 build;
});

struct GlfwVersion_t
{
	i32 major;
	i32 minor;
	i32 revision;
};

enum DbgLevel_t
{
	DbgLevel_None = 0,
	DbgLevel_Debug,
	DbgLevel_Regular,
	DbgLevel_Info,
	DbgLevel_Notify,
	DbgLevel_Other,
	DbgLevel_Warning,
	DbgLevel_Error,
	DbgLevel_NumLevels,
};
const char* GetDbgLevelStr(DbgLevel_t dbgLevel)
{
	switch (dbgLevel)
	{
		case DbgLevel_None:    return "None";
		case DbgLevel_Debug:   return "Debug";
		case DbgLevel_Regular: return "Regular";
		case DbgLevel_Info:    return "Info";
		case DbgLevel_Notify:  return "Notify";
		case DbgLevel_Other:   return "Other";
		case DbgLevel_Warning: return "Warning";
		case DbgLevel_Error:   return "Error";
		default: return "Unknown";
	}
}

DbgLevel_t GetDbgLevelForGyDbgLevel(GyDbgLevel_t gyDbgLevel)
{
	switch (gyDbgLevel)
	{
		case GyDbgLevel_Debug:   return DbgLevel_Debug;
		case GyDbgLevel_Info:    return DbgLevel_Info;
		case GyDbgLevel_Warning: return DbgLevel_Warning;
		case GyDbgLevel_Error:   return DbgLevel_Error;
		default: return DbgLevel_None;
	}
}

Color_t GetDbgLevelTextColor(DbgLevel_t dbgLevel)
{
	switch (dbgLevel)
	{
		case DbgLevel_Debug:   return MonokaiGray1;     break;
		case DbgLevel_Regular: return MonokaiWhite;     break;
		case DbgLevel_Info:    return MonokaiGreen;     break;
		case DbgLevel_Notify:  return MonokaiPurple;    break;
		case DbgLevel_Other:   return MonokaiLightBlue; break;
		case DbgLevel_Warning: return MonokaiOrange;    break;
		case DbgLevel_Error:   return MonokaiMagenta;   break;
		default: return MonokaiWhite; break;
	}
}

struct ProgramArguments_t
{
	u64 count;
	MyStr_t* args;
};

#define MAX_MONITOR_FRAMERATES_PER_RESOLUTION 16
struct PlatMonitorVideoMode_t
{
	u64 id;
	u64 index;
	bool isCurrent;
	u64 currentFramerateIndex;
	v2i resolution;
	u64 numFramerates;
	i64 framerates[MAX_MONITOR_FRAMERATES_PER_RESOLUTION];
};
struct PlatMonitorInfo_t
{
	MemArena_t* allocArena;
	u64 id;
	
	bool isPrimary;
	u64 designatedNumber;
	MyStr_t name;
	
	reci desktopSpaceRec;
	reci workAreaRec;
	v2i physicalSize;
	v2 contentScale;
	
	u64 currentVideoModeIndex;
	VarArray_t videoModes; //PlatMonitorVideoMode_t
	VarArray_t framerates; //i64
	
	GLFWmonitor* glfwHandle;
};
struct PlatMonitorList_t
{
	u64 primaryIndex;
	reci desktopRec;
	LinkedList_t list; //PlatMonitorInfo_t 
};

struct PlatWindowCreateOptions_t
{
	bool resizableWindow;
	bool topmostWindow;
	bool decoratedWindow;
	u64 antialiasingNumSamples;
	i32 requestRefreshRate;
	bool autoIconify;
	v2i requestSize;
	PlatMonitorInfo_t* requestMonitor;
	MyStr_t windowTitle;
};
struct PlatWindowOptions_t
{
	PlatWindowCreateOptions_t create;
	bool enforceMinSize;
	v2i minWindowSize;
	bool enforceMaxSize;
	v2i maxWindowSize;
	bool forceAspectRatio;
	v2i aspectRatio;
};

struct PlatImageData_t
{
	u64 id;
	MemArena_t* allocArena;
	u8 pixelSize;
	u64 rowSize;
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
	u64 dataSize;
	union
	{
		u8*  data8;
		u32* data32;
	};
};

struct PlatFontRange_t
{
	float fontSize; //font_size
	u32 firstCodepoint; //first_unicode_codepoint_in_range
	u32* codePointList; //array_of_unicode_codepoints
	u32 numChars; //num_chars
};

struct PlatFontCharData_t
{
	u32 codepoint;
	reci sourceRec;
	v2 offset;
	v2 offset2;
	r32 advanceX;
};
struct PlatFontData_t
{
	u64 id;
	PlatImageData_t imageData;
	u64 numRanges;
	PlatFontRange_t* ranges;
	u64 numChars;
	PlatFontCharData_t* charData;
};

enum InputEventType_t
{
	InputEventType_None = 0,
	InputEventType_Character,
	InputEventType_Key,
	InputEventType_ControllerBtn,
	InputEventType_MouseBtn,
	InputEventType_MouseHover,
	InputEventType_WindowFocus,
	InputEventType_FileDropped,
	InputEventType_NumTypes,
};
const char* GetInputEventTypeStr(InputEventType_t eventType)
{
	switch (eventType)
	{
		case InputEventType_None:          return "None";
		case InputEventType_Character:     return "Character";
		case InputEventType_Key:           return "Key";
		case InputEventType_ControllerBtn: return "ControllerBtn";
		case InputEventType_MouseBtn:      return "MouseBtn";
		case InputEventType_MouseHover:    return "MouseHover";
		case InputEventType_WindowFocus:   return "WindowFocus";
		case InputEventType_FileDropped:   return "FileDropped";
		default: return "Unknown";
	}
}

struct InputEvent_t
{
	u64 id;
	u64 index;
	InputEventType_t type;
	bool handled; //can be set by the engine
	
	//common data
	const struct PlatWindow_t* window; //can be nullptr
	bool hadFocus;
	bool mouseInsideWindow;
	v2 mousePos;
	ModifierKey_t modifiers;
	i64 pairedEventIndex;
	
	union
	{
		struct
		{
			u32 codepoint;
		} character;
		struct
		{
			Key_t key;
			bool pressed;
			bool released;
			bool repeated;
		} key;
		struct
		{
			ControllerBtn_t btn;
			bool pressed;
			bool released;
			bool repeated;
		} controllerBtn;
		struct
		{
			MouseBtn_t btn;
			bool pressed;
			bool released;
			bool repeated;
		} mouseBtn;
		struct
		{
			bool focused;
			const struct PlatWindow_t* oldWindow; //can be nullptr
			const struct PlatWindow_t* newWindow; //can be nullptr
		} windowFocus;
		struct
		{
			bool entered;
			const struct PlatWindow_t* oldWindow; //can be nullptr
			const struct PlatWindow_t* newWindow; //can be nullptr
		} mouseHover;
		struct
		{
			MyStr_t filePath;
		} droppedFile;
	};
};

struct PlatBtnState_t
{
	u64 lastChangeTime;
	bool isDown;
	bool wasDown;
	u8 numPresses;
	u8 numReleases;
	u8 numTransitions;
	u8 numRepeats;
};

enum ControllerType_t
{
	ControllerType_Unknown = 0,
	ControllerType_Joystick,
	ControllerType_Playstation,
	ControllerType_Xbox,
	ControllerType_Nintendo,
	ControllerType_NumTypes,
};
const char* GetControllerTypeStr(ControllerType_t controllerType)
{
	switch (controllerType)
	{
		case ControllerType_Unknown:     return "Unknown";
		case ControllerType_Joystick:    return "Joystick";
		case ControllerType_Playstation: return "Playstation";
		case ControllerType_Xbox:        return "Xbox";
		case ControllerType_Nintendo:    return "Nintendo";
		default: return "Unknown";
	}
}

struct PlatControllerState_t
{
	u64 index;
	MyStr_t name;
	MyStr_t typeIdStr;
	ControllerType_t type;
	u64 numAxes;
	u64 numButtons;
	
	bool connected;
	bool connectedChanged;
	
	PlatBtnState_t btnStates[ControllerBtn_NumBtns];
	v2i dpad;
	union
	{
		v2 sticks[2];
		struct { v2 leftStick, rightStick; };
	};
	union
	{
		v2 sticksRaw[2];
		struct { v2 leftStickRaw, rightStickRaw; };
	};
	union
	{
		r32 triggers[2];
		struct { r32 leftTrigger, rightTrigger; };
	};
};

struct WindowEngineInput_t
{
	bool windowInteractionOccurred; //anything that could cause us to block for a long time and extend ElapsedMs undesirably
	
	bool minimizedChanged;
	bool minimized;
	
	bool resized;
	v2i pixelResolution;       //the actual number of pixels we think we have to work with
	v2i prevPixelResolution;
	v2i windowResolution;      //the size reported to us by GLFW for the window (in screen space coordinates)
	v2i prevWindowResolution;
	v2i contextResolution;     //the size that OpenGL or whatever wants us to talk in when doing stuff like glViewport
	v2  renderResolution;      //the size used for all of our render logic. The "effective" resolution
	
	bool moved;
	v2i position;
	
	bool isFocusedChanged;
	bool isFocused;
	
	bool mouseInsideWindowChanged;
	bool mouseInsideWindow;
	bool mouseMoved;
	v2i mousePosi;
	v2 mousePos;
};

enum PlatCursor_t
{
	PlatCursor_Default = 0,
	PlatCursor_TextIBeam,
	PlatCursor_Pointer,
	PlatCursor_ResizeHorizontal,
	PlatCursor_ResizeVertical,
	PlatCursor_NumCursors,
};
const char* GetPlatCursorStr(PlatCursor_t platCursor)
{
	switch (platCursor)
	{
		case PlatCursor_Default:          return "Default";
		case PlatCursor_TextIBeam:        return "TextIBeam";
		case PlatCursor_Pointer:          return "Pointer";
		case PlatCursor_ResizeHorizontal: return "ResizeHorizontal";
		case PlatCursor_ResizeVertical:   return "ResizeVertical";
		default: return "Unknown";
	}
}

struct PlatAudioFormat_t
{
	u64 bitsPerSample;
	u64 numChannels;
	u64 samplesPerSecond;
};

enum PlatImageFormat_t
{
	PlatImageFormat_Png = 0,
	PlatImageFormat_Bmp,
	PlatImageFormat_Tga,
	PlatImageFormat_NumFormats,
};
const char* GetPlatImageFormatStr(PlatImageFormat_t imageFormat)
{
	switch (imageFormat)
	{
		case PlatImageFormat_Png: return "Png";
		case PlatImageFormat_Bmp: return "Bmp";
		case PlatImageFormat_Tga: return "Tga";
		default: return "Unknown";
	}
}

#endif //  _COMMON_TYPES_H
