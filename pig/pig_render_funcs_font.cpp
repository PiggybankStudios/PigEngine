/*
File:   pig_render_funcs_font.cpp
Author: Taylor Robbins
Date:   12\30\2021
Description: 
	** Holds a bunch of useful functions that help us easily render text on screen using
	** the Render Context state to assume certain aspects (like which font to use)
*/

// +--------------------------------------------------------------+
// |                      Callback Functions                      |
// +--------------------------------------------------------------+
struct RcRenderTextWithColoredRegionCallbackContext_t
{
	Color_t color;
	Color_t regionColor;
	u64 startIndex;
	u64 numBytes;
};
// bool RcRenderTextWithColoredRegionBeforeCharCallback(u32 codepoint, const FontCharInfo_t* charInfo, rec logicalRec, rec renderRec, FontFlowState_t* state, void* context)
FFCB_BEFORE_CHAR_DEFINITION(RcRenderTextWithColoredRegionBeforeCharCallback) // | RcRenderTextWithColoredRegionBeforeCharCallback |
{
	UNUSED(codepoint);
	UNUSED(charInfo);
	UNUSED(logicalRec);
	UNUSED(renderRec);
	NotNull(context);
	RcRenderTextWithColoredRegionCallbackContext_t* contextPntr = (RcRenderTextWithColoredRegionCallbackContext_t*)context;
	if (state->byteIndex >= contextPntr->startIndex && state->byteIndex < contextPntr->startIndex + contextPntr->numBytes)
	{
		state->color = contextPntr->regionColor;
	}
	else
	{
		state->color = contextPntr->color;
	}
	return true;
}

struct RcRenderTextWithSelectionCallbackContext_t
{
	Color_t selectionColor;
	u64 startIndex;
	u64 numBytes;
	bool insideSelection;
	v2 selectionStartPos;
};
void RcRenderTextWithSelectionDrawSelectionRec(v2 startPos, v2 currentPos, Color_t selectionColor)
{
	rec selectionRec = NewRec(startPos.x, startPos.y - RcGetMaxAscend(), currentPos.x - startPos.x, RcGetMaxAscend() + RcGetMaxDescend());
	selectionRec = RecInflateX(selectionRec, 1);
	RcDrawRectangle(selectionRec, selectionColor);
}
// void RcRenderTextWithSelectionBetweenCharCallback(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
FFCB_BETWEEN_CHAR_DEFINITION(RcRenderTextWithSelectionBetweenCharCallback) // | RcRenderTextWithSelectionBetweenCharCallback |
{
	UNUSED(byteIndex);
	UNUSED(charIndex);
	UNUSED(position);
	NotNull(context);
	RcRenderTextWithSelectionCallbackContext_t* contextPntr = (RcRenderTextWithSelectionCallbackContext_t*)context;
	if (state->byteIndex >= contextPntr->startIndex && state->byteIndex < contextPntr->startIndex + contextPntr->numBytes)
	{
		if (!contextPntr->insideSelection)
		{
			contextPntr->insideSelection = true;
			contextPntr->selectionStartPos = state->position;
		}
	}
	else
	{
		if (contextPntr->insideSelection)
		{
			contextPntr->insideSelection = false;
			RcRenderTextWithSelectionDrawSelectionRec(contextPntr->selectionStartPos, state->position, contextPntr->selectionColor);
		}
	}
}
// void RcRenderTextWithSelectionBeforeLineCallback(u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
FFCB_BEFORE_LINE_DEFINITION(RcRenderTextWithSelectionBeforeLineCallback) // | RcRenderTextWithSelectionBeforeLineCallback |
{
	UNUSED(lineIndex);
	UNUSED(byteIndex);
	NotNull(context);
	RcRenderTextWithSelectionCallbackContext_t* contextPntr = (RcRenderTextWithSelectionCallbackContext_t*)context;
	if (contextPntr->insideSelection)
	{
		contextPntr->selectionStartPos = state->position;
	}
}
// void RcRenderTextWithSelectionAfterLineCallback(bool isLineWrap, u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
FFCB_AFTER_LINE_DEFINITION(RcRenderTextWithSelectionAfterLineCallback) // | RcRenderTextWithSelectionAfterLineCallback |
{
	UNUSED(isLineWrap);
	UNUSED(lineIndex);
	NotNull(context);
	RcRenderTextWithSelectionCallbackContext_t* contextPntr = (RcRenderTextWithSelectionCallbackContext_t*)context;
	if (contextPntr->insideSelection)
	{
		RcRenderTextWithSelectionDrawSelectionRec(contextPntr->selectionStartPos, state->position, contextPntr->selectionColor);
		if (byteIndex >= contextPntr->startIndex + contextPntr->numBytes)
		{
			contextPntr->insideSelection = false;
		}
	}
}

