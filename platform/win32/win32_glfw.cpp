/*
File:   win32_glfw.cpp
Author: Taylor Robbins
Date:   09\24\2021
Description: 
	** Holds the initilization and interop code for GLFW
*/

#define MIN_OPENGL_VERSION_MAJOR  3
#define MIN_OPENGL_VERSION_MINOR  0
#define OPENGL_REQUEST_VERSION_MAJOR 1
#define OPENGL_REQUEST_VERSION_MINOR 0
//NOTE: Setting this to true causes weird texture problems when we run with the Steam Overlay on top.
//      This setting is supposed to make sure that all deprecated functionality in version 3.0 of OpenGL
//      are unsupported. Maybe we are using some deprecated features that we shouldn't be?
#define OPENGL_FORCE_FORWARD_COMPAT  false
#define OPENGL_DEBUG_CONTEXT         DEBUG_BUILD
#define OPENGL_REQUEST_PROFILE       GLFW_OPENGL_ANY_PROFILE //GLFW_OPENGL_CORE_PROFILE

// +--------------------------------------------------------------+
// |                          Callbacks                           |
// +--------------------------------------------------------------+
PlatWindow_t* Win32_GetWindowByGlfwPntr(GLFWwindow* glfwWindowPntr)
{
	PlatWindow_t* window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		if (window->handle == glfwWindowPntr) { return window; }
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
	return nullptr;
}
PlatMonitorInfo_t* Win32_GetMonitorByGlfwPntr(GLFWmonitor* glfwMonitorPntr)
{
	PlatMonitorInfo_t* monitor = LinkedListFirst(&Platform->monitors.list, PlatMonitorInfo_t);
	for (u64 wIndex = 0; wIndex < Platform->monitors.list.count; wIndex++)
	{
		if (monitor->glfwHandle == glfwMonitorPntr) { return monitor; }
		monitor = LinkedListNext(&Platform->monitors.list, PlatMonitorInfo_t, monitor);
	}
	return nullptr;
}
PlatWindow_t* Win32_GetWindowById(u64 id)
{
	PlatWindow_t* window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		if (window->id == id) { return window; }
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
	return nullptr;
}
PlatMonitorInfo_t* Win32_GetMonitorById(u64 monitorId)
{
	PlatMonitorInfo_t* monitor = LinkedListFirst(&Platform->monitors.list, PlatMonitorInfo_t);
	for (u64 wIndex = 0; wIndex < Platform->monitors.list.count; wIndex++)
	{
		if (monitor->id == monitorId) { return monitor; }
		monitor = LinkedListNext(&Platform->monitors.list, PlatMonitorInfo_t, monitor);
	}
	return nullptr;
}

void Win32_GlfwErrorCallback(i32 errorCode, const char* description)
{
	PrintLine_E("GLFW Error: 0x%08X \"%s\"", errorCode, description);
	//TODO: Should we store the error code and description somewhere?
}

void Win32_GlfwDebugOutputCallback(const char* filePath, u32 lineNumber, const char* funcName, bool isError, bool newLine, const char* message)
{
	AssertSingleThreaded();
	//TODO: Should we add a flag to indicate the message comes from GLFW?
	Win32_DebugOutputFromPlat(0x00, filePath, lineNumber, funcName, isError ? DbgLevel_Error : DbgLevel_None, newLine, message);
}

void Win32_GlfwJoystickCallback(int jid, int event)
{
	AssertSingleThreaded();
	if (event == GLFW_CONNECTED)
	{
		// PrintLine_D("Joystick[%d]: Connected", jid);
	}
	else if (event == GLFW_DISCONNECTED)
	{
		// PrintLine_D("Joystick[%d]: Disconnected", jid);
	}
	else
	{
		PrintLine_W("Joystick[%d]: Unknown event %d (0x%08X)", jid, event, event);
	}
}

void Win32_GlfwWindowCloseCallback(GLFWwindow* glfwWindowPntr)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowCloseCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	if (Platform->engine.isValid && InitPhase >= Win32InitPhase_DoingFirstUpdate)
	{
		bool shouldClose = Platform->engine.ShouldWindowClose(&Platform->info, &Platform->api, &Platform->engineMemory, window);
		if (!shouldClose)
		{
			PrintLine_W("Engine DLL requested that we don't allow window[%u] to close", GetItemIndexInLinkedList(&Platform->windows, window));
			glfwSetWindowShouldClose(window->handle, false);
		}
	}
}

