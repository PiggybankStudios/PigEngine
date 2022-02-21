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
	if (!CreateFrameBuffer(mainHeap, &newState->frameBuffer, window->input.contextResolution, window->options.create.antialiasingNumSamples))
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
	pig->currentWindowState->takeScreenshot = true;
	pig->currentWindowState->screenshotSubPartRec = subPartRec;
}
void Pig_CaptureScreenshot()
{
	Pig_CaptureScreenshotSub(NewReci(0, 0, Vec2Roundi(ScreenSize)));
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Pig_SaveScreenshot(const PlatWindow_t* window, PigWindowState_t* state)
{
	NotNull(window);
	NotNull(state);
	if (state->takeScreenshot)
	{
		state->takeScreenshot = false;
		TempPushMark();
		//TODO: Move the output folder to a user directory like Documents
		MyStr_t outputFilePath = TempPrintStr("screenshot_%04u-%02u-%02u_%02u-%02u-%02u.png", pigIn->localTime.year, pigIn->localTime.month+1, pigIn->localTime.day+1, pigIn->localTime.hour, pigIn->localTime.minute, pigIn->localTime.second);
		PlatImageData_t imageData = {};
		if (GetTextureDataSubPart(&state->frameBuffer.outTexture, state->screenshotSubPartRec, TempArena, &imageData))
		{
			if (plat->SaveImageDataToFile(outputFilePath, &imageData, PlatImageFormat_Png, nullptr))
			{
				PrintLine_I("Saved screenshot %dx%d (%llu bytes) to %.*s", imageData.width, imageData.height, imageData.dataSize, outputFilePath.length, outputFilePath.pntr);
			}
			else
			{
				NotifyPrint_E("Failed to save screenshot to %.*s!", outputFilePath.length, outputFilePath.pntr);
			}
		}
		else
		{
			NotifyWrite_E("Failed to take screenshot!");
		}
		TempPopMark();
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
			if (windowState->frameBuffer.size != window->input.contextResolution && window->input.contextResolution.width > 0 && window->input.contextResolution.height > 0)
			{
				PrintLine_D("Resizing window[%llu] frameBuffer from %dx%d to %dx%d",
					wIndex,
					windowState->frameBuffer.width, windowState->frameBuffer.height,
					window->input.contextResolution.width, window->input.contextResolution.height
				);
				//TODO: Is there a better way to resize buffers without destroying and recreating?
				DestroyFrameBuffer(&windowState->frameBuffer);
				if (!CreateFrameBuffer(mainHeap, &windowState->frameBuffer, window->input.contextResolution, window->options.create.antialiasingNumSamples))
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
