/*
File:   pig_perf_graph.cpp
Author: Taylor Robbins
Date:   01\04\2022
Description: 
	** Holds functions that help us draw a nice graph of elapsed milliseconds for the last 120 frames
*/

//TODO: Add a pause button
//TODO: Add a lock scale button
//TODO: Add exact readouts when you are hovering over the graph

#define PERF_GRAPH_PAUSE_LOCK_BTNS_SIZE      16 //px
#define PERF_GRAPH_PAUSE_LOCK_BTNS_MARGIN    5 //px

void InitializePigPerfGraph(PigPerfGraph_t* graph)
{
	NotNull(graph);
	ClearPointer(graph);
	for (u64 vIndex = 0; vIndex < PERF_GRAPH_LENGTH; vIndex++)
	{
		graph->msValues[vIndex] = 0.0;
		graph->updateMsValues[vIndex] = 0.0;
		graph->waitMsValues[vIndex] = 0.0;
	}
	graph->scale = PERF_GRAPH_MAX_SCALE;
	graph->enabled = SHOW_PERF_GRAPH_ON_STARTUP;
	graph->lockedScale = LOCK_PERF_GRAPH_SCALE;
	graph->hasAutoPaused = false;
}

void PigPerfGraphLayout(PigPerfGraph_t* graph)
{
	NotNull(graph);
	graph->scale = ClampR32(graph->scale, PERF_GRAPH_MIN_SCALE, PERF_GRAPH_MAX_SCALE);
	graph->mainRec.size = NewVec2(PERF_GRAPH_DATA_WIDTH * (PERF_GRAPH_LENGTH-1), PERF_GRAPH_HEIGHT);
	graph->mainRec.topLeft = NewVec2(10, 10);
	RecAlign(&graph->mainRec);
	
	graph->lockScaleBtnRec.size = NewVec2(PERF_GRAPH_PAUSE_LOCK_BTNS_SIZE, PERF_GRAPH_PAUSE_LOCK_BTNS_SIZE);
	graph->lockScaleBtnRec.x = graph->mainRec.x + graph->mainRec.width - PERF_GRAPH_PAUSE_LOCK_BTNS_MARGIN - graph->lockScaleBtnRec.width;
	graph->lockScaleBtnRec.y = graph->mainRec.y + PERF_GRAPH_PAUSE_LOCK_BTNS_MARGIN;
	RecAlign(&graph->lockScaleBtnRec);
	
	graph->pauseBtnRec.size = NewVec2(PERF_GRAPH_PAUSE_LOCK_BTNS_SIZE, PERF_GRAPH_PAUSE_LOCK_BTNS_SIZE);
	graph->pauseBtnRec.x = graph->mainRec.x + PERF_GRAPH_PAUSE_LOCK_BTNS_MARGIN;
	graph->pauseBtnRec.y = graph->mainRec.y + PERF_GRAPH_PAUSE_LOCK_BTNS_MARGIN;
	RecAlign(&graph->pauseBtnRec);
	
	RcBindFont(&pig->resources.debugFont, SelectFontFace(12));
	graph->infoTextPos = NewVec2(graph->mainRec.x, graph->mainRec.y + graph->mainRec.height + 2 + RcGetMaxAscend());
	Vec2Align(&graph->infoTextPos);
}

void PigPerfGraphCaptureMouse(PigPerfGraph_t* graph)
{
	NotNull(graph);
	PigPerfGraphLayout(graph);
	if (graph->enabled)
	{
		MouseHitRecNamed(graph->pauseBtnRec, "PerfGraphPauseBtn");
		MouseHitRecNamed(graph->lockScaleBtnRec, "PerfGraphLockScaleBtn");
		// MouseHitRecNamed(graph->mainRec, "PerfGraph");
	}
}

void PigPerfGraphMark_(PigPerfGraph_t* graph) //pre-declared in pig_func_defs.h
{
	NotNull(graph);
	graph->nextFrameMarked = true;
}

