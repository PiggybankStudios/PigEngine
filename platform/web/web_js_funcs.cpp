/*
File:   web_js_funcs.cpp
Author: Taylor Robbins
Date:   10\15\2023
*/

WASM_IMPORTED_FUNC void jsPrintInteger(const char* labelStrPntr, int number);
WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);
WASM_IMPORTED_FUNC void jsPrintCallStack(const char* labelStrPntr);
WASM_IMPORTED_FUNC void jsConsoleWriteLine(int level, const char* messagePntr);
WASM_IMPORTED_FUNC double jsGetTime();
