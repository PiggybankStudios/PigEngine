/*
File:   pig_main.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds all of the entry points for the engine DLL and includes all other files that need to be included to compile the engine DLL 
*/

#include "pig_config.h"

// +--------------------------------------------------------------+
// |                           Includes                           |
// +--------------------------------------------------------------+
#define ENGINE_LAYER
#if WASM_COMPILATION
#define PIG_COMMON_HEADER_ONLY
#endif
#define GYLIB_USE_ASSERT_FAILURE_FUNC
#define GYLIB_SCRATCH_ARENA_AVAILABLE
#define GYLIB_NUM_STANDARD_ROCK_NAMES 100
#include "common_includes.h"

#include "game_early_include.h"
#ifndef GAME_USES_FUNC_TABLE
#error You must define GAME_USES_FUNC_TABLE inside game_early_include.h
#endif
#if GAME_USES_FUNC_TABLE
#include "gylib/gy_func_table.h"
#include "gylib/gy_func_table.cpp"
#endif

#define MSFGIF_NO_STD_LIB
#include "msf_gif/msf_gif.h"

#include "pig_imconfig.h"
#include "imgui/imgui.h"
#include "gylib/gy_imgui_helpers.h"

//NOTE: Since PigWasmStdLib already uses stb_sprintf.h as the implementation of vsnprintf, we don't need to re-include the source for that here
#if !USING_PIG_WASM_STD_LIB
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"
#endif

#if SLUG_SUPPORTED
// #define TERATHON_NO_SYSTEM
// #define SLUG_WINDOWS
// #define SLUG_D3D11
// #define SLUG_DEBUG
#include "slug/SlugCode/SLSlug.h"
#include "slug/TerathonCode/TSCompression.h"
#endif

#if JSON_SUPPORTED
#include "yajl/yajl_tree.h"
#endif

#if LUA_SUPPORTED
#if DEBUG_BUILD
#define LUA_USE_APICHECK
#endif
extern "C" {
#include "lualib.h"
#include "lauxlib.h"
}
#endif

#if PYTHON_SUPPORTED
#include <Python.h>
#endif

#include "pig/pig_version.h"
#include "pig/pig_defines.h"
#include "pig/pig_serialization.h"
#include "pig/pig_input_chars.h"
#include "pig/pig_render.h"
#include "pig/pig_types.h"
#include "pig/pig_slug_types.h"
#include "pig/pig_svg.h"
#include "pig/pig_vector_img.h"
#include "pig/pig_font.h"
#include "pig/pig_slug.h"
#include "pig/pig_render_context.h"
#include "pig/pig_resources.h"
#include "pig/pig_resource_pool.h"
#include "pig/pig_particles.h"
#include "pig/pig_input.h"
#include "pig/pig_imgui.h"
#include "pig/pig_button.h"
#include "pig/pig_textbox.h"
#include "pig/pig_value_slider.h"
#include "pig/pig_checkbox.h"
#include "pig/pig_lua.h"
#include "pig/pig_python.h"
#include "pig/pig_debug_commands.h"
#include "pig/pig_debug_console.h"
#include "pig/pig_confirm_dialog.h"
#include "pig/pig_tasks.h"

#include "game_main.h"

#include "pig/pig_app_states.h"
#include "pig/pig_main.h"

// +--------------------------------------------------------------+
// |                         GLAD Source                          |
// +--------------------------------------------------------------+
#if OPENGL_SUPPORTED && !WASM_COMPILATION
#include "glad/glad.c"
#endif

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
static const StartupInfo_t*   startup       = nullptr;
static const PlatformInfo_t*  platInfo      = nullptr;
static const PlatformApi_t*   plat          = nullptr;
static       EngineInput_t*   pigIn         = nullptr;
static       EngineOutput_t*  pigOut        = nullptr;
static       PigEntryPoint_t  pigEntryPoint = PigEntryPoint_None;
static       PigState_t*      pig           = nullptr;
static       MemArena_t*      fixedHeap     = nullptr;
static       MemArena_t*      mainHeap      = nullptr;
static       RenderContext_t* rc            = nullptr;

