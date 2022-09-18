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

#define EXIT_CODE_ASSERTION_FAILED 2

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
	
	#if WINDOWS_COMPILATION
	GLFWmonitor* glfwHandle;
	#endif
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
	v2i requestWindowedLocation;
	bool requestMaximizedWindow;
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
	bool floatChannels;
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
	bool maximizedChanged;
	bool maximized;
	
	bool resized;
	v2i pixelResolution;       //the actual number of pixels we think we have to work with
	v2i prevPixelResolution;
	v2i windowResolution;      //the size reported to us by GLFW for the window (in screen space coordinates)
	v2i prevWindowResolution;
	v2i contextResolution;     //the size that OpenGL or whatever wants us to talk in when doing stuff like glViewport
	v2  renderResolution;      //the size used for all of our render logic. The "effective" resolution
	
	bool moved;
	reci desktopRec; //This can be used in conjunction with PlatMonitorList_t desktopRec to get an idea of where our window is across all monitors
	reci desktopInnerRec; //Similar to desktopRec but doesn't include the title bar or borders. i.e. it's only the render portion of the window
	
	v2i prevUnmaximizedWindowPos; //NOTE: window position callback comes as (0, 0) BEFORE we get the maximized callback. And also comes before we get the unmaximized callback. So we need to revert the value sometimes
	v2i unmaximizedWindowPos;  //the last value of desktopInnerRec.topLeft when we weren't maximized, minimized, or full screen
	v2i unmaximizedWindowSize; //the last value of windowResolution when we weren't maximized, minimized, or full screen
	
	bool isFocusedChanged;
	bool isFocused;
	
	bool mouseInsideWindowChanged;
	bool mouseInsideWindow;
	bool mouseMoved;
	v2i mousePosi;
	v2 mousePos;
	v2 mouseDelta; //useful for PlatMouseMode_FirstPersonCamera
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

enum PlatMouseMode_t
{
	PlatMouseMode_Default = 0,
	PlatMouseMode_Invisible,
	PlatMouseMode_FirstPersonCamera, //mouse is invisible, centered every frame, and position info is delta only
	PlatMouseMode_NumModes,
};
const char* GetPlatMouseModeStr(PlatMouseMode_t mouseMode)
{
	switch (mouseMode)
	{
		case PlatMouseMode_Default:           return "Default";
		case PlatMouseMode_Invisible:         return "Invisible";
		case PlatMouseMode_FirstPersonCamera: return "FirstPersonCamera";
		default: return "Unknown";
	}
}

struct PlatAudioFormat_t
{
	u64 bitsPerSample;
	u64 numChannels;
	u64 samplesPerSecond;
};
inline bool operator == (PlatAudioFormat_t left, PlatAudioFormat_t right)  { return (left.bitsPerSample == right.bitsPerSample && left.numChannels == right.numChannels && left.samplesPerSecond == right.samplesPerSecond); }

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

#if PROCMON_SUPPORTED

// #define IRP_MJ_QUERY_EA                 0x07
// #define IRP_MJ_SET_EA                   0x08
// #define IRP_MJ_FLUSH_BUFFERS            0x09
// #define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
// #define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
// #define IRP_MJ_DIRECTORY_CONTROL        0x0c
// #define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
// #define IRP_MJ_DEVICE_CONTROL           0x0e
// #define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
// #define IRP_MJ_SHUTDOWN                 0x10
// #define IRP_MJ_LOCK_CONTROL             0x11
// #define IRP_MJ_CLEANUP                  0x12
// #define IRP_MJ_CREATE_MAILSLOT          0x13
// #define IRP_MJ_QUERY_SECURITY           0x14
// #define IRP_MJ_SET_SECURITY             0x15
// #define IRP_MJ_POWER                    0x16
// #define IRP_MJ_SYSTEM_CONTROL           0x17
// #define IRP_MJ_DEVICE_CHANGE            0x18
// #define IRP_MJ_QUERY_QUOTA              0x19
// #define IRP_MJ_SET_QUOTA                0x1a
// #define IRP_MJ_PNP                      0x1b

