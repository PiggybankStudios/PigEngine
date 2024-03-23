/*
File:   pig_func_defs.h
Author: Taylor Robbins
Date:   01\02\2022
Description:
	** Because of the way we #include mostly .cpp files and in a specific order based on dependencies, sometimes we
	** have the need to pre-declare specific functions so that a dependency loop can be resolved. Functions in this
	** file are implemented in various other .cpp files (generally files in the pig folder and with pig_ prefix)
*/

#ifndef _PIG_FUNC_DEFS_H
#define _PIG_FUNC_DEFS_H

//game_main.h
void GameInitAppGlobals(AppGlobals_t* globals);
void GameAllocateAppStateStructs(AppStateStructs_t* appStateStructs);
void GameUpdateGlobals();
void GameLoadDebugBindings(PigDebugBindings_t* bindings);
void GameGeneralInit();
void GameGeneralUpdate();
void GameHandleReload();
void GamePinResources();
void GamePrepareForClose();

//pig_main_functions.cpp
void CheckScratchNumMarksDuringUpdate(u64 numExtraMarks1 = 0, u64 numExtraMarks2 = 0, u64 numExtraMarks3 = 0);

//pig_resources.cpp
void AccessResource(ResourceType_t type, u64 resourceIndex);
void AccessResource(const Texture_t* texture);
void AccessResource(const VectorImg_t* vectorImg);
void AccessResource(const SpriteSheet_t* sheet);
void AccessResource(const Shader_t* shader);
void AccessResource(const Font_t* font);
void AccessResource(const Sound_t* soundOrMusic);
void AccessResource(const Model_t* model);
Texture_t* FindTextureResourceByFilename(MyStr_t filename, u64* textureIndexOut = nullptr);

//pig_debug_console.cpp
void DebugConsoleRegisterCommand(DebugConsole_t* console, MyStr_t command, MyStr_t description, u64 numArguments = 0, MyStr_t* arguments = nullptr);
void DebugConsoleLineAdded(DebugConsole_t* console, StringFifoLine_t* newLine);

//pig_perf_graph.cpp
void PigPerfGraphMark_(PigPerfGraph_t* graph);
#define PigPerfGraphMark() PigPerfGraphMark_(&pig->perfGraph)

//pig_app_states.cpp
bool IsAppStateInitialized(AppState_t appState);
bool IsAppStateActive(AppState_t appState);
AppState_t GetCurrentAppState();
void ChangeAppState(AppState_t newState);
void PushAppState(AppState_t newState);
void PopAppState();

//pig_debug_commands.cpp
bool PigParseDebugCommand(MyStr_t commandStr);

//pig_render_funcs_imgui.cpp
void RcRenderImDrawData(ImDrawData* imDrawData);

#endif //  _PIG_FUNC_DEFS_H
