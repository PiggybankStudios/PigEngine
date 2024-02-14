/*
File:   pig_gfx_debug.h
Author: Taylor Robbins
Date:   02\11\2024
*/

#ifndef _PIG_GFX_DEBUG_H
#define _PIG_GFX_DEBUG_H

#define PigGfx_InitFailure(message) do { if (gfx != nullptr && gfx->ctx.InitFailure != nullptr) { gfx->ctx.InitFailure(message); } } while(0)

#define PigGfx_WriteAt(dbgLevel, message)               do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, (dbgLevel), false, message); } } while(0)
#define PigGfx_WriteLineAt(dbgLevel, message)           do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, (dbgLevel), true,  message); } } while(0)
#define PigGfx_PrintAt(dbgLevel, formatString, ...)     do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, (dbgLevel), false, formatString, ##__VA_ARGS__) } } while(0)
#define PigGfx_PrintLineAt(dbgLevel, formatString, ...) do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, (dbgLevel), true,  formatString, ##__VA_ARGS__) } } while(0)

#define PigGfx_Write_D(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Debug, false, message); } } while(0)
#define PigGfx_WriteLine_D(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message); } } while(0)
#define PigGfx_Print_D(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_D(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_R(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Regular, false, message); } } while(0)
#define PigGfx_WriteLine_R(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message); } } while(0)
#define PigGfx_Print_R(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_R(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_I(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Info, false, message); } } while(0)
#define PigGfx_WriteLine_I(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Info, true,  message); } } while(0)
#define PigGfx_Print_I(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_I(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_N(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Notify, false, message); } } while(0)
#define PigGfx_WriteLine_N(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message); } } while(0)
#define PigGfx_Print_N(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_N(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_O(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Other, false, message); } } while(0)
#define PigGfx_WriteLine_O(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Other, true,  message); } } while(0)
#define PigGfx_Print_O(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_O(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_W(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Warning, false, message); } } while(0)
#define PigGfx_WriteLine_W(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message); } } while(0)
#define PigGfx_Print_W(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_W(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__); } } while(0)

#define PigGfx_Write_E(message)                         do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Error, false, message); } } while(0)
#define PigGfx_WriteLine_E(message)                     do { if (gfx != nullptr && gfx->ctx.DebugOutput != nullptr) { gfx->ctx.DebugOutput(__FILE__, __LINE__, __func__, DbgLevel_Error, true,  message); } } while(0)
#define PigGfx_Print_E(formatString, ...)               do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__); } } while(0)
#define PigGfx_PrintLine_E(formatString, ...)           do { if (gfx != nullptr && gfx->ctx.DebugPrint  != nullptr) { gfx->ctx.DebugPrint (__FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__); } } while(0)

#endif //  _PIG_GFX_DEBUG_H
