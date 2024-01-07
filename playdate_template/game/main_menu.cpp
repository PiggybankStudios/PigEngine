/*
File:   main_menu.cpp
Author: Taylor Robbins
Date:   12\19\2023
Description: 
	** Holds the AppState that runs the Main Menu (where you can choose to play the game, change settings, or exit )
*/

MainMenuState_t* mmenu = nullptr;

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartAppState_MainMenu(bool initialize, AppState_t prevState, MyStr_t transitionStr)
{
	if (initialize)
	{
		
		mmenu->initialized = true;
	}
}

// +--------------------------------------------------------------+
// |                             Stop                             |
// +--------------------------------------------------------------+
void StopAppState_MainMenu(bool deinitialize, AppState_t nextState)
{
	if (deinitialize)
	{
		ClearPointer(mmenu);
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateAppState_MainMenu()
{
	MemArena_t* scratch = GetScratchArena();
	
	//TODO: Remove me!
	if (BtnPressed(Btn_A))
	{
		PushAppState(AppState_Game);
	}
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderAppState_MainMenu(bool isOnTop)
{
	MemArena_t* scratch = GetScratchArena();
	
	pd->graphics->clear(kColorWhite);
	PdSetDrawMode(kDrawModeCopy);
	
	//TODO: Remove me!
	PdBindFont(&pig->debugFont);
	PdDrawText("Main Menu (Press A)", NewVec2i(150, 115));
	
	// +==============================+
	// |      Render Debug Info       |
	// +==============================+
	if (pig->debugEnabled)
	{
		LCDBitmapDrawMode oldDrawMode = PdSetDrawMode(kDrawModeNXOR);
		
		v2i textPos = NewVec2i(1, 1);
		if (pig->perfGraph.enabled) { textPos.y += pig->perfGraph.mainRec.y + pig->perfGraph.mainRec.height + 1; }
		PdBindFont(&pig->debugFont);
		i32 stepY = pig->debugFont.lineHeight + 1;
		
		PdDrawTextPrint(textPos, "Game: v%u.%u(%03u)", GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_BUILD);
		textPos.y += stepY;
		PdDrawTextPrint(textPos, "Engine: v%u.%u(%03u)", PIG_VERSION_MAJOR, PIG_VERSION_MINOR, PIG_VERSION_BUILD);
		textPos.y += stepY;
		PdDrawTextPrint(textPos, "Memory: %.2lf%%", ((r64)mainHeap->used / (r64)MAIN_HEAP_MAX_SIZE) * 100.0);
		textPos.y += stepY;
		
		#if 0
		{
			textPos.y += stepY;
			
			PdDrawTextPrint(textPos, "Static: ~%p", &pd);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Stack: ~%p", &scratch);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Std: %llu", pig->stdHeap.used);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "PigEngineState_t: %p %u", pig, sizeof(PigEngineState_t));
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Scratch[0]: %p %llu/%llu", pig->scratchArenas[0].mainPntr, pig->scratchArenas[0].highUsedMark, pig->scratchArenas[0].size);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Scratch[1]: %p %llu/%llu", pig->scratchArenas[1].mainPntr, pig->scratchArenas[1].highUsedMark, pig->scratchArenas[1].size);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Scratch[2]: %p %llu/%llu", pig->scratchArenas[2].mainPntr, pig->scratchArenas[2].highUsedMark, pig->scratchArenas[2].size);
			textPos.y += stepY;
			PdDrawTextPrint(textPos, "Fixed: %p %llu/%llu", fixedHeap->mainPntr, fixedHeap->highUsedMark, fixedHeap->size);
			textPos.y += stepY;
			
			textPos.y += stepY;
			
			PdDrawTextPrint(textPos, "Main: %llu/%llu %.2lf%% (%llu/%llu pages)", mainHeap->used, mainHeap->size, ((r64)mainHeap->used / (r64)mainHeap->size) * 100.0, mainHeap->numPages, mainHeap->maxNumPages);
			textPos.y += stepY;
			HeapPageHeader_t* pageHeader = (HeapPageHeader_t*)mainHeap->headerPntr;
			u64 pageIndex = 0;
			while (pageHeader != nullptr)
			{
				PdDrawTextPrint(textPos, "  Page[%llu]: %p %llu/%llu %.2lf%%", pageIndex, pageHeader, pageHeader->used, pageHeader->size, ((r64)pageHeader->used / (r64)pageHeader->size) * 100.0);
				textPos.y += stepY;
				pageHeader = pageHeader->next;
				pageIndex++;
			}
		}
		#endif
		
		PdSetDrawMode(oldDrawMode);
	}
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                           Register                           |
// +--------------------------------------------------------------+
void RegisterAppState_MainMenu()
{
	mmenu = (MainMenuState_t*)RegisterAppState(
		AppState_MainMenu,
		sizeof(MainMenuState_t),
		StartAppState_MainMenu,
		StopAppState_MainMenu,
		UpdateAppState_MainMenu,
		RenderAppState_MainMenu
	);
}
