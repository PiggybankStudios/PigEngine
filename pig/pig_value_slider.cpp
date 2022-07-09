/*
File:   pig_value_slider.cpp
Author: Taylor Robbins
Date:   07\08\2022
Description: 
	** A Value Slider is a bar that has a handle that can be grabbed with the mouse and
	** dragged to select a value within a specified range. Like a volume slider for example
*/

//TODO: Add support for focus grabbing
//TODO: Add support for sprite sheet based rendering style
//TODO: Add support for color choosing

void FreeValueSlider(ValueSlider_t* slider)
{
	NotNull(slider);
	//TODO: Anything we need to deallocate?
	ClearPointer(slider);
}

void CreateValueSlider(ValueSlider_t* slider, MemArena_t* memArena, ValueSliderStyle_t style, r32 minValue, r32 maxValue, r32 notchSize, r32 initialValue)
{
	AssertSingleThreaded();
	NotNull2(slider, memArena);
	Assert(minValue < maxValue);
	Assert(initialValue >= minValue && initialValue <= maxValue);
	ClearPointer(slider);
	slider->allocArena = memArena;
	slider->style = style;
	slider->minValue = minValue;
	slider->maxValue = maxValue;
	slider->notchSize = notchSize;
	slider->currentValue = initialValue;
	slider->sliderPos = (initialValue - minValue) / (maxValue - minValue);
	
	slider->id = pig->nextUiId;
	pig->nextUiId++;
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void ValueSliderMove(ValueSlider_t* slider, rec targetRec, r32 sliderWidth)
{
	NotNull(slider);
	slider->targetRec = targetRec;
	slider->sliderWidth = sliderWidth;
}

//TODO: Add support for passing in a SpriteSheet_t to use for rendering

// +--------------------------------------------------------------+
// |                   Layout and Mouse Capture                   |
// +--------------------------------------------------------------+
void ValueSliderLayout(ValueSlider_t* slider)
{
	NotNull(slider);
	
	slider->mainRec = slider->targetRec;
	if (slider->style == ValueSliderStyle_Default)
	{
		slider->mainRec.height = 4; //TODO: Should this thickness be set by the user?
		slider->mainRec.y = slider->targetRec.y + slider->targetRec.height/2 - slider->mainRec.height/2;
	}
	RecAlign(&slider->mainRec);
	slider->usableRec = slider->mainRec;
	if (slider->style == ValueSliderStyle_Default)
	{
		slider->usableRec = RecDeflateX(slider->usableRec, slider->sliderWidth/2);
		if (slider->usableRec.width <= 0) { slider->usableRec = slider->mainRec; }
	}
	RecAlign(&slider->usableRec);
	slider->sliderRec.size = NewVec2(slider->sliderWidth, slider->targetRec.height);
	slider->sliderRec.y = slider->targetRec.y;
	slider->sliderRec.x = slider->usableRec.x + (slider->usableRec.width * slider->sliderPos) - slider->sliderRec.width/2;
	RecAlign(&slider->sliderRec);
	slider->fillRec = slider->mainRec;
	slider->fillRec.width = slider->mainRec.width * slider->sliderPos;
	RecAlign(&slider->fillRec);
}
void ValueSliderCaptureMouse(ValueSlider_t* slider)
{
	NotNull(slider);
	ValueSliderLayout(slider);
	if (slider->style == ValueSliderStyle_Default)
	{
		MouseHitRecPrint(slider->sliderRec, "Slider%lluHandle", slider->id);
		MouseHitRecPrint(RecInflateY(slider->mainRec, 8), "Slider%llu", slider->id);
	}
	else
	{
		MouseHitRecPrint(slider->mainRec, "Slider%llu", slider->id);
	}
}

void UpdateValueSlider(ValueSlider_t* slider)
{
	NotNull(slider);
	ValueSliderLayout(slider);
	
	// +==============================+
	// |   Handle Mouse Left Click    |
	// +==============================+
	if (IsMouseOverPrint("Slider%lluHandle", slider->id))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			slider->isDraggingSlider = true;
			slider->dragSliderOffset = (MousePos - (slider->sliderRec.topLeft + slider->sliderRec.size/2));
		}
	}
	if (IsMouseOverPrint("Slider%llu", slider->id))
	{
		pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			slider->isDraggingSlider = true;
			slider->dragSliderOffset = Vec2_Zero;
		}
	}
	
	// +==============================+
	// | Handle Mouse Dragging Slider |
	// +==============================+
	if (slider->isDraggingSlider)
	{
		HandleMouse(MouseBtn_Left);
		if (MouseDownRaw(MouseBtn_Left))
		{
			r32 newSliderOffset = (MousePos.x - slider->usableRec.x) - slider->dragSliderOffset.x;
			r32 newValue = slider->minValue + ((newSliderOffset / slider->usableRec.width) * (slider->maxValue - slider->minValue));
			if (newValue < slider->minValue) { newValue = slider->minValue; }
			if (newValue > slider->maxValue) { newValue = slider->maxValue; }
			if (slider->notchSize != 0 && newValue != slider->maxValue)
			{
				newValue = (RoundR32((newValue - slider->minValue) / slider->notchSize) * slider->notchSize) + slider->minValue;
			}
			if (newValue != slider->currentValue)
			{
				slider->currentValue = newValue;
				slider->valueChanged = true;
			}
		}
		else { slider->isDraggingSlider = false; }
	}
	
	// +==============================+
	// |      Animate the Slider      |
	// +==============================+
	r32 targetPos = ((slider->currentValue - slider->minValue) / (slider->maxValue - slider->minValue));
	if (AbsR32(targetPos - slider->sliderPos) > 0.01f)
	{
		slider->sliderPos += (targetPos - slider->sliderPos) / 3; //TODO: Tune this damping variable, or expose it to be set by application
	}
	else { slider->sliderPos = targetPos; }
}

