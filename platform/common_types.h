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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetPlatTypeStr(PlatType_t platType);
const char* GetKeyStrOnPlatform(PlatType_t platType, Key_t key);
const char* GetModifierKeyStrOnPlatform(PlatType_t platType, ModifierKey_t modifierKey);
#else
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
#endif

#ifndef WIN32_GFX_TEST
enum RenderApi_t
{
	RenderApi_None = 0,
	RenderApi_OpenGL,
	RenderApi_DirectX,
	RenderApi_Vulkan,
	RenderApi_NumModes,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetRenderApiStr(RenderApi_t renderApi);
#else
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
#endif
#endif //WIN32_GFX_TEST

struct Version_t
{
	u32 major;
	u32 minor;
	u32 build;
};

Version_t NewVersion(u32 major, u32 minor, u32 build = 0)
{
	Version_t result;
	result.major = major;
	result.minor = minor;
	result.build = build;
	return result;
}

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
	bool autoIconify;
	MyStr_t windowTitle;
	
	bool fullscreen;
	const PlatMonitorInfo_t* fullscreenMonitor;
	const PlatMonitorVideoMode_t* fullscreenVideoMode;
	u64 fullscreenFramerateIndex;
	v2i windowedResolution;
	v2i windowedLocation;
	bool windowedMaximized;
	i64 windowedFramerate;
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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetControllerTypeStr(ControllerType_t controllerType);
#else
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
#endif

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
	
	bool isFocusedChanged;
	bool isFocused;
	
	bool fullscreen;
	bool fullscreenChanged;
	const PlatMonitorInfo_t* fullscreenMonitor;
	const PlatMonitorVideoMode_t* fullscreenVideoMode;
	u64 fullscreenFramerateIndex;
	i64 fullscreenFramerate;
	
	bool moved;
	reci desktopRec; //This can be used in conjunction with PlatMonitorList_t desktopRec to get an idea of where our window is across all monitors
	reci desktopInnerRec; //Similar to desktopRec but doesn't include the title bar or borders. i.e. it's only the render portion of the window
	i64 windowedFramerate;
	
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
	