void Win32_GlfwWindowSizeCallback(GLFWwindow* glfwWindowPntr, int windowWidth, int windowHeight)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowSizeCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwGetFramebufferSize(glfwWindowPntr, &framebufferWidth, &framebufferHeight);
	
	//Ignore this size event. It's probably because we are minimized
	if (framebufferWidth <= 0 || framebufferHeight <= 0) { return; }
	
	int windowLeftBorderSize = 0;
	int windowTopBorderSize = 0;
	int windowRightBorderSize = 0;
	int windowBottomBorderSize = 0;
	glfwGetWindowFrameSize(glfwWindowPntr, &windowLeftBorderSize, &windowTopBorderSize, &windowRightBorderSize, &windowBottomBorderSize);
	
	//NOTE: On windows 10, this returns an extra 8 pixels of extra padding on all sides. We don't actually want that
	if (windowLeftBorderSize >= 8) { windowLeftBorderSize -= 8; }
	else { windowLeftBorderSize = 0; }
	if (windowTopBorderSize >= 8) { windowTopBorderSize -= 8; }
	else { windowTopBorderSize = 0; }
	if (windowRightBorderSize >= 8) { windowRightBorderSize -= 8; }
	else { windowRightBorderSize = 0; }
	if (windowBottomBorderSize >= 8) { windowBottomBorderSize -= 8; }
	else { windowBottomBorderSize = 0; }
	
	//NOTE: We really don't like having to deal with 0 or negative resolutions so we
	//      will just maintain whatever valid resolution we had if we minimize or something
	if (windowWidth <= 0) { windowWidth = window->activeInput.windowResolution.width; }
	if (windowHeight <= 0) { windowHeight = window->activeInput.windowResolution.height; }
	if (framebufferWidth <= 0) { framebufferWidth = window->activeInput.pixelResolution.width; }
	if (framebufferHeight <= 0) { framebufferHeight = window->activeInput.pixelResolution.height; }
	
	if (window->activeInput.pixelResolution.width != framebufferWidth || window->activeInput.pixelResolution.height != framebufferHeight ||
		window->activeInput.windowResolution.width != windowWidth || window->activeInput.windowResolution.height != windowHeight)
	{
		window->activeInput.pixelResolution   = NewVec2i(framebufferWidth, framebufferHeight);
		window->activeInput.windowResolution  = NewVec2i(     windowWidth,      windowHeight);
		window->activeInput.contextResolution = NewVec2i(framebufferWidth, framebufferHeight); //TODO: When is this different? Only on OSX or High DPI screens?
		window->activeInput.renderResolution  = NewVec2 ((r32)windowWidth, (r32)windowHeight); //TODO: When is this different? Only on OSX or High DPI screens?
		window->activeInput.desktopRec.size = NewVec2i(
			windowLeftBorderSize + windowWidth + windowRightBorderSize,
			windowTopBorderSize + windowHeight + windowBottomBorderSize
		);
		window->activeInput.desktopInnerRec.topLeft = NewVec2i(
			window->activeInput.desktopRec.x + windowLeftBorderSize,
			window->activeInput.desktopRec.y + windowTopBorderSize
		);
		window->activeInput.desktopInnerRec.size = NewVec2i(windowWidth, windowHeight);
		if (InitPhase >= Win32InitPhase_DoingFirstUpdate)
		{
			window->activeInput.resized = true;
		}
	}
	
	if (!window->activeInput.maximized)
	{
		window->activeInput.unmaximizedWindowSize = window->activeInput.windowResolution;
	}
}

void Win32_GlfwWindowMoveCallback(GLFWwindow* glfwWindowPntr, int posX, int posY)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowMoveCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	//Ignore insane negative values for window position. It's probably use getting minimized (in testing I get (-3200, -32000) when getting minimized)
	if (posX < Platform->monitors.desktopRec.x - 100 || posY < Platform->monitors.desktopRec.y - 100) { return; }
	
	// PrintLine_D("Window Moved (%d, %d)", posX, posY);
	window->activeInput.windowInteractionOccurred = true;
	if (window->activeInput.desktopInnerRec.x != posX || window->activeInput.desktopInnerRec.y != posY)
	{
		v2i desktopInnerRecOffset = window->activeInput.desktopInnerRec.topLeft - window->activeInput.desktopRec.topLeft;
		window->activeInput.desktopInnerRec.topLeft = NewVec2i((i32)posX, (i32)posY);
		window->activeInput.desktopRec.topLeft = window->activeInput.desktopInnerRec.topLeft - desktopInnerRecOffset;
		window->activeInput.moved = true;
	}
	
	if (!window->activeInput.maximized && !window->activeInput.minimized)
	{
		window->activeInput.prevUnmaximizedWindowPos = window->activeInput.unmaximizedWindowPos;
		window->activeInput.unmaximizedWindowPos = window->activeInput.desktopInnerRec.topLeft;
		// PrintLine_D("unmaximizedWindowPos: (%d, %d)", window->activeInput.unmaximizedWindowPos.x, window->activeInput.unmaximizedWindowPos.y);
	}
}

void Win32_GlfwWindowMinimizeCallback(GLFWwindow* glfwWindowPntr, int isMinimized)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowMinimizeCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	// PrintLine_D("Window %s", ((isMinimized > 0) ? "Minimized" : "Unminimized"));
	window->activeInput.windowInteractionOccurred = true;
	if (window->activeInput.minimized != (isMinimized > 0))
	{
		window->activeInput.minimized = (isMinimized > 0);
		window->activeInput.minimizedChanged = true;
		if (window->activeInput.minimized)
		{
			window->activeInput.unmaximizedWindowPos = window->activeInput.prevUnmaximizedWindowPos;
			// PrintLine_D("Revert unmaximizedWindowPos: (%d, %d)", window->activeInput.unmaximizedWindowPos.x, window->activeInput.unmaximizedWindowPos.y);
		}
		else if (!window->activeInput.maximized)
		{
			int windowPosX = 0;
			int windowPosY = 0;
			glfwGetWindowPos(glfwWindowPntr, &windowPosX, &windowPosY);
			window->activeInput.prevUnmaximizedWindowPos = window->activeInput.unmaximizedWindowPos;
			window->activeInput.unmaximizedWindowPos = NewVec2i(windowPosX, windowPosY);
			// PrintLine_D("Forced update unmaximizedWindowPos: (%d, %d)", window->activeInput.unmaximizedWindowPos.x, window->activeInput.unmaximizedWindowPos.y);
		}
	}
}

