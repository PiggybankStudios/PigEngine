/*
File:   offscreen_rendering.h
Author: Taylor Robbins
Date:   12\26\2023
*/

#ifndef _OFFSCREEN_RENDERING_H
#define _OFFSCREEN_RENDERING_H

// Offscreen bitmaps are the squared up size of the playdate screen
#define OFFSCREEN_BITMAP_WIDTH   PLAYDATE_SCREEN_WIDTH
#define OFFSCREEN_BITMAP_HEIGHT  PLAYDATE_SCREEN_WIDTH

#define MAX_NUM_OFFSCREEN_BITMAPS     4
#define MAX_NUM_OFFSCREEN_DIRTY_RECS  16

struct OffscreenBitmap_t
{
	u8 index;
	Texture_t texture;
	u8 numDirtyRecs;
	reci dirtyRecs[MAX_NUM_OFFSCREEN_DIRTY_RECS];
};

struct OffscreenTarget_t
{
	OffscreenBitmap_t* pntr;
	reci targetRec;
};

struct OffscreenRenderState_t
{
	OffscreenBitmap_t bitmaps[MAX_NUM_OFFSCREEN_BITMAPS];
};

#endif //  _OFFSCREEN_RENDERING_H
