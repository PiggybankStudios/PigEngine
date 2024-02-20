/*
File:   common_includes.h
Author: Taylor Robbins
Date:   09\14\2021
*/

#ifndef _COMMON_INCLUDES_H
#define _COMMON_INCLUDES_H

// +--------------------------------------------------------------+
// |                     _LAYER Defines Check                     |
// +--------------------------------------------------------------+
#if !defined(PLATFORM_LAYER) && !defined(ENGINE_LAYER) && !defined(PLUGIN_LAYER)
#error Either PLATFORM_LAYER, ENGINE_LAYER, or PLUGIN_LAYER must be defined before including anything from Common
#endif
#ifndef WIN32_GFX_TEST //TODO: Once we move over to pig_gfx.h fully, we should remove these checks from here
	#if !defined(OPENGL_SUPPORTED)
	#error You must define OPENGL_SUPPORTED in the build options
	#endif
	#if !defined(WEBGL_SUPPORTED)
	#error You must define WEBGL_SUPPORTED in the build options
	#endif
	#if !defined(VULKAN_SUPPORTED)
	#error You must define VULKAN_SUPPORTED in the build options
	#endif
	#if !defined(DIRECTX_SUPPORTED)
	#error You must define DIRECTX_SUPPORTED in the build options
	#endif
#endif
#if !defined(SLUG_SUPPORTED)
#error You must define SLUG_SUPPORTED in the build options
#endif
#if !defined(JSON_SUPPORTED)
#error You must define JSON_SUPPORTED in the build options
#endif
#if !defined(BOX2D_SUPPORTED)
#error You must define BOX2D_SUPPORTED in the build options
#endif
#if !defined(PROCMON_SUPPORTED)
#error You must define PROCMON_SUPPORTED in the build options
#endif
#if !defined(SOCKETS_SUPPORTED)
#error You must define SOCKETS_SUPPORTED in the build options
#endif
#if !defined(ASSERTIONS_ENABLED)
#error You must define ASSERTIONS_ENABLED in the build options
#endif
#if !defined(DEVELOPER_BUILD)
#error You must define DEVELOPER_BUILD in the build options
#endif
#if !defined(DEBUG_BUILD)
#error You must define DEBUG_BUILD in the build options
#endif
#if !defined(DEMO_BUILD)
#error You must define DEMO_BUILD in the build options
#endif
#if !defined(STEAM_BUILD)
#error You must define STEAM_BUILD in the build options
#endif

#ifdef PLATFORM_LAYER
#undef PLATFORM_LAYER
#define PLATFORM_LAYER 1
#else
#define PLATFORM_LAYER 0
#endif

#ifdef ENGINE_LAYER
#undef ENGINE_LAYER
#define ENGINE_LAYER 1
#else
#define ENGINE_LAYER 0
#endif

#ifdef PLUGIN_LAYER
#undef PLUGIN_LAYER
#define PLUGIN_LAYER 1
#else
#define PLUGIN_LAYER 0
#endif

// +--------------------------------------------------------------+
// |                     Gylib First Include                      |
// +--------------------------------------------------------------+
#define GYLIB_ASSERTIONS_ENABLED ASSERTIONS_ENABLED
#ifdef PIG_COMMON_HEADER_ONLY
#define GYLIB_HEADER_ONLY
#endif
#ifdef WASM_COMPILATION
// In web builds, the platform and the engine are compiled separately and then linked. We are going to utilize static to make their TempArena and scratchArena globals be separate per-compilation unit
#define GYLIB_TEMP_MEMORY_STATIC
#define GYLIB_SCRATCH_ARENAS_STATIC
#endif
#if DEBUG_BUILD
#define GYLIB_DEBUG_NAMES_ENABLED
#endif
// #define GYLIB_MEM_ARENA_DEBUG_ENABLED //also see PIG_MAIN_ARENA_DEBUG in pig_defines.h
#include "gylib/gy_defines_check.h"
#include "gylib/gy_basic_macros.h"

