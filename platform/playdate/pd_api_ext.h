/*
File:   pd_api_ext.h
Author: Taylor Robbins
Date:   09\08\2023
Description: 
	** Holds some functions that we want to add on top of the default pd_api.h
*/

#ifndef _PD_API_EXT_H
#define _PD_API_EXT_H

// +--------------------------------------------------------------+
// |                         Enum Strings                         |
// +--------------------------------------------------------------+
const char* GetPDSystemEventStr(PDSystemEvent event)
{
	switch (event)
	{
		case kEventInit:        return "kEventInit";
		case kEventInitLua:     return "kEventInitLua";
		case kEventLock:        return "kEventLock";
		case kEventUnlock:      return "kEventUnlock";
		case kEventPause:       return "kEventPause";
		case kEventResume:      return "kEventResume";
		case kEventTerminate:   return "kEventTerminate";
		case kEventKeyPressed:  return "kEventKeyPressed";
		case kEventKeyReleased: return "kEventKeyReleased";
		case kEventLowPower:    return "kEventLowPower";
		default: return "Unknown";
	}
}
const char* GetPDLanguageStr(PDLanguage language)
{
	switch (language)
	{
		case kPDLanguageEnglish:  return "kPDLanguageEnglish";
		case kPDLanguageJapanese: return "kPDLanguageJapanese";
		case kPDLanguageUnknown:  return "kPDLanguageUnknown";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                          Functions                           |
// +--------------------------------------------------------------+
struct BitmapData_t
{
	v2i size;
	i32 rowWidth;
	u8* mask;
	u8* data;
};
BitmapData_t GetBitmapData(LCDBitmap* bitmap, bool sizeOnly = false)
{
	BitmapData_t result = {};
	pd->graphics->getBitmapData(
		bitmap,
		&result.size.width,
		&result.size.height,
		&result.rowWidth, //rowbytes
		(sizeOnly ? nullptr : &result.mask), //mask
		(sizeOnly ? nullptr : &result.data) //data
	);
	return result;
}

v2i GetBitmapSize(LCDBitmap* bitmap)
{
	v2i result = Vec2i_Zero;
	pd->graphics->getBitmapData(
		bitmap,
		&result.width,
		&result.height,
		nullptr, //rowbytes
		nullptr, //mask
		nullptr //data
	);
	return result;
}

v2i MeasureText(LCDFont* font, MyStr_t text, i32 tracking = 0)
{
	v2i result;
	result.width  = pd->graphics->getTextWidth(font, text.chars, text.length, kUTF8Encoding, tracking);
	result.height = pd->graphics->getFontHeight(font);
	return result;
}

// +--------------------------------------------------------------+
// |                           Drawing                            |
// +--------------------------------------------------------------+
Font_t* boundFont = nullptr;
Font_t* PdBindFont(Font_t* font)
{
	pd->graphics->setFont(font->font);
	Font_t* result = boundFont;
	boundFont = font;
	return result;
}

LCDBitmapDrawMode currentDrawMode = kDrawModeCopy;
LCDBitmapDrawMode PdSetDrawMode(LCDBitmapDrawMode drawMode)
{
	pd->graphics->setDrawMode(drawMode);
	LCDBitmapDrawMode result = currentDrawMode;
	currentDrawMode = drawMode;
	return result;
}

void PdBeginFrame()
{
	boundFont = nullptr;
	currentDrawMode = kDrawModeCopy;
}

void PdDrawText(MyStr_t text, v2i position)
{
	pd->graphics->drawText(text.chars, text.length, kUTF8Encoding, position.x, position.y);
}
void PdDrawTextPrint(v2i position, const char* formatString, ...)
{
	MemArena_t* scratch = GetScratchArena();
	PrintInArenaVa(scratch, printedText, printedLength, formatString);
	//TODO: Add error checking!
	PdDrawText(NewStr(printedLength, printedText), position);
	FreeScratchArena(scratch);
}

void PdDrawRec(reci drawRec, LCDColor color = kColorBlack)
{
	pd->graphics->drawRect(
		drawRec.x, //x
		drawRec.y, //y
		drawRec.width, //width
		drawRec.height, //height
		color //color
	);
}
void PdDrawRecOutline(reci drawRec, i32 thickness, bool outside = false, LCDColor color = kColorBlack)
{
	if (outside) { drawRec = ReciInflate(drawRec, thickness, thickness); }
	PdDrawRec(NewReci(drawRec.x, drawRec.y, thickness, drawRec.height), color); //left
	PdDrawRec(NewReci(drawRec.x + drawRec.width - thickness, drawRec.y, thickness, drawRec.height), color); //right
	if (drawRec.width > 2*thickness)
	{
		PdDrawRec(NewReci(drawRec.x + thickness, drawRec.y, drawRec.width - 2*thickness, thickness), color); //top
		PdDrawRec(NewReci(drawRec.x + thickness, drawRec.y + drawRec.height - thickness, drawRec.width - 2*thickness, thickness), color); //bottom
	}
}

void PdDrawTexturedRec(LCDBitmap* bitmap, v2i bitmapSize, reci drawRec)
{
	NotNull(bitmap);
	pd->graphics->drawRotatedBitmap(
		bitmap,
		drawRec.x, drawRec.y,
		0.0f, //rotation
		0.0f, 0.0f, //centerx/y
		(r32)drawRec.width / (r32)bitmapSize.width, //scalex
		(r32)drawRec.height / (r32)bitmapSize.height //scaley
	);
}
void PdDrawTexturedRec(Texture_t texture, reci drawRec)
{
	PdDrawTexturedRec(texture.bitmap, texture.size, drawRec);
}

void PdDrawLine(v2i start, v2i end, i32 width, LCDColor color = kColorBlack)
{
	pd->graphics->drawLine(
		start.x, //x1
		start.y, //y1
		end.x, //x2
		end.y, //y2
		width, //width
		color //color
	);
}

void PdDrawSheetFrame(SpriteSheet_t sheet, v2i frame, reci drawRec)
{
	i32 frameIndex = (frame.y * sheet.numFramesX) + frame.x;
	LCDBitmap* frameBitmap = pd->graphics->getTableBitmap(sheet.table, frameIndex);
	if (frameBitmap != nullptr)
	{
		PdDrawTexturedRec(frameBitmap, sheet.frameSize, drawRec);
	}
}
void PdDrawSheetFrame(SpriteSheet_t sheet, v2i frame, v2i position)
{
	PdDrawSheetFrame(sheet, frame, NewReci(position, sheet.frameSize));
}

#endif //  _PD_API_EXT_H
