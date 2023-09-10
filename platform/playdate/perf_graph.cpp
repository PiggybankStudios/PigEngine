/*
File:   perf_graph.cpp
Author: Taylor Robbins
Date:   09\09\2023
Description: 
	** A little line graph of elapsed ms for each frame over the past few seconds
*/

void InitPerfGraph(PerfGraph_t* graph)
{
	NotNull(graph);
	ClearPointer(graph);
	graph->enabled = false;
	graph->paused = false;
	graph->mainRec = NewReci(2, 2, PERF_GRAPH_WIDTH * PERF_TICK_WIDTH, PERF_GRAPH_DISPLAY_HEIGHT);
	graph->font = LoadFont(NewStr(PERF_GRAPH_FONT_PATH));
	Assert(graph->font.isValid);
}

void UpdatePerfGraph(PerfGraph_t* graph)
{
	NotNull(graph);
	
	//Shift all the values up
	for (u64 vIndex = PERF_GRAPH_WIDTH-1; vIndex > 0; vIndex--)
	{
		graph->values[vIndex] = graph->values[vIndex-1];
	}
	graph->values[0] = ElapsedMs;
}

void RenderPerfGraph(PerfGraph_t* graph)
{
	NotNull(graph);
	if (graph->enabled)
	{
		i32 graphBottom = graph->mainRec.y + graph->mainRec.height;
		
		// PdDrawRec(graph->mainRec, kColorWhite);
		
		r32 scaleY = (r32)graph->mainRec.height / (r32)(2*TARGET_FRAMERATE);
		i32 targetLineY = graphBottom - RoundR32i(TARGET_FRAMERATE * scaleY);
		PdDrawRec(NewReci(graph->mainRec.x, targetLineY, graph->mainRec.width, 2), kColorXOR);
		
		v2i prevPos = Vec2i_Zero;
		for (i32 vIndex = 0; vIndex < PERF_GRAPH_WIDTH; vIndex++)
		{
			v2i newPos = NewVec2i(
				graph->mainRec.x + vIndex * PERF_TICK_WIDTH,
				graphBottom - RoundR32i(graph->values[vIndex] * scaleY)
			);
			if (vIndex > 0)
			{
				PdDrawLine(prevPos, newPos, 2, kColorXOR);
			}
			prevPos = newPos;
		}
		
		PdDrawRecOutline(graph->mainRec, 2, true, kColorXOR);
		
		PdBindFont(&graph->font);
		LCDBitmapDrawMode oldDrawMode = PdSetDrawMode(kDrawModeNXOR);
		v2i textPos = NewVec2i(graph->mainRec.x + graph->mainRec.width + 3, graph->mainRec.y);
		i32 advanceY = boundFont->lineHeight + 1;
		
		pd->system->drawFPS(textPos.x, textPos.y);
		textPos.y += 15;
		
		PdDrawTextPrint(textPos, "%dms", RoundR32i(graph->values[0]));
		textPos.y += advanceY;
		
		PdDrawTextPrint(textPos, "%.2f", TimeScale);
		textPos.y += advanceY;
		
		PdSetDrawMode(oldDrawMode);
	}
}
