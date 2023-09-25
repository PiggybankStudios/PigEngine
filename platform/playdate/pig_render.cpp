/*
File:   pig_render.cpp
Author: Taylor Robbins
Date:   09\24\2023
Description: 
	** Holds a bunch of functions that help us render things on Playdate
*/

//TODO: This doesn't work perfectly if startPercent and endPercent are more than 0.25f apart
void PdDrawRecOutlineArc(reci rectangle, i32 thickness, r32 startPercent, r32 endPercent, LCDColor color)
{
	// Assert(BasicallyBetweenR32(startPercent, 0.0f, 1.0f));
	// Assert(BasicallyBetweenR32(endPercent, 0.0f, 1.0f));
	Assert(BasicallyGreaterThanR32(endPercent, startPercent));
	
	r32 normalizedStartPercent = ModR32(startPercent, 1.0f);
	endPercent += (normalizedStartPercent - startPercent);
	startPercent = normalizedStartPercent;
	
	Range_t topRange = NewRange(InverseLerpR32(0.00f, 0.25f, startPercent, true), InverseLerpR32(0.00f, 0.25f, endPercent, true));
	if (endPercent > 1.0f)
	{
		r32 topEndPercent = ModR32(endPercent, 1.0f);
		r32 topStartPercent = startPercent + (topEndPercent - endPercent);
		topRange = NewRange(InverseLerpR32(0.00f, 0.25f, topStartPercent, true), InverseLerpR32(0.00f, 0.25f, topEndPercent, true));
	}
	if (topRange.max > topRange.min)
	{
		reci topRec = NewReci(
			RoundR32i(rectangle.x + (topRange.min * rectangle.width)),
			rectangle.y,
			RoundR32i((topRange.max - topRange.min) * rectangle.width),
			thickness
		);
		PdDrawRec(topRec, color);
	}
	
	Range_t rightRange = NewRange(InverseLerpR32(0.25f, 0.50f, startPercent, true), InverseLerpR32(0.25f, 0.50f, endPercent, true));
	if (rightRange.max > rightRange.min)
	{
		reci rightRec = NewReci(
			rectangle.x + rectangle.width - thickness,
			RoundR32i(rectangle.y + (rightRange.min * rectangle.height)),
			thickness,
			RoundR32i((rightRange.max - rightRange.min) * rectangle.height)
		);
		PdDrawRec(rightRec, color);
	}
	
	Range_t bottomRange = NewRange(InverseLerpR32(0.50f, 0.75f, startPercent, true), InverseLerpR32(0.50f, 0.75f, endPercent, true));
	if (bottomRange.max > bottomRange.min)
	{
		reci bottomRec = NewReci(
			RoundR32i(rectangle.x + ((1.0f - bottomRange.max) * rectangle.width)),
			rectangle.y + rectangle.height - thickness,
			RoundR32i((bottomRange.max - bottomRange.min) * rectangle.width),
			thickness
		);
		PdDrawRec(bottomRec, color);
	}
	
	Range_t leftRange = NewRange(InverseLerpR32(0.75f, 1.00f, startPercent, true), InverseLerpR32(0.75f, 1.00f, endPercent, true));
	if (leftRange.max > leftRange.min)
	{
		reci leftRec = NewReci(
			rectangle.x,
			RoundR32i(rectangle.y + ((1.0f - leftRange.max) * rectangle.height)),
			thickness,
			RoundR32i((leftRange.max - leftRange.min) * rectangle.height)
		);
		PdDrawRec(leftRec, color);
	}
}
