/*
File:   web_javascript_interface.h
Author: Taylor Robbins
Date:   03\13\2022
*/

#ifndef _WEB_JAVASCRIPT_INTERFACE_H
#define _WEB_JAVASCRIPT_INTERFACE_H

EXTERN_C_START

// +--------------------------------------------------------------+
// |                      Intrinsic Imports                       |
// +--------------------------------------------------------------+
// NOTE: By using the __builtin functions in gy_std_wasm.h we implicitly import these functions from javascript
//       fminf, fmin, fmaxf, fmax, fmodf, fmod, roundf, round, sinf, sin, asinf, asin, cosf, cos, acosf, acos
//       tanf, tan, atanf, atan, atan2f, atan2, powf, pow, cbrtf, cbrt

// NOTE: But the following functions actually don't need to get imported since they have instructions in web assembly
//       absf, abs, floorf, floor, ceilf, ceil, sqrtf, sqrt, isnan, isinf

// +--------------------------------------------------------------+
// |                            Memory                            |
// +--------------------------------------------------------------+
extern int  js_GetHeapSize();
extern void js_GrowHeap(int numBytes);

// +--------------------------------------------------------------+
// |                        Console Output                        |
// +--------------------------------------------------------------+
extern void js_TestFunc();
extern void js_PrintNumber(int number);
extern void js_ConsoleDebug(const char* str);
extern void js_ConsoleLog(const char* str);
extern void js_ConsoleInfo(const char* str);
extern void js_ConsoleNotify(const char* str);
extern void js_ConsoleOther(const char* str);
extern void js_ConsoleWarn(const char* str);
extern void js_ConsoleError(const char* str);

// +--------------------------------------------------------------+
// |                     Canvas Manipulation                      |
// +--------------------------------------------------------------+
extern void js_ClearCanvas();
extern void js_DrawRectangle(float x, float y, float width, float height, int red, int green, int blue, int alpha);
extern void js_DrawRoundedRectangle(float x, float y, float width, float height, float radius, int red, int green, int blue, int alpha);
extern void js_DrawLine(float x1, float y1, float x2, float y2, float thickness, int red, int green, int blue, int alpha);

EXTERN_C_END

#endif //  _WEB_JAVASCRIPT_INTERFACE_H
