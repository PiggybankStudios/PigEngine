/*
File:   win32_engine_output.cpp
Author: Taylor Robbins
Date:   09\18\2022
Description: 
	** Holds functions that handle the various pieces of information that get piped
	** to us through the EngineOutput_t structure. These are often things that act
	** like API function calls but don't need immediate responses so the parameters
	** can just be stuffed to a structure and handled at the end of a update iteration
*/

void Win32_FillEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	ClearPointer(output);
	output->cursorType = Platform->currentCursorType;
	output->mouseMode = Platform->currentMouseMode;
}

void Win32_ClearEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	output->exit = false;
	output->moveWindow = false;
	output->moveWindowId = 0;
	output->changeFullscreen = false;
	output->changeFullscreenWindowId = 0;
}

void Win32_ProcessEngineOutput(EngineOutput_t* output)
{
	NotNull(output);
	
	// +==============================+
	// |             exit             |
	// +==============================+
	if (output->exit)
	{
		WriteLine_E("Application requested an exit...");
		Platform->exitRequested = true;
	}
	
	// +==============================+
	// |          cursorType          |
	// +==============================+
	//TODO: Does this work for multiple windows? Do we need to set the cursor in each window?
	if (output->cursorType != Platform->currentCursorType)
	{
		Assert(output->cursorType < PlatCursor_NumCursors);
		glfwSetCursor(Platform->currentWindow->handle, Platform->glfwCursors[output->cursorType]);
		Platform->currentCursorType = output->cursorType;
	}
	
	// +==============================+
	// |          mouseMode           |
	// +==============================+
	if (output->mouseMode != Platform->currentMouseMode)
	{
		int glfwNewValue = GLFW_CURSOR_NORMAL;
		if (output->mouseMode == PlatMouseMode_Invisible) { glfwNewValue = GLFW_CURSOR_HIDDEN; }
		if (output->mouseMode == PlatMouseMode_FirstPersonCamera) { glfwNewValue = GLFW_CURSOR_DISABLED; } //TODO: We may want to also set GLFW_RAW_MOUSE_MOTION to true on some platforms to disable mouse motion scaling and acceleration?
		glfwSetInputMode(Platform->currentWindow->handle, GLFW_CURSOR, glfwNewValue);
		Platform->currentMouseMode = output->mouseMode;
	}
	
	// +==============================+
	// |          moveWindow          |
	// +==============================+
	if (output->moveWindow)
	{
		Assert(output->moveWindowId != 0);
		Assert(output->moveWindowRec.width > 0 && output->moveWindowRec.height > 0);
		
		PlatWindow_t* window = Win32_GetWindowById(output->moveWindowId);
		NotNull(window);
		NotNull(window->handle);
		Assert(!window->closed);
		
		//TODO: Should we take out border/titlebar size into account here?
		//      Do we need to meet the exact position the application wanted the window to move to?
		Platform->movingWindowGlfwPntr = window->handle;
		if (window->input.maximized || window->input.minimized)
		{
			glfwRestoreWindow(window->handle);
		}
		glfwSetWindowPos(window->handle, output->moveWindowRec.x, output->moveWindowRec.y);
		glfwSetWindowSize(window->handle, output->moveWindowRec.width, output->moveWindowRec.height);
		Platform->movingWindowGlfwPntr = nullptr;
		//TODO: Do we need to do any masking of events to make sure we don't get false feedback for stuff we enacted?
	}
	
	// +==============================+
	// |       changeFullscreen       |
	// +==============================+
	if (output->changeFullscreen)
	{
		PlatWindow_t* window = Win32_GetWindowById(output->changeFullscreenWindowId);
		NotNull(window);
		NotNull(window->handle);
		Assert(!window->closed);
		
		if (output->fullscreenEnabled)
		{
			const PlatMonitorInfo_t* monitor = Win32_GetMonitorById(output->fullscreenMonitorId);
			NotNull(monitor);
			Assert(output->fullscreenVideoModeIndex < monitor->videoModes.length);
			const PlatMonitorVideoMode_t* videoMode = VarArrayGet(&monitor->videoModes, output->fullscreenVideoModeIndex, PlatMonitorVideoMode_t);
			Assert(output->fullscreenFramerateIndex < videoMode->numFramerates);
			i64 framerate = videoMode->framerates[output->fullscreenFramerateIndex];
			
			Platform->movingWindowGlfwPntr = window->handle;
			glfwSetWindowMonitor(window->handle,
				monitor->glfwHandle,
				0, 0,
				videoMode->resolution.width, videoMode->resolution.height,
				(i32)framerate
			);
			Platform->movingWindowGlfwPntr = nullptr;
			
			if (!window->activeInput.fullscreen)
			{
				window->activeInput.fullscreen = true;
				window->activeInput.fullscreenChanged = true;
			}
			window->activeInput.fullscreenMonitor = monitor;
			window->activeInput.fullscreenVideoMode = videoMode;
			window->activeInput.fullscreenFramerateIndex = output->fullscreenFramerateIndex;
			window->activeInput.fullscreenFramerate = framerate;
			//TODO: Do we need to manually update framebufferSize, pixelResolution, renderResolution, desktopRec, etc?
		}
		else
		{
			const PlatMonitorInfo_t* monitor = window->activeInput.fullscreenMonitor;
			if (!window->activeInput.fullscreen)
			{
				monitor = LinkedListGet(&Platform->monitors.list, PlatMonitorInfo_t, Platform->monitors.primaryIndex);
				NotNull(monitor);
			}
			
			v2i windowPosition = NewVec2i(
				monitor->desktopSpaceRec.x + monitor->desktopSpaceRec.width/2 - output->windowedResolution.width/2,
				monitor->desktopSpaceRec.y + monitor->desktopSpaceRec.height/2 - output->windowedResolution.height/2
			);
			
			Platform->movingWindowGlfwPntr = window->handle;
			if (window->activeInput.fullscreen)
			{
				glfwSetWindowMonitor(window->handle,
					nullptr,
					windowPosition.x, windowPosition.y,
					output->windowedResolution.width, output->windowedResolution.height,
					(i32)output->windowedFramerate
				);
				
				if (window->activeInput.maximized || window->input.minimized)
				{
					glfwRestoreWindow(window->handle);
				}
				
				window->activeInput.fullscreen = false;
				window->activeInput.fullscreenChanged = true;
				//TODO: Make a function that will update information about the window after coming out of fullscreen? Maybe we don't need to if we always get all the appropriate callbacks?
			}
			else
			{
				if (window->activeInput.maximized || window->input.minimized)
				{
					glfwRestoreWindow(window->handle);
				}
				glfwSetWindowPos(window->handle, windowPosition.x, windowPosition.y);
				glfwSetWindowSize(window->handle, output->windowedResolution.width, output->windowedResolution.height);
			}
			Platform->movingWindowGlfwPntr = nullptr;
		}
	}
}

