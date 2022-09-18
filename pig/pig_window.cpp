/*
File:   pig_window.cpp
Author: Taylor Robbins
Date:   02\20\2022
Description: 
	** Holds functions that help us do things using the PigWindowState_t structures
	** Like keeping track of which window is current, handling screenshot 
*/

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
PigWindowState_t* Pig_AddNewWindowState(const PlatWindow_t* window)
{
	PigWindowState_t* newState = LinkedListAdd(&pig->windowStates, PigWindowState_t);
	NotNull(newState);
	ClearPointer(newState);
	newState->windowId = window->id;
	if (!CreateFrameBuffer(mainHeap, &newState->frameBuffer, window->input.contextResolution, window->options.create.antialiasingNumSamples, false, FrameBufferChannel_All))
	{
		u64 windowIndex = GetItemIndexInLinkedList(platInfo->windows, window);
		NotifyPrint_E("Failed to create frameBuffer for window[%llu] at resolution %dx%d: %s",
			windowIndex, window->input.contextResolution.width, window->input.contextResolution.height,
			PrintFrameBufferError(&newState->frameBuffer)
		);
		DestroyFrameBuffer(&newState->frameBuffer);
	}
	return newState;
}

const PlatWindow_t* GetWindowById(u64 windowId)
{
	const PlatWindow_t* window = LinkedListFirst(platInfo->windows, PlatWindow_t);
	while (window != nullptr)
	{
		if (window->id == windowId)
		{
			return window;
		}
		window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
	}
	return nullptr;
}
const PlatWindow_t* GetWindowByIdHard(u64 windowId)
{
	const PlatWindow_t* result = GetWindowById(windowId);
	NotNull(result);
	return result;
}

PigWindowState_t* GetWindowStateById(u64 windowId, bool createState = false)
{
	PigWindowState_t* windowState = LinkedListFirst(&pig->windowStates, PigWindowState_t);
	while (windowState != nullptr)
	{
		if (windowState->windowId == windowId)
		{
			return windowState;
		}
		windowState = LinkedListNext(&pig->windowStates, PigWindowState_t, windowState);
	}
	if (createState)
	{
		const PlatWindow_t* window = GetWindowByIdHard(windowId);
		PigWindowState_t* newState = Pig_AddNewWindowState(window);
		return newState;
	}
	return nullptr;
}