// IRP_MAP_DEF(IRP_MJ_QUERY_EA, TEXT("QueryEAFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_SET_EA, TEXT("SetEAFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_FLUSH_BUFFERS, TEXT("FlushBuffersFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_QUERY_VOLUME_INFORMATION, TEXT("QueryVolumeInformation"), gFileSubMapQueryVolumeInfo),
// IRP_MAP_DEF(IRP_MJ_SET_VOLUME_INFORMATION, TEXT("SetVolumeInformation"), NULL),
// IRP_MAP_DEF(IRP_MJ_DIRECTORY_CONTROL, TEXT("DirectoryControl"), gFileSubMapDirControl),
// IRP_MAP_DEF(IRP_MJ_FILE_SYSTEM_CONTROL, TEXT("FileSystemControl"), NULL),
// IRP_MAP_DEF(IRP_MJ_DEVICE_CONTROL, TEXT("DeviceIoControl"), NULL),
// IRP_MAP_DEF(IRP_MJ_INTERNAL_DEVICE_CONTROL, TEXT("InternalDeviceIoControl"), NULL),
// IRP_MAP_DEF(IRP_MJ_SHUTDOWN, TEXT("Shutdown"), NULL),
// IRP_MAP_DEF(IRP_MJ_LOCK_CONTROL, TEXT("LockUnlockFile"), gFileSubMapLockControl),
// IRP_MAP_DEF(IRP_MJ_CLEANUP, TEXT("CloseFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_CREATE_MAILSLOT, TEXT("CreateMailSlot"), NULL),
// IRP_MAP_DEF(IRP_MJ_QUERY_SECURITY, TEXT("QuerySecurityFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_SET_SECURITY, TEXT("SetSecurityFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_POWER, TEXT("Power"), NULL),
// IRP_MAP_DEF(IRP_MJ_SYSTEM_CONTROL, TEXT("SystemControl"), NULL),
// IRP_MAP_DEF(IRP_MJ_DEVICE_CHANGE, TEXT("DeviceChange"), NULL),
// IRP_MAP_DEF(IRP_MJ_QUERY_QUOTA, TEXT("QueryFileQuota"), NULL),
// IRP_MAP_DEF(IRP_MJ_SET_QUOTA, TEXT("SetFileQuota"), NULL),
// IRP_MAP_DEF(IRP_MJ_PNP, TEXT("PlugAndPlay"), gFileSubMapPnp),

enum ProcmonEventType_t
{
	ProcmonEventType_CreateFile              = 0x00,
	ProcmonEventType_CreatePipe              = 0x01,
	ProcmonEventType_CloseOther              = 0x02,
	ProcmonEventType_ReadFile                = 0x03,
	ProcmonEventType_WriteFile               = 0x04,
	ProcmonEventType_QueryFileInfo           = 0x05,
	ProcmonEventType_SetFileInfo             = 0x06,
	ProcmonEventType_QueryEAFile             = 0x07,
	ProcmonEventType_SetEAFile               = 0x08,
	ProcmonEventType_FlushBuffersFile        = 0x09,
	ProcmonEventType_QueryVolumeInformation  = 0x0A,
	ProcmonEventType_SetVolumeInformation    = 0x0B,
	ProcmonEventType_DirectoryControl        = 0x0C,
	ProcmonEventType_FileSystemControl       = 0x0D,
	ProcmonEventType_FileDeviceIoControl     = 0x0E,
	ProcmonEventType_InternalDeviceIoControl = 0x0F,
	ProcmonEventType_Shutdown                = 0x10,
	ProcmonEventType_LockUnlockFile          = 0x11,
	ProcmonEventType_CloseFile               = 0x12,
	ProcmonEventType_CreateMailSlot          = 0x13,
	ProcmonEventType_QueryFileSecurity       = 0x14,
	ProcmonEventType_SetFileSecurity         = 0x15,
	ProcmonEventType_Power                   = 0x16,
	ProcmonEventType_SystemControl           = 0x17,
	ProcmonEventType_DeviceChange            = 0x18,
	ProcmonEventType_QueryFileQuota          = 0x19,
	ProcmonEventType_SetFileQuota            = 0x1A,
	ProcmonEventType_PlugAndPlay             = 0x1B,
	ProcmonEventType_NumTypes,
};
enum ProcmonEventBit_t
{
	ProcmonEventBit_None = 0x00,
	ProcmonEventBit_CreateFile    = 0x01,
	ProcmonEventBit_ReadFile      = 0x02,
	ProcmonEventBit_WriteFile     = 0x04,
	ProcmonEventBit_QueryFileInfo = 0x08,
	ProcmonEventBit_SetFileInfo   = 0x10,
	ProcmonEventBit_CloseFile     = 0x20,
	ProcmonEventBit_Mask = 0x3F,
};

