/*
File:   pig_audio_out_graph.cpp
Author: Taylor Robbins
Date:   01\15\2022
Description: 
	** Holds a bunch of functions that help us manage and render
	** the debug visual for the audio output.
*/

#define AUDIO_OUT_GRAPH_WIDTH 400 //px
#define AUDIO_OUT_GRAPH_HEIGHT 100 //px

#define AUDIO_OUT_GRAPH_SCROLL_SPEED 0.2f //scale
#define AUDIO_OUT_GRAPH_MAX_SCALE    20 //px/sample
#define AUDIO_OUT_GRAPH_SCALE_LAG    4 //divisor
#define AUDIO_OUT_GRAPH_PANNING_LAG  5 //divisor

void InitPigAudioOutGraph(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	ClearPointer(graph);
	graph->viewCenter = 0.5f;
	graph->viewCenterGoto = graph->viewCenter;
	graph->minScale = 1.0 / ((r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH / (r64)AUDIO_OUT_GRAPH_WIDTH);
	graph->scale = graph->minScale;
	graph->scaleGoto = graph->minScale;
}

void PigAudioOutGraphLoadIcons(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	if (!LoadTexture(mainHeap, &graph->pauseIcon, NewStr(RESOURCE_FOLDER_SPRITES "/perf_graph_pause_icon.png"), false, false))
	{
		DebugAssert_(false);
	}
	if (!LoadTexture(mainHeap, &graph->playIcon, NewStr(RESOURCE_FOLDER_SPRITES "/perf_graph_play_icon.png"), false, false))
	{
		DebugAssert_(false);
	}
	if (!LoadTexture(mainHeap, &graph->resetIcon, NewStr(RESOURCE_FOLDER_SPRITES "/audio_out_graph_reset_icon.png"), false, false))
	{
		DebugAssert_(false);
	}
}

void PigAudioOutGraphLayout(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	graph->mainRec.width = AUDIO_OUT_GRAPH_WIDTH;
	graph->mainRec.height = AUDIO_OUT_GRAPH_HEIGHT;
	graph->mainRec.y = 10;
	RecLayoutLeftOf(&graph->mainRec, ScreenSize.width, 10);
	RecAlign(&graph->mainRec);
	
	graph->pauseBtnRec.size = Vec2Fill(16);
	RecLayoutLeftOf(&graph->pauseBtnRec, graph->mainRec.x + graph->mainRec.width, 5);
	graph->pauseBtnRec.y = graph->mainRec.y + 5;
	RecAlign(&graph->pauseBtnRec);
	
	graph->resetViewBtnRec.size = Vec2Fill(16);
	graph->resetViewBtnRec.topLeft = graph->mainRec.topLeft + NewVec2(5, 5);
	RecAlign(&graph->resetViewBtnRec);
	
	graph->scaleGoto = ClampR64(graph->scaleGoto, graph->minScale, AUDIO_OUT_GRAPH_MAX_SCALE);
	graph->scale = ClampR64(graph->scale, graph->minScale, AUDIO_OUT_GRAPH_MAX_SCALE);
	graph->viewCenter = ClampR64(graph->viewCenter, 0.0, 1.0);
	graph->viewCenterGoto = ClampR64(graph->viewCenterGoto, 0.0, 1.0);
}

void PigAudioOutGraphCaptureMouse(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	PigAudioOutGraphLayout(graph);
	if (graph->enabled)
	{
		MouseHitRecNamed(graph->pauseBtnRec, "AudioOutGraphPauseBtn");
		MouseHitRecNamed(graph->resetViewBtnRec, "AudioOutGraphResetViewBtn");
		MouseHitRecNamed(graph->mainRec, "AudioOutGraph");
	}
}

void UpdatePigAudioOutGraph(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	PigAudioOutGraphLayout(graph);
	
	// +==============================+
	// |    Handle Reset View Btn     |
	// +==============================+
	if (IsMouseOverNamed("AudioOutGraphResetViewBtn"))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			graph->viewCenterGoto = 0.5f;
			graph->scaleGoto = graph->minScale;
		}
	}
	// +==============================+
	// |     Handle Scroll Wheel      |
	// +==============================+
	if (IsMouseOverNamed("AudioOutGraph"))
	{
		if (MouseScrolledY())
		{
			HandleMouseScrollY();
			graph->scaleGoto *= 1.0f + ((pigIn->scrollDelta.y) * AUDIO_OUT_GRAPH_SCROLL_SPEED);
		}
	}
	
	// +==============================+
	// |   Handle Mouse Drag To Pan   |
	// +==============================+
	if (IsMouseOverNamed("AudioOutGraph") || graph->mouseGrabbed)
	{
		r64 viewWidth = ((AUDIO_OUT_GRAPH_WIDTH / graph->scale) / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH); //0-1 range
		r64 viewLeft = graph->viewCenter - viewWidth/2;
		r64 mouseBufferPos = viewLeft + ((MousePos.x - graph->mainRec.x) / graph->mainRec.width) * viewWidth;
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			graph->mouseGrabbed = true;
			graph->mouseGrabPos = mouseBufferPos;
		}
		if (MouseDownRaw(MouseBtn_Left) && graph->mouseGrabbed)
		{
			HandleMouse(MouseBtn_Left);
			r32 mouseRelativeOffset = (MousePos.x - (graph->mainRec.x + graph->mainRec.width/2)) / graph->mainRec.width;
			r64 offsetPos = (mouseRelativeOffset * viewWidth);
			graph->viewCenter = graph->mouseGrabPos - offsetPos;
			graph->viewCenterGoto = graph->viewCenter;
		}
		if (!MouseDownRaw(MouseBtn_Left) && graph->mouseGrabbed)
		{
			HandleMouse(MouseBtn_Left);
			graph->mouseGrabbed = false;
		}
	}
	
	// +==============================+
	// |         Update Scale         |
	// +==============================+
	if (AbsR64(graph->scaleGoto - graph->scale) > (graph->scaleGoto * 0.01))
	{
		graph->scale += (graph->scaleGoto - graph->scale) / AUDIO_OUT_GRAPH_SCALE_LAG;
	}
	else
	{
		graph->scale = graph->scaleGoto;
	}
	
	// +==============================+
	// |      Update View Center      |
	// +==============================+
	{
		r64 viewWidth = ((AUDIO_OUT_GRAPH_WIDTH / graph->scale) / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH); //0-1 range
		r64 pixelPercentWidth = (viewWidth / AUDIO_OUT_GRAPH_WIDTH);
		if (AbsR64(graph->viewCenterGoto - graph->viewCenter) > pixelPercentWidth)
		{
			graph->viewCenter += (graph->viewCenterGoto - graph->viewCenter) / AUDIO_OUT_GRAPH_PANNING_LAG;
		}
		else
		{
			graph->viewCenter = graph->viewCenterGoto;
		}
	}
	
	PigAudioOutGraphLayout(graph);
	
	// +==============================+
	// |       Handle Pause Btn       |
	// +==============================+
	if (IsMouseOverNamed("AudioOutGraphPauseBtn"))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			graph->paused = !graph->paused;
		}
	}
	
}