//Variables that for one reason or another should have a slightly longer/looser lifespan than the global pointer they come from
static       u64              ProgramTime    = 0;
static       u64              UnixTimestamp  = 0;
static       u64              LocalTimestamp = 0;
static       r64              TimeScale      = 1.0;
static       r64              ElapsedMs      = 0.0;
static       r64              UncappedElapsedMs = 0.0;
static       v2               MousePos       = {};
static       v2               ScreenSize     = {};

#include "gylib/gy_temp_memory.cpp"

//NOTE: We don't actually use the normal interface! We reroute things to the platform layer's scratch arenas. See pig_dll_management.cpp for actual function implementations
// #include "gylib/gy_scratch_arenas.cpp"


// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "pig/pig_func_defs.h"
#include "pig/pig_debug.h"
#include "pig/pig_opengl.h"

#include "pig/pig_helpers.cpp"
#include "pig/pig_debug.cpp"
#include "pig/pig_log.cpp"
#include "pig/pig_notification_macros.cpp"
#include "pig/pig_time.cpp"

#include "pig/pig_json.cpp"
#include "pig/pig_wav.cpp"
#include "pig/pig_ogg.cpp"
#include "pig/pig_vertex_buffer.cpp"
#include "pig/pig_shader.cpp"
#include "pig/pig_texture.cpp"
#include "pig/pig_texture_array.cpp"
#include "pig/pig_frame_buffer.cpp"
#include "pig/pig_sprite_sheet.cpp"
#include "pig/pig_sprite_sheet_meta.cpp"
#include "pig/pig_svg.cpp"
#include "pig/pig_vector_img.cpp"
#include "pig/pig_font.cpp"
#include "pig/pig_slug.cpp"
#include "pig/pig_model_obj_serialization.cpp"
#include "pig/pig_model_armature.cpp"
#include "pig/pig_model_armature_serialization.cpp"
#include "pig/pig_model.cpp"
#include "pig/pig_sounds.cpp"
#include "pig/pig_music.cpp"
#include "pig/pig_resources.cpp"
#include "pig/pig_resource_pool.cpp"
#include "pig/pig_settings.cpp"
#include "pig/pig_debug_bindings.cpp"
#include "pig/pig_steam.cpp"

#include "pig/pig_input.cpp"
#include "pig/pig_window.cpp"
#include "pig/pig_imgui.cpp"

#include "pig/pig_render.cpp"
#include "pig/pig_slug_render.cpp"
#include "pig/pig_render_funcs_2d.cpp"
#include "pig/pig_font_flow.cpp"
#include "pig/pig_render_funcs_3d.cpp"
#include "pig/pig_render_funcs_font.cpp"
#include "pig/pig_render_funcs_imgui.cpp"
#include "pig/pig_particle_serialization.cpp"
#include "pig/pig_particles.cpp"
#include "pig/pig_particles_collection.cpp"
#include "pig/pig_button.cpp"
#include "pig/pig_textbox.cpp"
#include "pig/pig_value_slider.cpp"
#include "pig/pig_checkbox.cpp"
#include "pig/pig_render_funcs_extra.cpp"
#include "pig/pig_lua.cpp"
#include "pig/pig_python.cpp"
#include "pig/pig_debug_commands.cpp"
#include "pig/pig_debug_console.cpp"
#include "pig/pig_notifications.cpp"
#include "pig/pig_confirm_dialog.cpp"
#include "pig/pig_perf_graph.cpp"
#include "pig/pig_mem_graph.cpp"
#include "pig/pig_audio_debug.cpp"
#include "pig/pig_debug_overlay.cpp"
#include "pig/pig_app_states.cpp"

#include "pig/pig_tasks.cpp"

#include "game_main.cpp"

#include "pig/pig_dll_management.cpp"
#include "pig/pig_audio_mixer.cpp"
#include "pig/pig_main_functions.cpp"