void Win32_GlfwWindowMaximizeCallback(GLFWwindow* glfwWindowPntr, int isMaximized)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowMaximizeCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	// PrintLine_D("Window %s", ((isMaximized > 0) ? "Maximized" : "Unmaximized"));
	window->activeInput.windowInteractionOccurred = true;
	if (window->activeInput.maximized != (isMaximized > 0))
	{
		window->activeInput.maximized = (isMaximized > 0);
		window->activeInput.maximizedChanged = true;
		if (window->activeInput.maximized)
		{
			window->activeInput.unmaximizedWindowPos = window->activeInput.prevUnmaximizedWindowPos;
			// PrintLine_D("Revert unmaximizedWindowPos: (%d, %d)", window->activeInput.unmaximizedWindowPos.x, window->activeInput.unmaximizedWindowPos.y);
		}
		else if (!window->activeInput.minimized)
		{
			int windowPosX = 0;
			int windowPosY = 0;
			glfwGetWindowPos(glfwWindowPntr, &windowPosX, &windowPosY);
			window->activeInput.prevUnmaximizedWindowPos = window->activeInput.unmaximizedWindowPos;
			window->activeInput.unmaximizedWindowPos = NewVec2i(windowPosX, windowPosY);
			// PrintLine_D("Forced update unmaximizedWindowPos: (%d, %d)", window->activeInput.unmaximizedWindowPos.x, window->activeInput.unmaximizedWindowPos.y);
			
			if (window->activeInput.desktopInnerRec.x != windowPosX || window->activeInput.desktopInnerRec.y != windowPosY)
			{
				v2i desktopInnerRecOffset = window->activeInput.desktopInnerRec.topLeft - window->activeInput.desktopRec.topLeft;
				window->activeInput.desktopInnerRec.topLeft = NewVec2i((i32)windowPosX, (i32)windowPosY);
				window->activeInput.desktopRec.topLeft = window->activeInput.desktopInnerRec.topLeft - desktopInnerRecOffset;
				window->activeInput.moved = true;
			}
		}
	}
}

void Win32_GlfwWindowFocusCallback(GLFWwindow* glfwWindowPntr, int isFocused)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwWindowFocusCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	if (window->activeInput.isFocused != (isFocused > 0))
	{
		window->activeInput.isFocused = (isFocused > 0);
		window->activeInput.isFocusedChanged = true;
		
		ModifierKey_t modifiers = Win32_GetCurrentModifierKeys();
		InputEvent_t* newEvent = Win32_CreateInputEvent(window, &Platform->engineActiveInput, modifiers, InputEventType_WindowFocus);
		if (newEvent != nullptr)
		{
			newEvent->windowFocus.focused = (isFocused > 0);
			if (newEvent->windowFocus.focused) { newEvent->windowFocus.newWindow = window; }
			else { newEvent->windowFocus.oldWindow = window; }
			Win32_PairInputEvent(&Platform->engineActiveInput, newEvent);
		}
	}
}

void Win32_GlfwKeyPressedCallback(GLFWwindow* glfwWindowPntr, int key, int scanCode, int action, int modifiers)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	UNUSED(scanCode); //TODO: Should we do something with this?
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwKeyPressedCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	Win32_HandleKeyEvent(window, &Platform->engineActiveInput, key, action, modifiers);
}

void Win32_GlfwCharPressedCallback(GLFWwindow* glfwWindowPntr, u32 codepoint)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwCharPressedCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	ModifierKey_t modifiers = Win32_GetCurrentModifierKeys();
	InputEvent_t* newEvent = Win32_CreateInputEvent(window, &Platform->engineActiveInput, modifiers, InputEventType_Character);
	if (newEvent != nullptr)
	{
		newEvent->character.codepoint = codepoint;
		Win32_PairInputEvent(&Platform->engineActiveInput, newEvent);
	}
}

void Win32_GlfwCursorPosCallback(GLFWwindow* glfwWindowPntr, double mouseX, double mouseY)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwCursorPosCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	if (!BasicallyEqualR32(window->activeInput.mousePos.x, (r32)mouseX) || !BasicallyEqualR32(window->activeInput.mousePos.y, (r32)mouseY))
	{
		//TODO: Do we need to do any manipulations of this value for high DPI screens?
		window->activeInput.mouseDelta.x += (r32)(mouseX - window->activeInput.mousePos.x);
		window->activeInput.mouseDelta.y += (r32)(mouseY - window->activeInput.mousePos.y);
		window->activeInput.mousePos = NewVec2((r32)mouseX, (r32)mouseY);
		window->activeInput.mousePosi = Vec2Roundi(window->activeInput.mousePos);
		window->activeInput.mouseMoved = true;
	}
}

void Win32_GlfwMousePressCallback(GLFWwindow* glfwWindowPntr, int button, int action, int modifiers)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwMousePressCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	Win32_HandleMouseEvent(window, &Platform->engineActiveInput, button, action, modifiers);
}

//TODO: Add events for mouse scrolling
void Win32_GlfwMouseScrollCallback(GLFWwindow* glfwWindowPntr, r64 deltaX, r64 deltaY)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwMousePressCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	if (!BasicallyEqualR64(deltaX, 0.0))
	{
		Platform->engineActiveInput.scrollChangedX = true;
		Platform->engineActiveInput.scrollValue.x += (r32)deltaX;
		Platform->engineActiveInput.scrollDelta.x += (r32)deltaX;
	}
	if (!BasicallyEqualR64(deltaY, 0.0))
	{
		Platform->engineActiveInput.scrollChangedY = true;
		Platform->engineActiveInput.scrollValue.y += (r32)deltaY;
		Platform->engineActiveInput.scrollDelta.y += (r32)deltaY;
	}
}

