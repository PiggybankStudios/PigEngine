/*
File:   debug.h
Author: Taylor Robbins
Date:   10\07\2024
*/

#ifndef _DEBUG_H
#define _DEBUG_H

enum DbgFlags_t
{
	DbgFlags_None         = 0x00,
	DbgFlag_Inverted      = 0x01,
	DbgFlag_New           = 0x04,
	// DbgFlag_Unused     = 0x08,
	DbgFlag_UserInput     = 0x10,
	DbgFlag_PlatformLayer = 0x20,
	DbgFlag_TaskThread    = 0x40,
	// DbgFlag_Unused        = 0x80,
};

void AppDebugOutput(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* message);
void AppDebugPrint(u8 flags, const char* filePath, u32 lineNumber, const char* funcName, DbgLevel_t dbgLevel, bool addNewLine, const char* formatString, ...);

#define WriteAt(dbgLevel, message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), false, message)
#define WriteLineAt(dbgLevel, message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), true,  message)
#define PrintAt(dbgLevel, formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), false, formatString, ##__VA_ARGS__)
#define PrintLineAt(dbgLevel, formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, (dbgLevel), true,  formatString, ##__VA_ARGS__)
#define WriteAtx(flags, dbgLevel, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, (dbgLevel), false, message)
#define WriteLineAtx(flags, dbgLevel, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, (dbgLevel), true,  message)
#define PrintAtx(flags, dbgLevel, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, (dbgLevel), false, formatString, ##__VA_ARGS__)
#define PrintLineAtx(flags, dbgLevel, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, (dbgLevel), true,  formatString, ##__VA_ARGS__)

#if DEBUG_OUTPUT_ENABLED
	#define Write_D(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
	#define WriteLine_D(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
	#define Print_D(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
	#define PrintLine_D(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)
	#define Write_Dx(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, false, message)
	#define WriteLine_Dx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  message)
	#define Print_Dx(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Dx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Debug, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_D(message)                       //nothing
	#define WriteLine_D(message)                   //nothing
	#define Print_D(formatString, ...)             //nothing
	#define PrintLine_D(formatString, ...)         //nothing
	#define Write_Dx(flags, message)               //nothing
	#define WriteLine_Dx(flags, message)           //nothing
	#define Print_Dx(flags, formatString, ...)     //nothing
	#define PrintLine_Dx(flags, formatString, ...) //nothing
#endif

#if REGULAR_OUTPUT_ENABLED
	#define Write_R(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
	#define WriteLine_R(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
	#define Print_R(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
	#define PrintLine_R(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)
	#define Write_Rx(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, false, message)
	#define WriteLine_Rx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  message)
	#define Print_Rx(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Rx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Regular, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_R(message)                       //nothing
	#define WriteLine_R(message)                   //nothing
	#define Print_R(formatString, ...)             //nothing
	#define PrintLine_R(formatString, ...)         //nothing
	#define Write_Rx(flags, message)               //nothing
	#define WriteLine_Rx(flags, message)           //nothing
	#define Print_Rx(flags, formatString, ...)     //nothing
	#define PrintLine_Rx(flags, formatString, ...) //nothing
#endif

#if INFO_OUTPUT_ENABLED
	#define Write_I(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
	#define WriteLine_I(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
	#define Print_I(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
	#define PrintLine_I(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)
	#define Write_Ix(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, false, message)
	#define WriteLine_Ix(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  message)
	#define Print_Ix(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Ix(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Info, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_I(message)                       //nothing
	#define WriteLine_I(message)                   //nothing
	#define Print_I(formatString, ...)             //nothing
	#define PrintLine_I(formatString, ...)         //nothing
	#define Write_Ix(flags, message)               //nothing
	#define WriteLine_Ix(flags, message)           //nothing
	#define Print_Ix(flags, formatString, ...)     //nothing
	#define PrintLine_Ix(flags, formatString, ...) //nothing
#endif

#if NOTIFY_OUTPUT_ENABLED
	#define Write_N(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, message)
	#define WriteLine_N(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
	#define Print_N(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
	#define PrintLine_N(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)
	#define Write_Nx(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, false, message)
	#define WriteLine_Nx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  message)
	#define Print_Nx(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Nx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Notify, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_N(message)                       //nothing
	#define WriteLine_N(message)                   //nothing
	#define Print_N(formatString, ...)             //nothing
	#define PrintLine_N(formatString, ...)         //nothing
	#define Write_Nx(flags, message)               //nothing
	#define WriteLine_Nx(flags, message)           //nothing
	#define Print_Nx(flags, formatString, ...)     //nothing
	#define PrintLine_Nx(flags, formatString, ...) //nothing
#endif

#if OTHER_OUTPUT_ENABLED
	#define Write_O(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
	#define WriteLine_O(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
	#define Print_O(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
	#define PrintLine_O(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)
	#define Write_Ox(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, false, message)
	#define WriteLine_Ox(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  message)
	#define Print_Ox(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Ox(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Other, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_O(message)                       //nothing
	#define WriteLine_O(message)                   //nothing
	#define Print_O(formatString, ...)             //nothing
	#define PrintLine_O(formatString, ...)         //nothing
	#define Write_Ox(flags, message)               //nothing
	#define WriteLine_Ox(flags, message)           //nothing
	#define Print_Ox(flags, formatString, ...)     //nothing
	#define PrintLine_Ox(flags, formatString, ...) //nothing
#endif

#if WARNING_OUTPUT_ENABLED
	#define Write_W(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
	#define WriteLine_W(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
	#define Print_W(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
	#define PrintLine_W(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)
	#define Write_Wx(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, false, message)
	#define WriteLine_Wx(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  message)
	#define Print_Wx(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Wx(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Warning, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_W(message)                       //nothing
	#define WriteLine_W(message)                   //nothing
	#define Print_W(formatString, ...)             //nothing
	#define PrintLine_W(formatString, ...)         //nothing
	#define Write_Wx(flags, message)               //nothing
	#define WriteLine_Wx(flags, message)           //nothing
	#define Print_Wx(flags, formatString, ...)     //nothing
	#define PrintLine_Wx(flags, formatString, ...) //nothing
#endif

#if ERROR_OUTPUT_ENABLED
	#define Write_E(message)                       AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
	#define WriteLine_E(message)                   AppDebugOutput(DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
	#define Print_E(formatString, ...)             AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
	#define PrintLine_E(formatString, ...)         AppDebugPrint (DbgFlags_None, __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
	#define Write_Ex(flags, message)               AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, false, message)
	#define WriteLine_Ex(flags, message)           AppDebugOutput((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  message)
	#define Print_Ex(flags, formatString, ...)     AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, false, formatString, ##__VA_ARGS__)
	#define PrintLine_Ex(flags, formatString, ...) AppDebugPrint ((flags),       __FILE__, __LINE__, __func__, DbgLevel_Error, true,  formatString, ##__VA_ARGS__)
#else
	#define Write_E(message)                       //nothing
	#define WriteLine_E(message)                   //nothing
	#define Print_E(formatString, ...)             //nothing
	#define PrintLine_E(formatString, ...)         //nothing
	#define Write_Ex(flags, message)               //nothing
	#define WriteLine_Ex(flags, message)           //nothing
	#define Print_Ex(flags, formatString, ...)     //nothing
	#define PrintLine_Ex(flags, formatString, ...) //nothing
#endif

#endif //  _DEBUG_H
