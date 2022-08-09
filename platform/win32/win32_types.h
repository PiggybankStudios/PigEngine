/*
File:   win32_types.h
Author: Taylor Robbins
Date:   09\24\2021
Description:
	** Holds a bunch of typedefs, structs, and enums that the Win32 platform layer
	** uses but that the engine doesn't need to know about
*/

#ifndef _WIN_32_TYPES_H
#define _WIN_32_TYPES_H

enum Win32InitPhase_t
{
	Win32InitPhase_Entry = 0,
	Win32InitPhase_CoreInitialized,
	Win32InitPhase_DebugOutputInitialized,
	Win32InitPhase_ProgramArgsParsed,
	Win32InitPhase_ThreadingInitialized,
	Win32InitPhase_AudioInitialized,
	#if PROCMON_SUPPORTED
	Win32InitPhase_ProcmonDriverLoaded,
	#endif
	Win32InitPhase_EngineDllLoaded,
	Win32InitPhase_GlfwInitialized,
	Win32InitPhase_StartupOptionsObtained,
	Win32InitPhase_ThreadPoolsCreated,
	Win32InitPhase_WindowOpened,
	Win32InitPhase_AudioOutputStarted,
	Win32InitPhase_ResourcesLoaded,
	Win32InitPhase_SteamInitialized,
	Win32InitPhase_EngineInitialization,
	Win32InitPhase_Initialized,
	Win32InitPhase_DoingFirstUpdate,
	Win32InitPhase_PostFirstUpdate,
	Win32InitPhase_Closing,
	Win32InitPhase_NumPhases,
	
	Win32InitPhase_Running = Win32InitPhase_DoingFirstUpdate,
};
const char* GetWin32InitPhaseStr(Win32InitPhase_t initPhase)
{
	switch (initPhase)
	{
		case Win32InitPhase_Entry:                  return "Entry";
		case Win32InitPhase_CoreInitialized:        return "CoreInitialized";
		case Win32InitPhase_DebugOutputInitialized: return "DebugOutputInitialized";
		case Win32InitPhase_ProgramArgsParsed:      return "ProgramArgsParsed";
		case Win32InitPhase_ThreadingInitialized:   return "ThreadingInitialized";
		case Win32InitPhase_AudioInitialized:       return "AudioInitialized";
		#if PROCMON_SUPPORTED
		case Win32InitPhase_ProcmonDriverLoaded:    return "ProcmonDriverLoaded";
		#endif
		case Win32InitPhase_EngineDllLoaded:        return "EngineDllLoaded";
		case Win32InitPhase_GlfwInitialized:        return "GlfwInitialized";
		case Win32InitPhase_StartupOptionsObtained: return "StartupOptionsObtained";
		case Win32InitPhase_ThreadPoolsCreated:     return "ThreadPoolsCreated";
		case Win32InitPhase_WindowOpened:           return "WindowOpened";
		case Win32InitPhase_AudioOutputStarted:     return "AudioOutputStarted";
		case Win32InitPhase_ResourcesLoaded:        return "ResourcesLoaded";
		case Win32InitPhase_SteamInitialized:       return "SteamInitialized";
		case Win32InitPhase_EngineInitialization:   return "EngineInitialization";
		case Win32InitPhase_Initialized:            return "Initialized";
		case Win32InitPhase_DoingFirstUpdate:       return "DoingFirstUpdate";
		case Win32InitPhase_PostFirstUpdate:        return "PostFirstUpdate";
		case Win32InitPhase_Closing:                return "Closing";
		default: return "Unknown";
	}
}

#define WIN32_MAX_NUM_VERTEX_ATTRIBUTES 3
struct Win32_VertexAttribLocations_t
{
	u32 placeholder; //so we don't get an empty struct if all render APIs are disabled
	#if OPENGL_SUPPORTED
	union
	{
		GLint values[WIN32_MAX_NUM_VERTEX_ATTRIBUTES];
		struct
		{
			GLint position;
			GLint color;
			GLint texCoord;
		};
	} gl;
	#endif
};
struct Win32_VertexArrayObject_t
{
	bool boundOnce;
	u64 windowId;
	#if OPENGL_SUPPORTED
	GLuint glId;
	Win32_VertexAttribLocations_t attribLocations;
	#endif
};

struct Win32_Texture_t
{
	bool isValid;
	#if OPENGL_SUPPORTED
	GLuint glId;
	#endif
	
	bool singleChannel;
	
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
};

struct Win32_Shader_t
{
	bool isValid;
	#if OPENGL_SUPPORTED
	GLuint glId;
	GLuint vertId;
	GLuint fragId;
	GLuint vertexArrayId;
	#endif
	
	MyStr_t fragmentLog;
	MyStr_t vertexLog;
	MyStr_t linkLog;
	
	Win32_VertexAttribLocations_t attribLocations;
	
	#if OPENGL_SUPPORTED
	struct
	{
		//Uniforms
		GLint worldMatrix;
		GLint viewMatrix;
		GLint projectionMatrix;
		GLint texture;
		GLint textureSize;
		GLint singleChannelTexture;
		GLint sourceRectangle;
		GLint primaryColor;
		GLint secondaryColor;
		GLint value[2];
	} locations;
	#endif
};

struct Win32_Vertex_t
{
	union
	{
		v3 position;
		struct { r32 x, y, z; };
	};
	union
	{
		v4 color;
		struct { r32 r, g, b, a; };
	};
	union
	{
		v2 texCoord;
		struct { r32 u, v; };
	};
};

struct Win32_VertBuffer_t
{
	bool isValid;
	#if OPENGL_SUPPORTED
	GLuint glId;
	#endif
	
	bool isDynamic;
	u64 numVertices;
};

struct Win32_FontChar_t
{
	u32 codepoint;
	union
	{
		reci sourceRec;
		struct { v2i sourcePos, sourceSize; };
		struct { v2i unused, renderSize; };
	};
	union
	{
		rec logicalRec;
		struct { v2 logicalOffset, logicalSize; };
	};
	v2 renderOffset;
	r32 advanceX;
};
struct Win32_Font_t
{
	bool isValid;
	Win32_Texture_t bake;
	
	u64 numRanges;
	PlatFontRange_t* ranges;
	
	u64 numChars;
	Win32_FontChar_t* chars;
	
	r32 lineAdvance;
	r32 maxHeight;
	r32 maxHeightRender;
	r32 maxLineHeight;
	r32 maxLineHeightRender;
	r32 maxLeftOverang;
	r32 maxLeftOverangRender;
};

struct Win32_DebugReadoutLine_t
{
	MemArena_t* allocArena;
	MyStr_t str;
	Color_t color;
	r32 scale;
	u64 windowId;
};

// +--------------------------------------------------------------+
// |                Callback Interface Declaration                |
// +--------------------------------------------------------------+
// +==============================+
// |     AudioCallbackClass_c     |
// +==============================+
class AudioCallbackClass_c : public IMMNotificationClient
{
	//IUnknown
	ULONG AddRef();
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG Release();
	
	//IMMNotificationClient
	HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);
	HRESULT OnDeviceAdded(LPCWSTR pwstrDeviceId);
	HRESULT OnDeviceRemoved(LPCWSTR pwstrDeviceId);
	HRESULT OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
	HRESULT OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
};

#endif //  _WIN_32_TYPES_H