#define MSF_GIF_MALLOC(contextPointer, newSize) PlatAllocFunc(newSize)
#define MSF_GIF_REALLOC(contextPointer, oldMemory, oldSize, newSize) PlatReallocFunc((oldMemory), (newSize), (oldSize))
#define MSF_GIF_FREE(contextPointer, oldMemory, oldSize) PlatFreeFunc(oldMemory)
#define MSF_GIF_IMPL
#pragma warning(push)
#pragma warning(disable:4100) //line 398: unreferenced formal parameter
#pragma warning(disable:4244) //lines 422, 433, 434, 435, 472, 498, 515: '=': conversion from 'int' to 'uint8_t', possible loss of data
#pragma warning(disable:4456) //line 339: declaration of 'k' hides previous local declaration
#pragma warning(disable:4459) //line 553, 617: declaration of 'empty' hides global declaration
#include "msf_gif/msf_gif.h"
#pragma warning(pop)

// +--------------------------------------------------------------+
// |                      Exported Functions                      |
// +--------------------------------------------------------------+
//NOTE: These functions are generally all implemented in pig_main_functions.cpp

// +==============================+
// |        Pig_GetVersion        |
// +==============================+
// Version_t Pig_GetVersion()
PIG_GET_VERSION_DEF(Pig_GetVersion)
{
	PigEntryPoint(PigEntryPoint_GetVersion, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	Version_t result;
	result.major = ENGINE_VERSION_MAJOR;
	result.minor = ENGINE_VERSION_MINOR;
	result.build = ENGINE_VERSION_BUILD;
	PigExitPoint(PigEntryPoint_GetVersion);
	return result;
}

// +==============================+
// |    Pig_GetStartupOptions     |
// +==============================+
// void Pig_GetStartupOptions(const StartupInfo_t* info, StartupOptions_t* optionsOut)
PIG_GET_STARTUP_OPTIONS_DEF(Pig_GetStartupOptions)
{
	PigEntryPoint(PigEntryPoint_GetStartupOptions, info, nullptr, nullptr, nullptr, nullptr, nullptr);
	GameGetStartupOptions(optionsOut);
	PigExitPoint(PigEntryPoint_GetStartupOptions);
}

// +==============================+
// |        Pig_Initialize        |
// +==============================+
// void Pig_Initialize(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, u64 programTime, u64 unixTimestamp, u64 localTimestamp)
PIG_INITIALIZE_DEF(Pig_Initialize)
{
	PigEntryPoint(PigEntryPoint_Initialize, nullptr, info, api, memory, nullptr, nullptr);
	ProgramTime = programTime;
	UnixTimestamp = unixTimestamp;
	LocalTimestamp = localTimestamp;
	PigInitialize(memory);
	PigExitPoint(PigEntryPoint_Initialize);
}

// +==============================+
// |          Pig_Update          |
// +==============================+
// void Pig_Update(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, EngineInput_t* input, EngineOutput_t* output)
PIG_UPDATE_DEF(Pig_Update)
{
	PigEntryPoint(PigEntryPoint_Update, nullptr, info, api, memory, input, output);
	PigUpdate();
	PigExitPoint(PigEntryPoint_Update);
}

// +==============================+
// |       Pig_AudioService       |
// +==============================+
// void Pig_AudioService(AudioServiceInfo_t* audioInfo)
PIG_AUDIO_SERVICE_DEF(Pig_AudioService)
{
	//NOTE: This function runs on the audio thread
	PigAudioService(audioInfo);
}

// +==============================+
// |    Pig_ShouldWindowClose     |
// +==============================+
// bool Pig_ShouldWindowClose(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, const PlatWindow_t* window)
PIG_SHOULD_WINDOW_CLOSE_DEF(Pig_ShouldWindowClose)
{
	PigEntryPoint(PigEntryPoint_ShouldWindowClose, nullptr, info, api, memory, nullptr, nullptr);
	bool result = PigShouldWindowClose(window);
	PigExitPoint(PigEntryPoint_ShouldWindowClose);
	return result;
}

// +==============================+
// |       Pig_PerformTask        |
// +==============================+
// void Pig_PerformTask(const PlatformInfo_t* info, const PlatformApi_t* api, PlatThreadPoolThread_t* thread, PlatTask_t* task)
PIG_PERFORM_TASK_DEF(Pig_PerformTask)
{
	//NOTE: This function runs on a thread pool thread
	if (task->input.type >= GameTask_Base)
	{
		GameHandleTask(info, api, thread, task);
	}
	else
	{
		PigHandleTask(info, api, thread, task);
	}
	AssertIf(thread->tempArena.size > 0, GetNumMarks(&thread->tempArena) == 0);
}

// +==============================+
// |        Pig_PreReload         |
// +==============================+
// void Pig_PreReload(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t newVersion)
PIG_PRE_RELOAD_DEF(Pig_PreReload)
{
	PigEntryPoint(PigEntryPoint_PreReload, nullptr, info, api, memory, nullptr, nullptr);
	PigPreReload(newVersion);
	PigExitPoint(PigEntryPoint_PreReload);
}

// +==============================+
// |        Pig_PostReload        |
// +==============================+
// void Pig_PostReload(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, Version_t oldVersion, u64 programTime, u64 unixTimestamp, u64 localTimestamp)
PIG_POST_RELOAD_DEF(Pig_PostReload)
{
	PigEntryPoint(PigEntryPoint_PostReload, nullptr, info, api, memory, nullptr, nullptr);
	ProgramTime = programTime;
	UnixTimestamp = unixTimestamp;
	LocalTimestamp = localTimestamp;
	PigPostReload(oldVersion);
	PigExitPoint(PigEntryPoint_PostReload);
}

// +==============================+
// |         Pig_Closing          |
// +==============================+
// void Pig_Closing(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory)
PIG_CLOSING_DEF(Pig_Closing)
{
	PigEntryPoint(PigEntryPoint_Closing, nullptr, info, api, memory, nullptr, nullptr);
	PigClosing();
	PigExitPoint(PigEntryPoint_Closing);
}

// +--------------------------------------------------------------+
// |                   Assert Failure Function                    |
// +--------------------------------------------------------------+
static bool insideAssertFailure = false;
void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	if (insideAssertFailure) { return; } //try to stop accidental recursions
	insideAssertFailure = true;
	if (pigEntryPoint == PigEntryPoint_None || pigEntryPoint == PigEntryPoint_GetStartupOptions || pigEntryPoint == PigEntryPoint_GetVersion)
	{
		//TODO: Can we do anything about these assertions really?
		MyBreak();
		exit(EXIT_CODE_ASSERTION_FAILED);
		return;
	}
	ThreadId_t threadId = (plat != nullptr && plat->GetThisThreadId != nullptr) ? plat->GetThisThreadId() : 0;
	bool isMainThread = (pig == nullptr || threadId == pig->mainThreadId);
	
	//TODO: Check for as much invalid state as possible
	if (isMainThread)
	{
		if (messageStr != nullptr && messageStr[0] != '\0')
		{
			PrintLine_E("Assertion Failure! %s (Expression: %s) in %s %s:%d", messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
		else
		{
			PrintLine_E("Assertion Failure! (%s) is not true in %s %s:%d", expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
	}
	else
	{
		if (messageStr != nullptr && messageStr[0] != '\0')
		{
			PrintLine_E("Assertion Failure on thread %u! %s (Expression: %s) in %s %s:%d", threadId, messageStr, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
		else
		{
			PrintLine_E("Assertion Failure on thread %u! (%s) is not true in %s %s:%d", threadId, expressionStr, funcName, filePath, lineNumber); //TODO: Shorten path to just fileName
		}
	}
	
	bool shouldExit = true;
	if (pig != nullptr && pig->dontExitOnAssert) { shouldExit = false; }
	
	if (plat != nullptr && plat->HandleAssertion != nullptr)
	{
		plat->HandleAssertion(shouldExit, filePath, lineNumber, funcName, expressionStr, messageStr);
	}
	else if (shouldExit)
	{
		MyBreak();
		exit(EXIT_CODE_ASSERTION_FAILED);
	}
	
	insideAssertFailure = false;
}