void UpdatePigPerfGraph(PigPerfGraph_t* graph)
{
	NotNull(graph);
	
	#if PAUSE_PERF_GRAPH_ONCE_FULL
	if (!graph->hasAutoPaused && graph->msValues[PERF_GRAPH_LENGTH-2] != 0)
	{
		graph->paused = true;
		graph->hasAutoPaused = true;
	}
	#endif
	
	// +==============================+
	// |        Record Values         |
	// +==============================+
	if (!graph->paused)
	{
		//shift all the values up
		for (u64 vIndex = PERF_GRAPH_LENGTH-1; vIndex > 0; vIndex--)
		{
			graph->msValues[vIndex]            = graph->msValues[vIndex-1];
			graph->updateMsValues[vIndex]      = graph->updateMsValues[vIndex-1];
			graph->waitMsValues[vIndex]        = graph->waitMsValues[vIndex-1];
			graph->readoutProgramTimes[vIndex] = graph->readoutProgramTimes[vIndex-1];
			graph->isFrameMarked[vIndex]       = graph->isFrameMarked[vIndex-1];
		}
		graph->msValues[0] = ElapsedMs;
		graph->updateMsValues[0] = pigIn->lastUpdateElapsedMs;
		graph->waitMsValues[0] = pigIn->timeSpentWaitingLastFrame;
		graph->readoutProgramTimes[0] = ProgramTime;
		graph->isFrameMarked[0] = graph->nextFrameMarked;
		graph->nextFrameMarked = false;
	}
	
	// +==============================+
	// |      Calculate maxValue      |
	// +==============================+
	if (!graph->paused)
	{
		graph->maxValue = 0.0;
		for (u64 vIndex = 0; vIndex < PERF_GRAPH_LENGTH; vIndex++)
		{
			if (graph->msValues[vIndex]       > graph->maxValue) { graph->maxValue = graph->msValues[vIndex];       }
			if (graph->updateMsValues[vIndex] > graph->maxValue) { graph->maxValue = graph->updateMsValues[vIndex]; }
			if (graph->waitMsValues[vIndex]   > graph->maxValue) { graph->maxValue = graph->waitMsValues[vIndex];   }
		}
	}
	
	PigPerfGraphLayout(graph);
	
	// +==============================+
	// |       Handle Pause Btn       |
	// +==============================+
	if (IsMouseOverNamed("PerfGraphPauseBtn"))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			graph->paused = !graph->paused;
		}
	}
	
	// +==============================+
	// |    Handle Lock Scale Btn     |
	// +==============================+
	if (IsMouseOverNamed("PerfGraphLockScaleBtn"))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			graph->lockedScale = !graph->lockedScale;
		}
	}
	
	// +==============================+
	// |     Handle Scroll Wheel      |
	// +==============================+
	if (IsMouseInsideRec(graph->mainRec) && graph->lockedScale)
	{
		if (MouseScrolledY())
		{
			HandleMouseScrollY();
			graph->scale = graph->scale * (1.0f + pigIn->scrollDelta.y * 0.1f);
		}
	}
	
	// +==============================+
	// |         Update Scale         |
	// +==============================+
	if (graph->scale != 0 && !graph->lockedScale)
	{
		if (graph->maxValue > graph->mainRec.height / graph->scale)
		{
			graph->scale = (r32)(graph->mainRec.height / graph->maxValue);
		}
		if (graph->scale < PERF_GRAPH_MAX_SCALE && graph->maxValue <= (graph->mainRec.height / graph->scale) - 1)
		{
			graph->scale *= 1.03f;
		}
	}
}