void RenderValueSlider(ValueSlider_t* slider)
{
	NotNull(slider);
	ValueSliderLayout(slider);
	
	r32 cornerRadius = 5;
	r32 notchSize = 3;
	
	if (slider->style == ValueSliderStyle_Default)
	{
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + (slider->usableRec.width-1)*0.00f), slider->usableRec.y - notchSize*2, 1, notchSize*2), MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + (slider->usableRec.width-1)*0.25f), slider->usableRec.y - notchSize,   1, notchSize),   MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + (slider->usableRec.width-1)*0.50f), slider->usableRec.y - notchSize*2, 1, notchSize*2), MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + (slider->usableRec.width-1)*0.75f), slider->usableRec.y - notchSize,   1, notchSize),   MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + (slider->usableRec.width-1)*1.00f), slider->usableRec.y - notchSize*2, 1, notchSize*2), MonokaiWhite);
		RcDrawRectangle(slider->usableRec, MonokaiWhite);
		RcDrawRoundedRectangle(slider->sliderRec, cornerRadius, MonokaiBlue);
		RcDrawRectangle(RecOverlap(slider->usableRec, slider->sliderRec), ColorDarken(MonokaiBlue, 30));
	}
	else if (slider->style == ValueSliderStyle_Bar)
	{
		r32 outlineSize = 1;
		RcDrawRectangle(NewRec(slider->usableRec.x - outlineSize,                             slider->usableRec.y - (outlineSize + notchSize*2), 1, (outlineSize + notchSize*2 + cornerRadius)), MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + slider->usableRec.width*0.25f), slider->usableRec.y - (outlineSize + notchSize),   1, (outlineSize + notchSize)),   MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + slider->usableRec.width*0.50f), slider->usableRec.y - (outlineSize + notchSize*2), 1, (outlineSize + notchSize*2)), MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + slider->usableRec.width*0.75f), slider->usableRec.y - (outlineSize + notchSize),   1, (outlineSize + notchSize)),   MonokaiWhite);
		RcDrawRectangle(NewRec(RoundR32(slider->usableRec.x + slider->usableRec.width*1.00f), slider->usableRec.y - (outlineSize + notchSize*2), 1, (outlineSize + notchSize*2 + cornerRadius)), MonokaiWhite);
		RcDrawRoundedRectangle(RecInflate(slider->mainRec, outlineSize, outlineSize), cornerRadius, MonokaiWhite);
		RcDrawRoundedRectangle(slider->mainRec, cornerRadius, MonokaiBack);
		RcDrawRoundedRectangle(slider->fillRec, cornerRadius, MonokaiBlue);
	}
	else { Unimplemented(); }
}

