/*
File:   win32_overlays.cpp
Author: Taylor Robbins
Date:   10\17\2021
Description: 
	** Holds functions that draw various debug overlays on top of the engine's render every frame
*/

void Win32_InitOverlays()
{
	CreateLinkedList(&Platform->debugReadoutLines, &Platform->mainHeap, Win32_DebugReadoutLine_t);
}

void Win32_DrawOverlays(PlatWindow_t* window) //pre-declared in win32_func_defs.cpp
{
	NotNull(window);
	//TODO: Make sure the window and OpenGL context are in a good state
	Win32_ChangeWindowTarget(window);
	Win32_BindShader(&Platform->loadingShader);
	Win32_InitBasicRendering();
	Win32_SetupViewportForWindow(window);
	
	Win32_DebugReadoutLine_t* line = LinkedListFirst(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t);
	v2 drawPos = NewVec2(5, 5 + Platform->debugFont.maxHeight);
	if (InitPhase < Win32InitPhase_DoingFirstUpdate)
	{
		drawPos.y += 10; //so it doesn't overlap with top loading bar
	}
	for (u64 lIndex = 0; lIndex < Platform->debugReadoutLines.count; lIndex++)
	{
		NotNull(line);
		if (line->windowId == window->id)
		{
			Win32_DrawString(&Platform->debugFont, drawPos + NewVec2(0, 1), line->scale, Black, line->str);
			Win32_DrawString(&Platform->debugFont, drawPos, line->scale, line->color, line->str);
			//TODO: Handle \n characters inside the line
			drawPos.y += Platform->debugFont.lineAdvance * line->scale;
		}
		line = LinkedListNext(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t, line);
	}
}

void Win32_OverlaysPostUpdateCleanup()
{
	Win32_DebugReadoutLine_t* line = LinkedListFirst(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t);
	v2 drawPos = NewVec2(5, 5 + Platform->debugFont.maxHeight);
	for (u64 lIndex = 0; lIndex < Platform->debugReadoutLines.count; lIndex++)
	{
		NotNull(line);
		if (line->str.pntr != nullptr)
		{
			NotNull(line->allocArena);
			FreeString(line->allocArena, &line->str);
		}
		line = LinkedListNext(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t, line);
	}
	LinkedListClear(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t);
}

// +==============================+
// |      Win32_DebugReadout      |
// +==============================+
// void DebugReadout(MyStr_t displayStr, Color_t color, r32 scale)
PLAT_API_DEBUG_READOUT(Win32_DebugReadout)
{
	AssertSingleThreaded();
	NotNullStr(&displayStr);
	NotNull(Platform->debugReadoutLines.allocArena);
	
	Win32_DebugReadoutLine_t* newLine = LinkedListAdd(&Platform->debugReadoutLines, Win32_DebugReadoutLine_t);
	NotNull(newLine);
	ClearPointer(newLine);
	newLine->allocArena = &Platform->mainHeap;
	newLine->color = color;
	newLine->scale = scale;
	newLine->str = AllocString(newLine->allocArena, &displayStr);
	newLine->windowId = Platform->currentWindow->id;
	NotNullStr(&newLine->str);
}
