/*
File:   playdate_input_hints.cpp
Author: Taylor Robbins
Date:   09\12\2023
Description: 
	** Holds functions that help us render hints for various inputs on the Playdate
*/

reci PlaydateCrankHintBubble(bool doRender, bool pointRight, r32 scale, v2i targetPos)
{
	reci bubbleRec;
	bubbleRec.size = Vec2Roundi(ToVec2(pig->crankHintBackSheet.frameSize) * scale);
	if (pointRight) { bubbleRec.x = targetPos.x - bubbleRec.width; }
	else { bubbleRec.x = targetPos.x; }
	bubbleRec.y = targetPos.y - bubbleRec.height/2;
	if (!doRender) { return bubbleRec; }
	
	reci crankRec;
	crankRec.size = Vec2Roundi(ToVec2(pig->crankHintSheet.frameSize) * scale);
	v2i bubbleCenter = bubbleRec.topLeft;
	if (pointRight) { bubbleCenter += Vec2iFill(bubbleRec.height/2); }
	else { bubbleCenter += NewVec2i(bubbleRec.width - bubbleRec.height/2, bubbleRec.height/2); }
	crankRec.topLeft = bubbleCenter - NewVec2i(crankRec.width/2, crankRec.height/2);
	
	v2i crankFrame = NewVec2i((i32)AnimateU64(0, pig->crankHintSheet.numFramesX, 600), (pointRight ? 0 : 1));
	
	PdDrawSheetFrame(pig->crankHintBackSheet, NewVec2i(0, 0), bubbleRec);
	PdDrawSheetFrame(pig->crankHintSheet, crankFrame, crankRec);
	
	return bubbleRec;
}
