/*
File:   web_callbacks.cpp
Author: Taylor Robbins
Date:   03\20\2022
Description: 
	** Holds a bunch of functions that the javascript layer will
	** call whenever certain events happen in the browser
*/

// +==============================+
// |          MouseMoved          |
// +==============================+
EXPORTED_FUNC(void, MouseMoved, r64 mouseX, r64 mouseY)
{
	if (Platform->mousePos.x != mouseX || Platform->mousePos.y != mouseY)
	{
		Platform->mouseMoved = true;
		Platform->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	}
}

// +==============================+
// |       MouseBtnChanged        |
// +==============================+
EXPORTED_FUNC(void, MouseBtnChanged, i32 button, bool isDown, r64 mouseX, r64 mouseY)
{
	if (button == 0) //left
	{
		Platform->mouseLeftBtnDown = isDown;
		if (isDown) { Platform->mouseLeftBtnPressed = true; }
		else { Platform->mouseLeftBtnReleased = true; }
	}
	else if (button == 1) //middle
	{
		Platform->mouseMiddleBtnDown = isDown;
		if (isDown) { Platform->mouseMiddleBtnPressed = true; }
		else { Platform->mouseMiddleBtnReleased = true; }
	}
	else if (button == 2) //right
	{
		Platform->mouseRightBtnDown = isDown;
		if (isDown) { Platform->mouseRightBtnPressed = true; }
		else { Platform->mouseRightBtnReleased = true; }
	}
	else
	{
		PrintLine_E("WARNING: Unknown mouse button index %d in MouseBtnChanged!", button);
	}
	if (Platform->mousePos.x != mouseX || Platform->mousePos.y != mouseY)
	{
		Platform->mouseMoved = true;
		Platform->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	}
}

