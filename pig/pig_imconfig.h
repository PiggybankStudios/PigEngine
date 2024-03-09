/*
File:   pig_imconfig.h
Author: Taylor Robbins
Date:   03\01\2024
*/

#ifndef _PIG_IMCONFIG_H
#define _PIG_IMCONFIG_H

#define IMGUI_DEFINE_MATH_OPERATORS

//---- Define assertion handler. Defaults to calling assert().
#define IM_ASSERT(expression) Assert(expression)

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows
// Using Dear ImGui via a shared library is not recommended, because of function call overhead and because we don't guarantee backward nor forward ABI compatibility.
// DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
// for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for more details.
//#define IMGUI_API __declspec( dllexport )
//#define IMGUI_API __declspec( dllimport )

//---- Don't define obsolete functions/enums/behaviors. Consider enabling from time to time after updating to clean your code of obsolete function/names.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

//---- Disable all of Dear ImGui or don't implement standard windows/tools.
// It is very strongly recommended to NOT disable the demo windows and debug tool during development. They are extremely useful in day to day work. Please read comments in imgui_demo.cpp.
//#define IMGUI_DISABLE                                     // Disable everything: all headers and source files will be empty.
#if !DEBUG_BUILD
#define IMGUI_DISABLE_DEMO_WINDOWS                        // Disable demo windows: ShowDemoWindow()/ShowStyleEditor() will be empty.
#endif
//#define IMGUI_DISABLE_DEBUG_TOOLS                         // Disable metrics/debugger and other debug tools: ShowMetricsWindow(), ShowDebugLogWindow() and ShowIDStackToolWindow() will be empty.

//---- Don't implement some functions to reduce linkage requirements.
// #define IMGUI_DISABLE_WIN32_FUNCTIONS                     // [Win32] Won't use and link with any Win32 function (clipboard, IME).
//#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS   // [Win32] Don't implement default clipboard handler. Won't use and link with OpenClipboard/GetClipboardData/CloseClipboard etc. (user32.lib/.a, kernel32.lib/.a)
//#define IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS          // [Win32] [Default with Visual Studio] Implement default IME handler (require imm32.lib/.a, auto-link for Visual Studio, -limm32 on command-line for MinGW)
//#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS         // [Win32] [Default with non-Visual Studio compilers] Don't implement default IME handler (won't require imm32.lib/.a)
//#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS      // [OSX] Implement default OSX clipboard handler (need to link with '-framework ApplicationServices', this is why this is not the default).
// #define IMGUI_DISABLE_FILE_FUNCTIONS                      // Don't implement ImFileOpen/ImFileClose/ImFileRead/ImFileWrite and ImFileHandle at all (replace them with dummies)
//#define IMGUI_DISABLE_SSE                                 // Disable use of SSE intrinsics even if available

//---- Pack colors to BGRA8 instead of RGBA8 (to avoid converting from one to another)
// #define IMGUI_USE_BGRA_PACKED_COLOR

//---- Use 32-bit for ImWchar (default is 16-bit) to support Unicode planes 1-16. (e.g. point beyond 0xFFFF like emoticons, dingbats, symbols, shapes, ancient languages, etc...)
//#define IMGUI_USE_WCHAR32

//---- Avoid multiple STB libraries implementations, or redefine path/filenames to prioritize another version
// By default the embedded implementations are declared static and not available outside of Dear ImGui sources files.
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_STB_SPRINTF_FILENAME    "my_folder/stb_sprintf.h"    // only used if IMGUI_USE_STB_SPRINTF is defined.
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_SPRINTF_IMPLEMENTATION                   // only disabled if IMGUI_USE_STB_SPRINTF is defined.

//---- Use stb_sprintf.h for a faster implementation of vsnprintf instead of the one from libc (unless IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS is defined)
// Compatibility checks of arguments and formats done by clang and GCC will be disabled in order to support the extra formats provided by stb_sprintf.h.
//#define IMGUI_USE_STB_SPRINTF

//---- Use FreeType to build and rasterize the font atlas (instead of stb_truetype which is embedded by default in Dear ImGui)
// Requires FreeType headers to be available in the include path. Requires program to be compiled with 'misc/freetype/imgui_freetype.cpp' (in this repository) + the FreeType library (not provided).
// On Windows you may use vcpkg with 'vcpkg install freetype --triplet=x64-windows' + 'vcpkg integrate install'.
//#define IMGUI_ENABLE_FREETYPE

//---- Use FreeType+lunasvg library to render OpenType SVG fonts (SVGinOT)
// Requires lunasvg headers to be available in the include path + program to be linked with the lunasvg library (not provided).
// Only works in combination with IMGUI_ENABLE_FREETYPE.
// (implementation is based on Freetype's rsvg-port.c which is licensed under CeCILL-C Free Software License Agreement)
//#define IMGUI_ENABLE_FREETYPE_LUNASVG

