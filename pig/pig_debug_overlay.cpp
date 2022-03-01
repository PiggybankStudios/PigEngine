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
	overlay->enabled = (SHOW_PERF_GRAPH_ON_STARTUP || SHOW_MEM_GRAPH_ON_STARTUP || SHOW_PIE_GRAPHS_ON_STARTUP || SHOW_AUDIO_OUT_GRAPH_ON_STARTUP);
	overlay->pieChartsEnabled = SHOW_PIE_GRAPHS_ON_STARTUP;
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
						case 3: pig->memGraph.enabled           = !pig->memGraph.enabled;           break; //toggleMemGraphBtnRec
						case 4: overlay->pieChartsEnabled       = !overlay->pieChartsEnabled;       break; //togglePieChartsBtnRec
						case 5: overlay->easingFuncsEnabled     = !overlay->easingFuncsEnabled;     break; //toggleEasingFuncsBtnRec
						case 6: overlay->controllerDebugEnabled = !overlay->controllerDebugEnabled; break; //toggleContollerDebugBtnRec
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
				overlay->perfGraphWasEnabled     = pig->perfGraph.enabled;
				overlay->memGraphWasEnabled      = pig->memGraph.enabled;
				overlay->audioOutGraphWasEnabled = pig->audioOutGraph.enabled;
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
		if (KeyPressed(Key_4)) { HandleKey(Key_4); pig->memGraph.enabled           = !pig->memGraph.enabled;           overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_5)) { HandleKey(Key_5); overlay->pieChartsEnabled       = !overlay->pieChartsEnabled;       overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_6)) { HandleKey(Key_6); overlay->easingFuncsEnabled     = !overlay->easingFuncsEnabled;     overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
		if (KeyPressed(Key_7)) { HandleKey(Key_7); overlay->controllerDebugEnabled = !overlay->controllerDebugEnabled; overlay->hotkeyPlusNumberPressed = true; overlay->lastMouseCloseTime = ProgramTime; }
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
	RcBindFont(&pig->resources.debugFont, SelectFontFace(12));
	
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
			
			RcBindSpriteSheet(&pig->resources.controllerBtnsSheet);
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

void RenderPigDebugOverlay(PigDebugOverlay_t* overlay)
{
	NotNull(overlay);
	PigDebugOverlayLayout(overlay);
	
	if (overlay->enabled)
	{
		RcBindShader(&pig->resources.mainShader2D);
		RcBindFont(&pig->resources.debugFont, SelectFontFace(12));
		v2 textPos = NewVec2(10, 10);
		if (pig->perfGraph.enabled) { textPos.y = pig->perfGraph.infoTextPos.y + 10; }
		if (pig->memGraph.enabled) { textPos.x = pig->memGraph.mainRec.x + pig->memGraph.mainRec.width + 10; }
		textPos.y += RcGetMaxAscend();
		Vec2Align(&textPos);
		
		// +==============================+
		// |    Render Debug Readouts     |
		// +==============================+
		if (overlay->debugReadoutsEnabled)
		{
			RcDrawTextPrint(textPos, MonokaiWhite, "ProgramTime: %llu (%lf)", pigIn->programTime, pigIn->programTimeF);
			textPos.y += RoundR32(RcGetLineHeight());
			
			RcDrawTextPrint(textPos, MonokaiWhite, "Unix Time: %s (%llu)", TempFormatRealTimeNt(&pigIn->unixTime), pigIn->unixTime.timestamp);
			textPos.y += RoundR32(RcGetLineHeight());
			
			RcDrawTextPrint(textPos, MonokaiWhite, "Local Time: %s (%llu)", TempFormatRealTimeNt(&pigIn->localTime), pigIn->localTime.timestamp);
			textPos.y += RoundR32(RcGetLineHeight());
			
			RcDrawTextPrint(textPos, MonokaiWhite,
				"Timezone: %.*s %s%s",
				pigIn->localTimezoneName.length, pigIn->localTimezoneName.pntr,
				(pigIn->localTimezoneOffset > 0) ? "+" : "-", TempFormatMillisecondsNt((u64)AbsI64(pigIn->localTimezoneOffset) * 1000ULL)
			);
			textPos.y += RoundR32(RcGetLineHeight());
		}
		
		// +==============================+
		// |     Render Easing Styles     |
		// +==============================+
		if (overlay->easingFuncsEnabled)
		{
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
			RcBindShader(&pig->resources.pieChartShader);
			
			RcBindFont(&pig->resources.debugFont, SelectFontFace(12));
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
			
			RcBindShader(&pig->resources.mainShader2D);
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
				case 3: enabled = pig->memGraph.enabled;           btnColor = MonokaiOrange;    break;
				case 4: enabled = overlay->pieChartsEnabled;       btnColor = MonokaiMagenta;   break;
				case 5: enabled = overlay->easingFuncsEnabled;     btnColor = MonokaiBrown;     break;
				case 6: enabled = overlay->controllerDebugEnabled; btnColor = MonokaiGreen;     break;
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
			
			RcBindFont(&pig->resources.debugFont, SelectFontFace(12, true));
			v2 textPos = NewVec2(overlay->totalToggleBtnsRec.x + overlay->totalToggleBtnsRec.width/2, overlay->totalToggleBtnsRec.y + overlay->totalToggleBtnsRec.height + RcGetMaxAscend());
			Vec2Align(&textPos, 2);
			RcDrawTextPrintEx(textPos, ColorTransparent(MonokaiWhite, overlay->openAnimTime), TextAlignment_Center, 0, "Press %s to hide", GetKeyStr(DEBUG_OVERLAY_TOGGLE_KEY));
		}
	}
}
