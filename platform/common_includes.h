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
#if !defined(OPENGL_SUPPORTED)
#error You must define OPENGL_SUPPORTED in the build options
#endif
#if !defined(VULKAN_SUPPORTED)
#error You must define VULKAN_SUPPORTED in the build options
#endif
#if !defined(DIRECTX_SUPPORTED)
#error You must define DIRECTX_SUPPORTED in the build options
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
#define GY_STD_LIB_ALLOWED
#include "gylib/gy_defines_check.h"

// +--------------------------------------------------------------+
// |                   Platform Layer Includes                    |
// +--------------------------------------------------------------+
#if WINDOWS_COMPILATION
	#include <windows.h>
	// #include <Winhttp.h>
	// #include <dsound.h>
	// #include <Dbt.h> //DBT_CONFIGCHANGED and other wParam values for WM_DEVICECHANGE
	#include <mmdeviceapi.h>
	#include <Functiondiscoverykeys_devpkey.h>
	#include <Audioclient.h>
	#include <Audiopolicy.h>
	// #if !USE_CUSTOM_AUDIO
	// 	#include <openal/al.h>
	// 	#include <openal/alc.h>
	// #endif
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
#if WINDOWS_COMPILATION
	#define GLFW_EXPOSE_NATIVE_WIN32
	#if OPENGL_SUPPORTED
	#include "glad/glad.h"
	#endif
	#include "GLFW/glfw3.h"
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

#include "stb/stb_rect_pack.h"

#include "gylib/gy_std.h"
#include "gylib/gy_assert.h"
#include "gylib/gy_types.h"
#define GYLIB_LOOKUP_PRIMES_1000
#include "gylib/gy_lookup.h"
#include "gylib/gy_intrinsics.h"
#include "gylib/gy_debug.h"
#include "gylib/gy_random.h"
#include "gylib/gy_easing.h"
#include "gylib/gy_memory.h"
#include "gylib/gy_unicode.h"
#include "gylib/gy_time.h"
#include "gylib/gy_audio.h"
#include "gylib/gy_string.h"
#include "gylib/gy_vectors.h"
#include "gylib/gy_directions.h"
#include "gylib/gy_matrices.h"
#include "gylib/gy_quaternions.h"
#include "gylib/gy_rectangles.h"
#include "gylib/gy_rays.h"
#include "gylib/gy_primitives.h"
#include "gylib/gy_colors.h"
#include "gylib/gy_standard_colors.h"
#include "gylib/gy_parsing.h"
#include "gylib/gy_math.h"
#include "gylib/gy_triangulation.h"
#include "gylib/gy_sorting.h"
#include "gylib/gy_linked_list.h"
#include "gylib/gy_variable_array.h"
#include "gylib/gy_bezier.h"
#include "gylib/gy_bucket_array.h"
#include "gylib/gy_string_fifo.h"
#include "gylib/gy_collision.h"

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
#endif

#include "common_api_funcs.h"
#include "common_interface_types.h"
//NOTE: common_performance.h is included later in pig_main.cpp and win32_main.cpp

#if WINDOWS_COMPILATION
#include "win32/win32_dll_loading.h"
#include "win32/win32_file_watching.h"
#elif OSX_COMPILATION
// #include "osx/osx_dll_loading.h" //TODO: Implement me!
#elif LINUX_COMPILATION
// #include "linux/linux_dll_loading.h" //TODO: Implement me!
#endif

#endif //  _COMMON_INCLUDES_H
