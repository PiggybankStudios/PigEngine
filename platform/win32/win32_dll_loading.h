/*
File:   win32_dll_loading.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Just a place to declare EngineDll_t which needs all of the _f typedefs from common_interface_types.h but has some platform specific members
*/

#ifndef _WIN_32_DLL_LOADING_H
#define _WIN_32_DLL_LOADING_H

struct EngineDll_t
{
	bool isValid;
	Version_t version;
	
	PigGetVersion_f*        GetVersion;
	PigGetStartupOptions_f* GetStartupOptions;
	PigInitialize_f*        Initialize;
	PigUpdate_f*            Update;
	PigAudioService_f*      AudioService;
	PigShouldWindowClose_f* ShouldWindowClose;
	PigClosing_f*           Closing;
	PigPreReload_f*         PreReload;
	PigPostReload_f*        PostReload;
	PigPerformTask_f*       PerformTask;
	
	HMODULE handle;
	PlatWatchedFile_t* watch;
	bool needToReload;
	u64 lastChangeProgramTime; //NOTE: This is programTime unaffected by time scale fixing
};

#endif //  _WIN_32_DLL_LOADING_H
