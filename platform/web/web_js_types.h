/*
File:   web_js_types.h
Author: Taylor Robbins
Date:   10\23\2023
*/

#ifndef _WEB_JS_TYPES_H
#define _WEB_JS_TYPES_H

#define FILE_DOWNLOADED_CALLBACK_DEF(functionName) void functionName(const char* filePath, u32 fileSize, const u8* fileContentsPntr)
typedef FILE_DOWNLOADED_CALLBACK_DEF(FileDownloadedCallback_f);

typedef u32 AsyncId_t;

#endif //  _WEB_JS_TYPES_H