//---- Use stb_truetype to build and rasterize the font atlas (default)
// The only purpose of this define is if you want force compilation of the stb_truetype backend ALONG with the FreeType backend.
//#define IMGUI_ENABLE_STB_TRUETYPE

//---- Define constructor and implicit cast operators to convert back<>forth between your math types and ImVec2/ImVec4.
// This will be inlined as part of ImVec2 and ImVec4 class declarations.

//TODO: Can we do this somehow?
// #define IM_VEC2_CLASS_EXTRA                                              \
//         constexpr ImVec2(const v2& vector) : x(vector.x), y(vector.y) {} \
//         operator Vec2_t() const { return NewVec2(x, y); }

// #define IM_VEC4_CLASS_EXTRA                                                     \
//         constexpr ImVec4(const MyVec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
//         operator MyVec4() const { return MyVec4(x,y,z,w); }

//---- ...Or use Dear ImGui's own very basic math operators.
//#define IMGUI_DEFINE_MATH_OPERATORS

//---- Use 32-bit vertex indices (default is 16-bit) is one way to allow large meshes with more than 64K vertices.
// Your renderer backend will need to support it (most example renderer backends support both 16/32-bit indices).
// Another way to allow large meshes while keeping 16-bit indices is to handle ImDrawCmd::VtxOffset in your renderer.
// Read about ImGuiBackendFlags_RendererHasVtxOffset for details.
//#define ImDrawIdx unsigned int

//---- Override ImDrawCallback signature (will need to modify renderer backends accordingly)
//struct ImDrawList;
//struct ImDrawCmd;
//typedef void (*MyImDrawCallback)(const ImDrawList* draw_list, const ImDrawCmd* cmd, void* my_renderer_user_data);
//#define ImDrawCallback MyImDrawCallback

//---- Debug Tools: Macro to break in Debugger (we provide a default implementation of this in the codebase)
// (use 'Metrics->Tools->Item Picker' to pick widgets with the mouse and break into them for easy debugging.)
#define IM_DEBUG_BREAK() MyBreak()

//---- Debug Tools: Enable slower asserts
//#define IMGUI_DEBUG_PARANOID

#define IMGUI_DISABLE_DEFAULT_ALLOCATORS                  // Don't implement default allocators calling malloc()/free() to avoid linking with them. You will need to call ImGui::SetAllocatorFunctions().
//NOTE: We call ImGui::SetAllocatorFunctions in PigInitImgui and PigImguiHandleReload

#define IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
int ImFormatString(char* bufferPntr, size_t bufferSize, const char* fmt, ...);
int ImFormatStringV(char* bufferPntr, size_t bufferSize, const char* fmt, va_list args);

#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
struct ImguiFile_t
{
	bool writing;
	bool convertNewLines;
	Stream_t stream;
	PlatOpenFile_t openFile;
};
typedef ImguiFile_t* ImFileHandle;
ImFileHandle ImFileOpen(const char* filename, const char* mode);
bool ImFileClose(ImFileHandle file);
u64 ImFileGetSize(ImFileHandle file);
u64 ImFileRead(void* data, u64 size, u64 count, ImFileHandle file);
u64 ImFileWrite(const void* data, u64 size, u64 count, ImFileHandle file);

// +--------------------------------------------------------------+
// |              Default Math Function Replacements              |
// +--------------------------------------------------------------+
#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define ImFabs(X)           AbsR32(X)
#define ImSqrt(x)           SqrtR32(x)
#define ImFmod(X, Y)        ModR32((X), (Y))
#define ImCos(X)            CosR32(X)
#define ImSin(X)            SinR32(X)
#define ImAcos(X)           AcosR32(X)
#define ImAtan2(Y, X)       AtanR32((Y), (X))
#define ImAtof(STR)         MyStrToFloat(STR)
#define ImCeil(X)           CeilR32(X)
static inline float  ImPow(float x, float y)    { return PowR32(x, y); }
static inline double ImPow(double x, double y)  { return PowR64(x, y); }
static inline float  ImLog(float x)             { return Log2R32(x); }
static inline double ImLog(double x)            { return Log2R64(x); }
static inline int    ImAbs(int x)               { return AbsI32(x); }
static inline float  ImAbs(float x)             { return AbsR32(x); }
static inline double ImAbs(double x)            { return AbsR64(x); }
static inline float  ImSign(float x)            { return SignOfR32(x); }
static inline double ImSign(double x)           { return SignOfR64(x); }
#ifdef IMGUI_ENABLE_SSE
static inline float  ImRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
static inline float  ImRsqrt(float x)           { return 1.0f / SqrtR32(x); }
#endif
static inline double ImRsqrt(double x)          { return 1.0 / SqrtR64(x); }


#endif //  _PIG_IMCONFIG_H