void RenderPigPerfGraph(PigPerfGraph_t* graph)
{
	NotNull(graph);
	PigPerfGraphLayout(graph);
	RcBindShader(&pig->resources.mainShader2D);
	RcBindFont(&pig->resources.pixelFont, SelectFontFace(8));
	
	if (graph->enabled)
	{
		RcDrawRectangle(graph->mainRec, ColorTransparent(MonokaiDarkGray, 0.5f));
		r32 baseY = graph->mainRec.y + graph->mainRec.height;
		
		r32 targetFramerateY = baseY - (1000.0f / PIG_TARGET_FRAMERATE) * graph->scale;
		rec targetFramerateLine = NewRec(graph->mainRec.x, targetFramerateY, graph->mainRec.width, 1);
		RcDrawRectangle(targetFramerateLine, MonokaiGray1);
		
		if (graph->scale > 0 && !IsInfiniteR32(graph->scale))
		{
			r64 notchSize = 4; //ms
			r64 graphMaxValue = (graph->mainRec.height / graph->scale);
			if (graphMaxValue >= 600)
			{
				notchSize = 200; //ms
			}
			else if (graphMaxValue >= 300)
			{
				notchSize = 100; //ms
			}
			else if (graphMaxValue >= 175)
			{
				notchSize = 50; //ms
			}
			else if (graphMaxValue >= 75)
			{
				notchSize = 25; //ms
			}
			else if (graphMaxValue >= 40)
			{
				notchSize = 10; //ms
			}
			for (r64 notchValue = notchSize; notchValue < graphMaxValue; notchValue += notchSize)
			{
				rec notchRec = NewRec(
					graph->mainRec.x + graph->mainRec.width + 1,
					baseY - (r32)(notchValue * graph->scale),
					5, 1
				);
				v2 textPos = NewVec2(notchRec.x + notchRec.width + 2, notchRec.y - RcGetLineHeight()/2 + RcGetMaxAscend());
				Vec2Align(&textPos);
				RcDrawRectangle(notchRec, MonokaiWhite);
				RcDrawText(TempPrintStr("%.0lf", notchValue), textPos, MonokaiWhite);
				if (IsInsideRec(rc->flowInfo.logicalRec, MousePos))
				{
					rec horizontalRulerRec = NewRec(graph->mainRec.x, notchRec.y, graph->mainRec.width, notchRec.height);
					RcDrawRectangle(horizontalRulerRec, MonokaiGray1);
				}
			}
		}
		
		RcSetViewport(graph->mainRec);
		bool showingMsReadout = false;
		u64 msReadoutIndex = 0;
		r32 drawX = graph->mainRec.x + graph->mainRec.width - PERF_GRAPH_DATA_WIDTH;
		for (u64 vIndex = 1; vIndex < PERF_GRAPH_LENGTH; vIndex++)
		{
			r64 elapsedMs1 = graph->updateMsValues[vIndex];
			r64 elapsedMs2 = graph->updateMsValues[vIndex-1];
			v2 lineLeft = NewVec2(drawX, baseY - (r32)(elapsedMs1 * graph->scale));
			v2 lineRight = NewVec2(drawX + PERF_GRAPH_DATA_WIDTH, baseY - (r32)(elapsedMs2 * graph->scale));
			RcDrawLine(lineLeft, lineRight, 1.0f, MonokaiYellow);
			drawX -= PERF_GRAPH_DATA_WIDTH;
		}
		drawX = graph->mainRec.x + graph->mainRec.width - PERF_GRAPH_DATA_WIDTH;
		for (u64 vIndex = 1; vIndex < PERF_GRAPH_LENGTH; vIndex++)
		{
			r64 elapsedMs1 = graph->msValues[vIndex];
			r64 elapsedMs2 = graph->msValues[vIndex-1];
			v2 lineLeft = NewVec2(drawX, baseY - (r32)(elapsedMs1 * graph->scale));
			v2 lineRight = NewVec2(drawX + PERF_GRAPH_DATA_WIDTH, baseY - (r32)(elapsedMs2 * graph->scale));
			
			if (graph->isFrameMarked[vIndex])
			{
				rec markRec = NewRec(lineLeft.x-1, graph->mainRec.y, 2, graph->mainRec.height);
				RecAlign(&markRec);
				RcDrawRectangle(markRec, MonokaiGray1);
			}
			
			if (!showingMsReadout && IsInsideRec(graph->mainRec, MousePos) && lineLeft.x <= MousePos.x && lineRight.x > MousePos.x)
			{
				bool closerToRight = (MousePos.x >= (lineLeft.x + lineRight.x)/2);
				rec cursorRec = NewRec((closerToRight ? lineRight.x : lineLeft.x)-1, graph->mainRec.y, 2, graph->mainRec.height);
				RecAlign(&cursorRec);
				RcDrawRectangle(cursorRec, MonokaiLightGray);
				msReadoutIndex = closerToRight ? (vIndex-1) : vIndex;
				showingMsReadout = true;
			}
			
			RcDrawLine(lineLeft, lineRight, 1.0f, MonokaiMagenta);
			
			drawX -= PERF_GRAPH_DATA_WIDTH;
		}
		RcSetViewport(NewRec(Vec2_Zero, ScreenSize));
		
		RcDrawRectangleOutline(graph->mainRec, MonokaiWhite, 1, true);
		
		MyStr_t lockIconFrame = (graph->lockedScale ? NewStr("LockIcon") : NewStr("UnlockIcon"));
		RcBindSpriteSheet(&pig->resources.vectorIcons64);
		RcDrawSheetFrame(lockIconFrame, graph->lockScaleBtnRec, ColorTransparent(MonokaiWhite, (IsMouseOverNamed("PerfGraphLockScaleBtn") ? 1.0f : 0.3f)));
		
		MyStr_t playIconFrame = (graph->paused ? NewStr("PlayIcon") : NewStr("PauseIcon"));
		RcBindSpriteSheet(&pig->resources.vectorIcons64);
		RcDrawSheetFrame(playIconFrame, graph->pauseBtnRec, ColorTransparent(MonokaiWhite, (IsMouseOverNamed("PerfGraphPauseBtn") ? 1.0f : 0.3f)));
		
		if (showingMsReadout)
		{
			RcBindFont(&pig->resources.debugFont, SelectFontFace(12));
			RcDrawTextPrint(graph->infoTextPos, MonokaiWhite,
				"CPU: %.2lfms Total: %.2lfms ProgramTime: %llu",
				graph->updateMsValues[msReadoutIndex], graph->msValues[msReadoutIndex], graph->readoutProgramTimes[msReadoutIndex]
			);
		}
	}
}