const char* GetProcmonEventTypeStr(ProcmonEventType_t eventType)
{
	switch (eventType)
	{
		case ProcmonEventType_CreateFile:              return "CreateFile";
		case ProcmonEventType_CreatePipe:              return "CreatePipe";
		case ProcmonEventType_CloseOther:              return "CloseOther";
		case ProcmonEventType_ReadFile:                return "ReadFile";
		case ProcmonEventType_WriteFile:               return "WriteFile";
		case ProcmonEventType_QueryFileInfo:           return "QueryFileInfo";
		case ProcmonEventType_SetFileInfo:             return "SetFileInfo";
		case ProcmonEventType_QueryEAFile:             return "QueryEAFile";
		case ProcmonEventType_SetEAFile:               return "SetEAFile";
		case ProcmonEventType_FlushBuffersFile:        return "FlushBuffersFile";
		case ProcmonEventType_QueryVolumeInformation:  return "QueryVolumeInformation";
		case ProcmonEventType_SetVolumeInformation:    return "SetVolumeInformation";
		case ProcmonEventType_DirectoryControl:        return "DirectoryControl";
		case ProcmonEventType_FileSystemControl:       return "FileSystemControl";
		case ProcmonEventType_FileDeviceIoControl:     return "FileDeviceIoControl";
		case ProcmonEventType_InternalDeviceIoControl: return "InternalDeviceIoControl";
		case ProcmonEventType_Shutdown:                return "Shutdown";
		case ProcmonEventType_LockUnlockFile:          return "LockUnlockFile";
		case ProcmonEventType_CloseFile:               return "CloseFile";
		case ProcmonEventType_CreateMailSlot:          return "CreateMailSlot";
		case ProcmonEventType_QueryFileSecurity:       return "QueryFileSecurity";
		case ProcmonEventType_SetFileSecurity:         return "SetFileSecurity";
		case ProcmonEventType_Power:                   return "Power";
		case ProcmonEventType_SystemControl:           return "SystemControl";
		case ProcmonEventType_DeviceChange:            return "DeviceChange";
		case ProcmonEventType_QueryFileQuota:          return "QueryFileQuota";
		case ProcmonEventType_SetFileQuota:            return "SetFileQuota";
		case ProcmonEventType_PlugAndPlay:             return "PlugAndPlay";
		default: return "Unknown";
	}
}