// +--------------------------------------------------------------+
// |                   Platform Layer Includes                    |
// +--------------------------------------------------------------+
#if WINDOWS_COMPILATION
	#if SOCKETS_SUPPORTED
		#include <winsock2.h> //NOTE: Has to precede <windows.h> (I guess)
		#include <Ws2ipdef.h>
		// #include <Winhttp.h>
	#endif
	#include <windows.h>
	// #include <dsound.h>
	// #include <Dbt.h> //DBT_CONFIGCHANGED and other wParam values for WM_DEVICECHANGE
	#include <mmdeviceapi.h>
	#include <Functiondiscoverykeys_devpkey.h>
	#include <Audioclient.h>
	#include <Audiopolicy.h>
	#include <Shlobj.h>
	#if PROCMON_SUPPORTED
		#include <conio.h>
		#include <atltime.h>
		#include "sdk.hpp"
	#endif //PROCMON_SUPPORTED
	// #if !USE_CUSTOM_AUDIO
	// 	#include <openal/al.h>
	// 	#include <openal/alc.h>
	// #endif
#elif WASM_COMPILATION
	// TODO: Anything we want to #include here? There really isn't a whole lot of built-in "platform" header files
#elif OSX_COMPILATION
	// #include <pthread.h>
	// #include <semaphore.h>
	// #if !USE_CUSTOM_AUDIO
	// 	#ifdef HAVE_AL_AL_H
	// 	#include <AL/al.h>
	// 	#elif defined(__APPLE__)
	// 	#include <OpenAL/al.h>
	// 	#else
	// 	#include <al.h>
	// 	#endif
	// 	#ifdef HAVE_AL_ALC_H
	// 	#include <AL/alc.h>
	// 	#elif defined(__APPLE__)
	// 	#include <OpenAL/alc.h>
	// 	#else
	// 	#include <alc.h>
	// 	#endif
	// #endif
#elif LINUX_COMPILATION
	// #if !USE_CUSTOM_AUDIO
	// 	#include <AL/al.h>
	// 	#include <AL/alc.h>
	// #endif
#endif

// +--------------------------------------------------------------+
// |                       Library Includes                       |
// +--------------------------------------------------------------+
#ifdef WIN32_GFX_TEST
#include "graphics/pig_gfx_early_include.h"
#endif

#if WINDOWS_COMPILATION
	#ifndef WIN32_GFX_TEST
		#if OPENGL_SUPPORTED
		#include "glad/glad.h"
		#endif
	#endif
	#ifdef WIN32_GFX_TEST
		#if PIG_GFX_VULKAN_SUPPORTED
		#define GLFW_INCLUDE_VULKAN
		#endif
	#endif
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include "GLFW/glfw3.h"
#elif WASM_COMPILATION
	//TODO: Any platform files we want to include?
#elif OSX_COMPILATION
	#define GLFW_EXPOSE_NATIVE_NSGL
	#define GLFW_EXPOSE_NATIVE_COCOA
	#if OPENGL_SUPPORTED
	#include "GL/glew.h"
	#endif
	#include "GLFW/glfw3.h"
	#include "GLFW/glfw3native.h"
#elif LINUX_COMPILATION
	#if OPENGL_SUPPORTED
	#include "GL/glew.h"
	#endif
	#include "GLFW/glfw3.h"
#endif

#if STEAM_BUILD
#include "steam/steam_api.h"
#endif

#include "stb/stb_rect_pack.h"

#define GYLIB_LOOKUP_PRIMES_1000
#define GYLIB_DEFAULT_RANDOM_SERIES_TYPE RandomSeriesType_XoroShiro128
#include "gylib/gy.h"

#ifdef WIN32_GFX_TEST
#include "graphics/pig_gfx.h"
#endif

// +--------------------------------------------------------------+
// |                      Interface Includes                      |
// +--------------------------------------------------------------+
#include "common_defines.h"
#include "common_button_enums.h"
#include "common_types.h"

#if WINDOWS_COMPILATION
#include "win32/win32_shared_types.h"
#elif OSX_COMPILATION
#include "osx/osx_shared_types.h"
#elif LINUX_COMPILATION
#include "linux/linux_shared_types.h"
#elif WASM_COMPILATION
#include "web/web_shared_types.h"
#endif

#include "common_api_funcs.h"
#include "common_interface_types.h"
//NOTE: common_performance.h is included later in pig_main.cpp and win32_main.cpp

#if WINDOWS_COMPILATION
#include "win32/win32_glfw.h"
#include "win32/win32_dll_loading.h"
#include "win32/win32_file_watching.h"
#include "win32/win32_box2d.h"
#elif OSX_COMPILATION
// TODO: Include any other files that need to declare functions or types
#elif LINUX_COMPILATION
// TODO: Include any other files that need to declare functions or types
#elif WASM_COMPILATION
#include "web/web_gl_defines.h"
#include "web/web_gl_types.h"
#include "web/web_gl_api.h"
#endif

#endif //  _COMMON_INCLUDES_H
