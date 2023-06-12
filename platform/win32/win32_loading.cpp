/*
File:   win32_loading.cpp
Author: Taylor Robbins
Date:   10\06\2021
Description: 
	** Holds functions that help us render the loading screen that shows while the engine is initializing
*/

#define PLATFORM_LOADING_TIME_SLICE  0.2f //percentage of loading bar dedicated to platform loading events

void Win32_LoadBasicResources(const StartupOptions_t* options)
{
	CreateVarArray(&Platform->vertexArrayObjs, &Platform->mainHeap, sizeof(Win32_VertexArrayObject_t));
	
	PlatImageData_t dotImageData = {};
	Color_t dotColor = White;
	dotImageData.data32 = (u32*)&dotColor;
	dotImageData.size = Vec2i_One;
	dotImageData.pixelSize = sizeof(u32);
	dotImageData.rowSize = dotImageData.pixelSize * dotImageData.width;
	dotImageData.dataSize = dotImageData.rowSize * dotImageData.height;
	Platform->dotTexture = Win32_CreateTexture_OpenGL(&dotImageData, true, true);
	//TODO: Check if the dotTexture isValid
	
	if (!IsEmptyStr(options->loadingImagePath))
	{
		Platform->loadingImage = Win32_LoadTexture(options->loadingImagePath, true, false);
	}
	if (!IsEmptyStr(options->loadingBackPath))
	{
		Platform->loadingBackground = Win32_LoadTexture(options->loadingBackPath, true, true);
	}
	
	Platform->loadingShader = Win32_LoadShader(NewStr("Resources/Shaders/pig_loading.glsl"));
	//TODO: Check if the initShader isValid
	
	Win32_Vertex_t squareVerts[] =
	{
		{ { 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } }, //top-left
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } }, //top-right
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 } }, //bottom-left
		
		{ { 1, 1, 0 }, { 1, 1, 1, 1 }, { 1, 1 } }, //bottom-right
		{ { 0, 1, 0 }, { 1, 1, 1, 1 }, { 0, 1 } }, //bottom-left
		{ { 1, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } }, //top-right
	};
	Platform->squareBuffer = Win32_CreateVertBuffer(false, ArrayCount(squareVerts), &squareVerts[0]);
	
	PlatFileContents_t fontFile;
	if (Win32_ReadPlatformFont(NewStr("Consolas"), 12, false, false, &fontFile))
	{
		PlatFontData_t fontData;
		
		PlatFontRange_t ranges[] = {
			{ /*fontSize*/ 14, /*firstCodepoint*/ 0x20, /*codePointList*/ nullptr, /*numChars*/ 0x7F-0x20 }
		};
		if (Win32_BakeFont(&fontFile, NewVec2i(256, 256), false, ArrayCount(ranges), &ranges[0], &fontData, nullptr))
		{
			Platform->debugFont = Win32_CreateFont(&fontData, true);
			if (!Platform->debugFont.isValid)
			{
				WriteLine_E("WARNING: Failed to create debugFont!");
			}
			
			Win32_FreeFontData(&fontData);
		}
		else
		{
			WriteLine_E("WARNING: Failed to bake debugFont!");
		}
		
		PlatFontRange_t hexRanges[] = {
			{ /*fontSize*/ 12, /*firstCodepoint*/ '0', /*codePointList*/ nullptr, /*numChars*/ 10 },
			{ /*fontSize*/ 12, /*firstCodepoint*/ 'A', /*codePointList*/ nullptr, /*numChars*/ 6 },
			{ /*fontSize*/ 12, /*firstCodepoint*/ 'a', /*codePointList*/ nullptr, /*numChars*/ 6 },
			{ /*fontSize*/ 12, /*firstCodepoint*/ 'U', /*codePointList*/ nullptr, /*numChars*/ 1 },
			{ /*fontSize*/ 12, /*firstCodepoint*/ '+', /*codePointList*/ nullptr, /*numChars*/ 1 }
		};
		if (Win32_BakeFont(&fontFile, NewVec2i(128, 128), false, ArrayCount(hexRanges), &hexRanges[0], &fontData, nullptr))
		{
			//Invert the font bake alpha
			Assert(fontData.imageData.pixelSize == 1);
			for (u64 pIndex = 0; pIndex < fontData.imageData.dataSize; pIndex++)
			{
				fontData.imageData.data8[pIndex] = 255 - fontData.imageData.data8[pIndex];
			}
			
			Platform->hexFont = Win32_CreateFont(&fontData, true);
			if (!Platform->hexFont.isValid)
			{
				WriteLine_E("WARNING: Failed to create hexFont!");
			}
			
			Win32_FreeFontData(&fontData);
		}
		else
		{
			WriteLine_E("WARNING: Failed to bake hexFont!");
		}
		
		Win32_FreeFileContents(&fontFile);
	}
	else
	{
		WriteLine_E("WARNING: Failed to load hexFont file by name \"Consolas\"!");
	}
}