r32 ConvertSampleIndexToScreenSpace(PigAudioOutGraph_t* graph, u64 sampleIndex)
{
	NotNull(graph);
	r64 viewWidth = ((AUDIO_OUT_GRAPH_WIDTH / graph->scale) / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH); //0-1 range
	r64 viewLeft = graph->viewCenter - viewWidth/2;
	r64 normalizedBufferIndex = (r64)sampleIndex / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH;
	r64 clipSpacePos = (normalizedBufferIndex - viewLeft) / viewWidth;
	r32 screenSpaceX = graph->mainRec.x + ((r32)clipSpacePos * graph->mainRec.width);
	return screenSpaceX;
}

void RenderPigAudioOutGraph(PigAudioOutGraph_t* graph)
{
	NotNull(graph);
	PigAudioOutGraphLayout(graph);
	
	if (graph->enabled)
	{
		RcBindShader(&pig->resources.shaders->main2D);
		RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12));
		
		RcDrawTextPrint(NewVec2(graph->mainRec.x, graph->mainRec.y + graph->mainRec.height + RcGetLineHeight()), White, "Scale %.3f -> %.3f (min %f)", graph->scale, graph->scaleGoto, graph->minScale);
		RcDrawTextPrint(NewVec2(graph->mainRec.x, graph->mainRec.y + graph->mainRec.height + RcGetLineHeight()*2), White, "ViewCenter %.3f -> %.3f", graph->viewCenter, graph->viewCenterGoto);
		
		RcDrawRectangleOutline(graph->mainRec, MonokaiWhite, 1, true);
		
		r64 viewWidth = ((AUDIO_OUT_GRAPH_WIDTH / graph->scale) / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH); //0-1 range
		r64 viewLeft = graph->viewCenter - viewWidth/2;
		
		// +==============================+
		// |   Render Buffer Background   |
		// +==============================+
		{
			rec backRec = graph->mainRec;
			r32 buff6erLeft = ConvertSampleIndexToScreenSpace(graph, 0);
			r32 bufferRight = ConvertSampleIndexToScreenSpace(graph, PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
			if (bufferLeft > backRec.x) { backRec = RecExpandLeft(backRec, -(bufferLeft - backRec.x)); }
			if (bufferRight < backRec.x + backRec.width) { backRec = RecExpandRight(backRec, -((backRec.x + backRec.width) - bufferRight)); }
			RcDrawRectangle(backRec, ColorTransparent(Black, 0.5f));
		}
		
		u64 numSamplesInView = (u64)RoundR64i(viewWidth * PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
		if (graph->scale < 0.4)
		{
			u64 numSamplesPerBar = numSamplesInView / AUDIO_OUT_GRAPH_WIDTH;
			if (numSamplesPerBar < 1) { numSamplesPerBar = 1; }
			for (u64 pIndex = 0; pIndex < AUDIO_OUT_GRAPH_WIDTH; pIndex++)
			{
				i64 sampleIndex = FloorR64i((viewLeft + (viewWidth / (r64)AUDIO_OUT_GRAPH_WIDTH) * (r64)pIndex) * PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
				r64 sampleMax = 0.0;
				r64 sampleMin = 0.0;
				for (u64 sIndex = 0; sIndex < numSamplesPerBar; sIndex++)
				{
					u64 thisSampleIndex = (sampleIndex + sIndex);
					if (thisSampleIndex >= 0 && thisSampleIndex < PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH)
					{
						sampleMin = MinR64(pig->audioOutSamples[thisSampleIndex], sampleMin);
						sampleMax = MaxR64(pig->audioOutSamples[thisSampleIndex], sampleMax);
					}
				}
				sampleMax = ConvertVolumeToLoudness(AbsR64(sampleMax)) * SignR64(sampleMax);
				sampleMin = ConvertVolumeToLoudness(AbsR64(sampleMin)) * SignR64(sampleMin);
				rec sampleBarRec = NewRec(graph->mainRec.x + (r32)pIndex, 0, 1, graph->mainRec.height);
				sampleBarRec.height *= ((r32)(sampleMax - sampleMin) / 2);
				sampleBarRec.y = graph->mainRec.y + (graph->mainRec.height * (r32)(1 - (sampleMax + 1)/2));
				RecAlign(&sampleBarRec);
				RcDrawRectangle(sampleBarRec, MonokaiYellow);
			}
		}
		else
		{
			i64 baseSampleIndex = FloorR64i(viewLeft * PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
			v2 prevSamplePos = Vec2_Zero;
			for (u64 sIndex = 0; sIndex <= numSamplesInView; sIndex++)
			{
				i64 sampleIndex = baseSampleIndex + sIndex;
				r64 sampleValue = 0.0;
				if (sampleIndex >= 0 && (u64)sampleIndex < PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH)
				{
					sampleValue = pig->audioOutSamples[sampleIndex];
				}
				
				r64 sampleNormalizedIndex = ((r64)sampleIndex / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
				v2 samplePos = NewVec2(
					graph->mainRec.x + (r32)(((sampleNormalizedIndex - viewLeft) / viewWidth) * AUDIO_OUT_GRAPH_WIDTH),
					graph->mainRec.y + graph->mainRec.height/2 - (r32)(graph->mainRec.height * (sampleValue / 2.0))
				);
				if (sIndex > 0)
				{
					RcDrawLine(prevSamplePos, samplePos, 1, MonokaiYellow);
				}
				prevSamplePos = samplePos;
			}
		}
		
		// +==============================+
		// |      Draw Write Cursor       |
		// +==============================+
		r64 writeCursorNormalizedIndex = ((r64)pig->audioOutWriteIndex / (r64)PIG_AUDIO_OUT_SAMPLES_BUFFER_LENGTH);
		if (writeCursorNormalizedIndex >= viewLeft && writeCursorNormalizedIndex< viewLeft + viewWidth)
		{
			rec writeCursorRec = NewRec(
				graph->mainRec.x + (r32)((writeCursorNormalizedIndex - viewLeft) / viewWidth) * graph->mainRec.width,
				graph->mainRec.y,
				1,
				graph->mainRec.height
			);
			RecAlign(&writeCursorRec);
			RcDrawRectangle(writeCursorRec, MonokaiRed);
		}
		
		// +==============================+
		// |       Render Pause Btn       |
		// +==============================+
		{
			Color_t btnColor = ColorTransparent(White, (IsMouseOverNamed("AudioOutGraphPauseBtn") ? 1.0f : 0.5f));
			RcBindTexture1(graph->paused ? &graph->playIcon : &graph->pauseIcon);
			RcDrawTexturedRectangle(graph->pauseBtnRec, btnColor);
		}
		
		// +==============================+
		// |    Render Reset View Btn     |
		// +==============================+
		{
			Color_t btnColor = ColorTransparent(White, (IsMouseOverNamed("AudioOutGraphResetViewBtn") ? 1.0f : 0.5f));
			RcBindTexture1(&graph->resetIcon);
			RcDrawTexturedRectangle(graph->resetViewBtnRec, btnColor);
		}
	}
}
