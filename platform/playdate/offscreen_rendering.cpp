/*
File:   offscreen_rendering.cpp
Author: Taylor Robbins
Date:   12\26\2023
Description: 
	** Holds functions that help us render complex things using offscreen bitmaps.
	** For example, rendering rotated text by drawing into bitmaps and then drawing those bitmaps rotated
*/

void InitOffscreenRendering()
{
	NotNull(pig);
	OffscreenRenderState_t* state = &pig->offscreenRenderState;
	ClearPointer(state);
	
	for (u8 bIndex = 0; bIndex < MAX_NUM_OFFSCREEN_BITMAPS; bIndex++)
	{
		state->bitmaps[bIndex].index = bIndex;
		state->bitmaps[bIndex].texture = CreateTexture(NewVec2i(OFFSCREEN_BITMAP_WIDTH, OFFSCREEN_BITMAP_HEIGHT), 0, nullptr, nullptr);
	}
}

void OffscreenRenderingBeginFrame()
{
	NotNull(pig);
	OffscreenRenderState_t* state = &pig->offscreenRenderState;
	
	for (u8 bIndex = 0; bIndex < MAX_NUM_OFFSCREEN_BITMAPS; bIndex++)
	{
		state->bitmaps[bIndex].numDirtyRecs = 0;
	}
}

bool TryPackAnotherRec(reci boundingRec, u64 numExistingRecs, const reci* existingRecs, v2i newRecSize, v2i* packPosOut)
{
	//If it doesn't fit in the boundingRec on it's own, it will never file
	if (newRecSize.width > boundingRec.width || newRecSize.height > boundingRec.height) { return false; }
	
	if (numExistingRecs == 0)
	{
		SetOptionalOutPntr(packPosOut, NewVec2i(0, 0));
		return true;
	}
	else
	{
		//Try and fit the newRec to the right of one of the existing recs that is at least as tall
		for (u64 rIndex = 0; rIndex < numExistingRecs; rIndex++)
		{
			reci existingRec = existingRecs[rIndex];
			if (existingRec.height >= newRecSize.height && (boundingRec.x + boundingRec.width) - (existingRec.x + existingRec.width) >= newRecSize.width)
			{
				SetOptionalOutPntr(packPosOut, NewVec2i(
					existingRec.x + existingRec.width,
					existingRec.y
				));
				return true;
			}
		}
		
		//Try to fit the newRec below all the existing recs
		reci lastRec = existingRecs[numExistingRecs-1];
		if ((boundingRec.y + boundingRec.height) - (lastRec.y + lastRec.height) >= newRecSize.height)
		{
			SetOptionalOutPntr(packPosOut, NewVec2i(
				0,
				lastRec.y + lastRec.height
			));
			return true;
		}
		else
		{
			return false;
		}
	}
}

OffscreenTarget_t GetOffscreenTarget(v2i neededSize, bool forceTopLeft = false)
{
	Assert(neededSize.width <= OFFSCREEN_BITMAP_WIDTH);
	Assert(neededSize.height <= OFFSCREEN_BITMAP_HEIGHT);
	
	NotNull(pig);
	OffscreenRenderState_t* state = &pig->offscreenRenderState;
	OffscreenTarget_t result = {};
	
	for (u8 bIndex = 0; bIndex < MAX_NUM_OFFSCREEN_BITMAPS; bIndex++)
	{
		OffscreenBitmap_t* bitmap = &state->bitmaps[bIndex];
		if (bitmap->numDirtyRecs < MAX_NUM_OFFSCREEN_DIRTY_RECS)
		{
			v2i newDirtyRecPos = Vec2i_Zero;
			if (TryPackAnotherRec(
				NewReci(0, 0, OFFSCREEN_BITMAP_WIDTH, OFFSCREEN_BITMAP_HEIGHT),
				bitmap->numDirtyRecs,
				&bitmap->dirtyRecs[0],
				neededSize,
				&newDirtyRecPos))
			{
				if (!forceTopLeft || newDirtyRecPos == Vec2i_Zero)
				{
					result.pntr = bitmap;
					result.targetRec = NewReci(newDirtyRecPos, neededSize);
					bitmap->dirtyRecs[bitmap->numDirtyRecs] = result.targetRec;
					bitmap->numDirtyRecs++;
					break;
				}
			}
		}
	}
	
	return result;
}

OffscreenTarget_t currentOffscreenTarget = {};
void PdPushOffscreenTarget(OffscreenTarget_t target)
{
	NotNull(target.pntr);
	if (currentOffscreenTarget.pntr != nullptr) { pd->graphics->popContext(); }
	pd->graphics->pushContext(target.pntr->texture.bitmap);
	PdSetRenderOffset(-target.targetRec.topLeft);
	currentOffscreenTarget = target;
}
void PdPopOffscreenTarget(OffscreenTarget_t target)
{
	NotNull(target.pntr);
	Assert(target.pntr == currentOffscreenTarget.pntr);
	pd->graphics->popContext();
	currentOffscreenTarget = {};
}

void PdDrawOffscreenResults(OffscreenTarget_t target, reci drawRec, Dir2_t rotation = Dir2_Right)
{
	PdDrawTexturedRecPart(target.pntr->texture, drawRec, target.targetRec, rotation);
}
