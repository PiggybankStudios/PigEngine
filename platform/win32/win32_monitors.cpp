/*
File:   win32_monitors.cpp
Author: Taylor Robbins
Date:   02\21\2022
Description: 
	** Holds functions that help us manage our information about the monitors on the users computer
	** This information is important for the engine to make decisions about what screen and
	** with what rendering options it wants to be displayed on
*/

#if 0
void Win32_FillMonitorInfo()
{
	//TODO: Implement this
	int numMonitors = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&numMonitors);
	foreach
	{
		glfwGetMonitorPos(monitor, &info->offset.x, &info->offset.y);
		glfwGetMonitorPhysicalSize(monitor, &info->physicalSize.width, &info->physicalSize.height);
		glfwGetMonitorContentScale(monitor, &info->contentScale.x, &info->contentScale.y);
		glfwGetMonitorWorkarea(monitor, &info->workArea.x, &info->workArea.y, &info->workArea.width, &info->workArea.height);
		const char* monitorName = glfwGetMonitorName(monitor);
		const GLFWvidmode* videoModes = glfwGetVideoModes(monitor, &numVideoModes);
		const GLFWvidmode* currentVideoMode = glfwGetVideoMode(monitor);
		foreach
		{
			if (Win32_AreGlfwVideoModesEqual(vidMode, currentVideoMode)) { info->currentMode = dIndex; }
		}
	}
}
#endif

void Win32_InitMonitors()
{
	Platform->nextMonitorId = 1;
	CreateLinkedList(&Platform->monitors.list, &Platform->mainHeap, PlatMonitorInfo_t);
}

void Win32_FreeMonitorInfo(PlatMonitorInfo_t* monitorInfo)
{
	NotNull(monitorInfo);
	AssertIf(monitorInfo->name.pntr != nullptr, monitorInfo->allocArena != nullptr);
	FreeString(monitorInfo->allocArena, &monitorInfo->name);
	ClearPointer(monitorInfo);
}

void Win32_FillMonitorInfo()
{
	PlatMonitorInfo_t* monitorToFree = LinkedListFirst(&Platform->monitors.list, PlatMonitorInfo_t);
	while (monitorToFree != nullptr)
	{
		Win32_FreeMonitorInfo(monitorToFree);
		monitorToFree = LinkedListNext(&Platform->monitors.list, PlatMonitorInfo_t, monitorToFree);
	}
	LinkedListClear(&Platform->monitors.list, PlatMonitorInfo_t);
	
	int numMonitors = 0;
	GLFWmonitor** glfwMonitors = glfwGetMonitors(&numMonitors);
	if (numMonitors <= 0)
	{
		Win32_InitError("Failed to get information about the monitors");
	}
	
	GLFWmonitor* glfwPrimaryMonitor = glfwGetPrimaryMonitor();
	if (glfwPrimaryMonitor == nullptr)
	{
		Win32_InitError("Failed to determine the primary monitor through GLFW");
	}
	
	Platform->monitors.desktopRec = Reci_Zero;
	bool foundPrimaryMonitor = false;
	for (int mIndex = 0; mIndex < numMonitors; mIndex++)
	{
		GLFWmonitor* glfwMonitor = glfwMonitors[mIndex];
		NotNull(glfwMonitor);
		PlatMonitorInfo_t* newMonitor = LinkedListAdd(&Platform->monitors.list, PlatMonitorInfo_t);
		NotNull(newMonitor);
		ClearPointer(newMonitor);
		newMonitor->allocArena = &Platform->mainHeap;
		newMonitor->id = Platform->nextMonitorId;
		Platform->nextMonitorId++;
		newMonitor->glfwHandle = glfwMonitor;
		
		//Query info from GLFW
		int positionX = 0, positionY = 0;
		glfwGetMonitorPos(glfwMonitor, &positionX, &positionY);
		int physicalWidth = 0, physicalHeight = 0;
		glfwGetMonitorPhysicalSize(glfwMonitor, &physicalWidth, &physicalHeight);
		float contentScaleX = 0, contentScaleY = 0;
		glfwGetMonitorContentScale(glfwMonitor, &contentScaleX, &contentScaleY);
		int workAreaX = 0, workAreaY = 0, workAreaWidth = 0, workAreaHeight = 0;
		glfwGetMonitorWorkarea(glfwMonitor, &workAreaX, &workAreaY, &workAreaWidth, &workAreaHeight);
		const char* monitorName = glfwGetMonitorName(glfwMonitor);
		if (monitorName == nullptr) { monitorName = "[Unnamed]"; }
		int glfwNumVideoModes = 0;
		const GLFWvidmode* glfwVideoModes = glfwGetVideoModes(glfwMonitor, &glfwNumVideoModes);
		AssertIf(glfwNumVideoModes > 0, glfwVideoModes != nullptr);
		const GLFWvidmode* glfwCurrentVideoMode = glfwGetVideoMode(glfwMonitor);
		if (glfwCurrentVideoMode == nullptr && glfwNumVideoModes > 0) { glfwCurrentVideoMode = &glfwVideoModes[0]; }
		NotNull(glfwCurrentVideoMode);
		
		//TODO: Can we request this information from GLFW or the operating system to match up with the numbers you see when you do "Identify" in the monitor manager thing
		newMonitor->designatedNumber = mIndex+1;
		newMonitor->desktopSpaceRec.topLeft = NewVec2i(positionX, positionY);
		newMonitor->desktopSpaceRec.size = NewVec2i(glfwCurrentVideoMode->width, glfwCurrentVideoMode->height);
		//TODO: Is this workAreaX and workAreaY relative to the position of the monitor in desktop space or is it itself a value in desktop space?
		newMonitor->workAreaRec = NewReci(workAreaX, workAreaY, workAreaWidth, workAreaHeight);
		newMonitor->physicalSize = NewVec2i(physicalWidth, physicalHeight);
		newMonitor->contentScale = NewVec2(contentScaleX, contentScaleY);
		//TODO: Should we assert any sort of restrictions on these values? Like work area being inside desktopSpaceRec or contentScale being positive and non-zero?
		
		newMonitor->name = NewStringInArenaNt(newMonitor->allocArena, monitorName);
		
		if (glfwMonitor == glfwPrimaryMonitor)
		{
			newMonitor->isPrimary = true;
			Platform->monitors.primaryIndex = mIndex;
			foundPrimaryMonitor = true;
		}
		
		if (mIndex == 0) { Platform->monitors.desktopRec = newMonitor->desktopSpaceRec; }
		else { Platform->monitors.desktopRec = ReciBoth(Platform->monitors.desktopRec, newMonitor->desktopSpaceRec); }
		
		bool foundPrimaryVideoMode = false;
		//TODO: Utilize/store the video mode information
	}
	
	if (!foundPrimaryMonitor)
	{
		Win32_InitError("Failed to locate the primary monitor pointer in the list of monitors given by GLFW");
	}
}

