/*
File:   web_js_funcs.h
Author: Taylor Robbins
Date:   10\15\2023
*/

#ifndef _WEB_JS_FUNCS_H
#define _WEB_JS_FUNCS_H

WASM_IMPORTED_FUNC void jsPrintInteger(const char* labelStrPntr, int number);
WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);
WASM_IMPORTED_FUNC void jsPrintCallStack(const char* labelStrPntr);
WASM_IMPORTED_FUNC void jsConsoleWriteLine(int level, const char* messagePntr);
WASM_IMPORTED_FUNC double jsGetTime();
WASM_IMPORTED_FUNC void jsStartRendering(int canvasWidth, int canvasHeight);

#endif //  _WEB_JS_FUNCS_H
