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
	int width, height, rowWidth;
	pd->graphics->getBitmapData(
		bitmap,
		&width,
		&height,
		&rowWidth, //rowbytes
		(sizeOnly ? nullptr : &result.mask), //mask
		(sizeOnly ? nullptr : &result.data) //data
	);
	result.size.width = width;
	result.size.height = height;
	result.rowWidth = rowWidth;
	return result;
}

v2i GetBitmapSize(LCDBitmap* bitmap)
{
	int width, height;
	pd->graphics->getBitmapData(
		bitmap,
		&width,
		&height,
		nullptr, //rowbytes
		nullptr, //mask
		nullptr //data
	);
	return NewVec2i(width, height);
}

v2i MeasureText(LCDFont* font, MyStr_t text, i32 tracking = 0)
{
	v2i result;
	result.width  = pd->graphics->getTextWidth(font, text.chars, text.length, kUTF8Encoding, tracking);
	result.height = pd->graphics->getFontHeight(font);
	return result;
}

static volatile char dummy;

void MicroDelay(unsigned int milliseconds)
{
    unsigned int start = pd->system->getCurrentTimeMilliseconds();
    while (start + milliseconds > pd->system->getCurrentTimeMilliseconds())
    {
        (void)dummy;
    }
}

// +--------------------------------------------------------------+
// |                           Drawing                            |
// +--------------------------------------------------------------+
v2i renderOffset = Vec2i_Zero;
v2i PdSetRenderOffset(v2i offset)
{
	v2i result = renderOffset;
	renderOffset = offset;
	return result;
}

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
	renderOffset = Vec2i_Zero;
	boundFont = nullptr;
	currentDrawMode = kDrawModeCopy;
}

void PdDrawText(MyStr_t text, v2i position)
{
	pd->graphics->drawText(text.chars, text.length, kUTF8Encoding, renderOffset.x + position.x, renderOffset.y + position.y);
}
void PdDrawText(const char* textNullTerm, v2i position)
{
	PdDrawText(NewStr(textNullTerm), position);
}
void PdDrawTextPrint(v2i position, const char* formatString, ...)
{
	MemArena_t* scratch = GetScratchArena();
	PrintInArenaVa(scratch, printedText, printedLength, formatString);
	//TODO: Add error checking!
	PdDrawText(NewStr(printedLength, printedText), position);
	FreeScratchArena(scratch);
}

void PdDrawTexturedRec(LCDBitmap* bitmap, v2i bitmapSize, reci drawRec)
{
	NotNull(bitmap);
	pd->graphics->drawRotatedBitmap(
		bitmap,
		renderOffset.x + drawRec.x,
		renderOffset.y + drawRec.y,
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

void PdDrawTexturedObb(LCDBitmap* bitmap, v2i bitmapSize, obb2 drawObb)
{
	NotNull(bitmap);
	pd->graphics->drawRotatedBitmap(
		bitmap,
		(int)(renderOffset.x + drawObb.x),
		(int)(renderOffset.y + drawObb.y),
		ToDegrees32(drawObb.rotation), //rotation
		0.5f, 0.5f, //centerx/y
		(r32)drawObb.width / (r32)bitmapSize.width, //scalex
		(r32)drawObb.height / (r32)bitmapSize.height //scaley
	);
}
void PdDrawTexturedObb(Texture_t texture, obb2 drawObb)
{
	PdDrawTexturedObb(texture.bitmap, texture.size, drawObb);
}

bool currentClipRecActive = false;
reci currentClipRec = { -1, -1, -1, -1 };
reci PdSetClipRec(reci rectangle)
{
	reci oldClipRec = currentClipRec;
	if (rectangle.x == -1 && rectangle.y == -1 &&
		rectangle.width == -1 && rectangle.height == -1)
	{
		pd->graphics->clearClipRect();
		currentClipRecActive = false;
	}
	else
	{
		pd->graphics->setScreenClipRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height);
		currentClipRecActive = true;
	}
	currentClipRec = rectangle;
	return oldClipRec;
}
reci PdAddClipRec(reci rectangle)
{
	if (currentClipRecActive)
	{
		return PdSetClipRec(ReciOverlap(currentClipRec, rectangle));
	}
	else
	{
		return PdSetClipRec(rectangle);
	}
}
reci PdClearClipRec()
{
	return PdSetClipRec(NewReci(-1, -1, -1, -1));
}

void PdDrawTexturedRecPart(Texture_t texture, reci drawRec, reci sourceRec, Dir2_t rotation = Dir2_Right)
{
	reci oldClipRec = PdAddClipRec(drawRec);
	if (rotation == Dir2_Right)
	{
		//TODO: This needs to take sourceRec.size into account!
		reci largerRec = NewReci(
			drawRec.x - sourceRec.x,
			drawRec.y - sourceRec.y,
			texture.size
		);
		PdDrawTexturedRec(texture.bitmap, texture.size, largerRec);
	}
	else
	{
		//TODO: This needs to take sourceRec.size into account!
		obb2 largerObb = NewObb2D(
			ToVec2(drawRec.topLeft) + ToVec2(drawRec.size)/2,
			ToVec2(texture.size),
			GetDir2AngleR32(rotation)
		);
		v2 rightVec = ToVec2(rotation);
		v2 downVec = ToVec2(Dir2Clockwise(rotation));
		largerObb.center -= (((r32)sourceRec.x + (r32)sourceRec.width/2) - ((r32)texture.width/2)) * rightVec;
		largerObb.center -= (((r32)sourceRec.y + (r32)sourceRec.height/2) - ((r32)texture.height/2)) * downVec;
		PdDrawTexturedObb(texture.bitmap, texture.size, largerObb);
		
	}
	PdSetClipRec(oldClipRec);
}

Texture_t whiteDotTexture = {};
Texture_t blackDotTexture = {};
void PdDrawRec(reci drawRec, LCDColor color = kColorBlack)
{
	#if 0
	pd->graphics->drawRect(
		renderOffset.x + drawRec.x, //x
		renderOffset.y + drawRec.y, //y
		drawRec.width, //width
		drawRec.height, //height
		color //color
	);
	#else
	Assert(color == kColorBlack || color == kColorWhite);
	PdDrawTexturedRec((color == kColorWhite) ? whiteDotTexture : blackDotTexture, drawRec);
	#endif
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

void PdDrawLine(v2i start, v2i end, i32 width, LCDColor color = kColorBlack)
{
	pd->graphics->drawLine(
		renderOffset.x + start.x, //x1
		renderOffset.y + start.y, //y1
		renderOffset.x + end.x, //x2
		renderOffset.y + end.y, //y2
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
