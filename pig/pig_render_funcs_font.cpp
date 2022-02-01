/*
File:   pig_render_funcs_font.cpp
Author: Taylor Robbins
Date:   12\30\2021
Description: 
	** Holds a bunch of useful functions that help us easily render text on screen using
	** the Render Context state to assume certain aspects (like which font to use)
*/

// +--------------------------------------------------------------+
// |                       Render Functions                       |
// +--------------------------------------------------------------+
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

TextMeasure_t RcMeasureText(MyStr_t text, r32 maxWidth = 0, FontFlowInfo_t* infoOut = nullptr)
{
	return MeasureTextInFont(text, rc->state.boundFont, rc->state.faceSelector, rc->state.fontScale, maxWidth, infoOut, &rc->state.flowCallbacks);
}
TextMeasure_t RcMeasureText(const char* nulltermStr, r32 maxWidth = 0, FontFlowInfo_t* infoOut = nullptr)
{
	return RcMeasureText(NewStr(nulltermStr), maxWidth, infoOut);
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
		if (rc->state.boundShader != &pig->resources.mainShader2D) { RcBindShader(&pig->resources.mainShader2D); }
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
		if (oldShader != &pig->resources.mainShader2D) { RcBindShader(oldShader); }
	}
	TempPopMark();
}