// +==============================+
// |  Win32_RenderLoadingScreen   |
// +==============================+
// NOTE: Returns true if the main window should be closing
// bool Win32_RenderLoadingScreen(r32 completionPercent)
PLAT_API_RENDER_LOADING_SCREEN(Win32_RenderLoadingScreen)
{
	AssertSingleThreaded();
	TempPushMark();
	Win32_PollEventsAndCheckWindows();
	
	NotNull(Platform->mainWindow);
	if (Platform->mainWindow->closed) { TempPopMark(); return true; }
	if (glfwWindowShouldClose(Platform->mainWindow->handle))
	{
		Win32_GlfwCleanup();
		//TODO: This doesn't seem to be working like we want. It doesn't actually immediately close us!
		ExitProcess(1); //TODO: Turn this into a win32 specific call
		// return true; //TODO: Confirm whether exit REALLY does force an immediate exit
	}
	
	r32 actualLoadPercent = (Platform->loadingPercent * PLATFORM_LOADING_TIME_SLICE) + (completionPercent * (1.0f - PLATFORM_LOADING_TIME_SLICE));
	// PrintLine_D("Rendering loading screen for %g%% (%g %g)", actualLoadPercent*100, Platform->loadingPercent, completionPercent);
	
	PlatWindow_t* window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		NotNull(window);
		if (!window->closed)
		{
			Win32_ChangeWindowTarget(window);
			Win32_VertexArrayObject_t* compatibleVao = Win32_GetVertexArrayObj(window->id);
			NotNull(compatibleVao);
			Win32_BindVertexArrayObject(compatibleVao);
			Win32_SetFramebufferToWindow(window);
			Win32_ClearScreen(Platform->startupOptions.loadingBackgroundColor, 1.0f);
			
			if (Platform->loadingShader.isValid)
			{
				Win32_BindShader(&Platform->loadingShader);
				Win32_SetShaderValue(0, 0.0f);
				Win32_SetShaderValue(1, 0.0f);
				Win32_SetupViewportForWindow(window);
				Win32_BindVertBuffer(&Platform->squareBuffer);
				
				if (Platform->loadingBackground.isValid)
				{
					Win32_BindTexture(&Platform->loadingBackground);
					rec logoRec = Rec_Zero;
					rec logoSourceRec = NewRec(Vec2_Zero, ToVec2(Platform->loadingBackground.size));
					if (Platform->startupOptions.loadingBackTiling)
					{
						Unimplemented(); //TODO: Implement me!
					}
					else if (Platform->startupOptions.loadingBackScale != 0.0f)
					{
						logoRec = NewRec(window->input.renderResolution/2, Platform->startupOptions.loadingBackScale * ToVec2(Platform->loadingBackground.size));
						logoRec.topLeft -= logoRec.size/2;
					}
					else
					{
						logoRec = NewRec(0, 0, window->input.renderResolution);
					}
					logoRec.topLeft = Vec2Round(logoRec.topLeft);
					Win32_DrawTexturedRec(logoRec, White, logoSourceRec);
				}
				
				if (Platform->loadingImage.isValid)
				{
					Assert(Platform->loadingImage.width > 0 && Platform->loadingImage.height > 0);
					Win32_BindTexture(&Platform->loadingImage);
					rec logoRec = NewRec(window->input.renderResolution/2, ToVec2(Platform->loadingImage.size));
					r32 logoScale = 1.0f;
					r32 targetFill = (3.0f/4.0f);
					while (logoRec.width * (logoScale+1) < window->input.renderResolution.width*targetFill && logoRec.height * (logoScale+1) < window->input.renderResolution.height*targetFill)
					{
						logoScale += 1;
					}
					logoRec.size = logoRec.size * logoScale;
					logoRec.topLeft = logoRec.topLeft - logoRec.size/2;
					logoRec.topLeft = Vec2Round(logoRec.topLeft);
					Win32_DrawTexturedRec(logoRec, White, NewRec(Vec2_Zero, ToVec2(Platform->loadingImage.size)));
				}
				
				// if (Platform->debugFont.bake.isValid)
				// {
				// 	Assert(Platform->debugFont.bake.width > 0 && Platform->debugFont.bake.height > 0);
				// 	Win32_BindTexture(&Platform->debugFont.bake);
				// 	rec logoRec = NewRec(window->input.renderResolution/2, ToVec2(Platform->debugFont.bake.size));
				// 	r32 logoScale = 1.0f;
				// 	r32 targetFill = (3.0f/4.0f);
				// 	while (logoRec.width * (logoScale+1) < window->input.renderResolution.width*targetFill && logoRec.height * (logoScale+1) < window->input.renderResolution.height*targetFill)
				// 	{
				// 		logoScale += 1;
				// 	}
				// 	logoScale = 3.0f;
				// 	logoRec.size = logoRec.size * logoScale;
				// 	logoRec.topLeft = logoRec.topLeft - logoRec.size/2;
				// 	logoRec.topLeft = Vec2Round(logoRec.topLeft);
				// 	Win32_DrawTexturedRec(logoRec, White, NewRec(Vec2_Zero, ToVec2(Platform->loadingImage.size)));
				// }
				
				// Win32_DrawString(&Platform->debugFont, NewVec2(100, 100), 1.0f, White, NewStr("[a\xAB\xCD\xEF\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9z]"));
				// Win32_DrawString(&Platform->debugFont, NewVec2(100, 100), 1.0f, White, NewStr("お前はもう死んでいる!"));
				// Win32_DrawString(&Platform->debugFont, NewVec2(100, 150), 2.0f, White, NewStr("ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789\n,./;'[]<>?:\"{}|\\-=_+!@#$%^&*()~`"));
				
				Win32_SetShaderValue(0, LerpR32(0.0f, 0.4f, actualLoadPercent)); //vignetteInnerRadius
				Win32_SetShaderValue(1, LerpR32(1.5f, 1.8f, actualLoadPercent)); //vignetteOuterRadius
				Win32_BindTexture(&Platform->dotTexture);
				Win32_DrawTexturedRec(NewRec(Vec2_Zero, window->input.renderResolution), TransparentBlack, Rec_Default);
				Win32_SetShaderValue(0, 0.0f);
				Win32_SetShaderValue(1, 0.0f);
				// Win32_DrawTexturedRec(NewRec(Vec2_Zero, window->input.renderResolution), ColorTransparent(Black, LerpR32(1.0f, 0.0f, actualLoadPercent)), Rec_Default);
				
				Win32_BindTexture(&Platform->dotTexture);
				Win32_DrawTexturedRec(NewRec(0, 0, window->input.renderResolution.width * actualLoadPercent, 10), Platform->startupOptions.loadingBarColor, Rec_Default);
				Win32_DrawTexturedRec(NewRec(0, window->input.renderResolution.height - 10, window->input.renderResolution.width * actualLoadPercent, 10), Platform->startupOptions.loadingBarColor, Rec_Default);
			}
		}
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
	
	Win32_SwapBuffers();
	
	TempPopMark();
	return false;
}
