/*
File:   pig_checkbox.cpp
Author: Taylor Robbins
Date:   07\09\2022
Description: 
	** A checkbox is a simple UI widget that can be clicked to change the value of a boolean on\off
*/

// +--------------------------------------------------------------+
// |                       Create and Free                        |
// +--------------------------------------------------------------+
void FreeCheckbox(Checkbox_t* cb)
{
	NotNull(cb);
	//TODO: Anything to deallocate?
	ClearPointer(cb);
}

void CreateCheckbox(Checkbox_t* cb, MemArena_t* memArena, bool initialState)
{
	AssertSingleThreaded();
	NotNull2(cb, memArena);
	ClearPointer(cb);
	cb->id = pig->nextUiId;
	pig->nextUiId++;
	cb->allocArena = memArena;
	cb->enabled = true;
	cb->currentValue = initialState;
	cb->animProgress = (cb->currentValue ? 1.0f : 0.0f);
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
void CheckboxMove(Checkbox_t* cb, rec targetRec)
{
	NotNull(cb);
	cb->targetRec = targetRec;
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void CheckboxLayout(Checkbox_t* cb)
{
	NotNull(cb);
	cb->mainRec = cb->targetRec;
}

void CheckboxCaptureMouse(Checkbox_t* cb)
{
	NotNull(cb);
	CheckboxLayout(cb);
	
	MouseHitRecPrint(cb->mainRec, "Checkbox%llu", cb->id);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateCheckbox(Checkbox_t* cb)
{
	NotNull(cb);
	CheckboxLayout(cb);
	
	if (cb->enabled && IsMouseOverPrint("Checkbox%llu", cb->id))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			cb->valueChanged = true;
			cb->currentValue = !cb->currentValue;
		}
	}
	
	if (cb->currentValue) { UpdateAnimationUp(&cb->animProgress, CHECKBOX_CHECK_ANIM_SPEED); }
	else { UpdateAnimationDown(&cb->animProgress, CHECKBOX_CHECK_ANIM_SPEED); }
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderCheckbox(Checkbox_t* cb)
{
	NotNull(cb);
	CheckboxLayout(cb);
	
	r32 cornerRadius = cb->mainRec.height / 4;
	rec fillRec = cb->mainRec;
	// fillRec.height *= EaseQuadraticOut(cb->animProgress);
	rec checkRec = RecInflate(cb->mainRec, CHECKBOX_OUTLINE_THICKNESS*2, CHECKBOX_OUTLINE_THICKNESS*2);
	checkRec.x += CHECKBOX_OUTLINE_THICKNESS;
	checkRec.y -= CHECKBOX_OUTLINE_THICKNESS;
	Color_t fillColor = ColorLerp(MonokaiLightGray, MonokaiWhite, cb->animProgress);
	RcDrawRoundedRectangle(cb->mainRec, cornerRadius, fillColor);
	RcDrawRoundedRectangle(RecDeflate(cb->mainRec, CHECKBOX_OUTLINE_THICKNESS, CHECKBOX_OUTLINE_THICKNESS), cornerRadius, MonokaiBack);
	// RcDrawRoundedRectangle(fillRec, cornerRadius, fillColor);
	RcBindSpriteSheet(&pig->resources.sheets->vectorIcons64);
	RcDrawSheetFrame(NewStr("Check"), checkRec, ColorTransparent(fillColor, cb->animProgress));
}