void Win32_GlfwCursorEnteredCallback(GLFWwindow* glfwWindowPntr, int entered)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwCursorEnteredCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	if (window->activeInput.mouseInsideWindow != (entered > 0))
	{
		window->activeInput.mouseInsideWindow = (entered > 0);
		window->activeInput.mouseInsideWindowChanged = true;
		window->activeInput.mouseMoved = true;
		
		ModifierKey_t modifiers = Win32_GetCurrentModifierKeys();
		InputEvent_t* newEvent = Win32_CreateInputEvent(window, &Platform->engineActiveInput, modifiers, InputEventType_MouseHover);
		if (newEvent != nullptr)
		{
			newEvent->mouseHover.entered = (entered > 0);
			if (newEvent->mouseHover.entered) { newEvent->mouseHover.newWindow = window; }
			else { newEvent->mouseHover.oldWindow = window; }
			Win32_PairInputEvent(&Platform->engineActiveInput, newEvent);
		}
	}
}

void Win32_GlfwFileDropCallback(GLFWwindow* glfwWindowPntr, int count, const char** filePaths)
{
	AssertSingleThreaded();
	NotNull(glfwWindowPntr);
	Assert(count >= 0);
	Assert(filePaths != nullptr || count == 0);
	if (count == 0) { return; }
	
	PlatWindow_t* window = Win32_GetWindowByGlfwPntr(glfwWindowPntr);
	if (window == nullptr)
	{
		PrintLine_E("Got Win32_GlfwFileDropCallback for unknown window %p out of %llu window(s)", glfwWindowPntr, Platform->windows.count);
		return;
	}
	if (window->closed) { WriteLine_W("Got GLFW callback for closed window?"); return; }
	
	for (u64 fIndex = 0; fIndex < (u64)count; fIndex++)
	{
		ModifierKey_t modifiers = Win32_GetCurrentModifierKeys();
		InputEvent_t* newEvent = Win32_CreateInputEvent(window, &Platform->engineActiveInput, modifiers, InputEventType_FileDropped);
		if (newEvent != nullptr)
		{
			newEvent->droppedFile.filePath = NewStringInArenaNt(&Platform->mainHeap, filePaths[fIndex]);
			NotNullStr(&newEvent->droppedFile.filePath);
		}
	}
}

void Win32_GlfwSystemEventCallback(GLFWwindow* glfwWindowPntr, unsigned int uMsg, __int64 wParam, __int64 lParam)
{
	UNUSED(glfwWindowPntr);
	UNUSED(wParam);
	UNUSED(lParam);
	switch (uMsg)
	{
		//NOTE: This allows us to re-render the screen when the user is resizing or moving the window
		case WM_PAINT:
		case WM_MOVE:
		case WM_SIZE:
		// case WM_SIZING:
		{
			if (InitPhase >= Win32InitPhase_PostFirstUpdate && glfwWindowPntr != Platform->movingWindowGlfwPntr && IsMainThread() && !Platform->isChangingMinimization)
			{
				Win32_DoMainLoopIteration(false);
			}
		} break;
	}
}

void Win32_RegisterCallbacks()
{
	glfwSetJoystickCallback(Win32_GlfwJoystickCallback);
}

void Win32_RegisterWindowCallbacks(PlatWindow_t* window)
{
	glfwSetWindowCloseCallback(window->handle,     Win32_GlfwWindowCloseCallback);
	glfwSetFramebufferSizeCallback(window->handle, Win32_GlfwWindowSizeCallback);
	glfwSetWindowPosCallback(window->handle,       Win32_GlfwWindowMoveCallback);
	glfwSetWindowMaximizeCallback(window->handle,  Win32_GlfwWindowMaximizeCallback);
	glfwSetWindowIconifyCallback(window->handle,   Win32_GlfwWindowMinimizeCallback);
	glfwSetWindowFocusCallback(window->handle,     Win32_GlfwWindowFocusCallback);
	glfwSetKeyCallback(window->handle,             Win32_GlfwKeyPressedCallback);
	glfwSetCharCallback(window->handle,            Win32_GlfwCharPressedCallback);
	glfwSetCursorPosCallback(window->handle,       Win32_GlfwCursorPosCallback);
	glfwSetMouseButtonCallback(window->handle,     Win32_GlfwMousePressCallback);
	glfwSetScrollCallback(window->handle,          Win32_GlfwMouseScrollCallback);
	glfwSetCursorEnterCallback(window->handle,     Win32_GlfwCursorEnteredCallback);
	glfwSetDropCallback(window->handle,            Win32_GlfwFileDropCallback);
	glfwSetSystemEventCallback(window->handle,     Win32_GlfwSystemEventCallback);
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_GlfwInit()
{
	WriteLine_N("Initializing GLFW...");
	
	Platform->nextWindowId = 1;
	CreateLinkedList(&Platform->windows, &Platform->mainHeap, PlatWindow_t);
	
	glfwSetErrorCallback(Win32_GlfwErrorCallback);
	WriteLine_I("GLFW error callback registered");
	
	// +==============================+
	// |           glfwInit           |
	// +==============================+
	if (!glfwInit(Win32_GlfwDebugOutputCallback))
	{
		Win32_InitError("GLFW Initialization Failed!");
	}
	WriteLine_I("glfwInit succeeded!");
	
	glfwGetVersion(&Platform->glfwVersion.major, &Platform->glfwVersion.minor, &Platform->glfwVersion.revision);
	PrintLine_D("GLFW Version: %d.%d(%d)", Platform->info.glfwVersion.major, Platform->info.glfwVersion.minor, Platform->info.glfwVersion.revision);
	
	//TODO: Call
	//      int glfwUpdateGamepadMappings(const char* string);
	//      using the gamecontrollerdb.txt from SDL
	
	Win32_RegisterCallbacks();
	
	WriteLine_N("Initializing GLFW Done!");
}

bool Win32_LoadGameControllerDbFile(MyStr_t filePath)
{
	bool result = false;
	PlatFileContents_t dbFile = {};
	if (Win32_ReadFileContents(filePath, &dbFile))
	{
		int updateMappingResult = glfwUpdateGamepadMappings(dbFile.chars);
		if (updateMappingResult == GLFW_TRUE)
		{
			PrintLine_I("Loaded controller DB file at \"%.*s\"", filePath.length, filePath.pntr);
		}
		else
		{
			PrintLine_E("Failed to parse the controller DB file at \"%.*s\"", filePath.length, filePath.pntr);
		}
		Win32_FreeFileContents(&dbFile);
	}
	else
	{
		PrintLine_W("No controller DB file at \"%.*s\"", filePath.length, filePath.pntr);
	}
	return result;
}

void Win32_GladInit()
{
	WriteLine_R("Initializing GLAD...");
	// struct timeb gladInitStartTime; ftime(&gladInitStartTime);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		Win32_InitError("Could not initialize GLAD. gladLoadGLLoader failed");
	}
	
	//TODO: Use these globals that are now populated
	// GLVersion.major
	// GLVersion.minor
	// if (GLAD_GL_VERSION_3_0) { /* at least OpenGL 3.0 is supported */ }
	// if (GLAD_GL_EXT_framebuffer_multisample) { /* it's supported */ }
	
	// struct timeb gladInitEndTime; ftime(&gladInitEndTime);
	// u32 gladInitElapsedTime = (u32)(1000.0 * (gladInitEndTime.time - gladInitStartTime.time) + (gladInitEndTime.millitm - gladInitStartTime.millitm));
	// PrintLine_D("gladInit took %s", FormattedMilliseconds(gladInitElapsedTime));
	WriteLine_R("GLAD Init Succeeded!");
}