// +--------------------------------------------------------------+
// |                       Render Functions                       |
// +--------------------------------------------------------------+
TextMeasure_t RcMeasureText(MyStr_t text, r32 maxWidth = 0, FontFlowInfo_t* infoOut = nullptr)
{
	return MeasureTextInFont(text, rc->state.boundFont, rc->state.faceSelector, rc->state.fontScale, maxWidth, infoOut, &rc->state.flowCallbacks);
}
TextMeasure_t RcMeasureText(const char* nulltermStr, r32 maxWidth = 0, FontFlowInfo_t* infoOut = nullptr)
{
	return RcMeasureText(NewStr(nulltermStr), maxWidth, infoOut);
}

void RcDrawText(const char* str, v2 position, Color_t color, TextAlignment_t alignment = TextAlignment_Left, r32 maxWidth = 0)
{
	NotNull(str);
	NotNull(rc->state.boundFont);
	FontFlow_RenderText(
		NewStr(str),
		rc->state.boundFont, rc->state.faceSelector, color, position,
		alignment, rc->state.fontScale, maxWidth,
		&rc->state.flowCallbacks, &rc->flowInfo
	);
}
void RcDrawText(MyStr_t str, v2 position, Color_t color, TextAlignment_t alignment = TextAlignment_Left, r32 maxWidth = 0)
{
	NotNullStr(&str);
	NotNull(rc->state.boundFont);
	FontFlow_RenderText(
		str,
		rc->state.boundFont, rc->state.faceSelector, color, position,
		alignment, rc->state.fontScale, maxWidth,
		&rc->state.flowCallbacks, &rc->flowInfo
	);
}

void RcDrawTextPrintEx(v2 position, Color_t color, TextAlignment_t alignment, r32 maxWidth, const char* formatString, ...)
{
	NotNull(formatString);
	TempPrintVa(textPntr, textLength, formatString);
	if (textPntr != nullptr)
	{
		RcDrawText(NewStr(textLength, textPntr), position, color, alignment, maxWidth);
	}
	else
	{
		RcDrawText(NewStr(formatString), position, color, alignment, maxWidth);
	}
}
void RcDrawTextPrint(v2 position, Color_t color, const char* formatString, ...)
{
	NotNull(formatString);
	TempPrintVa(textPntr, textLength, formatString);
	if (textPntr != nullptr)
	{
		RcDrawText(NewStr(textLength, textPntr), position, color);
	}
	else
	{
		RcDrawText(NewStr(formatString), position, color);
	}
}

void RcDrawTextPrintWithBackground(v2 position, Color_t textColor, Color_t backgroundColor, v2 padding, const char* formatString, ...)
{
	TempPrintVa(textPntr, textLength, formatString);
	MyStr_t textStr = MyStr_Empty;
	if (textPntr != nullptr) { textStr = NewStr(textLength, textPntr); }
	else { textStr = NewStr(formatString); }
	TextMeasure_t textMeasure = RcMeasureText(textPntr);
	rec backgroundRec = NewRec(position.x - textMeasure.offset.x, position.y - textMeasure.offset.y, textMeasure.size.width, textMeasure.size.height);
	backgroundRec = RecInflate(backgroundRec, padding);
	RcDrawRectangle(backgroundRec, backgroundColor);
	RcDrawText(textStr, position, textColor);
}

void RcDrawTextWithColoredRegion(MyStr_t str, v2 position,
	Color_t color, Color_t regionColor, u64 regionStart, u64 regionNumBytes,
	TextAlignment_t alignment = TextAlignment_Left, r32 maxWidth = 0)
{
	RcRenderTextWithColoredRegionCallbackContext_t context = {};
	context.startIndex = regionStart;
	context.numBytes = regionNumBytes;
	context.color = color;
	context.regionColor = regionColor;
	FontFlowCallbacks_t flowCallbacks = {};
	flowCallbacks.beforeChar = RcRenderTextWithColoredRegionBeforeCharCallback;
	flowCallbacks.context = &context;
	FontFlow_RenderText(
		str,
		rc->state.boundFont, rc->state.faceSelector, color, position,
		alignment, rc->state.fontScale, maxWidth,
		&flowCallbacks, &rc->flowInfo
	);
}