void Pig_ChangeWindow(const PlatWindow_t* window)
{
	AssertSingleThreaded();
	NotNull(window);
	plat->ChangeWindowTarget(window);
	pig->currentWindow = window;
	pig->currentWindowState = GetWindowStateById(window->id, true);
	MousePos = window->input.mousePos;
	ScreenSize = window->input.renderResolution;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Pig_InitializeWindowStates()
{
	CreateLinkedList(&pig->windowStates, mainHeap, PigWindowState_t);
	const PlatWindow_t* window = LinkedListFirst(platInfo->windows, PlatWindow_t);
	for (u64 wIndex = 0; window != nullptr; wIndex++)
	{
		PigWindowState_t* newState = Pig_AddNewWindowState(window);
		if (pig->currentWindow == window) { pig->currentWindowState = newState; }
		window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
	}
}

// +--------------------------------------------------------------+
// |                      Capture Functions                       |
// +--------------------------------------------------------------+
void Pig_CaptureScreenshotSub(reci subPartRec)
{
	NotNull2(pig->currentWindow, pig->currentWindowState);
	Assert(!pig->currentWindowState->recordingGif);
	pig->currentWindowState->takeScreenshot = true;
	pig->currentWindowState->screenSubPart = subPartRec;
}
void Pig_CaptureScreenshot()
{
	Pig_CaptureScreenshotSub(NewReci(0, 0, Vec2Roundi(ScreenSize)));
}

void Pig_StartGifRecordingSub(reci subPartRec)
{
	NotNull2(pig->currentWindow, pig->currentWindowState);
	Assert(!pig->currentWindowState->recordingGif);
	pig->currentWindowState->recordingGif = true;
	pig->currentWindowState->finishGif = false;
	pig->currentWindowState->screenSubPart = subPartRec;
	CreateLinkedList(&pig->currentWindowState->gifFrames, &pig->stdHeap, PigGifFrame_t);
}
void Pig_StartGifRecording()
{
	Pig_StartGifRecordingSub(NewReci(0, 0, Vec2Roundi(ScreenSize)));
}
void Pig_FinishGif()
{
	NotNull2(pig->currentWindow, pig->currentWindowState);
	Assert(pig->currentWindowState->recordingGif);
	pig->currentWindowState->finishGif = true;
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Pig_HandleScreenshotHotkeys()
{
	// +==============================+
	// |      Screenshot Hotkey       |
	// +==============================+
	if (KeyPressedRaw(Key_F2))
	{
		pig->currentWindowState->screenshotKeyWasUsedForSelection = false;
	}
	if (KeyDownRaw(Key_F2) && MousePressedRaw(MouseBtn_Left) && IsMouseInsideWindow())
	{
		pig->currentWindowState->selectingSubPart = true;
		pig->currentWindowState->screenshotKeyWasUsedForSelection = true;
		pig->currentWindowState->subPartStartPos = Vec2Roundi(MousePos);
	}
	if (pig->currentWindowState->selectingSubPart) { HandleMouse(MouseBtn_Left); }
	if (pig->currentWindowState->selectingSubPart && pig->currentWindowState->screenshotKeyWasUsedForSelection && !(KeyDownRaw(Key_F2) && MouseDownRaw(MouseBtn_Left)))
	{
		pig->currentWindowState->selectingSubPart = false;
		reci selectedRec = NewReciBetween(pig->currentWindowState->subPartStartPos, Vec2Roundi(MousePos));
		selectedRec = ReciOverlap(selectedRec, NewReci(0, 0, pig->currentWindow->input.contextResolution));
		Pig_CaptureScreenshotSub(selectedRec);
	}
	if (KeyReleasedRaw(Key_F2) && !pig->currentWindowState->screenshotKeyWasUsedForSelection)
	{
		Pig_CaptureScreenshot();
	}
	if (KeyDownRaw(Key_F2) || KeyReleasedRaw(Key_F2)) { HandleKey(Key_F2); }
	
	// +==============================+
	// |          Gif Hotkey          |
	// +==============================+
	if (KeyPressedRaw(Key_F4))
	{
		pig->currentWindowState->gifKeyWasUsedForSelection = false;
	}
	if (KeyDownRaw(Key_F4) && MousePressedRaw(MouseBtn_Left) && IsMouseInsideWindow() && !pig->currentWindowState->recordingGif)
	{
		pig->currentWindowState->selectingSubPart = true;
		pig->currentWindowState->gifKeyWasUsedForSelection = true;
		pig->currentWindowState->subPartStartPos = Vec2Roundi(MousePos);
	}
	if (pig->currentWindowState->selectingSubPart) { HandleMouse(MouseBtn_Left); }
	if (pig->currentWindowState->selectingSubPart && pig->currentWindowState->gifKeyWasUsedForSelection && !(KeyDownRaw(Key_F4) && MouseDownRaw(MouseBtn_Left)))
	{
		pig->currentWindowState->selectingSubPart = false;
		reci selectedRec = NewReciBetween(pig->currentWindowState->subPartStartPos, Vec2Roundi(MousePos));
		selectedRec = ReciOverlap(selectedRec, NewReci(0, 0, pig->currentWindow->input.contextResolution));
		Pig_StartGifRecordingSub(selectedRec);
	}
	if (KeyReleasedRaw(Key_F4) && !pig->currentWindowState->gifKeyWasUsedForSelection)
	{
		if (pig->currentWindowState->recordingGif)
		{
			pig->currentWindowState->finishGif = true;
		}
		else
		{
			Pig_StartGifRecording();
		}
	}
	if (KeyDownRaw(Key_F4) || KeyReleasedRaw(Key_F4)) { HandleKey(Key_F4); }
}

void Pig_SaveScreenshot(FrameBuffer_t* frameBuffer, reci subPartRec, MyStr_t filePath)
{
	NotNull(frameBuffer);
	PlatImageData_t imageData = {};
	if (GetTextureDataSubPart(&frameBuffer->outTexture, subPartRec, TempArena, &imageData))
	{
		if (plat->SaveImageDataToFile(filePath, &imageData, PlatImageFormat_Png, nullptr))
		{
			PrintLine_I("Saved screenshot %dx%d (%llu bytes) to %.*s", imageData.width, imageData.height, imageData.dataSize, filePath.length, filePath.pntr);
		}
		else
		{
			NotifyPrint_E("Failed to save screenshot to %.*s!", filePath.length, filePath.pntr);
		}
	}
	else
	{
		NotifyWrite_E("Failed to take screenshot!");
	}
}

void Pig_StoreGifFrame(FrameBuffer_t* frameBuffer, reci subPartRec, LinkedList_t* frames)
{
	NotNull(frameBuffer);
	NotNull(frames);
	Assert(frames->itemSize == sizeof(PigGifFrame_t));
	
	PigGifFrame_t* newFrame = LinkedListAdd(frames, PigGifFrame_t);
	NotNull(newFrame);
	ClearPointer(newFrame);
	
	if (!GetTextureDataSubPart(&frameBuffer->outTexture, subPartRec, &pig->stdHeap, &newFrame->imageData))
	{
		DebugAssert(false);//TODO: What should we do in case of frame get failure?
	}
}
void Pig_SaveGif(LinkedList_t* frames, MyStr_t filePath)
{
	NotNull(frames);
	PrintLine_I("Finishing GIF with %llu frames", pig->currentWindowState->gifFrames.count);
	v2i gifSize = pig->currentWindowState->screenSubPart.size;
	MsfGifState msfState = {};
	int beginResult = msf_gif_begin(&msfState, gifSize.width, gifSize.height);
	Assert(beginResult != 0);
	PigGifFrame_t* frame = LinkedListFirst(&pig->currentWindowState->gifFrames, PigGifFrame_t);
	u64 totalImageDataSize = 0;
	u64 totalNumFrames = 0;
	while (frame != nullptr)
	{
		int frameResult = msf_gif_frame(&msfState, frame->imageData.data8, 2, frame->imageData.pixelSize*8, (int)frame->imageData.rowSize);
		Assert(frameResult != 0);
		totalImageDataSize += frame->imageData.dataSize;
		totalNumFrames++;
		FreeMem(frame->imageData.allocArena, frame->imageData.data8, frame->imageData.dataSize);
		frame = LinkedListNext(frames, PigGifFrame_t, frame);
	}
	
	MsfGifResult gifResult = msf_gif_end(&msfState);
	NotNull(gifResult.data);
	PrintLine_I("Uncompressed data for GIF was %s", FormatBytesNt(totalImageDataSize, TempArena));
	PrintLine_I("Output GIF is %s", FormatBytesNt(gifResult.dataSize, TempArena));
	
	if (plat->WriteEntireFile(filePath, gifResult.data, gifResult.dataSize))
	{
		NotifyPrint_I("Saved %llu frame %dx%d resolution %s GIF to \"%.*s\"", totalNumFrames, gifSize.width, gifSize.height, FormatBytesNt(gifResult.dataSize, TempArena), filePath.length, filePath.pntr);
	}
	else
	{
		NotifyPrint_E("Failed to save %s GIF data to \"%.*s\"", FormatBytesNt(gifResult.dataSize, TempArena), filePath.length, filePath.pntr);
	}
	
	FreeLinkedList(&pig->currentWindowState->gifFrames);
}

void Pig_UpdateCaptureHandling(const PlatWindow_t* window, PigWindowState_t* state)
{
	NotNull(window);
	NotNull(state);
	
	if (state->takeScreenshot)
	{
		state->takeScreenshot = false;
		TempPushMark();
		//TODO: Move the output folder to a user directory like Documents
		MyStr_t outputFilePath = TempPrintStr("screenshot_%04u-%02u-%02u_%02u-%02u-%02u.png", pigIn->localTime.year, pigIn->localTime.month+1, pigIn->localTime.day+1, pigIn->localTime.hour, pigIn->localTime.minute, pigIn->localTime.second);
		Pig_SaveScreenshot(&state->frameBuffer, state->screenSubPart, outputFilePath);
		TempPopMark();
	}
	
	if (state->recordingGif)
	{
		Pig_StoreGifFrame(&state->frameBuffer, state->screenSubPart, &state->gifFrames);
		
		u64 gifFrameSize = state->screenSubPart.width * state->screenSubPart.height * sizeof(u32);
		if (state->gifFrames.count * gifFrameSize >= MAX_GIF_SIZE) { state->finishGif = true; }
		
		if (state->finishGif)
		{
			TempPushMark();
			MyStr_t outputFilePath = TempPrintStr("recording_%04u-%02u-%02u_%02u-%02u-%02u.gif", pigIn->localTime.year, pigIn->localTime.month+1, pigIn->localTime.day+1, pigIn->localTime.hour, pigIn->localTime.minute, pigIn->localTime.second);
			Pig_SaveGif(&state->gifFrames, outputFilePath);
			pig->currentWindowState->recordingGif = false;
			TempPopMark();
		}
	}
	
	
}

void Pig_UpdateWindowStates()
{
	const PlatWindow_t* window = LinkedListFirst(platInfo->windows, PlatWindow_t);
	u64 wIndex = 0;
	while (window != nullptr)
	{
		PigWindowState_t* windowState = GetWindowStateById(window->id);
		if (windowState != nullptr)
		{
			// +======================================+
			// | Handle Window Lost Between Monitors  |
			// +======================================+
			if (IsWindowLostBetweenMonitors(window))
			{
				u64 primaryMonitorIndex = 0;
				const PlatMonitorInfo_t* primaryMonitorInfo = GetPrimaryMonitorInfo(&primaryMonitorIndex);
				NotifyPrint_W("We were lost between %llu monitors! windowRec: (%d, %d, %d, %d) Moving to the primary monitor[%llu] at (%d, %d, %d, %d)",
					platInfo->monitors->list.count,
					window->input.desktopRec.x, window->input.desktopRec.y, window->input.desktopRec.width, window->input.desktopRec.height,
					primaryMonitorIndex,
					primaryMonitorInfo->desktopSpaceRec.x, primaryMonitorInfo->desktopSpaceRec.y, primaryMonitorInfo->desktopSpaceRec.width, primaryMonitorInfo->desktopSpaceRec.height
				);
				pigOut->moveWindow = true;
				pigOut->moveWindowId = window->id;
				pigOut->moveWindowRec = NewReci(
					primaryMonitorInfo->desktopSpaceRec.x + primaryMonitorInfo->desktopSpaceRec.width/4,
					primaryMonitorInfo->desktopSpaceRec.y + primaryMonitorInfo->desktopSpaceRec.height/4,
					primaryMonitorInfo->desktopSpaceRec.width/2,
					primaryMonitorInfo->desktopSpaceRec.height/2
				);
			}
			
			// +====================================+
			// | Handle Window FrameBuffer Resizing |
			// +====================================+
			if (windowState->frameBuffer.size != window->input.contextResolution && window->input.contextResolution.width > 0 && window->input.contextResolution.height > 0)
			{
				PrintLine_D("Resizing window[%llu] frameBuffer from %dx%d to %dx%d",
					wIndex,
					windowState->frameBuffer.width, windowState->frameBuffer.height,
					window->input.contextResolution.width, window->input.contextResolution.height
				);
				//TODO: Is there a better way to resize buffers without destroying and recreating?
				DestroyFrameBuffer(&windowState->frameBuffer);
				if (!CreateFrameBuffer(mainHeap, &windowState->frameBuffer, window->input.contextResolution, window->options.create.antialiasingNumSamples, false, FrameBufferChannel_All))
				{
					NotifyPrint_E("Failed to resize frameBuffer for window[%llu] at resolution %dx%d: %s", wIndex, window->input.contextResolution.width, window->input.contextResolution.height, PrintFrameBufferError(&windowState->frameBuffer));
					DestroyFrameBuffer(&windowState->frameBuffer);
				}
			}
		}
		window = LinkedListNext(platInfo->windows, PlatWindow_t, window);
		wIndex++;
	}
}