// +==============================+
// | Win32_GetLoadProcAddressFunc |
// +==============================+
// GLADloadproc GetLoadProcAddressFunc()
PLAT_API_GET_LOAD_PROC_ADDRESS_FUNC(Win32_GetLoadProcAddressFunc)
{
	return (GLADloadproc)glfwGetProcAddress;
}

// +--------------------------------------------------------------+
// |                           Cleanup                            |
// +--------------------------------------------------------------+
void Win32_GlfwCleanup()
{
	PlatWindow_t* windowPntr = LinkedListFirst(&Platform->windows, PlatWindow_t);
	while (windowPntr != nullptr)
	{
		if (!windowPntr->closed)
		{
			//TODO: Should we let the engine DLL know that a window is closing? Maybe they can reject the close?
			glfwDestroyWindow(windowPntr->handle);
			windowPntr->closed = true;
			windowPntr->handle = nullptr;
			Assert(Platform->numOpenWindows > 0);
			Platform->numOpenWindows--;
		}
		windowPntr = LinkedListNext(&Platform->windows, PlatWindow_t, windowPntr);
	}
}

// +--------------------------------------------------------------+
// |                        Create Window                         |
// +--------------------------------------------------------------+
PlatWindow_t* Win32_GlfwCreateWindow(const PlatWindowCreateOptions_t* options)
{
	NotNull(options);
	AssertNullTerm(&options->windowTitle);
	AssertSingleThreaded();
	
	if (Platform->renderApi == RenderApi_OpenGL)
	{
		glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_REQUEST_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_REQUEST_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, OPENGL_FORCE_FORWARD_COMPAT ? GLFW_TRUE : GLFW_FALSE); //Makes MacOSX happy?
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  OPENGL_DEBUG_CONTEXT ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_OPENGL_PROFILE,        OPENGL_REQUEST_PROFILE);
	}
	else
	{
		AssertMsg(false, "Unsupported render mode requested when creating window!");
		return nullptr;
	}
	
	PlatWindow_t* newWindow = LinkedListAdd(&Platform->windows, PlatWindow_t);
	NotNull(newWindow);
	ClearPointer(newWindow);
	MyMemCopy(&newWindow->options, options, sizeof(PlatWindowOptions_t));
	
	glfwWindowHint(GLFW_RESIZABLE,      options->resizableWindow ? GL_TRUE : GL_FALSE);
	glfwWindowHint(GLFW_FLOATING,      	options->topmostWindow   ? GL_TRUE : GL_FALSE);
	glfwWindowHint(GLFW_DECORATED,      options->decoratedWindow ? GL_TRUE : GL_FALSE);
	glfwWindowHint(GLFW_FOCUSED,        GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER,   GL_TRUE);
	glfwWindowHint(GLFW_RED_BITS,       8);
	glfwWindowHint(GLFW_GREEN_BITS,     8);
	glfwWindowHint(GLFW_BLUE_BITS,      8);
	glfwWindowHint(GLFW_ALPHA_BITS,     8);
	glfwWindowHint(GLFW_DEPTH_BITS,     8);
	glfwWindowHint(GLFW_STENCIL_BITS,   8);
	glfwWindowHint(GLFW_SAMPLES,        (int)options->antialiasingNumSamples);
	glfwWindowHint(GLFW_AUTO_ICONIFY,   options->autoIconify ? GL_TRUE : GL_FALSE);
	
	if (options->fullscreen)
	{
		NotNull(options->fullscreenMonitor);
		NotNull(options->fullscreenVideoMode);
		Assert(options->fullscreenFramerateIndex < options->fullscreenVideoMode->numFramerates);
		
		glfwWindowHint(GLFW_REFRESH_RATE, (int)options->fullscreenVideoMode->framerates[options->fullscreenFramerateIndex]);
		
		newWindow->handle = glfwCreateWindow(
			options->fullscreenVideoMode->resolution.width,
			options->fullscreenVideoMode->resolution.height,
			options->windowTitle.pntr,
			options->fullscreenMonitor->glfwHandle,
			((Platform->mainWindow != nullptr) ? Platform->mainWindow->handle : nullptr)
		);
	}
	else
	{
		glfwWindowHint(GLFW_REFRESH_RATE, (int)options->windowedFramerate);
		
		newWindow->handle = glfwCreateWindow(
			options->windowedResolution.width,
			options->windowedResolution.height,
			options->windowTitle.pntr,
			nullptr,
			((Platform->mainWindow != nullptr) ? Platform->mainWindow->handle : nullptr)
		);
	}
	
	if (newWindow->handle == nullptr)
	{
		WriteLine_E("Failed to create GLFW window.");
		LinkedListRemove(&Platform->windows, PlatWindow_t, newWindow);
		return nullptr;
	}
	newWindow->id = Platform->nextWindowId;
	Platform->nextWindowId++;
	Platform->numOpenWindows++;
	
	if (options->windowedLocation != NewVec2i(-1, -1))
	{
		glfwSetWindowPos(newWindow->handle, options->windowedLocation.x, options->windowedLocation.y);
	}
	if (options->windowedMaximized)
	{
		glfwMaximizeWindow(newWindow->handle);
	}
	
	Win32_InitWindowEngineInput(newWindow, options, &newWindow->activeInput);
	Win32_CopyWindowEngineInput(&newWindow->input, &newWindow->activeInput);
	Win32_CopyWindowEngineInput(&newWindow->prevInput, &newWindow->activeInput);
	
	Win32_RegisterWindowCallbacks(newWindow);
	
	return newWindow;
}

