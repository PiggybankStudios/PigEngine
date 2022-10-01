/*
File:   pig_button.cpp
Author: Taylor Robbins
Date:   09\21\2022
Description: 
	** Holds functions that help us handle the standard "button" that might be clicked on
*/

// +--------------------------------------------------------------+
// |                       Create and Free                        |
// +--------------------------------------------------------------+
void FreeButton(Button_t* btn)
{
	NotNull(btn);
	if (btn->text.pntr != nullptr)
	{
		NotNull(btn->allocArena);
		FreeString(btn->allocArena, &btn->text);
	}
	ClearPointer(btn);
}

void CreateButton(Button_t* btn, MemArena_t* memArena, ButtonStyle_t style, MyStr_t text)
{
	AssertSingleThreaded();
	NotNull(btn);
	NotNull(memArena);
	NotNullStr(&text);
	
	ClearPointer(btn);
	btn->id = pig->nextUiId;
	pig->nextUiId++;
	btn->allocArena = memArena;
	btn->style = style;
	btn->text = AllocString(memArena, &text);
	NotNullStr(&btn->text);
	btn->font = GetFontHandle(&pig->resources.fonts->debug);
	btn->fontFaceSelector = SelectDefaultFontFace();
	btn->fontScale = 1.0f;
	btn->cornerRadius = 0;
	btn->borderThickness = 1;
	btn->backColor = ColorTransparent(Black, 0.5f);
	btn->textColor = MonokaiWhite;
	btn->borderColor = MonokaiWhite;
	btn->innerMargin = NewVec2(5, 5);
	btn->isVisible = true;
	btn->isEnabled = true;
	btn->opacity = 1.0f;
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void ButtonMove(Button_t* btn, rec targetRec)
{
	NotNull(btn);
	btn->hasTargetRec = true;
	if (!RecBasicallyEqual(btn->targetRec, targetRec))
	{
		btn->targetRec = targetRec;
		btn->targetRecMoved = true;
	}
}

void SetButtonFont(Button_t* btn, Font_t* font, FontFaceSelector_t faceSelector)
{
	NotNull(btn);
	NotNull(btn->allocArena);
	NotNull(font);
	btn->font = GetFontHandle(font);
	btn->fontFaceSelector = faceSelector;
}

void ButtonSetMouseCapturePrefix(Button_t* btn, MyStr_t prefixStr)
{
	NotNull(btn);
	NotNull(btn->allocArena);
	if (!IsEmptyStr(btn->mouseCapturePrefix))
	{
		FreeString(btn->allocArena, &btn->mouseCapturePrefix);
		btn->mouseCapturePrefix = MyStr_Empty;
	}
	if (!IsEmptyStr(prefixStr))
	{
		btn->mouseCapturePrefix = AllocString(btn->allocArena, &prefixStr);
		NotNullStr(&btn->mouseCapturePrefix);
	}
}

bool IsMouseOverButton(Button_t* btn)
{
	NotNull(btn);
	if (!IsEmptyStr(btn->mouseCapturePrefix))
	{
		return IsMouseOverPrint("%.*s%llu", btn->mouseCapturePrefix.length, btn->mouseCapturePrefix.pntr, btn->id);
	}
	else
	{
		return IsMouseOverPrint("Button%llu", btn->id);
	}
}

r32 ButtonGetAutoHeight(Button_t* btn)
{
	NotNull(btn);
	const FontFace_t* fontFace = GetFontFace(GetPointer(&btn->font), btn->fontFaceSelector);
	r32 fontLineHeight = (fontFace != nullptr) ? (fontFace->lineHeight * btn->fontScale) : 0;
	return (fontLineHeight + btn->innerMargin.y*2);
}

// +--------------------------------------------------------------+
// |                   Layout and Mouse Capture                   |
// +--------------------------------------------------------------+
void ButtonLayout(Button_t* btn)
{
	NotNull(btn);
	AssertFilledHandle(&btn->font);
	
	if (btn->hasTargetRec)
	{
		btn->mainRec.topLeft = btn->targetRec.topLeft;
		if (!btn->autoSizeWidth) { btn->mainRec.width = btn->targetRec.width; }
		if (!btn->autoSizeHeight) { btn->mainRec.height = btn->targetRec.height; }
	}
	
	r32 maxTextWidth = 0;
	if (!btn->autoSizeWidth)
	{
		maxTextWidth = btn->mainRec.width - 2*btn->innerMargin.x;
	}
	
	btn->textMeasure = MeasureTextInFont(btn->text, GetPointer(&btn->font), btn->fontFaceSelector, btn->fontScale, maxTextWidth);
	
	if (btn->autoSizeWidth)
	{
		btn->mainRec.width = btn->textMeasure.size.width + 2*btn->innerMargin.x;
	}
	if (btn->autoSizeHeight)
	{
		btn->mainRec.height = btn->textMeasure.size.height + 2*btn->innerMargin.y;
	}
	
	RecAlign(&btn->mainRec);
	btn->usableRec = RecDeflate(btn->mainRec, btn->innerMargin) - btn->mainRec.topLeft;
	
	btn->textRec.size = btn->textMeasure.size;
	btn->textRec.topLeft = btn->mainRec.size/2 - btn->textRec.size/2;
	RecAlign(&btn->textRec);
}

void ButtonCaptureMouse(Button_t* btn, v2 renderOffset)
{
	NotNull(btn);
	ButtonLayout(btn);
	if (btn->isVisible)
	{
		rec mainRec = btn->mainRec + renderOffset;
		if (!IsEmptyStr(btn->mouseCapturePrefix))
		{
			MouseHitRecPrint(mainRec, "%.*s%llu", btn->mouseCapturePrefix.length, btn->mouseCapturePrefix.pntr, btn->id);
		}
		else
		{
			MouseHitRecPrint(mainRec, "Button%llu", btn->id);
		}
	}
}
void ButtonCaptureMouse(Button_t* btn)
{
	ButtonCaptureMouse(btn, Vec2_Zero);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateButton(Button_t* btn, v2 renderOffset)
{
	NotNull(btn);
	ButtonLayout(btn);
	UNUSED(renderOffset);
	
	// +==============================+
	// |     Handle Mouse Clicked     |
	// +==============================+
	if (IsMouseOverButton(btn))
	{
		if (btn->isEnabled)
		{
			pigOut->cursorType = PlatCursor_Pointer;
			//TODO: Make this logic more complicated to make it only trigger once the mouse is release and make sure the mouse started inside the button
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				btn->clicked = true;
			}
		}
	}
}
void UpdateButton(Button_t* btn)
{
	UpdateButton(btn, Vec2_Zero);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderButton(Button_t* btn, v2 renderOffset)
{
	NotNull(btn);
	ButtonLayout(btn);
	if (!btn->isVisible) { return; }
	
	RcBindFont(GetPointer(&btn->font), btn->fontFaceSelector, btn->fontScale);
	
	bool isHovered = IsMouseOverButton(btn);
	bool isDown = (isHovered && MouseDownRaw(MouseBtn_Left));
	rec mainRec = btn->mainRec + renderOffset;
	rec usableRec = btn->usableRec + mainRec.topLeft;
	rec textRec = btn->textRec + mainRec.topLeft;
	v2 textPos = textRec.topLeft + NewVec2(textRec.width/2, btn->textMeasure.offset.y);
	Vec2Align(&textPos);
	r32 maxTextWidth = 0;
	if (!btn->autoSizeWidth)
	{
		maxTextWidth = mainRec.width - 2*btn->innerMargin.x;
	}
	
	switch (btn->style)
	{
		// +==============================+
		// |     ButtonStyle_Default      |
		// +==============================+
		case ButtonStyle_Default:
		{
			Color_t backColor = btn->backColor;
			Color_t textColor = btn->textColor;
			if (isDown) { backColor = ColorDarken(backColor, 50); }
			else if (isHovered) { backColor = ColorLighten(backColor, 20); }
			
			if (btn->cornerRadius > 0)
			{
				RcDrawRoundedRectangle(mainRec, btn->cornerRadius, backColor);
			}
			else
			{
				RcDrawRectangle(mainRec, backColor);
			}
			
			rec oldViewport = rc->state.viewportRec;
			RcSetViewport(RecOverlap(usableRec, oldViewport));
			RcDrawText(btn->text, textPos, textColor, TextAlignment_Center, maxTextWidth);
			RcSetViewport(oldViewport);
		} break;
		
		// +==============================+
		// |     ButtonStyle_Bordered     |
		// +==============================+
		case ButtonStyle_Bordered:
		{
			Color_t backColor = btn->backColor;
			Color_t textColor = btn->textColor;
			Color_t borderColor = btn->borderColor;
			if (isDown) { backColor = ColorDarken(backColor, 50); }
			else if (isHovered) { backColor = ColorLighten(backColor, 20); }
			
			RcDrawRectangle(mainRec, backColor);
			RcDrawRectangleOutline(mainRec, borderColor, btn->borderThickness);
			
			rec oldViewport = rc->state.viewportRec;
			RcSetViewport(RecOverlap(usableRec, oldViewport));
			RcDrawText(btn->text, textPos, textColor, TextAlignment_Center, maxTextWidth);
			RcSetViewport(oldViewport);
		} break;
		
		default: DebugAssertMsg(false, "Unimplemented ButtonStyle!"); break;
	}
}
void RenderButton(Button_t* btn)
{
	RenderButton(btn, Vec2_Zero);
}