	v2i prevUnmaximizedWindowPos; //NOTE: window position callback comes as (0, 0) BEFORE we get the maximized callback. And also comes before we get the unmaximized callback. So we need to revert the value sometimes
	v2i unmaximizedWindowPos;  //the last value of desktopInnerRec.topLeft when we weren't maximized, minimized, or full screen
	v2i unmaximizedWindowSize; //the last value of windowResolution when we weren't maximized, minimized, or full screen
	
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
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetPlatCursorStr(PlatCursor_t platCursor);
#else
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
#endif

enum PlatMouseMode_t
{
	PlatMouseMode_Default = 0,
	PlatMouseMode_Invisible,
	PlatMouseMode_FirstPersonCamera, //mouse is invisible, centered every frame, and position info is delta only
	PlatMouseMode_NumModes,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetPlatMouseModeStr(PlatMouseMode_t mouseMode);
#else
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
#endif

struct PlatAudioFormat_t
{
	u64 bitsPerSample;
	u64 numChannels;
	u64 samplesPerSecond;
};
#ifdef PIG_COMMON_HEADER_ONLY
bool operator == (PlatAudioFormat_t left, PlatAudioFormat_t right);
#else
bool operator == (PlatAudioFormat_t left, PlatAudioFormat_t right)  { return (left.bitsPerSample == right.bitsPerSample && left.numChannels == right.numChannels && left.samplesPerSecond == right.samplesPerSecond); }
#endif

enum PlatImageFormat_t
{
	PlatImageFormat_Png = 0,
	PlatImageFormat_Bmp,
	PlatImageFormat_Tga,
	PlatImageFormat_NumFormats,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetPlatImageFormatStr(PlatImageFormat_t imageFormat);
#else
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
#endif

enum SpecialFolder_t
{
	SpecialFolder_None = 0,
	SpecialFolder_Home,
	SpecialFolder_SavesAndSettings,
	SpecialFolder_Screenshots,
	SpecialFolder_Share,
	SpecialFolder_NumTypes,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetSpecialFolderStr(SpecialFolder_t specialFolder);
#else
const char* GetSpecialFolderStr(SpecialFolder_t specialFolder)
{
	switch (specialFolder)
	{
		case SpecialFolder_None:             return "None";
		case SpecialFolder_Home:             return "Home";
		case SpecialFolder_SavesAndSettings: return "SavesAndSettings";
		case SpecialFolder_Screenshots:      return "Screenshots";
		case SpecialFolder_Share:            return "Share";
		case SpecialFolder_NumTypes:         return "NumTypes";
		default: return "Unknown";
	}
}
#endif

enum OpenFileMode_t
{
	OpenFileMode_None = 0,
	OpenFileMode_Read,
	OpenFileMode_Write,
	OpenFileMode_Append,
	OpenFileMode_NumModes,
};
#ifdef PIG_COMMON_HEADER_ONLY
const char* GetOpenFileModeStr(OpenFileMode_t enumValue);
#else
const char* GetOpenFileModeStr(OpenFileMode_t enumValue)
{
	switch (enumValue)
	{
		case OpenFileMode_None:   return "None";
		case OpenFileMode_Read:   return "Read";
		case OpenFileMode_Write:  return "Write";
		case OpenFileMode_Append: return "Append";
		default: return "Unknown";
	}
}
#endif

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

// #define IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION   ((UCHAR)-1)
// #define IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION   ((UCHAR)-2)
// #define IRP_MJ_ACQUIRE_FOR_MOD_WRITE                 ((UCHAR)-3)
// #define IRP_MJ_RELEASE_FOR_MOD_WRITE                 ((UCHAR)-4)
// #define IRP_MJ_ACQUIRE_FOR_CC_FLUSH                  ((UCHAR)-5)
// #define IRP_MJ_RELEASE_FOR_CC_FLUSH                  ((UCHAR)-6)
// #define IRP_MJ_QUERY_OPEN                            ((UCHAR)-7)

// #define IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE             ((UCHAR)-13)
// #define IRP_MJ_NETWORK_QUERY_OPEN                    ((UCHAR)-14)
// #define IRP_MJ_MDL_READ                              ((UCHAR)-15)
// #define IRP_MJ_MDL_READ_COMPLETE                     ((UCHAR)-16)
// #define IRP_MJ_PREPARE_MDL_WRITE                     ((UCHAR)-17)
// #define IRP_MJ_MDL_WRITE_COMPLETE                    ((UCHAR)-18)
// #define IRP_MJ_VOLUME_MOUNT                          ((UCHAR)-19)
// #define IRP_MJ_VOLUME_DISMOUNT                       ((UCHAR)-20)

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
// IRP_MAP_DEF(IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION, TEXT("CreateFileMapping"), NULL),
// IRP_MAP_DEF(IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION, TEXT("IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION"), NULL),
// IRP_MAP_DEF(IRP_MJ_ACQUIRE_FOR_MOD_WRITE, TEXT("IRP_MJ_ACQUIRE_FOR_MOD_WRITE"), NULL),
// IRP_MAP_DEF(IRP_MJ_RELEASE_FOR_MOD_WRITE, TEXT("IRP_MJ_RELEASE_FOR_MOD_WRITE"), NULL),
// IRP_MAP_DEF(IRP_MJ_ACQUIRE_FOR_CC_FLUSH, TEXT("IRP_MJ_ACQUIRE_FOR_CC_FLUSH"), NULL),
// IRP_MAP_DEF(IRP_MJ_RELEASE_FOR_CC_FLUSH, TEXT("IRP_MJ_RELEASE_FOR_CC_FLUSH"), NULL),
// IRP_MAP_DEF(IRP_MJ_QUERY_OPEN, TEXT("IRP_MJ_QUERY_OPEN"), NULL),
// IRP_MAP_DEF(IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE, TEXT("FASTIO_CHECK_IF_POSSIBLE"), NULL),
// IRP_MAP_DEF(IRP_MJ_NETWORK_QUERY_OPEN, TEXT("QueryOpen"), NULL),
// IRP_MAP_DEF(IRP_MJ_MDL_READ, TEXT("ReadFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_MDL_READ_COMPLETE, TEXT("FASTIO_MDL_READ_COMPLETE"), NULL),
// IRP_MAP_DEF(IRP_MJ_PREPARE_MDL_WRITE, TEXT("WriteFile"), NULL),
// IRP_MAP_DEF(IRP_MJ_MDL_WRITE_COMPLETE, TEXT("FASTIO_MDL_WRITE_COMPLETE"), NULL),
// IRP_MAP_DEF(IRP_MJ_VOLUME_MOUNT, TEXT("VolumeMount"), NULL),
// IRP_MAP_DEF(IRP_MJ_VOLUME_DISMOUNT, TEXT("VolumeDismount"), NULL),

enum ProcmonEventType_t
{
	ProcmonEventType_CreateFile              = 0x00, //IRP_MJ_CREATE
	ProcmonEventType_CreatePipe              = 0x01, //IRP_MJ_CREATE_NAMED_PIPE
	ProcmonEventType_CloseOther              = 0x02, //IRP_MJ_CLOSE
	ProcmonEventType_ReadFile                = 0x03, //IRP_MJ_READ
	ProcmonEventType_WriteFile               = 0x04, //IRP_MJ_WRITE
	ProcmonEventType_QueryFileInfo           = 0x05, //IRP_MJ_QUERY_INFORMATION
	ProcmonEventType_SetFileInfo             = 0x06, //IRP_MJ_SET_INFORMATION
	ProcmonEventType_QueryEAFile             = 0x07, //IRP_MJ_QUERY_EA
	ProcmonEventType_SetEAFile               = 0x08, //IRP_MJ_SET_EA
	ProcmonEventType_FlushBuffersFile        = 0x09, //IRP_MJ_FLUSH_BUFFERS
	ProcmonEventType_QueryVolumeInformation  = 0x0A, //IRP_MJ_QUERY_VOLUME_INFORMATION
	ProcmonEventType_SetVolumeInformation    = 0x0B, //IRP_MJ_SET_VOLUME_INFORMATION
	ProcmonEventType_DirectoryControl        = 0x0C, //IRP_MJ_DIRECTORY_CONTROL
	ProcmonEventType_FileSystemControl       = 0x0D, //IRP_MJ_FILE_SYSTEM_CONTROL
	ProcmonEventType_FileDeviceIoControl     = 0x0E, //IRP_MJ_DEVICE_CONTROL
	ProcmonEventType_InternalDeviceIoControl = 0x0F, //IRP_MJ_INTERNAL_DEVICE_CONTROL
	ProcmonEventType_Shutdown                = 0x10, //IRP_MJ_SHUTDOWN
	ProcmonEventType_LockUnlockFile          = 0x11, //IRP_MJ_LOCK_CONTROL
	ProcmonEventType_CloseFile               = 0x12, //IRP_MJ_CLEANUP
	ProcmonEventType_CreateMailSlot          = 0x13, //IRP_MJ_CREATE_MAILSLOT
	ProcmonEventType_QueryFileSecurity       = 0x14, //IRP_MJ_QUERY_SECURITY
	ProcmonEventType_SetFileSecurity         = 0x15, //IRP_MJ_SET_SECURITY
	ProcmonEventType_Power                   = 0x16, //IRP_MJ_POWER
	ProcmonEventType_SystemControl           = 0x17, //IRP_MJ_SYSTEM_CONTROL
	ProcmonEventType_DeviceChange            = 0x18, //IRP_MJ_DEVICE_CHANGE
	ProcmonEventType_QueryFileQuota          = 0x19, //IRP_MJ_QUERY_QUOTA
	ProcmonEventType_SetFileQuota            = 0x1A, //IRP_MJ_SET_QUOTA
	ProcmonEventType_PlugAndPlay             = 0x1B, //IRP_MJ_PNP
	ProcmonEventType_NumTypes,
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

struct ProcmonEvent_t
{
	u64 id;
	
