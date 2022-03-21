/*
File:   pig_mem_graph.cpp
Author: Taylor Robbins
Date:   02\28\2022
Description: 
	** Holds functions that help us draw a nice graph of memory arenas and their current usage
*/

#define PIG_MEM_GRAPH_PAGE_WIDTH           20 //px
#define PIG_MEM_GRAPH_PAGE_PADDING         5 //px
#define PIG_MEM_GRAPH_ARENA_PADDING        10 //px
#define PIG_MEM_GRAPH_NUM_INFO_LINES       4 //lines
#define PIG_MEM_GRAPH_PAGE_APPEAR_TIME     200 //ms
#define PIG_MEM_GRAPH_USED_DISPLAY_DELAY   15 //divisor
#define PIG_MEM_GRAPH_CHANGE_DISPLAY_TIME  1000 //ms

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
u64 PigMemGraphGetNumPagesForArena(MemArena_t* arenaPntr)
{
	NotNull(arenaPntr);
	switch (arenaPntr->type)
	{
		case MemArenaType_Alias:
		{
			NotNull(arenaPntr->sourceArena);
			return PigMemGraphGetNumPagesForArena(arenaPntr->sourceArena);
		} break;
		case MemArenaType_PagedHeap: return arenaPntr->numPages;
		default: return 1;
	}
}
void PigMemGraphGetPageInfo(MemArena_t* arenaPntr, u64 pageIndex, PigMemGraphArena_t* arena, PigMemGraphArenaPage_t* page)
{
	NotNull(arenaPntr);
	NotNull(arena);
	NotNull(page);
	switch (arenaPntr->type)
	{
		case MemArenaType_Alias:
		{
			NotNull(arenaPntr->sourceArena);
			PigMemGraphGetPageInfo(arenaPntr->sourceArena, pageIndex, arena, page);
		} break;
		case MemArenaType_FixedHeap:
		{
			page->size = arenaPntr->size;
			page->used = arenaPntr->used;
			page->numAllocations = arenaPntr->numAllocations;
			page->usedPercent = ((r32)page->used / (r32)page->size);
			if (IsInfiniteR32(page->usedPercent)) { page->usedPercent = 1.0f; }
		} break;
		case MemArenaType_MarkedStack:
		{
			page->size = arenaPntr->size;
			page->used = arenaPntr->highUsedMark;
			page->numAllocations = arenaPntr->numAllocations;
			page->usedPercent = ((r32)page->used / (r32)page->size);
			if (IsInfiniteR32(page->usedPercent)) { page->usedPercent = 1.0f; }
		} break;
		case MemArenaType_Redirect:
		{
			page->size = arenaPntr->highUsedMark;
			page->used = arenaPntr->used;
			page->usedPercent = ((r32)page->used / (r32)page->size);
			page->numAllocations = arenaPntr->numAllocations;
			if (IsInfiniteR32(page->usedPercent)) { page->usedPercent = 1.0f; }
		} break;
		case MemArenaType_StdHeap:
		{
			page->size = arenaPntr->highUsedMark;
			page->used = arenaPntr->used;
			page->usedPercent = ((r32)page->used / (r32)page->size);
			page->numAllocations = arenaPntr->numAllocations;
			if (IsInfiniteR32(page->usedPercent)) { page->usedPercent = 1.0f; }
		} break;
		case MemArenaType_PagedHeap:
		{
			Assert(pageIndex < arenaPntr->numPages);
			HeapPageHeader_t* pageHeader = (HeapPageHeader_t*)arenaPntr->headerPntr;
			for (u64 pIndex = 0; pIndex < pageIndex; pIndex++)
			{
				NotNull(pageHeader);
				pageHeader = pageHeader->next;
			}
			NotNull(pageHeader);
			
			page->size = pageHeader->size;
			page->used = pageHeader->used;
			page->usedPercent = ((r32)page->used / (r32)page->size);
			page->numAllocations = arenaPntr->numAllocations;
			if (IsInfiniteR32(page->usedPercent)) { page->usedPercent = 1.0f; }
		} break;
		default:
		{
			Unimplemented(); //TODO: Implement me!
		} break;
	}
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializePigMemGraph(PigMemGraph_t* graph)
{
	NotNull(graph);
	ClearPointer(graph);
	graph->enabled = SHOW_MEM_GRAPH_ON_STARTUP;
	graph->nextArenaId = 1;
	graph->nextPageId = 1;
	graph->selectedPageId = 0;
	CreateVarArray(&graph->arenas, fixedHeap, sizeof(PigMemGraphArena_t));
}

void PigMemGraphAddArena(PigMemGraph_t* graph, MemArena_t* arenaPntr, MyStr_t name)
{
	AssertSingleThreaded();
	NotNull(graph);
	NotNull(arenaPntr);
	
	PigMemGraphArena_t* newArena = VarArrayAdd(&graph->arenas, PigMemGraphArena_t);
	NotNull(newArena);
	ClearPointer(newArena);
	newArena->id = graph->nextArenaId;
	graph->nextArenaId++;
	newArena->pntr = arenaPntr;
	newArena->name = AllocString(fixedHeap, &name);
	NotNullStr(&newArena->name);
	CreateVarArray(&newArena->pages, fixedHeap, sizeof(PigMemGraphArenaPage_t), PigMemGraphGetNumPagesForArena(arenaPntr));
}

// +--------------------------------------------------------------+
// |                   Layout and Capture Mouse                   |
// +--------------------------------------------------------------+
void PigMemGraphLayout(PigMemGraph_t* graph)
{
	NotNull(graph);
	
	RcBindFont(&pig->resources.debugFont, SelectDefaultFontFace());
	
	rec usableArea = NewRec(Vec2_Zero, ScreenSize);
	if (pig->perfGraph.enabled)
	{
		usableArea.y = pig->perfGraph.infoTextPos.y + 5;
		usableArea.height = ScreenSize.height- usableArea.y;
	}
	
	r32 xPos = PIG_MEM_GRAPH_PAGE_PADDING;
	graph->mainRec = Rec_Zero;
	VarArrayLoop(&graph->arenas, aIndex)
	{
		VarArrayLoopGet(PigMemGraphArena_t, arena, &graph->arenas, aIndex);
		arena->mainRec = Rec_Zero;
		VarArrayLoop(&arena->pages, pIndex)
		{
			VarArrayLoopGet(PigMemGraphArenaPage_t, page, &arena->pages, pIndex);
			page->mainRec.x = xPos;
			page->mainRec.width = PIG_MEM_GRAPH_PAGE_WIDTH;
			r32 expectedTextHeight = RcGetLineHeight()*PIG_MEM_GRAPH_NUM_INFO_LINES;
			page->mainRec.y = usableArea.y + PIG_MEM_GRAPH_PAGE_PADDING;
			page->mainRec.height = (usableArea.y + usableArea.height - expectedTextHeight - PIG_MEM_GRAPH_PAGE_PADDING) - page->mainRec.y;
			page->mainRec.height *= EaseQuadraticOut(page->appearAnimTime);
			RecAlign(&page->mainRec);
			if (pIndex == 0) { arena->mainRec = page->mainRec; }
			else { arena->mainRec = RecBoth(arena->mainRec, page->mainRec); }
			r32 paddingToNextPage = (r32)((pIndex+1 == arena->pages.length) ? PIG_MEM_GRAPH_ARENA_PADDING : PIG_MEM_GRAPH_PAGE_PADDING);
			xPos = page->mainRec.x + (page->mainRec.width + paddingToNextPage) * EaseQuadraticOut(page->appearAnimTime);
		}
		if (aIndex == 0) { graph->mainRec = arena->mainRec; }
		else { graph->mainRec = RecBoth(graph->mainRec, arena->mainRec); }
	}
}

void PigMemGraphCaptureMouse(PigMemGraph_t* graph)
{
	NotNull(graph);
	PigMemGraphLayout(graph);
	if (graph->enabled)
	{
		VarArrayLoop(&graph->arenas, aIndex)
		{
			VarArrayLoopGet(PigMemGraphArena_t, arena, &graph->arenas, aIndex);
			VarArrayLoop(&arena->pages, pIndex)
			{
				VarArrayLoopGet(PigMemGraphArenaPage_t, page, &arena->pages, pIndex);
				MouseHitRecPrint(page->mainRec, "PigMemGraphArena%lluPage%llu", aIndex, pIndex);
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdatePigMemGraph(PigMemGraph_t* graph)
{
	NotNull(graph);
	
	PigMemGraphLayout(graph);
	
	// +==============================+
	// |         Update Pages         |
	// +==============================+
	VarArrayLoop(&graph->arenas, aIndex)
	{
		VarArrayLoopGet(PigMemGraphArena_t, arena, &graph->arenas, aIndex);
		
		u64 numPages = PigMemGraphGetNumPagesForArena(arena->pntr);
		while (arena->pages.length < numPages)
		{
			PigMemGraphArenaPage_t* newPage = VarArrayAdd(&arena->pages, PigMemGraphArenaPage_t);
			NotNull(newPage);
			ClearPointer(newPage);
			newPage->id = graph->nextPageId;
			graph->nextPageId++;
			newPage->appearAnimTime = 0.0f;
		}
		while (arena->pages.length > numPages)
		{
			VarArrayPop(&arena->pages, PigMemGraphArenaPage_t);
		}
		
		VarArrayLoop(&arena->pages, pIndex)
		{
			VarArrayLoopGet(PigMemGraphArenaPage_t, page, &arena->pages, pIndex);
			
			u64 prevUsed = page->used;
			u64 prevNumAllocations = page->numAllocations;
			PigMemGraphGetPageInfo(arena->pntr, pIndex, arena, page);
			if (page->appearAnimTime == 0.0f) //it was just created
			{
				page->usedPercentDisplay = page->usedPercent;
			}
			
			if (prevUsed != page->used)
			{
				if (TimeSince(page->lastUsedChangeTime) < PIG_MEM_GRAPH_CHANGE_DISPLAY_TIME)
				{
					page->lastUsedChangeAmount += (page->used > prevUsed) ? (i64)(page->used - prevUsed) : -(i64)(prevUsed - page->used);
				}
				else
				{
					page->lastUsedChangeAmount = (page->used > prevUsed) ? (i64)(page->used - prevUsed) : -(i64)(prevUsed - page->used);
				}
				page->lastUsedChangeTime = ProgramTime;
			}
			if (prevNumAllocations != page->numAllocations)
			{
				if (TimeSince(page->lastAllocationsChangeTime) < PIG_MEM_GRAPH_CHANGE_DISPLAY_TIME)
				{
					page->lastAllocationsChangeAmount += (page->numAllocations > prevNumAllocations) ? (i64)(page->numAllocations - prevNumAllocations) : -(i64)(prevNumAllocations - page->numAllocations);
				}
				else
				{
					page->lastAllocationsChangeAmount = (page->numAllocations > prevNumAllocations) ? (i64)(page->numAllocations - prevNumAllocations) : -(i64)(prevNumAllocations - page->numAllocations);
				}
				page->lastAllocationsChangeTime = ProgramTime;
			}
			
			UpdateAnimationUp(&page->appearAnimTime, PIG_MEM_GRAPH_PAGE_APPEAR_TIME);
			
			r32 usedPercentDelta = (page->usedPercent - page->usedPercentDisplay);
			if (AbsR32(usedPercentDelta) > (1 / page->mainRec.height))
			{
				page->usedPercentDisplay += usedPercentDelta / PIG_MEM_GRAPH_USED_DISPLAY_DELAY;
			}
			else
			{
				page->usedPercentDisplay = page->usedPercent;
			}
		}
	}
	
	// +==============================+
	// |    Handle Mouse on Pages     |
	// +==============================+
	VarArrayLoop(&graph->arenas, aIndex)
	{
		VarArrayLoopGet(PigMemGraphArena_t, arena, &graph->arenas, aIndex);
		VarArrayLoop(&arena->pages, pIndex)
		{
			VarArrayLoopGet(PigMemGraphArenaPage_t, page, &arena->pages, pIndex);
			if (IsMouseOverPrint("PigMemGraphArena%lluPage%llu", aIndex, pIndex))
			{
				pigOut->cursorType = PlatCursor_Pointer;
				if (MousePressedAndHandleExtended(MouseBtn_Left))
				{
					if (graph->selectedPageId != page->id)
					{
						graph->selectedPageId = page->id;
					}
					else
					{
						graph->selectedPageId = 0;
					}
				}
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderPigMemGraph(PigMemGraph_t* graph)
{
	NotNull(graph);
	PigMemGraphLayout(graph);
	
	// +==============================+
	// |         Render Pages         |
	// +==============================+
	if (graph->enabled)
	{
		bool isMouseOverAnyPage = IsMouseOverNamedPartial("PigMemGraphArena");
		VarArrayLoop(&graph->arenas, aIndex)
		{
			VarArrayLoopGet(PigMemGraphArena_t, arena, &graph->arenas, aIndex);
			VarArrayLoop(&arena->pages, pIndex)
			{
				VarArrayLoopGet(PigMemGraphArenaPage_t, page, &arena->pages, pIndex);
				Color_t backColor = ColorTransparent(Black, 0.5f);
				Color_t fillColor = MonokaiOrange;
				Color_t outlineColor = Black;
				bool isMouseOver = IsMouseOverPrint("PigMemGraphArena%lluPage%llu", aIndex, pIndex);
				bool isSelected = (graph->selectedPageId == page->id);
				if (isMouseOver)
				{
					backColor = MonokaiDarkGray;
					fillColor = MonokaiYellow;
				}
				if (isSelected)
				{
					outlineColor = MonokaiWhite;
				}
				rec fillRec = page->mainRec;
				fillRec.height *= page->usedPercent;
				rec fillDisplayRec = page->mainRec;
				fillDisplayRec.height *= page->usedPercentDisplay;
				
				RcDrawRectangle(page->mainRec, backColor);
				if (page->usedPercent > page->usedPercentDisplay)
				{
					RcDrawRectangle(fillRec, MonokaiRed);
					RcDrawRectangle(fillDisplayRec, fillColor);
				}
				else if (page->usedPercent < page->usedPercentDisplay)
				{
					RcDrawRectangle(fillDisplayRec, MonokaiDarkGreen);
					RcDrawRectangle(fillRec, fillColor);
				}
				else
				{
					RcDrawRectangle(fillRec, fillColor);
				}
				RcDrawRectangleOutline(page->mainRec, outlineColor, 1);
				
				if (isMouseOver || (isSelected && !isMouseOverAnyPage))
				{
					RcBindFont(&pig->resources.debugFont, SelectDefaultFontFace());
					v2 textPos = NewVec2(page->mainRec.x, page->mainRec.y + page->mainRec.height + RcGetMaxAscend());
					Vec2Align(&textPos);
					
					const char* lastUsedChangeAmountStr = "";
					if (TimeSince(page->lastUsedChangeTime) < PIG_MEM_GRAPH_CHANGE_DISPLAY_TIME)
					{
						lastUsedChangeAmountStr = TempPrint(" (%s%s)", (page->lastUsedChangeAmount >= 0) ? "+" : "-", FormatBytesNt((u64)AbsI64(page->lastUsedChangeAmount), TempArena));
					}
					const char* lastAllocationsChangeAmountStr = "";
					if (TimeSince(page->lastAllocationsChangeTime) < PIG_MEM_GRAPH_CHANGE_DISPLAY_TIME)
					{
						lastAllocationsChangeAmountStr = TempPrint(" (%s%llu)", (page->lastAllocationsChangeAmount >= 0) ? "+" : "-", (u64)AbsI64(page->lastAllocationsChangeAmount));
					}
					RcDrawTextPrint(textPos, MonokaiWhite, "%.*s\n%.1f%%%s\n%llu%s\n%s/%s",
						arena->name.length, arena->name.pntr,
						page->usedPercent * 100, lastUsedChangeAmountStr,
						page->numAllocations, lastAllocationsChangeAmountStr,
						FormatBytesNt(page->used, TempArena), FormatBytesNt(page->size, TempArena)
					);
				}
			}
		}
	}
}
