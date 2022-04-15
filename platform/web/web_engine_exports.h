/*
File:   web_engine_exports.h
Author: Taylor Robbins
Date:   03\27\2022
Description:
	** These are all the functions we expect the engine wasm file to export
	** They get routed to us as imports through the javascript layer
*/

#ifndef _WEB_ENGINE_EXPORTS_H
#define _WEB_ENGINE_EXPORTS_H

EXTERN_C_START

extern void Pig_Update(const PlatformInfo_t* info, const PlatformApi_t* api, EngineMemory_t* memory, EngineInput_t* input, EngineOutput_t* output);

EXTERN_C_END

#endif //  _WEB_ENGINE_EXPORTS_H