	ProcmonEventType_t type;
	MyStr_t path;
	MyStr_t detail;
	
	u64 processId;
	MyStr_t processName;
	MyStr_t processImagePath;
};

#endif

#if STEAM_BUILD

struct PlatSteamFriendPresenceStr_t
{
	bool needsToBeRemoved;
	MyStr_t key;
	MyStr_t value;
};

enum PlatSteamFriendOnlineStatus_t
{
	PlatSteamFriendOnlineStatus_Unknown = 0,
	PlatSteamFriendOnlineStatus_Offline,
	PlatSteamFriendOnlineStatus_Snooze,
	PlatSteamFriendOnlineStatus_Away,
	PlatSteamFriendOnlineStatus_Busy,
	PlatSteamFriendOnlineStatus_Online,
	PlatSteamFriendOnlineStatus_LookingForTrade,
	PlatSteamFriendOnlineStatus_LookingToPlay,
	PlatSteamFriendOnlineStatus_NumTypes,
};
const char* GetPlatSteamFriendOnlineStatusStr(PlatSteamFriendOnlineStatus_t onlineStatus)
{
	switch (onlineStatus)
	{
		case PlatSteamFriendOnlineStatus_Unknown:         return "Unknown";
		case PlatSteamFriendOnlineStatus_Offline:         return "Offline";
		case PlatSteamFriendOnlineStatus_Snooze:          return "Snooze";
		case PlatSteamFriendOnlineStatus_Away:            return "Away";
		case PlatSteamFriendOnlineStatus_Busy:            return "Busy";
		case PlatSteamFriendOnlineStatus_Online:          return "Online";
		case PlatSteamFriendOnlineStatus_LookingForTrade: return "LookingForTrade";
		case PlatSteamFriendOnlineStatus_LookingToPlay:   return "LookingToPlay";
		default: return "Unknown";
	}
}

enum PlatSteamFriendState_t
{
	PlatSteamFriendState_Unknown = 0,
	PlatSteamFriendState_Incoming,
	PlatSteamFriendState_Normal,
	PlatSteamFriendState_Outgoing,
	PlatSteamFriendState_NumStates,
};
const char* GetPlatSteamFriendStateStr(PlatSteamFriendState_t friendState)
{
	switch (friendState)
	{
		case PlatSteamFriendState_Unknown:  return "Unknown";
		case PlatSteamFriendState_Incoming: return "Incoming";
		case PlatSteamFriendState_Normal:   return "Normal";
		case PlatSteamFriendState_Outgoing: return "Outgoing";
		default: return "Unknown";
	}
}

enum PlatSteamFriendAvatarSize_t
{
	PlatSteamFriendAvatarSize_None = 0,
	PlatSteamFriendAvatarSize_Small, //?x?
	PlatSteamFriendAvatarSize_Medium, //?x?
	PlatSteamFriendAvatarSize_Large, //128x128
	PlatSteamFriendAvatarSize_NumSizes,
};
const char* GetPlatSteamFriendAvatarSizeStr(PlatSteamFriendAvatarSize_t avatarSize)
{
	switch (avatarSize)
	{
		case PlatSteamFriendAvatarSize_None:   return "None";
		case PlatSteamFriendAvatarSize_Small:  return "Small";
		case PlatSteamFriendAvatarSize_Medium: return "Medium";
		case PlatSteamFriendAvatarSize_Large:  return "Large";
		default: return "Unknown";
	}
}

struct PlatSteamFriendInfo_t
{
	u64 id;
	CSteamID steamId;
	bool needsToBeRemoved;
	
