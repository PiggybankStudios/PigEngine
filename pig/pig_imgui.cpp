/*
File:   pig_imgui.cpp
Author: Taylor Robbins
Date:   03\01\2024
Description: 
	** Holds functions that help us interact with Dear Imgui
*/

#undef R //TODO: Idk where this define is coming from, but it caused problems with code inside imgui source
#pragma warning(push)
#pragma warning(disable:4459) //there is a local "gl" variable that our global conflicts with
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#pragma warning(pop)

void PigInitImgui()
{
	NotNull(pig);
    ClearStruct(pig->imgui);
	pig->imgui.frameStarted = false;
	
	PrintLine_I("Initializing ImGui %s", IMGUI_VERSION);
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    pig->imgui.initialized = true;
}

void PigUpdateImguiBefore()
{
	// ImGui::NewFrame();
	// pig->imgui.frameStarted = true;
}

void PigUpdateImguiAfter()
{
	
}

void PigRenderImgui()
{
	
}