void RcDrawTextWithSelection(MyStr_t str, v2 position,
	Color_t textColor, Color_t selectionTextColor, Color_t selectionColor, u64 selectionStart, u64 selectionNumBytes,
	TextAlignment_t alignment = TextAlignment_Left, r32 maxWidth = 0)
{
	RcRenderTextWithSelectionCallbackContext_t context = {};
	context.startIndex = selectionStart;
	context.numBytes = selectionNumBytes;
	context.selectionColor = selectionColor;
	context.insideSelection = false;
	
	FontFlowCallbacks_t flowCallbacks = {};
	flowCallbacks.betweenChar = RcRenderTextWithSelectionBetweenCharCallback;
	flowCallbacks.beforeLine = RcRenderTextWithSelectionBeforeLineCallback;
	flowCallbacks.afterLine = RcRenderTextWithSelectionAfterLineCallback;
	flowCallbacks.context = &context;
	
	FontFlowState_t flowState;
	FontFlow_Initialize(&flowState, str, rc->state.boundFont, rc->state.faceSelector, textColor, position, alignment, rc->state.fontScale, maxWidth);
	flowState.justMeasuring = true;
	FontFlow_Main(&flowState, &flowCallbacks, &rc->flowInfo);
	if (context.insideSelection)
	{
		RcRenderTextWithSelectionDrawSelectionRec(context.selectionStartPos, flowState.position, selectionColor);
	}
	
	RcDrawTextWithColoredRegion(str, position, textColor, selectionTextColor, selectionStart, selectionNumBytes, alignment, maxWidth);
}

void RcDrawPieChartForPerfSectionBundle(const PerfSectionBundle_t* bundle, rec rectangle, Color_t tintColor, bool showPieceTextOnHover = false)
{
	if (bundle->sections.length < 2) { return; }
	TempPushMark();
	r64* percentages = AllocArray(TempArena, r64, bundle->sections.length-1);
	r64 totalTime = 0.0;
	NotNull(percentages);
	for (u64 sIndex = 0; sIndex+1 < bundle->sections.length; sIndex++)
	{
		PerfSection_t* section = VarArrayGet(&bundle->sections, sIndex, PerfSection_t);
		PerfSection_t* nextSection = VarArrayGet(&bundle->sections, sIndex+1, PerfSection_t);
		NotNull2(section, nextSection);
		percentages[sIndex] = plat->GetPerfTimeDiff(&section->time, &nextSection->time);
		totalTime += percentages[sIndex];
	}
	for (u64 sIndex = 0; sIndex+1 < bundle->sections.length; sIndex++)
	{
		percentages[sIndex] = percentages[sIndex] / totalTime;
	}
	RcDrawPieChart(bundle->sections.length-1, percentages, rectangle, tintColor);
	if (IsInsideRec(rectangle, MousePos) && showPieceTextOnHover)
	{
		Shader_t* oldShader = rc->state.boundShader;
		if (rc->state.boundShader != &pig->resources.shaders->main2D) { RcBindShader(&pig->resources.shaders->main2D); }
		NotNull(rc->state.boundFont);
		r32 mouseDistance = Vec2Length(MousePos - (rectangle.topLeft + rectangle.size/2));
		if (mouseDistance < MinR32(rectangle.width, rectangle.height)/2)
		{
			r32 mouseAngle = AtanR32(MousePos.y - (rectangle.y + rectangle.height/2), MousePos.x - (rectangle.x + rectangle.width/2)) / TwoPi32;
			if (mouseAngle < 0) { mouseAngle += 1.0f; }
			r32 anglePercentage = 0.0;
			for (u64 sIndex = 0; sIndex+1 < bundle->sections.length; sIndex++)
			{
				PerfSection_t* section = VarArrayGet(&bundle->sections, sIndex, PerfSection_t);
				PerfSection_t* nextSection = VarArrayGet(&bundle->sections, sIndex+1, PerfSection_t);
				if (mouseAngle >= anglePercentage && mouseAngle < anglePercentage + (r32)percentages[sIndex])
				{
					r64 sectionTimeMs = plat->GetPerfTimeDiff(&section->time, &nextSection->time);
					v2 textPos = NewVec2(rectangle.x, rectangle.y + rectangle.height - RcGetMaxDescend());
					Vec2Align(&textPos);
					RcDrawText(TempPrintStr("%s (%.1lfms)", (section->name != nullptr) ? section->name : "[Unnamed]", sectionTimeMs), textPos, MonokaiWhite, TextAlignment_Right);
					break;
				}
				anglePercentage += (r32)percentages[sIndex];
			}
		}
		if (oldShader != &pig->resources.shaders->main2D) { RcBindShader(oldShader); }
	}
	TempPopMark();
}
