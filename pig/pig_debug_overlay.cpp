/*
File:   pig_debug_overlay.cpp
Author: Taylor Robbins
Date:   01\06\2022
Description: 
	** Holds the code that renders the debug overlay (has a bunch of information about the state of the game+engine+platform)
*/

#define DEBUG_OVERLAY_TOGGLE_KEY                Key_F3
#define DEBUG_OVERLAY_TOGGLE_KEY_REQUIRES_SHIFT true

#define DEBUG_OVERLAY_OPEN_ANIM_TIME         400 //ms
#define DEBUG_OVERLAY_HIDE_TOGGLE_BTNS_TIME  5000 //ms

#define DEBUG_OVERLAY_DUCK_PERCENT           0.1f //the amount it "ducks" out of the way when the mouse hasn't been near it in a while

#define DEBUG_OVERLAY_PIE_CHARTS_SIZE   150 //px
#define DEBUG_OVERLAY_TOGGLE_BTNS_SIZE  30 //px

void InitializePigDebugOverlay(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	ClearPointer(overlay);
	overlay->enabled = (SHOW_PERF_GRAPH_ON_STARTUP || SHOW_MEM_GRAPH_ON_STARTUP || SHOW_PIE_GRAPHS_ON_STARTUP || SHOW_AUDIO_OUT_GRAPH_ON_STARTUP || SHOW_AUDIO_INSTANCES_ON_STARTUP);
	overlay->pieChartsEnabled = SHOW_PIE_GRAPHS_ON_STARTUP;
	overlay->audioInstancesEnabled = (overlay->enabled ? SHOW_AUDIO_INSTANCES_ON_STARTUP : true);
	overlay->perfGraphWasEnabled = true;
}

void PigDebugOverlayLayout(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	//TODO: Layout the mainRec
	overlay->initPieChartRec.size = NewVec2(DEBUG_OVERLAY_PIE_CHARTS_SIZE, DEBUG_OVERLAY_PIE_CHARTS_SIZE);
	RecLayoutLeftOf(&overlay->initPieChartRec, ScreenSize.width, 10);
	RecLayoutTopOf(&overlay->initPieChartRec, ScreenSize.height, 10);
	RecAlign(&overlay->initPieChartRec);
	
	v2 toggleBtnSize = NewVec2(DEBUG_OVERLAY_TOGGLE_BTNS_SIZE, DEBUG_OVERLAY_TOGGLE_BTNS_SIZE);
	overlay->totalToggleBtnsRec.size = NewVec2(toggleBtnSize.width * ArrayCount(overlay->toggleBtnRecs), toggleBtnSize.height);
	overlay->totalToggleBtnsRec.x = ScreenSize.width/2 - overlay->totalToggleBtnsRec.width/2;
	overlay->totalToggleBtnsRec.y = 0 - (overlay->totalToggleBtnsRec.height * EaseQuadraticIn(1.0f - overlay->openAnimTime));
	RecAlign(&overlay->totalToggleBtnsRec);
	
	rec toggleBtnBaseRec = NewRec(overlay->totalToggleBtnsRec.topLeft, toggleBtnSize);
	RecAlign(&toggleBtnBaseRec);
	for (u64 bIndex = 0; bIndex < ArrayCount(overlay->toggleBtnRecs); bIndex++)
	{
		overlay->toggleBtnRecs[bIndex] = toggleBtnBaseRec;
		RecAlign(&overlay->toggleBtnRecs[bIndex]);
		toggleBtnBaseRec.x += toggleBtnBaseRec.width;
	}
}
void PigDebugOverlayCaptureMouse(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	PigDebugOverlayLayout(overlay);
	if (overlay->enabled)
	{
		for (u64 bIndex = 0; bIndex < ArrayCount(overlay->toggleBtnRecs); bIndex++)
		{
			MouseHitRecPrint(overlay->toggleBtnRecs[bIndex], "DebugOverlayToggleBtn%llu", bIndex);
		}
	}
}