void Win32_ApplyWindowOptions(PlatWindow_t* window, PlatWindowOptions_t* options)
{
	NotNull(window);
	NotNull(options);
	Assert(!window->closed);
	NotNull(window->handle);
	glfwSetWindowSizeLimits(window->handle,
		options->enforceMinSize ? options->minWindowSize.width  : GLFW_DONT_CARE,
		options->enforceMinSize ? options->minWindowSize.height : GLFW_DONT_CARE,
		options->enforceMaxSize ? options->maxWindowSize.width  : GLFW_DONT_CARE,
		options->enforceMaxSize ? options->maxWindowSize.height : GLFW_DONT_CARE
	);
	if (options->forceAspectRatio)
	{
		Assert(options->aspectRatio.height > 0);
		glfwSetWindowAspectRatio(window->handle, options->aspectRatio.width, options->aspectRatio.height);
	}
	
	//TODO: Calculate the swap interval based off framerate options passed to us by the
	//      engine DLL and information about the monitor we are rendering through
	glfwSwapInterval(1);
}

void Win32_PollEventsAndCheckWindows()
{
	glfwPollEvents();
	
	PlatWindow_t* windowPntr = LinkedListFirst(&Platform->windows, PlatWindow_t);
	while (windowPntr != nullptr)
	{
		if (!windowPntr->closed && glfwWindowShouldClose(windowPntr->handle))
		{
			//TODO: Should we let the engine DLL know that a window is closing? Maybe they can reject the close?
			glfwDestroyWindow(windowPntr->handle);
			windowPntr->closed = true;
			windowPntr->handle = nullptr;
			Assert(Platform->numOpenWindows > 0);
			Platform->numOpenWindows--;
		}
		
		if (!windowPntr->closed)
		{
			//TODO: Anything else we should check on with each window?
		}
		
		windowPntr = LinkedListNext(&Platform->windows, PlatWindow_t, windowPntr);
	}
}

void Win32_LoadWindowIcon(PlatWindow_t* window, u64 numIconFiles, MyStr_t* iconFilePaths)
{
	if (numIconFiles > 0)
	{
		TempPushMark();
		NotNull(iconFilePaths);
		u64 numGlfwIconsLoaded = 0;
		GLFWimage* glfwIcons = AllocArray(TempArena, GLFWimage, numIconFiles);
		NotNull(glfwIcons);
		for (u64 fIndex = 0; fIndex < numIconFiles; fIndex++)
		{
			PlatFileContents_t iconFile;
			AssertNullTerm(&iconFilePaths[fIndex]);
			if (Win32_ReadFileContents(iconFilePaths[fIndex], &iconFile))
			{
				int bitmapNumChannels = 0;
				int bitmapWidth = 0;
				int bitmapHeight = 0;
				u8* bitmapData = stbi_load_from_memory(iconFile.data, (int)iconFile.size, &bitmapWidth, &bitmapHeight, &bitmapNumChannels, 4);
				if (bitmapData != nullptr && bitmapWidth > 0 && bitmapHeight > 0)
				{
					GLFWimage* glfwIcon = &glfwIcons[numGlfwIconsLoaded];
					glfwIcon->width = bitmapWidth;
					glfwIcon->height = bitmapHeight;
					glfwIcon->pixels = bitmapData;
					numGlfwIconsLoaded++;
				}
				else
				{
					DebugAssertMsg(false, "Failed to parse one of the icons that was requested by the application");
				}
				
				Win32_FreeFileContents(&iconFile);
			}
			else
			{
				DebugAssertMsg(false, "Failed to load one of the icons that was requested by the application. Make sure the .exe is next to the Resources folder or use -w command line option to load resources from the working directory.");
			}
		}
		
		if (numGlfwIconsLoaded > 0)
		{
			PrintLine_I("Loading %u icons into the GLFW window", numGlfwIconsLoaded);
			glfwSetWindowIcon(window->handle, (int)numGlfwIconsLoaded, glfwIcons);
		}
		
		for (u64 iIndex = 0; iIndex < numGlfwIconsLoaded; iIndex++)
		{
			stbi_image_free(glfwIcons[iIndex].pixels);
		}
		TempPopMark();
	}
}