	PlatSteamFriendState_t state;
	PlatSteamFriendOnlineStatus_t onlineStatus;
	MyStr_t name; //UTF-8 Encoded!
	MyStr_t nickname; //UTF-8 Encoded!
	u64 numGroups;
	
	bool isRequestingAvatar;
	PlatSteamFriendAvatarSize_t avatarSize;
	bool freeAvatarImageData;
	PlatImageData_t avatarImageData;
	
	//Status Information
	u64 lastStatusUpdateAttemptTime;
	u64 lastStatusUpdateSuccessTime;
	
	bool isInGame;
	u64 inGameSteamAppId;
	CSteamID inGameLobbySteamId;
	u32 inGameIP;
	u16 inGamePort;
	u16 inGameQueryPort;
	
	VarArray_t presenceStrs; //PlatSteamFriendPresenceStr_t
};

struct PlatSteamFriendGroup_t
{
	u64 id;
	FriendsGroupID_t steamId;
	bool needsToBeRemoved;
	MyStr_t name; //UTF-8 Encoded!
	VarArray_t memberIds; //u64 (sorted by friend info)
};

struct PlatSteamFriendsList_t
{
	MemArena_t* allocArena;
	
	u64 lastQueryAttemptTime; //0 means we've never queried before
	u64 lastQuerySuccessTime; //0 means we've never successfully queried before
	Result_t lastQueryError;
	
