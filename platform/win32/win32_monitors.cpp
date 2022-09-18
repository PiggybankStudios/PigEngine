/*
File:   win32_monitors.cpp
Author: Taylor Robbins
Date:   02\21\2022
Description: 
	** Holds functions that help us manage our information about the monitors on the users computer
	** This information is important for the engine to make decisions about what screen and
	** with what rendering options it wants to be displayed on
*/

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
void Win32_FreeMonitorInfo(PlatMonitorInfo_t* monitorInfo)
{
	NotNull(monitorInfo);
	AssertIf(monitorInfo->name.pntr != nullptr, monitorInfo->allocArena != nullptr);
	FreeString(monitorInfo->allocArena, &monitorInfo->name);
	FreeVarArray(&monitorInfo->videoModes);
	ClearPointer(monitorInfo);
}

bool Win32_AreGlfwVideoModesEqual(const GLFWvidmode* mode1, const GLFWvidmode* mode2)
{
	NotNull2(mode1, mode2);
	if (mode1->width != mode2->width)             { return false; }
	if (mode1->height != mode2->height)           { return false; }
	if (mode1->redBits != mode2->redBits)         { return false; }
	if (mode1->greenBits != mode2->greenBits)     { return false; }
	if (mode1->blueBits != mode2->blueBits)       { return false; }
	if (mode1->refreshRate != mode2->refreshRate) { return false; }
	return true;
}

// +--------------------------------------------------------------+
// |                        API Functions                         |
// +--------------------------------------------------------------+
// +==============================+
// |  Win32_GetMonitorVideoMode   |
// +==============================+
// const PlatMonitorVideoMode_t* GetMonitorVideoMode(const PlatMonitorInfo_t* monitor, v2i resolution, u64* indexOut)
PLAT_API_GET_MONITOR_VIDEO_MODE_DEFINITION(Win32_GetMonitorVideoMode)
{
	NotNull(monitor);
	VarArrayLoop(&monitor->videoModes, vIndex)
	{
		VarArrayLoopGet(PlatMonitorVideoMode_t, videoMode, &monitor->videoModes, vIndex);
		if (videoMode->resolution == resolution)
		{
			if (indexOut != nullptr) { *indexOut = vIndex; }
			return videoMode;
		}
	}
	return nullptr;
}

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void Win32_InitMonitors()
{
	Platform->nextMonitorId = 1;
	Platform->nextMonitorVideoModeId = 1;
	CreateLinkedList(&Platform->monitors.list, &Platform->mainHeap, PlatMonitorInfo_t);
}

// +--------------------------------------------------------------+
// |                           Filling                            |
// +--------------------------------------------------------------+
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
		Assert(glfwNumVideoModes >= 0);
		AssertIf(glfwNumVideoModes > 0, glfwVideoModes != nullptr);
		const GLFWvidmode* glfwCurrentVideoMode = glfwGetVideoMode(glfwMonitor);
		if (glfwCurrentVideoMode == nullptr && glfwNumVideoModes > 0) { glfwCurrentVideoMode = &glfwVideoModes[0]; }
		NotNull(glfwCurrentVideoMode);
		
		//TODO: Can we request this information from GLFW or the operating system to match up with the numbers you see when you do "Identify" in the monitor manager thing
		newMonitor->designatedNumber = mIndex+1;
		newMonitor->desktopSpaceRec.topLeft = NewVec2i(positionX, positionY);
		newMonitor->desktopSpaceRec.size = NewVec2i(glfwCurrentVideoMode->width, glfwCurrentVideoMode->height);
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
		CreateVarArray(&newMonitor->videoModes, newMonitor->allocArena, sizeof(PlatMonitorVideoMode_t), (u64)glfwNumVideoModes);
		CreateVarArray(&newMonitor->framerates, newMonitor->allocArena, sizeof(i64));
		for (u64 vIndex = 0; vIndex < (u64)glfwNumVideoModes; vIndex++)
		{
			const GLFWvidmode* glfwMode = &glfwVideoModes[vIndex];
			v2i resolution = NewVec2i(glfwMode->width, glfwMode->height);
			u64 existingModeIndex = 0;
			PlatMonitorVideoMode_t* existingMode = (PlatMonitorVideoMode_t*)Win32_GetMonitorVideoMode(newMonitor, resolution, &existingModeIndex);
			if (existingMode == nullptr)
			{
				PlatMonitorVideoMode_t* newMode = VarArrayAdd(&newMonitor->videoModes, PlatMonitorVideoMode_t);
				NotNull(newMode);
				ClearPointer(newMode);
				newMode->id = Platform->nextMonitorVideoModeId;
				Platform->nextMonitorVideoModeId++;
				newMode->index = newMonitor->videoModes.length-1;
				newMode->resolution = resolution;
				newMode->isCurrent = false;
				newMode->numFramerates = 0;
				//TODO: Should we incorporate the redBits, blueBits, greenBits members? Probably not because we don't ask glfw to set the bit depth when we activate fullscreen and choose a resolution+framerate
				//TODO: Will we ever find multiple video modes that share the same attributes besides bit depth changing?
				existingMode = newMode;
			}
			if (existingMode->numFramerates < MAX_MONITOR_FRAMERATES_PER_RESOLUTION)
			{
				existingMode->framerates[existingMode->numFramerates] = glfwMode->refreshRate;
				existingMode->numFramerates++;
			}
			if (!foundPrimaryVideoMode && Win32_AreGlfwVideoModesEqual(glfwMode, glfwCurrentVideoMode))
			{
				newMonitor->currentVideoModeIndex = existingModeIndex;
				existingMode->currentFramerateIndex = existingMode->numFramerates-1;
				existingMode->isCurrent = true;
				foundPrimaryVideoMode = true;
			}
			
			bool isNewFramerate = true;
			for (u64 fIndex = 0; fIndex < newMonitor->framerates.length; fIndex++)
			{
				i64* frameratePntr = VarArrayGetHard(&newMonitor->framerates, fIndex, i64);
				if (*frameratePntr == glfwMode->refreshRate) { isNewFramerate = false; break; }
			}
			if (isNewFramerate)
			{
				i64* newFramerateSpace = VarArrayAdd(&newMonitor->framerates, i64);
				NotNull(newFramerateSpace);
				*newFramerateSpace = glfwMode->refreshRate;
			}
		}
		VarArraySort(&newMonitor->framerates, CompareFuncI64, nullptr);
	}
	
	if (!foundPrimaryMonitor)
	{
		Win32_InitError("Failed to locate the primary monitor pointer in the list of monitors given by GLFW");
	}
}