// +--------------------------------------------------------------+
// |                     Controller Handling                      |
// +--------------------------------------------------------------+
ControllerType_t Win32_GetControllerTypeByGuidStr(MyStr_t guidStr)
{
	if (IsEmptyStr(guidStr)) { return ControllerType_Unknown; }
	for (u64 guidIndex = 0; guidIndex < ArrayCount(win32_PlaystationControllerGuids); guidIndex++)
	{
		if (StrEqualsIgnoreCase(guidStr, win32_PlaystationControllerGuids[guidIndex]))
		{
			return ControllerType_Playstation;
		}
	}
	for (u64 guidIndex = 0; guidIndex < ArrayCount(win32_NintendoControllerGuids); guidIndex++)
	{
		if (StrEqualsIgnoreCase(guidStr, win32_NintendoControllerGuids[guidIndex]))
		{
			return ControllerType_Nintendo;
		}
	}
	for (u64 guidIndex = 0; guidIndex < ArrayCount(win32_XboxControllerGuids); guidIndex++)
	{
		if (StrEqualsIgnoreCase(guidStr, win32_XboxControllerGuids[guidIndex]))
		{
			return ControllerType_Xbox;
		}
	}
	return ControllerType_Unknown;
}

void Win32_CheckControllerInputs(EngineInput_t* currentInput)
{
	NotNull(currentInput);
	for (u64 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
	{
		PlatControllerState_t* controller = &currentInput->controllerStates[cIndex];
		
		bool isConnected = (glfwJoystickPresent((int)cIndex) == GLFW_TRUE);
		bool isGamepad = (glfwJoystickIsGamepad((int)cIndex) == GLFW_TRUE);
		if (controller->connected != isConnected)
		{
			PrintLineAt(isConnected ? DbgLevel_Info : DbgLevel_Warning, "Controller[%llu] %s!", cIndex, isConnected ? "Connected" : "Disconnected");
			controller->connected = isConnected;
			controller->connectedChanged = true;
		}
		
		const char* controllerGuidNt = glfwGetJoystickGUID((int)cIndex);
		if (!StrEquals(controller->typeIdStr, controllerGuidNt))
		{
			FreeString(&Platform->mainHeap, &controller->typeIdStr);
			if (controllerGuidNt != nullptr)
			{
				controller->typeIdStr = NewStringInArenaNt(&Platform->mainHeap, controllerGuidNt);
			}
		}
		
		if (isConnected)
		{
			if (isGamepad)
			{
				GLFWgamepadstate gamepadState = {};
				int getStateResult = glfwGetGamepadState((int)cIndex, &gamepadState);
				DebugAssertAndUnused(getStateResult == GLFW_TRUE, getStateResult);
				
				controller->type = Win32_GetControllerTypeByGuidStr(controller->typeIdStr);
				controller->numAxes = GLFW_GAMEPAD_AXIS_LAST+1;
				controller->numButtons = GLFW_GAMEPAD_BUTTON_LAST+1;
				
				const char* controllerNameNt = glfwGetGamepadName((int)cIndex);
				if (!StrEquals(controller->name, controllerNameNt))
				{
					FreeString(&Platform->mainHeap, &controller->name);
					if (controllerNameNt != nullptr)
					{
						controller->name = NewStringInArenaNt(&Platform->mainHeap, controllerNameNt);
					}
				}
				
				Win32_UpdateControllerStick(currentInput,   controller, false, NewVec2(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X], gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]));
				Win32_UpdateControllerStick(currentInput,   controller, true,  NewVec2(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]));
				Win32_UpdateControllerTrigger(currentInput, controller, false, (gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]  + 1) / 2);
				Win32_UpdateControllerTrigger(currentInput, controller, true,  (gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1) / 2);
				
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_A,           (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_A]            == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_B,           (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_B]            == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_X,           (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_X]            == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Y,           (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_Y]            == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftBumper,  (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]  == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightBumper, (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Back,        (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_BACK]         == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Start,       (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_START]        == GLFW_PRESS));
				// Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Guide,       (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_GUIDE]        == GLFW_PRESS)); //Unused currently
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftStick,   (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB]   == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightStick,  (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]  == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Up,          (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]      == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Right,       (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]   == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Down,        (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]    == GLFW_PRESS));
				Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Left,        (gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]    == GLFW_PRESS));
			}
			else
			{
				controller->type = ControllerType_Joystick;
				
				i32 axisCount;
				const r32* axes = glfwGetJoystickAxes((int)cIndex, &axisCount);
				controller->numAxes = (u64)axisCount;
				i32 buttonCount;
				const u8* buttons = glfwGetJoystickButtons((int)cIndex, &buttonCount);
				controller->numButtons = (u64)buttonCount;
				
				const char* controllerNameNt = glfwGetJoystickName((int)cIndex);
				if (!StrEquals(controller->name, controllerNameNt))
				{
					FreeString(&Platform->mainHeap, &controller->name);
					if (controllerNameNt != nullptr)
					{
						controller->name = NewStringInArenaNt(&Platform->mainHeap, controllerNameNt);
					}
				}
				
				//TODO: Why are we making this distinction between controllers with <= 14 buttons?
				if (buttonCount == 18) //ad-hoc PS4 mapping
				{
					if (axisCount >= 2) { Win32_UpdateControllerStick(currentInput,   controller, false, NewVec2(axes[0], axes[1])); }
					if (axisCount >= 6) { Win32_UpdateControllerStick(currentInput,   controller, true,  NewVec2(axes[2], axes[5])); }
					if (axisCount >= 4) { Win32_UpdateControllerTrigger(currentInput, controller, false, (axes[3] + 1) / 2); }
					if (axisCount >= 5) { Win32_UpdateControllerTrigger(currentInput, controller, true,  (axes[4] + 1) / 2); }
					
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_X,           (buttons[0]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_A,           (buttons[1]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_B,           (buttons[2]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Y,           (buttons[3]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftBumper,  (buttons[4]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightBumper, (buttons[5]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Back,        (buttons[8]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Start,       (buttons[9]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftStick,   (buttons[10] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightStick,  (buttons[11] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Up,          (buttons[14] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Right,       (buttons[15] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Down,        (buttons[16] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Left,        (buttons[17] == GLFW_PRESS));
				}
				else if (buttonCount == 19) //ad-hoc PS5 mapping
				{
					if (axisCount >= 2) { Win32_UpdateControllerStick(currentInput,   controller, false, NewVec2(axes[0], axes[1])); }
					if (axisCount >= 6) { Win32_UpdateControllerStick(currentInput,   controller, true,  NewVec2(axes[2], axes[5])); }
					if (axisCount >= 4) { Win32_UpdateControllerTrigger(currentInput, controller, false, (axes[3] + 1) / 2); }
					if (axisCount >= 5) { Win32_UpdateControllerTrigger(currentInput, controller, true,  (axes[4] + 1) / 2); }
					
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_X,           (buttons[0]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_A,           (buttons[1]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_B,           (buttons[2]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Y,           (buttons[3]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftBumper,  (buttons[4]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightBumper, (buttons[5]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Back,        (buttons[8]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Start,       (buttons[9]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftStick,   (buttons[10] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightStick,  (buttons[11] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Up,          (buttons[15] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Right,       (buttons[16] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Down,        (buttons[17] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Left,        (buttons[18] == GLFW_PRESS));
				}
				else //default controller mapping (presumably Xbox)
				{
					if (axisCount >= 2) { Win32_UpdateControllerStick(currentInput,   controller, false, NewVec2(axes[0], axes[1])); }
					if (axisCount >= 4) { Win32_UpdateControllerStick(currentInput,   controller, true,  NewVec2(axes[2], axes[3])); }
					if (axisCount >= 5) { Win32_UpdateControllerTrigger(currentInput, controller, false, (axes[4] + 1) / 2); }
					if (axisCount >= 6) { Win32_UpdateControllerTrigger(currentInput, controller, true,  (axes[5] + 1) / 2); }
					
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_A,           (buttons[0]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_B,           (buttons[1]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_X,           (buttons[2]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Y,           (buttons[3]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftBumper,  (buttons[4]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightBumper, (buttons[5]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Back,        (buttons[6]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Start,       (buttons[7]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_LeftStick,   (buttons[8]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_RightStick,  (buttons[9]  == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Up,          (buttons[10] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Right,       (buttons[11] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Down,        (buttons[12] == GLFW_PRESS));
					Win32_UpdateControllerButton(currentInput, controller, ControllerBtn_Left,        (buttons[13] == GLFW_PRESS));
				}
			}
		}
		else
		{
			Win32_UpdateControllerStick(currentInput,   controller, false, Vec2_Zero);
			Win32_UpdateControllerStick(currentInput,   controller, true,  Vec2_Zero);
			Win32_UpdateControllerTrigger(currentInput, controller, false, 0.0f);
			Win32_UpdateControllerTrigger(currentInput, controller, true,  0.0f);
			for (u64 bIndex = 0; bIndex < ControllerBtn_NumBtns; bIndex++)
			{
				ControllerBtn_t controllerBtn = (ControllerBtn_t)bIndex;
				Win32_UpdateControllerButton(currentInput, controller, controllerBtn, false);
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                        API Functions                         |
// +--------------------------------------------------------------+
// +==============================+
// |   Win32_ChangeWindowTarget   |
// +==============================+
// void Win32_ChangeWindowTarget(const PlatWindow_t* window)
PLAT_API_CHANGE_WINDOW_TARGET_DEF(Win32_ChangeWindowTarget)
{
	AssertSingleThreaded();
	if (window == nullptr) { window = Platform->mainWindow; }
	Assert(IsItemInLinkedList(&Platform->windows, window));
	NotNull(window);
	glfwMakeContextCurrent(window->handle);
	Platform->currentWindow = (PlatWindow_t*)window;
}

// +==============================+
// |      Win32_SwapBuffers       |
// +==============================+
// void Win32_SwapBuffers()
PLAT_API_SWAP_BUFFERS_DEF(Win32_SwapBuffers)
{
	u64 lastWindowIndex = 0;
	PlatWindow_t* window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		NotNull(window);
		if (!window->closed) { lastWindowIndex = wIndex; }
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
	
	glfwSwapInterval(0);
	window = LinkedListFirst(&Platform->windows, PlatWindow_t);
	for (u64 wIndex = 0; wIndex < Platform->windows.count; wIndex++)
	{
		NotNull(window);
		if (!window->closed)
		{
			NotNull(window->handle);
			Win32_DrawOverlays(window);
			if (wIndex == lastWindowIndex)
			{
				//TODO: Make sure this interval is actually the correct value
				glfwSwapInterval(1);
			}
			
			// r64 timeBeforeSwap = glfwGetTime() * 1000.0;
			PerfTime_t timeBeforeSwap = Win32_GetPerfTime();
			glfwSwapBuffers(window->handle);
			// r64 timeAfterSwap = glfwGetTime() * 1000.0;
			PerfTime_t timeAfterSwap = Win32_GetPerfTime();
			Platform->timeSpentOnSwapBuffers += Win32_GetPerfTimeDiff(&timeBeforeSwap, &timeAfterSwap);
		}
		window = LinkedListNext(&Platform->windows, PlatWindow_t, window);
	}
}