	u64 nextFriendId;
	u64 nextGroupId;
	VarArray_t friends; //PlatSteamFriendInfo_t //Sorted by friend info
	VarArray_t groups; //PlatSteamFriendGroup_t
};

#endif //STEAM_BUILD

#if BOX2D_SUPPORTED

enum PlatPhysicsBodyShape_t
{
	PlatPhysicsBodyShape_None = 0,
	PlatPhysicsBodyShape_Circle,
	PlatPhysicsBodyShape_Rectangle,
	// PlatPhysicsBodyShape_Edge,
	// PlatPhysicsBodyShape_Chain,
	PlatPhysicsBodyShape_Polygon,
	PlatPhysicsBodyShape_NumShapes,
};
const char* GetPlatPhysicsBodyShapeStr(PlatPhysicsBodyShape_t enumValue)
{
	switch (enumValue)
	{
		case PlatPhysicsBodyShape_None:      return "None";
		case PlatPhysicsBodyShape_Circle:    return "Circle";
		case PlatPhysicsBodyShape_Rectangle: return "Rectangle";
		// case PlatPhysicsBodyShape_Edge:      return "Edge";
		// case PlatPhysicsBodyShape_Chain:     return "Chain";
		case PlatPhysicsBodyShape_Polygon:   return "Polygon";
		default: return "Unknown";
	}
}

enum PlatPhysicsBodyType_t
{
	PlatPhysicsBodyType_None = 0,
	PlatPhysicsBodyType_Static,
	PlatPhysicsBodyType_Kinematic,
	PlatPhysicsBodyType_Dynamic,
	PlatPhysicsBodyType_Sensor,
	PlatPhysicsBodyType_NumTypes,
};
const char* GetPlatPhysicsBodyTypeStr(PlatPhysicsBodyType_t enumValue)
{
	switch (enumValue)
	{
		case PlatPhysicsBodyType_None:      return "None";
		case PlatPhysicsBodyType_Static:    return "Static";
		case PlatPhysicsBodyType_Kinematic: return "Kinematic";
		case PlatPhysicsBodyType_Dynamic:   return "Dynamic";
		case PlatPhysicsBodyType_Sensor:    return "Sensor";
		default: return "Unknown";
	}
}

#define MAX_NUM_SHAPES_PER_PHYSICS_BODY 8

//TODO: Can we have multiple shapes in a single body?? We should support that?
struct PlatPhysicsBodyDef_t
{
	PlatPhysicsBodyType_t type;
	PlatPhysicsBodyShape_t shape;
	union
	{
		Circle_t circle;
		rec rectangle;
		struct
		{
			u64 numPolygonVerts;
			v2* polygonVerts;
		};
	};
	r32 friction;
	r32 restitution;
	r32 density;
	u16 colCategoryBits;
	u16 colMaskBits;
	i16 groupIndex;
	Color_t debugColor;
};

struct PlatPhysicsBody_t
{
	u64 id; //doubles as empty indicator, if id == 0 then it's empty
	PlatPhysicsBodyDef_t bodyDef;
	
	#if PLATFORM_LAYER
	class b2Body* bodyPntr;
	#else
	void* bodyPntr; //b2Body*
	#endif
};

struct PlatPhysicsBodyState_t
{
	v2 position;
	v2 velocity;
	r32 rotation;
	r32 angularVelocity;
};

#endif

#endif //  _COMMON_TYPES_H
