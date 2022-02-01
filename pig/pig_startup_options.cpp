/*
File:   pig_startup_options.cpp
Author: Taylor Robbins
Date:   01\13\2022
Description: 
	** This file just holds PigGetStartupOptions
*/

void PigGetStartupOptions(const StartupInfo_t* info, StartupOptions_t* options)
{
	//TODO: Assertions in here are unclear to the user. Try to limit failure cases as much as possible
	NotNull(info);
	NotNull(options);
	
	options->mainMemoryRequest = Megabytes(32);
	options->tempMemoryRequest = Gigabytes(1);
	
	options->renderApi = RenderApi_OpenGL; //TODO: Can we somehow choose this smartly?
	options->openDebugConsole = false;
	if (info->GetProgramArg(info->platTempArena, NewStr("debug"), nullptr))
	{
		options->openDebugConsole = true;
	}
	
	options->audioDeviceIndex = info->defaultAudioDeviceIndex;
	options->audioOutputFormat.bitsPerSample    = 16;
	options->audioOutputFormat.numChannels      = 2;
	options->audioOutputFormat.samplesPerSecond = 44100;
	
	options->threadPoolSize = 0; //threads
	
	options->numIconFiles = 6;
	options->iconFilePaths = AllocArray(info->platTempArena, MyStr_t, options->numIconFiles);
	NotNull(options->iconFilePaths);
	options->iconFilePaths[0] = NewStr("Resources/icon16.png");
	options->iconFilePaths[1] = NewStr("Resources/icon24.png");
	options->iconFilePaths[2] = NewStr("Resources/icon32.png");
	options->iconFilePaths[3] = NewStr("Resources/icon64.png");
	options->iconFilePaths[4] = NewStr("Resources/icon120.png");
	options->iconFilePaths[5] = NewStr("Resources/icon256.png");
	
	options->numWindows = 1;
	options->windowOptions = AllocArray(info->platTempArena, PlatWindowOptions_t, options->numWindows);
	NotNull(options->windowOptions);
	
	options->windowOptions[0].create.resizableWindow = true;
	options->windowOptions[0].create.topmostWindow = PIG_WINDOW_ALWAYS_ON_TOP;
	options->windowOptions[0].create.decoratedWindow = true;
	options->windowOptions[0].create.antialiasingNumSamples = 4;
	options->windowOptions[0].create.requestRefreshRate = PIG_TARGET_FRAMERATE;
	options->windowOptions[0].create.autoIconify = true;
	options->windowOptions[0].create.requestSize = NewVec2i(1600, 900); //NewVec2i(1032, 808);
	options->windowOptions[0].create.requestMonitor = nullptr;
	options->windowOptions[0].create.windowTitle = NewStringInArenaNt(info->platTempArena, "Pig Engine");
	options->windowOptions[0].enforceMinSize = true;
	options->windowOptions[0].minWindowSize = NewVec2i(400, 100);
	options->windowOptions[0].enforceMaxSize = false;
	options->windowOptions[0].maxWindowSize = Vec2i_Zero;
	options->windowOptions[0].forceAspectRatio = false;
	options->windowOptions[0].aspectRatio = Vec2i_Zero;
	
	if (options->numWindows >= 2)
	{
		options->windowOptions[1].create.resizableWindow = true;
		options->windowOptions[1].create.topmostWindow = PIG_WINDOW_ALWAYS_ON_TOP;
		options->windowOptions[1].create.decoratedWindow = true;
		options->windowOptions[1].create.antialiasingNumSamples = 0;
		options->windowOptions[1].create.requestRefreshRate = PIG_TARGET_FRAMERATE;
		options->windowOptions[1].create.autoIconify = true;
		options->windowOptions[1].create.requestSize = NewVec2i(200, 100);
		options->windowOptions[1].create.requestMonitor = nullptr;
		options->windowOptions[1].create.windowTitle = NewStringInArenaNt(info->platTempArena, "Itty Bitty Window");
		options->windowOptions[1].minWindowSize = Vec2i_Zero;
		options->windowOptions[1].maxWindowSize = Vec2i_Zero;
		options->windowOptions[1].enforceMinSize = false;
		options->windowOptions[1].minWindowSize = Vec2i_Zero;
		options->windowOptions[1].enforceMaxSize = false;
		options->windowOptions[1].maxWindowSize = NewVec2i(400, 200);
		options->windowOptions[1].forceAspectRatio = false;
		options->windowOptions[1].aspectRatio = NewVec2i(2, 1);
	}
	
	options->loadingBackgroundColor = NewColor(0xFF772957);
	options->loadingBarColor = PalPinkLighter;
	options->loadingImagePath = NewStringInArenaNt(info->platTempArena, "Resources/Sprites/loading_image.png");
	// options->loadingBackPath = NewStringInArenaNt(info->platTempArena, "Resources/Sprites/piggybank.png"); //TODO: Fill me!
	// options->loadingBackTiling = true;
	// options->loadingBackScale = 2.0f;
}