void UpdatePigDebugOverlay(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	PigDebugOverlayLayout(overlay);
	
	overlay->physicsSimTimeLastFrame = pig->physicsSimTimeLastFrame;
	
	// +==============================+
	// |    Handle Toggle Buttons     |
	// +==============================+
	if (overlay->enabled)
	{
		for (u64 bIndex = 0; bIndex < ArrayCount(overlay->toggleBtnRecs); bIndex++)
		{
			rec btnRec = overlay->toggleBtnRecs[bIndex];
			if (IsMouseOverPrint("DebugOverlayToggleBtn%llu", bIndex))
			{
				pigOut->cursorType = PlatCursor_Pointer;
				if (MousePressedAndHandleExtended(MouseBtn_Left))
				{
					switch (bIndex)
					{
						case 0: overlay->debugReadoutsEnabled   = !overlay->debugReadoutsEnabled;   break; //toggleDebugReadoutBtnRec
						case 1: pig->perfGraph.enabled          = !pig->perfGraph.enabled;          break; //togglePerfGraphBtnRec
						case 2: pig->audioOutGraph.enabled      = !pig->audioOutGraph.enabled;      break; //toggleAudioGraphBtnRec
						case 3: overlay->audioInstancesEnabled  = !overlay->audioInstancesEnabled;  break; //toggleAudioInstancesBtnRec
						case 4: pig->memGraph.enabled           = !pig->memGraph.enabled;           break; //toggleMemGraphBtnRec
						case 5: overlay->pieChartsEnabled       = !overlay->pieChartsEnabled;       break; //togglePieChartsBtnRec
						case 6: overlay->easingFuncsEnabled     = !overlay->easingFuncsEnabled;     break; //toggleEasingFuncsBtnRec
						case 7: overlay->controllerDebugEnabled = !overlay->controllerDebugEnabled; break; //toggleContollerDebugBtnRec
						default: DebugAssert(false); break;
					}
				}
			}
		}
	}
	
	// +==============================+
	// | Handle Debug Overlay Hotkey  |
	// +==============================+
	if (KeyReleased(DEBUG_OVERLAY_TOGGLE_KEY) || (!overlay->enabled && KeyPressed(DEBUG_OVERLAY_TOGGLE_KEY)))
	{
		HandleKey(DEBUG_OVERLAY_TOGGLE_KEY);
		if (!overlay->enabled || !overlay->hotkeyPlusNumberPressed)
		{
			overlay->lastMouseCloseTime = ProgramTime;
			overlay->hotkeyPlusNumberPressed = true;
			overlay->enabled = !overlay->enabled;
			if (!overlay->enabled)
			{
				overlay->perfGraphWasEnabled      = pig->perfGraph.enabled;
				overlay->memGraphWasEnabled       = pig->memGraph.enabled;
				overlay->audioOutGraphWasEnabled  = pig->audioOutGraph.enabled;
				pig->perfGraph.enabled     = false;
				pig->audioOutGraph.enabled = false;
				pig->memGraph.enabled      = false;
			}
			else
			{
				pig->perfGraph.enabled     = overlay->perfGraphWasEnabled;
				pig->memGraph.enabled      = overlay->memGraphWasEnabled;
				pig->audioOutGraph.enabled = overlay->audioOutGraphWasEnabled;
			}
		}
	}
	if (KeyPressed(DEBUG_OVERLAY_TOGGLE_KEY)) { HandleKey(DEBUG_OVERLAY_TOGGLE_KEY); overlay->hotkeyPlusNumberPressed = false; }
	if (KeyDownRaw(DEBUG_OVERLAY_TOGGLE_KEY))
	{
		HandleKey(DEBUG_OVERLAY_TOGGLE_KEY);
		if (KeyPressed(Key_1)) { HandleKey(Key_1); overlay->debugReadoutsEnabled   = !overlay->debugReadoutsEnabled;   overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_2)) { HandleKey(Key_2); pig->perfGraph.enabled          = !pig->perfGraph.enabled;          overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_3)) { HandleKey(Key_3); pig->audioOutGraph.enabled      = !pig->audioOutGraph.enabled;      overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_4)) { HandleKey(Key_4); overlay->audioInstancesEnabled  = !overlay->audioInstancesEnabled;  overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_5)) { HandleKey(Key_5); pig->memGraph.enabled           = !pig->memGraph.enabled;           overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_6)) { HandleKey(Key_6); overlay->pieChartsEnabled       = !overlay->pieChartsEnabled;       overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_7)) { HandleKey(Key_7); overlay->easingFuncsEnabled     = !overlay->easingFuncsEnabled;     overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_8)) { HandleKey(Key_8); overlay->controllerDebugEnabled = !overlay->controllerDebugEnabled; overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		
		if (KeyPressed(Key_Num1)) { HandleKey(Key_Num1); overlay->debugReadoutsEnabled   = !overlay->debugReadoutsEnabled;   overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num2)) { HandleKey(Key_Num2); pig->perfGraph.enabled          = !pig->perfGraph.enabled;          overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num3)) { HandleKey(Key_Num3); pig->audioOutGraph.enabled      = !pig->audioOutGraph.enabled;      overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num4)) { HandleKey(Key_Num4); overlay->audioInstancesEnabled  = !overlay->audioInstancesEnabled;  overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num5)) { HandleKey(Key_Num5); pig->memGraph.enabled           = !pig->memGraph.enabled;           overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num6)) { HandleKey(Key_Num6); overlay->pieChartsEnabled       = !overlay->pieChartsEnabled;       overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num7)) { HandleKey(Key_Num7); overlay->easingFuncsEnabled     = !overlay->easingFuncsEnabled;     overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_Num8)) { HandleKey(Key_Num8); overlay->controllerDebugEnabled = !overlay->controllerDebugEnabled; overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
	}
	
	// +==============================+
	// |  Handle Mouse Getting Close  |
	// +==============================+
	rec closeMouseRec = RecInflate(overlay->totalToggleBtnsRec, 100, 100);
	if (IsInsideRec(closeMouseRec, MousePos))
	{
		overlay->lastMouseCloseTime = ProgramTime;
	}
	
	// +==============================+
	// |    Update Open Anim Time     |
	// +==============================+
	if (overlay->enabled)
	{
		if (TimeSince(overlay->lastMouseCloseTime) < DEBUG_OVERLAY_HIDE_TOGGLE_BTNS_TIME)
		{
			UpdateAnimationUp(&overlay->openAnimTime, DEBUG_OVERLAY_OPEN_ANIM_TIME);
		}
		else if (overlay->openAnimTime >= DEBUG_OVERLAY_DUCK_PERCENT)
		{
			UpdateAnimationDownTo(&overlay->openAnimTime, DEBUG_OVERLAY_OPEN_ANIM_TIME, DEBUG_OVERLAY_DUCK_PERCENT);
		}
		else
		{
			UpdateAnimationUpTo(&overlay->openAnimTime, DEBUG_OVERLAY_OPEN_ANIM_TIME, DEBUG_OVERLAY_DUCK_PERCENT);
		}
	}
	else
	{
		UpdateAnimationDown(&overlay->openAnimTime, DEBUG_OVERLAY_OPEN_ANIM_TIME);
	}
}