ProcmonEventBit_t GetProcmonEventBit(ProcmonEventType_t eventType)
{
	switch (eventType)
	{
		case ProcmonEventType_CreateFile:              return ProcmonEventBit_CreateFile;
		// case ProcmonEventType_CreatePipe:              return ProcmonEventBit_CreatePipe;
		// case ProcmonEventType_CloseOther:              return ProcmonEventBit_CloseOther;
		case ProcmonEventType_ReadFile:                return ProcmonEventBit_ReadFile;
		case ProcmonEventType_WriteFile:               return ProcmonEventBit_WriteFile;
		case ProcmonEventType_QueryFileInfo:           return ProcmonEventBit_QueryFileInfo;
		case ProcmonEventType_SetFileInfo:             return ProcmonEventBit_SetFileInfo;
		// case ProcmonEventType_QueryEAFile:             return ProcmonEventBit_QueryEAFile;
		// case ProcmonEventType_SetEAFile:               return ProcmonEventBit_SetEAFile;
		// case ProcmonEventType_FlushBuffersFile:        return ProcmonEventBit_FlushBuffersFile;
		// case ProcmonEventType_QueryVolumeInformation:  return ProcmonEventBit_QueryVolumeInformation;
		// case ProcmonEventType_SetVolumeInformation:    return ProcmonEventBit_SetVolumeInformation;
		// case ProcmonEventType_DirectoryControl:        return ProcmonEventBit_DirectoryControl;
		// case ProcmonEventType_FileSystemControl:       return ProcmonEventBit_FileSystemControl;
		// case ProcmonEventType_FileDeviceIoControl:     return ProcmonEventBit_FileDeviceIoControl;
		// case ProcmonEventType_InternalDeviceIoControl: return ProcmonEventBit_InternalDeviceIoControl;
		// case ProcmonEventType_Shutdown:                return ProcmonEventBit_Shutdown;
		// case ProcmonEventType_LockUnlockFile:          return ProcmonEventBit_LockUnlockFile;
		case ProcmonEventType_CloseFile:               return ProcmonEventBit_CloseFile;
		// case ProcmonEventType_CreateMailSlot:          return ProcmonEventBit_CreateMailSlot;
		// case ProcmonEventType_QueryFileSecurity:       return ProcmonEventBit_QueryFileSecurity;
		// case ProcmonEventType_SetFileSecurity:         return ProcmonEventBit_SetFileSecurity;
		// case ProcmonEventType_Power:                   return ProcmonEventBit_Power;
		// case ProcmonEventType_SystemControl:           return ProcmonEventBit_SystemControl;
		// case ProcmonEventType_DeviceChange:            return ProcmonEventBit_DeviceChange;
		// case ProcmonEventType_QueryFileQuota:          return ProcmonEventBit_QueryFileQuota;
		// case ProcmonEventType_SetFileQuota:            return ProcmonEventBit_SetFileQuota;
		// case ProcmonEventType_PlugAndPlay:             return ProcmonEventBit_PlugAndPlay;
		// case ProcmonEventType_NumTypes:                return ProcmonEventBit_NumTypes;
		default: return ProcmonEventBit_None;
	}
}
ProcmonEventType_t GetProcmonEventType(ProcmonEventBit_t eventBit)
{
	switch (eventBit)
	{
		case ProcmonEventBit_CreateFile:    return ProcmonEventType_CreateFile;
		case ProcmonEventBit_ReadFile:      return ProcmonEventType_ReadFile;
		case ProcmonEventBit_WriteFile:     return ProcmonEventType_WriteFile;
		case ProcmonEventBit_QueryFileInfo: return ProcmonEventType_QueryFileInfo;
		case ProcmonEventBit_SetFileInfo:   return ProcmonEventType_SetFileInfo;
		case ProcmonEventBit_CloseFile:     return ProcmonEventType_CloseFile;
		default: return ProcmonEventType_NumTypes;
	}
}

struct ProcmonEntry_t
{
	u64 id;
	MyStr_t processName;
	u64 numEvents;
	// u64 eventIndex;
	// ProcmonEventType_t lastFewEvents[16];
	u32 eventBits;
};

struct ProcmonFile_t
{
	u64 id;
	MyStr_t filePath;
	u64 processId;
	u64 numTouches;
};

#endif

#endif //  _COMMON_TYPES_H