void RenderDebugOverlayControllerState(PigDebugOverlay_t* overlay, const PlatControllerState_t* controller, rec drawRec)
{
	UNUSED(overlay);
	RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12));
	
	RcDrawRectangleOutline(drawRec, White, 1);
	
	if (controller->connected)
	{
		for (u64 bIndex = ControllerBtn_NumBtns; bIndex > 0; bIndex--)
		{
			const PlatBtnState_t* btnState = &controller->btnStates[bIndex-1];
			ControllerBtn_t controllerBtn = (ControllerBtn_t)(bIndex-1);
			rec btnRec = Rec_Zero;
			r32 unitSize = 0.1f;
			v2i btnFrame = NewVec2i(0, 0);
			switch (controllerBtn)
			{
				case ControllerBtn_A:            btnRec = NewRec(8*unitSize, 4*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_B:            btnRec = NewRec(9*unitSize, 3*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_X:            btnRec = NewRec(7*unitSize, 3*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_Y:            btnRec = NewRec(8*unitSize, 2*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_Right:        btnRec = NewRec(2*unitSize, 3*unitSize, unitSize, unitSize); break;
				case ControllerBtn_Left:         btnRec = NewRec(0*unitSize, 3*unitSize, unitSize, unitSize); break;
				case ControllerBtn_Up:           btnRec = NewRec(1*unitSize, 2*unitSize, unitSize, unitSize); break;
				case ControllerBtn_Down:         btnRec = NewRec(1*unitSize, 4*unitSize, unitSize, unitSize); break;
				case ControllerBtn_LeftBumper:   btnRec = NewRec(2*unitSize, 1*unitSize, unitSize*2, unitSize); break;
				case ControllerBtn_RightBumper:  btnRec = NewRec(6*unitSize, 1*unitSize, unitSize*2, unitSize); break;
				case ControllerBtn_LeftTrigger:  btnRec = NewRec(2*unitSize, 0*unitSize, unitSize*2, unitSize); break;
				case ControllerBtn_RightTrigger: btnRec = NewRec(6*unitSize, 0*unitSize, unitSize*2, unitSize); break;
				case ControllerBtn_LeftStick:    btnRec = NewRec(2.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_RightStick:   btnRec = NewRec(6.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); btnFrame = NewVec2i(1, 0); break;
				case ControllerBtn_Start:        btnRec = NewRec(5*unitSize, 3*unitSize, unitSize, unitSize/2); break;
				case ControllerBtn_Back:         btnRec = NewRec(4*unitSize, 3*unitSize, unitSize, unitSize/2); break;
				case ControllerBtn_lsRight:      btnRec = NewRec(3.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_lsLeft:       btnRec = NewRec(1.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_lsUp:         btnRec = NewRec(2.5f*unitSize, 4.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_lsDown:       btnRec = NewRec(2.5f*unitSize, 6.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_rsRight:      btnRec = NewRec(7.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_rsLeft:       btnRec = NewRec(5.5f*unitSize, 5.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_rsUp:         btnRec = NewRec(6.5f*unitSize, 4.5f*unitSize, unitSize, unitSize); break;
				case ControllerBtn_rsDown:       btnRec = NewRec(6.5f*unitSize, 6.5f*unitSize, unitSize, unitSize); break;
				default: continue;
			}
			btnRec.topLeft = drawRec.topLeft + Vec2Multiply(btnRec.topLeft, drawRec.size);
			btnRec.size = Vec2Multiply(btnRec.size, drawRec.size);
			RecAlign(&btnRec);
			v2 btnCenter = btnRec.topLeft + btnRec.size/2;
			v2 btnTextPos = NewVec2(btnRec.x + btnRec.width/2, btnRec.y + btnRec.height + RcGetMaxAscend());
			Vec2Align(&btnTextPos);
			
			Color_t btnColor = MonokaiGray2;
			if (btnState->isDown)
			{
				btnColor = ColorLerp(MonokaiYellow, MonokaiOrange, ClampR32(TimeSince(btnState->lastChangeTime) / 300.0f, 0, 1));
				// RcDrawTextPrintEx(btnTextPos, White, TextAlignment_Center, 0, "%llu", btnState->lastChangeTime);
			}
			// Color_t outlineColor = (btnState->numReleases > 0) ? MonokaiWhite : MonokaiGray1;
			
			RcBindSpriteSheet(&pig->resources.sheets->controllerBtns);
			RcDrawSheetFrame(btnFrame, btnRec, btnColor);
			// RcDrawRectangleOutline(btnRec, outlineColor, 1);
			// RcDrawRectangle(btnRec, btnColor);
			
			if (controllerBtn == ControllerBtn_LeftStick || controllerBtn == ControllerBtn_RightStick)
			{
				rec analogRec = NewRecCentered(btnRec.topLeft + btnRec.size/2, btnRec.size*2);
				v2 analogValue    = (controllerBtn == ControllerBtn_LeftStick) ? controller->leftStick    : controller->rightStick;
				v2 analogValueRaw = (controllerBtn == ControllerBtn_LeftStick) ? controller->leftStickRaw : controller->rightStickRaw;
				RcDrawLine(btnCenter, btnCenter + Vec2Multiply(analogValue, analogRec.size/2), 2, (Vec2Length(analogValue) > 1.001f) ? MonokaiRed : MonokaiYellow);
			}
			if (controllerBtn == ControllerBtn_LeftTrigger || controllerBtn == ControllerBtn_RightTrigger)
			{
				bool isRightTrigger = (controllerBtn == ControllerBtn_RightTrigger);
				r32 analogValue = isRightTrigger ? controller->rightTrigger : controller->leftTrigger;
				rec analogRec = NewRec(btnRec.x - 2, btnRec.y, 2, btnRec.height*2);
				if (isRightTrigger) { analogRec.x += btnRec.width + 2; }
				if (analogValue >= 0.0f && analogValue <= 1.0f)
				{
					analogRec.height *= analogValue;
					RecAlign(&analogRec);
					RcDrawRectangle(analogRec, MonokaiYellow);
				}
				else
				{
					v2 textPos = NewVec2(analogRec.x, analogRec.y + analogRec.height/2);
					RcDrawTextPrintEx(textPos, MonokaiWhite, (isRightTrigger ? TextAlignment_Left : TextAlignment_Right), 0, "%f", analogValue);
				}
			}
		}
		
		v2 textPos = NewVec2(drawRec.x, drawRec.y + drawRec.height*0.75f + RcGetMaxAscend());
		RcDrawTextPrint(textPos, MonokaiWhite,
			"%.*s\n"
			"%s (%llu btns, %llu axes)\n"
			"%.*s",
			controller->name.length, controller->name.pntr,
			GetControllerTypeStr(controller->type), controller->numButtons, controller->numAxes,
			controller->typeIdStr.length, controller->typeIdStr.pntr
		);
	}
	else
	{
		// void RcDrawText(const char* str, v2 position, Color_t color, TextAlignment_t alignment = TextAlignment_Left, r32 maxWidth = 0)
		RcDrawText("Disconnected...", drawRec.topLeft + drawRec.size/2, MonokaiWhite, TextAlignment_Center);
	}
}

void RenderPigDebugOverlayBelowConsole(PigDebugOverlay_t* overlay)
{
	// +==================================+
	// | Render Cyclic Functions Overlay  |
	// +==================================+
	if (pig->cyclicFuncsDebug)
	{
		const u64 numVertices = 250;
		const r32 graphWidth  = 6; //-5 to +5
		
		if (KeyPressed(Key_PageUp))
		{
			HandleKeyExtended(Key_PageUp);
			pig->cyclicFunc.type = (CyclicFuncType_t)((u64)pig->cyclicFunc.type + 1);
			if (pig->cyclicFunc.type >= CyclicFuncType_NumTypes)
			{
				pig->cyclicFunc.type = CyclicFuncType_Constant;
			}
		}
		if (KeyPressed(Key_PageDown))
		{
			HandleKeyExtended(Key_PageDown);
			if (pig->cyclicFunc.type > CyclicFuncType_Constant)
			{
				pig->cyclicFunc.type = (CyclicFuncType_t)((u64)pig->cyclicFunc.type - 1);
			}
			else
			{
				pig->cyclicFunc.type = (CyclicFuncType_t)(CyclicFuncType_NumTypes - 1);
			}
		}
		if (KeyDown(Key_Up))
		{
			HandleKey(Key_Up);
			if (KeyDownRaw(Key_Shift))
			{
				pig->cyclicFunc.constant += 0.01f;
			}
			else
			{
				pig->cyclicFunc.amplitude += 0.01f;
			}
		}
		if (KeyDown(Key_Down))
		{
			HandleKey(Key_Down);
			if (KeyDownRaw(Key_Shift))
			{
				pig->cyclicFunc.constant -= 0.01f;
			}
			else
			{
				pig->cyclicFunc.amplitude -= 0.01f;
			}
		}
		if (KeyDown(Key_Right))
		{
			HandleKey(Key_Right);
			if (KeyDownRaw(Key_Shift))
			{
				pig->cyclicFunc.offset += 0.01f;
			}
			else
			{
				pig->cyclicFunc.period += 0.01f;
			}
		}
		if (KeyDown(Key_Left))
		{
			HandleKey(Key_Left);
			if (KeyDownRaw(Key_Shift))
			{
				pig->cyclicFunc.offset -= 0.01f;
			}
			else
			{
				pig->cyclicFunc.period -= 0.01f;
			}
		}
		
		rec funcRec = NewRec(20, ScreenSize.height/2 - 200, ScreenSize.width - 20*2, 400);
		RecAlign(&funcRec);
		rec baselineRec = NewRec(funcRec.x, (funcRec.y + funcRec.height/2) - 0.5f, funcRec.width, 1);
		RecAlign(&baselineRec); //TODO: Should we align this?
		//TODO: Calculate some vertical markers to show us where whole numbers are
		
		RcDrawRectangle(funcRec, ColorTransparent(Black, 0.5f));
		RcDrawRectangleOutline(funcRec, MonokaiWhite, 1, true);
		RcDrawRectangle(baselineRec, MonokaiGray2);
		
		for (i32 wholeNumberIndex = 0; wholeNumberIndex < FloorR32i(graphWidth)/2; wholeNumberIndex++)
		{
			rec posNumberBar = NewRec(
				funcRec.x + funcRec.width/2 + ((funcRec.width / graphWidth) * wholeNumberIndex),
				funcRec.y, 1, funcRec.height
			);
			RecAlign(&posNumberBar);
			RcDrawRectangle(posNumberBar, MonokaiGray2);
			if (wholeNumberIndex > 0)
			{
				rec negNumberBar = NewRec(
					funcRec.x + funcRec.width/2 - ((funcRec.width / graphWidth) * wholeNumberIndex),
					funcRec.y, 1, funcRec.height
				);
				RecAlign(&negNumberBar);
				RcDrawRectangle(negNumberBar, MonokaiGray2);
			}
		}
		
		RcBindFont(&pig->resources.fonts->pixel, SelectDefaultFontFace());
		v2 previousVert = Vec2_Zero;
		for (u64 xIndex = 0; xIndex <= numVertices; xIndex++)
		{
			r32 t = (((r32)xIndex / (r32)numVertices) - 0.5f) * graphWidth;
			r32 y = CyclicFuncGetValue(pig->cyclicFunc, t);
			r32 screenX = funcRec.x + ((r32)xIndex / (r32)numVertices) * funcRec.width;
			r32 screenY = funcRec.y + (funcRec.height * ((1.0f - y) / 2));
			v2 screenVert = NewVec2(screenX, screenY);
			if (xIndex > 0)
			{
				RcDrawLine(previousVert, screenVert, 2, MonokaiRed);
			}
			previousVert = screenVert;
			// if ((xIndex % 5) == 0) { RcDrawTextPrint(Vec2Round(screenVert), MonokaiWhite, "[%u] %g=%g", xIndex, t, y); }
		}
		
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
		v2 textPos = NewVec2(funcRec.x, funcRec.y - 4 - RcGetMaxDescend());
		r32 textLineHeight = RcGetLineHeight() + 4;
		RcDrawTextPrint(textPos, MonokaiWhite, "Constant:  %g", pig->cyclicFunc.constant);  textPos.y -= textLineHeight;
		RcDrawTextPrint(textPos, MonokaiWhite, "Offset:    %g", pig->cyclicFunc.offset);    textPos.y -= textLineHeight;
		RcDrawTextPrint(textPos, MonokaiWhite, "Amplitude: %g", pig->cyclicFunc.amplitude); textPos.y -= textLineHeight;
		RcDrawTextPrint(textPos, MonokaiWhite, "Period:    %g", pig->cyclicFunc.period);    textPos.y -= textLineHeight;
		RcDrawTextPrint(textPos, MonokaiWhite, "Type:   %s", GetCyclicFuncTypeStr(pig->cyclicFunc.type)); textPos.y -= textLineHeight;
	}
	
	// +===============================+
	// | Render Monitors Debug Overlay |
	// +===============================+
	if (pig->monitorsDebug)
	{
		AccessResource(&pig->resources.textures->blueGradientBack);
		
		const r32 renderMargin = 25; //px
		r32 renderScale = MinR32((ScreenSize.width - renderMargin*2) / (r32)platInfo->monitors->desktopRec.width, (ScreenSize.height - renderMargin*2) / (r32)platInfo->monitors->desktopRec.height);
		rec desktopRec = NewRec(renderMargin, renderMargin, ToVec2(platInfo->monitors->desktopRec.size) * renderScale);
		// RcDrawRectangle(desktopRec, MonokaiGray1);
		
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
		const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&platInfo->monitors->list, PlatMonitorInfo_t);
		for (u64 mIndex = 0; mIndex < platInfo->monitors->list.count; mIndex++)
		{
			const PlatMonitorVideoMode_t* currentVideoMode = VarArrayGet(&monitorInfo->videoModes, monitorInfo->currentVideoModeIndex, PlatMonitorVideoMode_t);
			v2i currentResolution = currentVideoMode->resolution;
			Assert(currentVideoMode->currentFramerateIndex < ArrayCount(currentVideoMode->framerates));
			i64 currentFramerate = currentVideoMode->framerates[currentVideoMode->currentFramerateIndex];
			
			rec monitorRec = NewRec(
				desktopRec.topLeft + ToVec2(monitorInfo->desktopSpaceRec.topLeft - platInfo->monitors->desktopRec.topLeft) * renderScale,
				ToVec2(monitorInfo->desktopSpaceRec.size) * renderScale
			);
			v2 infoTextPos = monitorRec.topLeft + NewVec2(5, 5 + RcGetMaxAscend());
			Vec2Align(&infoTextPos);
			
			Color_t textColor = MonokaiWhite;
			Color_t monitorColor = MonokaiGray2;
			if (monitorInfo->isPrimary) { textColor = Black; monitorColor = MonokaiWhite; }
			
			// RcDrawRectangle(monitorRec, ColorTransparent(monitorColor, 0.4f));
			RcBindTexture1(&pig->resources.textures->blueGradientBack);
			RcDrawTexturedRectangle(monitorRec, ColorTransparent(monitorColor, 0.4f));
			RcDrawRectangleOutline(monitorRec, textColor, 3);
			
			rec oldViewport = rc->state.viewportRec;
			RcSetViewport(monitorRec);
			
			RcDrawTextPrint(infoTextPos, textColor,
				"Name: \"%.*s\"%s\n"
				"Resolution: %dx%d (%lldHz)\n"
				"Number: %llu\n"
				"NumVideoModes: %llu\n",
				monitorInfo->name.length, monitorInfo->name.pntr, (monitorInfo->isPrimary ? " (Primary)" : ""),
				currentResolution.width, currentResolution.height, currentFramerate,
				monitorInfo->designatedNumber,
				monitorInfo->videoModes.length
			);
			
			RcSetViewport(oldViewport);
			
			monitorInfo = LinkedListNext(&platInfo->monitors->list, PlatMonitorInfo_t, monitorInfo);
		}
		
		const PlatWindow_t* window = LinkedListFirst(platInfo->windows, PlatWindow_t);
		for (u64 wIndex = 0; wIndex < platInfo->windows->count; wIndex++)
		{
			if (!window->closed)
			{
				const PlatMonitorInfo_t* currentMonitorInfo = GetCurrentMonitorInfoForWindow(window);
				NotNull(currentMonitorInfo);
				
				rec windowDrawRec = NewRec(
					desktopRec.topLeft + ToVec2(window->input.desktopRec.topLeft - platInfo->monitors->desktopRec.topLeft) * renderScale,
					ToVec2(window->input.desktopRec.size) * renderScale
				);
				RecAlign(&windowDrawRec);
				rec renderDrawRec = NewRec(
					desktopRec.topLeft + ToVec2(window->input.desktopInnerRec.topLeft - platInfo->monitors->desktopRec.topLeft) * renderScale,
					ToVec2(window->input.desktopInnerRec.size) * renderScale
				);
				RecAlign(&renderDrawRec);
				v2 windowInfoTextPos = windowDrawRec.topLeft + windowDrawRec.size + NewVec2(-5, -5 - RcGetMaxDescend() - 4*RcGetLineHeight());
				Vec2Align(&windowInfoTextPos);
				
				RcDrawRectangleOutline(renderDrawRec, MonokaiOrange, 1);
				RcDrawRectangleOutline(windowDrawRec, MonokaiMagenta, 2);
				
				RcDrawTextPrintEx(windowInfoTextPos, MonokaiMagenta, TextAlignment_Right, 0,
					"Window[%llu]\n"
					"Resolution: %gx%g\n"
					"DesktopRec: (%d, %d, %d, %d)\n"
					"DesktopInnerRec: (%d, %d, %d, %d)\n"
					"MonitorNumber: %llu",
					wIndex,
					window->input.renderResolution.width, window->input.renderResolution.height,
					window->input.desktopRec.x, window->input.desktopRec.y, window->input.desktopRec.width, window->input.desktopRec.height,
					window->input.desktopInnerRec.x, window->input.desktopInnerRec.y, window->input.desktopInnerRec.width, window->input.desktopInnerRec.height,
					currentMonitorInfo->designatedNumber
				);
				window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
			}
		}
		
		RcDrawRectangleOutline(desktopRec, Black, 2);
		RcDrawRectangleOutline(desktopRec, MonokaiYellow, 1);
	}
	
	#if STEAM_BUILD
	// +==============================+
	// |  Render Steam Friends List   |
	// +==============================+
	if (pig->debugRenderSteamFriendsList)
	{
		v2 minSize = NewVec2(300, 0);
		v2 maxSize = NewVec2(300, 0);
		r32 scale = 1.0f; //Animate(1.0f, 10.0f, 20000);
		v2 startPos = NewVec2(50, 15);
		r32 maxWidth = 0;
		v2 drawPos = startPos;
		VarArrayLoop(&platInfo->steamFriendsList->friends, fIndex)
		{
			VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &platInfo->steamFriendsList->friends, fIndex);
			v2 friendCardSize = RcDrawSteamFriendCard(friendInfo->id, drawPos, minSize, maxSize, scale, true);
			if (drawPos.y + friendCardSize.height >= ScreenSize.height - 15)
			{
				drawPos.x += maxWidth + 5;
				drawPos.y = startPos.y;
				maxWidth = 0;
			}
			RcDrawSteamFriendCard(friendInfo->id, drawPos, minSize, maxSize, scale);
			drawPos.y += friendCardSize.height + 5;
			if (maxWidth < friendCardSize.width) { maxWidth = friendCardSize.width; }
		}
	}
	#endif
}

void RenderPigDebugOverlay(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	PigDebugOverlayLayout(overlay);
	
	if (overlay->enabled)
	{
		RcBindShader(&pig->resources.shaders->main2D);
		RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12));
		v2 textPos = NewVec2(10, 10);
		if (pig->perfGraph.enabled) { textPos.y = pig->perfGraph.infoTextPos.y + 10; }
		if (pig->memGraph.enabled) { textPos.x = pig->memGraph.mainRec.x + pig->memGraph.mainRec.width + 10; }
		textPos.y += RcGetMaxAscend();
		Vec2Align(&textPos);
		Color_t backgroundColor = ColorTransparent(Black, 0.5f);
		v2 backgroundPadding = NewVec2(3, 1);
		r32 stepY = RoundR32(RcGetLineHeight());
		
		// +==============================+
		// |    Render Debug Readouts     |
		// +==============================+
		if (overlay->debugReadoutsEnabled)
		{
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Engine v%u.%02u(%04u) Game v%u.%02u(%04u) %s v%u.%02u(%04u)",
				ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD,
				GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_BUILD,
				GetPlatTypeStr(platInfo->type), platInfo->version.major, platInfo->version.minor, platInfo->version.build
			);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "ScreenSize: %.0fx%.0f MousePos (%.0f, %.0f) TimeScale: %lf ElapsedMs: %lf", ScreenSize.width, ScreenSize.height, MousePos.x, MousePos.y, TimeScale, ElapsedMs);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "AspectRatio: %g (w/h) %g (h/w)", ScreenSize.width/ScreenSize.height, ScreenSize.height/ScreenSize.width);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "ProgramTime: %llu (%lf)", pigIn->programTime, pigIn->programTimeF);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Unix Time: %s (%llu)", TempFormatRealTimeNt(&pigIn->unixTime), pigIn->unixTime.timestamp);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Local Time: %s (%llu)", TempFormatRealTimeNt(&pigIn->localTime), pigIn->localTime.timestamp);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding,
				"Timezone: %.*s %s%s",
				pigIn->localTimezoneName.length, pigIn->localTimezoneName.pntr,
				(pigIn->localTimezoneOffset > 0) ? "+" : "-", TempFormatMillisecondsNt((u64)AbsI64(pigIn->localTimezoneOffset) * 1000ULL)
			);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, ((pig->focusedItemPntr != nullptr) ? MonokaiWhite : MonokaiGray1), backgroundColor, backgroundPadding, "Focused Item: %p \"%.*s\"", pig->focusedItemPntr, pig->focusedItemName.length, pig->focusedItemName.pntr);
			textPos.y += stepY;
			
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Music Fade: %.0f%% %s", pig->musicSystem.currentFadeProgress*100, GetMusicFadeStr(pig->musicSystem.currentFade));
			textPos.y += stepY;
			
			if (pig->mouseHit.priority > 0)
			{
				RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Mouse Hit: \"\b%.*s\b\" (priority %llu, %p)", pig->mouseHit.name.length, pig->mouseHit.name.pntr, pig->mouseHit.priority, pig->mouseHit.pntr);
				textPos.y += stepY;
			}
			else
			{
				RcDrawTextPrintWithBackground(textPos, MonokaiGray1, backgroundColor, backgroundPadding, "Mouse Hit: Nothing at (%.0f, %.0f)", MousePos.x, MousePos.y);
				textPos.y += stepY;
			}
			
			#if PROCMON_SUPPORTED
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "nextProcmonEventId: %llu", pigIn->nextProcmonEventId);
			textPos.y += stepY;
			
			textPos.x += 600;
			textPos.y = 10 + RcGetMaxAscend();
			
			#if 1
			u64 numFilePaths = 0;
			StrHashDictIter_t touchedFilesIter = StrHashDictGetIter(&pigIn->touchedFiles, ProcmonFile_t);
			for (ProcmonFile_t* touchedFile = nullptr; StrHashDictIter(&touchedFilesIter, ProcmonFile_t, &touchedFile); )
			{
				RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "%3llu: %3llu %5llu %.*s", touchedFile->id, touchedFile->processId, touchedFile->numTouches, touchedFile->filePath.length, touchedFile->filePath.pntr);
				textPos.y += stepY;
				numFilePaths++;
			}
			
			textPos.x += 600;
			textPos.y = 10 + RcGetMaxAscend();
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "%llu files total", numFilePaths);
			textPos.y += stepY;
			#else
			u64 numProcessNames = 0;
			for (u64 pIndex = 0; pIndex < pigIn->processEntries.numItemsAlloc; pIndex++)
			{
				StrHashDictItem_t* slot = (StrHashDictItem_t*)(((u8*)pigIn->processEntries.base) + (pIndex * (sizeof(StrHashDictItem_t) + sizeof(ProcmonEntry_t))));
				if (slot->hash != 0)
				{
					ProcmonEntry_t* processEntry = (ProcmonEntry_t*)(slot + 1);
					RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "%3llu: %5llu 0x%02X %.*s", pIndex, processEntry->numEvents, processEntry->eventBits, processEntry->processName.length, processEntry->processName.pntr);
					#if 0
					r32 resetPosX = textPos.x;
					textPos.x += rc->flowInfo.renderRec.width + 10;
					for (u64 eIndex = 0; eIndex < ArrayCount(processEntry->lastFewEvents); eIndex++)
					{
						RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "%s", GetProcmonEventTypeStr(processEntry->lastFewEvents[eIndex]));
						textPos.x += rc->flowInfo.renderRec.width + 10;
					}
					textPos.x = resetPosX;
					#endif
					textPos.y += stepY;
					numProcessNames++;
				}
			}
			
			textPos.x += 600;
			textPos.y = 10 + RcGetMaxAscend();
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "%llu processes total", numProcessNames);
			textPos.y += stepY;
			#endif
			
			#endif //PROCMON_SUPPORTED
			
			#if STEAM_BUILD
			RcDrawTextPrintWithBackground(textPos, MonokaiWhite, backgroundColor, backgroundPadding, "Steam IPC Call Count: %llu", pigIn->steamIpcCallCount);
			textPos.y += stepY;
			#endif
		}
		
		// +==============================+
		// |     Render Easing Styles     |
		// +==============================+
		if (overlay->easingFuncsEnabled)
		{
			r32 bottomOfAudioOutGraph = pig->audioOutGraph.mainRec.y + pig->audioOutGraph.mainRec.height + RcGetLineHeight() + 10;
			if (pig->audioOutGraph.enabled && textPos.y < bottomOfAudioOutGraph) { textPos.y = bottomOfAudioOutGraph; }
			rec baseGraphRec = NewRec(textPos + NewVec2(0, RcGetLineHeight()), 100, 100);
			for (u64 eIndex = 1; eIndex < EasingStyle_NumStyles; eIndex++)
			{
				EasingStyle_t style = (EasingStyle_t)eIndex;
				
				RcDrawRectangleOutline(baseGraphRec, MonokaiWhite, 1, true);
				RcDrawRectangle(baseGraphRec, ColorTransparent(Black, 0.5f));
				
				RcDrawTextPrint(baseGraphRec.topLeft + NewVec2(0, -5), White, "%s", GetEasingStyleStr(style));
				
				u64 numPointsInCurve = 33;
				for (u64 pIndex = 0; pIndex < numPointsInCurve; pIndex++)
				{
					r32 inputValue1 = (r32)(pIndex + 0) / (r32)numPointsInCurve;
					r32 inputValue2 = (r32)(pIndex + 1) / (r32)numPointsInCurve;
					r32 outputValue1 = Ease(style, inputValue1);
					r32 outputValue2 = Ease(style, inputValue2);
					v2 lineStart = NewVec2(baseGraphRec.x + baseGraphRec.width * inputValue1, baseGraphRec.y + (baseGraphRec.height * (1.0f - outputValue1)));
					v2 lineEnd   = NewVec2(baseGraphRec.x + baseGraphRec.width * inputValue2, baseGraphRec.y + (baseGraphRec.height * (1.0f - outputValue2)));
					RcDrawLine(lineStart, lineEnd, 1, MonokaiYellow);
				}
				
				baseGraphRec.x += baseGraphRec.width + 5;
				if (baseGraphRec.x + baseGraphRec.width >= ScreenSize.width)
				{
					baseGraphRec.x = textPos.x;
					baseGraphRec.y += baseGraphRec.height + 5 + RcGetLineHeight();
				}
			}
			textPos.y = baseGraphRec.y + baseGraphRec.height + RcGetLineHeight();
		}
		
		// +==============================+
		// |   Render Controller States   |
		// +==============================+
		if (overlay->controllerDebugEnabled)
		{
			u64 lastConnectedControllerIndex = 0;
			for (u64 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
			{
				if (pigIn->controllerStates[cIndex].connected) { lastConnectedControllerIndex = cIndex; }
			}
			rec controllerBaseRec = NewRec(textPos + NewVec2(0, RcGetLineHeight()), 250, 250);
			for (u64 cIndex = 0; cIndex <= lastConnectedControllerIndex+1 && cIndex < MAX_NUM_CONTROLLERS; cIndex++)
			{
				RenderDebugOverlayControllerState(overlay, &pigIn->controllerStates[cIndex], controllerBaseRec);
				controllerBaseRec.x += controllerBaseRec.width + 5;
				if (controllerBaseRec.x + controllerBaseRec.width >= ScreenSize.width)
				{
					controllerBaseRec.x = textPos.x;
					controllerBaseRec.y += controllerBaseRec.height + 5;
				}
			}
			textPos.y = controllerBaseRec.y + controllerBaseRec.height + RcGetLineHeight();
		}
		
		// +==============================+
		// |      Render Pie Charts       |
		// +==============================+
		if (overlay->pieChartsEnabled)
		{
			RcBindShader(&pig->resources.shaders->pieChart);
			
			RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12));
			RcDrawPieChartForPerfSectionBundle(&platInfo->initPerfSectionBundle, overlay->initPieChartRec + NewVec2(0, 8), Grey11);
			RcDrawPieChartForPerfSectionBundle(&platInfo->initPerfSectionBundle, overlay->initPieChartRec, White, true);
			
			#if 1
			r64 elapsedMsPieChartPercentages[3];
			elapsedMsPieChartPercentages[0] = (pigIn->timeSpentWaitingLastFrame / pigIn->elapsedMs);
			elapsedMsPieChartPercentages[1] = ((pigIn->lastUpdateElapsedMs - overlay->physicsSimTimeLastFrame) / pigIn->elapsedMs);
			elapsedMsPieChartPercentages[2] = (overlay->physicsSimTimeLastFrame / pigIn->elapsedMs);
			RcDrawPieChart(3, &elapsedMsPieChartPercentages[0], NewRec(overlay->initPieChartRec.x, overlay->initPieChartRec.y - 10 - overlay->initPieChartRec.height, overlay->initPieChartRec.size), Grey11);
			RcDrawPieChart(3, &elapsedMsPieChartPercentages[0], NewRec(overlay->initPieChartRec.x, overlay->initPieChartRec.y - 18 - overlay->initPieChartRec.height, overlay->initPieChartRec.size), White);
			#endif
			
			// RcDrawPieChartTest(NewRec(ScreenSize.width - 630, ScreenSize.height - 210, 200, 200));
			
			RcBindShader(&pig->resources.shaders->main2D);
		}
		
		// +==============================+
		// |    Render Audio Instances    |
		// +==============================+
		if (overlay->audioInstancesEnabled)
		{
			RcBindFont(&pig->resources.fonts->pixel, SelectDefaultFontFace());
			rec baseProgressRec = NewRec(0, overlay->totalToggleBtnsRec.y + overlay->totalToggleBtnsRec.height + 5, 100, RcGetLineHeight());
			baseProgressRec.x = (pig->audioOutGraph.enabled ? pig->audioOutGraph.mainRec.x : ScreenSize.width) - 10 - baseProgressRec.width;
			RecAlign(&baseProgressRec);
			for (u64 iIndex = 0; iIndex < PIG_MAX_SOUND_INSTANCES; iIndex++)
			{
				SoundInstance_t* instance = &pig->soundInstances[iIndex];
				if (instance->playing && instance->sound != nullptr)
				{
					const Sound_t* soundPntr = instance->sound;
					u64 foundIndex = 0;
					ResourceType_t foundType = GetResourceByPntr(soundPntr, &foundIndex);
					MyStr_t displayText = TempPrintStr("Unknown[%p]", soundPntr);
					r32 progressRecWidth = baseProgressRec.width;
					Color_t displayColor = MonokaiMagenta;
					r32 soundDurationSecs = ((r32)soundPntr->numFrames / (r32)soundPntr->format.samplesPerSecond);
					r32 currentTimeSecs = ((r32)instance->frameIndex / (r32)soundPntr->numFrames) * soundDurationSecs;
					progressRecWidth = soundDurationSecs * 10;
					if (progressRecWidth < 10) { progressRecWidth = 10; }
					if (foundType == ResourceType_Music)
					{
						i32 totalMinutes = RoundR32i(soundDurationSecs) / 60;
						i32 totalSeconds = RoundR32i(soundDurationSecs) % 60;
						i32 totalHundredths = RoundR32i(DecimalPartR32(soundDurationSecs) * 100);
						i32 currentMinutes = RoundR32i(currentTimeSecs) / 60;
						i32 currentSeconds = RoundR32i(currentTimeSecs) % 60;
						i32 currentHundredths = RoundR32i(DecimalPartR32(currentTimeSecs) * 100);
						MyStr_t musicName = GetFileNamePart(NewStr(Resources_GetPathForMusic(foundIndex)));
						displayText = TempPrintStr("(%d:%02d.%02d/%d:%02d.%02d) %.*s",
							currentMinutes, currentSeconds, currentHundredths,
							totalMinutes, totalSeconds, totalHundredths,
							musicName.length, musicName.pntr
						);
						displayColor = MonokaiWhite;
					}
					else if (foundType == ResourceType_Sound)
					{
						displayText = GetFileNamePart(NewStr(Resources_GetPathForSound(foundIndex)));
						displayColor = MonokaiLightBlue;
					}
					
					rec progressTotalRec = baseProgressRec;
					progressTotalRec.width = progressRecWidth;
					progressTotalRec.x = baseProgressRec.x + baseProgressRec.width - progressTotalRec.width;
					RecAlign(&progressTotalRec);
					rec progressRec = progressTotalRec;
					progressRec.width *= ((r32)instance->frameIndex / (r32)soundPntr->numFrames);
					RecAlign(&progressRec);
					v2 displayTextPos = NewVec2(progressTotalRec.x - 5, progressTotalRec.y + progressTotalRec.height/2 - RcGetLineHeight()/2 + RcGetMaxAscend());
					Vec2Align(&displayTextPos);
					r32 progressAlpha = LerpR32(0.3f, 1.0f, ConvertVolumeToLoudness(instance->volume));
					if (instance->volume == 0) { progressAlpha = 0.0f; }
					
					RcDrawRectangle(RecInflate(progressTotalRec, 1, 1), displayColor);
					RcDrawRectangle(progressTotalRec, Black);
					RcDrawRectangle(progressRec, ColorTransparent(displayColor, progressAlpha));
					RcDrawText(displayText, displayTextPos, displayColor, TextAlignment_Right);
					
					baseProgressRec.y = progressTotalRec.y + progressTotalRec.height + 5;
				}
			}
		}
	}
	
	// +==============================+
	// |    Render Toggle Buttons     |
	// +==============================+
	if (overlay->openAnimTime > 0.0f)
	{
		RcDrawRectangleOutline(overlay->totalToggleBtnsRec, MonokaiWhite, 2, true);
		for (u64 bIndex = 0; bIndex < ArrayCount(overlay->toggleBtnRecs); bIndex++)
		{
			rec btnRec = overlay->toggleBtnRecs[bIndex];
			// bool isMouseOver = IsMouseOverPrint("DebugOverlayToggleBtn%llu", bIndex);
			bool enabled = false;
			Color_t btnColor = MonokaiBack;
			switch (bIndex)
			{
				case 0: enabled = overlay->debugReadoutsEnabled;   btnColor = MonokaiLightGray; break;
				case 1: enabled = pig->perfGraph.enabled;          btnColor = MonokaiYellow;    break;
				case 2: enabled = pig->audioOutGraph.enabled;      btnColor = MonokaiBlue;      break;
				case 3: enabled = overlay->audioInstancesEnabled;  btnColor = MonokaiPurple;    break;
				case 4: enabled = pig->memGraph.enabled;           btnColor = MonokaiOrange;    break;
				case 5: enabled = overlay->pieChartsEnabled;       btnColor = MonokaiMagenta;   break;
				case 6: enabled = overlay->easingFuncsEnabled;     btnColor = MonokaiBrown;     break;
				case 7: enabled = overlay->controllerDebugEnabled; btnColor = MonokaiGreen;     break;
				default: DebugAssert(false);
			}
			Color_t outlineColor = Transparent;
			if (enabled)
			{
				btnColor.a = 255;
			}
			else
			{
				outlineColor = btnColor;
				btnColor.a = 0;
			}
			RcDrawRectangle(btnRec, ColorDarken(btnColor, 60));
			RcDrawRectangle(RecRetractY(btnRec, 2), btnColor);
			if (outlineColor.a > 0)
			{
				RcDrawRectangleOutline(btnRec, outlineColor, 2);
			}
			
		}
		
		MyStr_t displayText = TempPrintStr("Press %s to hide", GetKeyStr(DEBUG_OVERLAY_TOGGLE_KEY));
		if (IsMouseOverNamed("DebugOverlayToggleBtn0")) { displayText = NewStr("Debug Readouts"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn1")) { displayText = NewStr("Perf Graph"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn2")) { displayText = NewStr("Audio Graph"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn3")) { displayText = NewStr("Audio Instances"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn4")) { displayText = NewStr("Memory Graph"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn5")) { displayText = NewStr("Pie Charts"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn6")) { displayText = NewStr("Easing Functions"); }
		if (IsMouseOverNamed("DebugOverlayToggleBtn7")) { displayText = NewStr("Controller Debug"); }
		
		RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12, true));
		v2 textPos = NewVec2(overlay->totalToggleBtnsRec.x + overlay->totalToggleBtnsRec.width/2, overlay->totalToggleBtnsRec.y + overlay->totalToggleBtnsRec.height + RcGetMaxAscend());
		Vec2Align(&textPos, 2);
		RcDrawText(displayText, textPos, ColorTransparent(MonokaiWhite, overlay->openAnimTime), TextAlignment_Center, 0);
	}
}
