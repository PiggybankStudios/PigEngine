/*
File:   pig_render_funcs_2d.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description: 
	** Holds a bunch of functions that help us render simple and commonly used geometry and patterns
*/

void RcDrawTriangle2D(v2* positions, Color_t color)
{
	NotNull(positions);
	RcSetWorldMatrix(Mat4_Identity);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	v4 colorVec = ToVec4(color);
	Vertex2D_t vertices[3];
	vertices[0].position = NewVec3(positions[0].x, positions[0].y, RcGetRealDepth()); vertices[0].color = colorVec; vertices[0].texCoord = NewVec2(0, 0);
	vertices[1].position = NewVec3(positions[1].x, positions[1].y, RcGetRealDepth()); vertices[1].color = colorVec; vertices[1].texCoord = NewVec2(1, 0);
	vertices[2].position = NewVec3(positions[2].x, positions[2].y, RcGetRealDepth()); vertices[2].color = colorVec; vertices[2].texCoord = NewVec2(0, 1);
	ChangeVertBufferVertices2D(&rc->scratchBuffer2D, 0, 3, &vertices[0]);
	RcBindVertBuffer(&rc->scratchBuffer2D);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0, 3);
}
void RcDrawTriangle2D(v2 p1, v2 p2, v2 p3, Color_t color)
{
	v2 positions[3];
	positions[0] = p1;
	positions[1] = p2;
	positions[2] = p3;
	RcDrawTriangle2D(&positions[0], color);
}

//Textured
void RcDrawTriangle2DEx(v2 p1, v2 p2, v2 p3, v2 tc1, v2 tc2, v2 tc3, Color_t color)
{
	RcSetWorldMatrix(Mat4_Identity);
	RcSetColor1(color);
	v4 colorVec = ToVec4(color);
	Vertex2D_t vertices[3];
	vertices[0].position = NewVec3(p1.x, p1.y, RcGetRealDepth()); vertices[0].color = colorVec; vertices[0].texCoord = tc1;
	vertices[1].position = NewVec3(p2.x, p2.y, RcGetRealDepth()); vertices[1].color = colorVec; vertices[1].texCoord = tc2;
	vertices[2].position = NewVec3(p3.x, p3.y, RcGetRealDepth()); vertices[2].color = colorVec; vertices[2].texCoord = tc3;
	ChangeVertBufferVertices2D(&rc->scratchBuffer2D, 0, 3, &vertices[0]);
	RcBindVertBuffer(&rc->scratchBuffer2D);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0, 3);
}

void RcDrawTexturedQuad(v2 topLeft, v2 topRight, v2 bottomRight, v2 bottomLeft, Color_t color)
{
	RcDrawTriangle2DEx(
		topLeft, topRight, bottomLeft,
		NewVec2(0, 0), NewVec2(1, 0), NewVec2(0, 1),
		color
	);
	RcDrawTriangle2DEx(
		bottomLeft, topRight, bottomRight,
		NewVec2(0, 1), NewVec2(1, 0), NewVec2(1, 1),
		color
	);
}
void RcDrawTexturedQuadSubPart(v2 topLeft, v2 topRight, v2 bottomRight, v2 bottomLeft, rec subPart, Color_t color)
{
	UNUSED(bottomRight);
	v2 rightVec = (topRight - topLeft);
	v2 downVec = (bottomLeft - topLeft);
	v2 realTopLeft = topLeft + (subPart.x * rightVec) + (subPart.y * downVec);
	v2 realTopRight = topLeft + ((subPart.x + subPart.width) * rightVec) + (subPart.y * downVec);
	v2 realBottomLeft = topLeft + (subPart.x * rightVec) + ((subPart.y + subPart.height) * downVec);
	v2 realBottomRight = topLeft + ((subPart.x + subPart.width) * rightVec) + ((subPart.y + subPart.height) * downVec);
	RcDrawTexturedQuad(realTopLeft, realTopRight, realBottomRight, realBottomLeft, color);
}

void RcDrawRectangle(rec rectangle, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

void RcDrawRectangleOutline(rec rectangle, Color_t color, r32 thickness, bool outsideRectangle = false)
{
	rec topRec    = NewRec(rectangle.x, rectangle.y - (outsideRectangle ? thickness : 0), rectangle.width, thickness);
	rec bottomRec = NewRec(rectangle.x, rectangle.y + rectangle.height - (outsideRectangle ? 0 : thickness), rectangle.width, thickness);
	rec leftRec   = NewRec(rectangle.x - (outsideRectangle ? thickness : 0), rectangle.y + (outsideRectangle ? -thickness : thickness), thickness, rectangle.height + (outsideRectangle ? thickness*2 : -thickness*2));
	rec rightRec  = NewRec(rectangle.x + rectangle.width - (outsideRectangle ? 0 : thickness), leftRec.y, thickness, leftRec.height);
	RcDrawRectangle(topRec,    color);
	RcDrawRectangle(bottomRec, color);
	RcDrawRectangle(leftRec,   color);
	RcDrawRectangle(rightRec,  color);
}

void RcDrawTexturedRectangle(rec rectangle, Color_t color)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	rec sourceRec = NewRec(Vec2_Zero, rc->state.boundTexture1->size);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedPartRectangle(rec rectangle, Color_t color, rec sourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

void RcDrawGradientRectangle(rec rectangle, bool horizontal, Color_t colorLeftOrTop, Color_t colorRightOrBottom, bool bindShader = true)
{
	Shader_t* oldShader = rc->state.boundShader;
	Color_t oldColor2 = rc->state.color2;
	if (bindShader) { RcBindShader(&pig->resources.shaders->gradient2D); }
	RcBindTexture1(horizontal ? &rc->gradientHorizontal : &rc->gradientVertical);
	RcSetColor2(colorRightOrBottom);
	RcDrawTexturedRectangle(rectangle, colorLeftOrTop);
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawRoundedRectangleWithBorder(rec rectangle, r32 cornerRadius, r32 borderThickness, Color_t fillColor, Color_t borderColor, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	Color_t oldColor2 = rc->state.color2;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldValue0 = rc->state.values[0];
	
	if (bindShader) { RcBindShader(&pig->resources.shaders->roundedCorners); }
	RcSetColor2(borderColor);
	RcSetShiftVec(rectangle.size);
	RcSetCircleRadius(cornerRadius);
	RcSetValue(0, borderThickness + 0.5f);
	
	RcDrawRectangle(rectangle, fillColor);
	
	RcSetColor2(oldColor2);
	RcSetShiftVec(oldShiftVec);
	RcSetCircleRadius(oldCircleRadius);
	RcSetValue(0, oldValue0);
	if (bindShader) { RcBindShader(oldShader); }
}
void RcDrawRoundedRectangle(rec rectangle, r32 cornerRadius, Color_t color, bool bindShader = true)
{
	RcDrawRoundedRectangleWithBorder(rectangle, cornerRadius, 0, color, Transparent, bindShader);
}

void RcDrawVoxelOrthoFace2D(rec rectangle, rec sourceRec, Color_t color, Dir3_t side)
{
	Assert(side == Dir3_Forward || side == Dir3_Left || side == Dir3_Up || side == Dir3_All);
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	if (side == Dir3_All)
	{
		RcBindVertBuffer(&rc->voxelOrtho2D);
		RcDrawBuffer(VertBufferPrimitive_Triangles);
	}
	else
	{
		u64 startIndex = 0;
		if (side == Dir3_Left) { startIndex = 6; }
		if (side == Dir3_Up) { startIndex = 12; }
		RcBindVertBuffer(&rc->voxelOrtho2D);
		RcDrawBuffer(VertBufferPrimitive_Triangles, startIndex, 6);
	}
}
void RcDrawVoxelOrtho2D(rec rectangle, rec forwardSourceRec, rec leftSourceRec, rec topSourceRec, Color_t forwardColor, Color_t leftColor, Color_t topColor)
{
	RcDrawVoxelOrthoFace2D(rectangle, forwardSourceRec, forwardColor, Dir3_Forward);
	RcDrawVoxelOrthoFace2D(rectangle, leftSourceRec,    leftColor,    Dir3_Left);
	RcDrawVoxelOrthoFace2D(rectangle, topSourceRec,     topColor,     Dir3_Up);
}
void RcDrawVoxelOrtho2D(rec rectangle, rec sourceRec, Color_t color)
{
	RcDrawVoxelOrthoFace2D(rectangle, sourceRec, color, Dir3_All);
}

void RcDrawCircle(v2 center, r32 radius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldInnerCircleRadius = rc->state.circleInnerRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.shaders->main2D); }
	RcSetCircleRadius(1.0f);
	RcSetCircleInnerRadius(0.0f);
	
	RcDrawRectangle(NewRecCentered(center, Vec2Fill(radius*2)), color);
	
	RcSetCircleRadius(oldCircleRadius);
	RcSetCircleInnerRadius(oldInnerCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}
void RcDrawDonut(v2 center, r32 innerRadius, r32 radius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldInnerCircleRadius = rc->state.circleInnerRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.shaders->main2D); }
	RcSetCircleRadius(1.0f);
	RcSetCircleInnerRadius(innerRadius / radius);
	
	RcDrawRectangle(NewRecCentered(center, Vec2Fill(radius*2)), color);
	
	RcSetCircleRadius(oldCircleRadius);
	RcSetCircleInnerRadius(oldInnerCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}
void RcDrawInverseCircle(v2 center, r32 radius, Color_t outerColor, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldInnerCircleRadius = rc->state.circleInnerRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.shaders->main2D); }
	RcSetCircleRadius(10.0f);
	RcSetCircleInnerRadius(1.0f);
	
	RcDrawRectangle(NewRecCentered(center, Vec2Fill(radius*2)), outerColor);
	
	RcSetCircleRadius(oldCircleRadius);
	RcSetCircleInnerRadius(oldInnerCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawObb2(obb2 boundingBox, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate2(-Vec2_Half));
	Mat4Transform(worldMatrix, Mat4Scale2(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4RotateZ(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.x, boundingBox.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawObb2(obb2 boundingBox, Colorf_t colorf)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate2(-Vec2_Half));
	Mat4Transform(worldMatrix, Mat4Scale2(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4RotateZ(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.x, boundingBox.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(colorf);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedObb2(obb2 boundingBox, Color_t color, rec sourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate2(-Vec2_Half));
	Mat4Transform(worldMatrix, Mat4Scale2(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4RotateZ(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.x, boundingBox.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedObb2(obb2 boundingBox, Color_t color)
{
	NotNull(rc->state.boundTexture1);
	rec sourceRec = NewRec(Vec2_Zero, rc->state.boundTexture1->size);
	RcDrawTexturedObb2(boundingBox, color, sourceRec);
}
void RcDrawObb2Outline(obb2 boundingBox, Color_t color, r32 thickness, bool outsideBox = false)
{
	if (outsideBox)
	{
		Unimplemented(); //TODO: Implement me!
	}
	else
	{
		obb2 topBox    = NewObb2D(GetObb2DWorldPoint(boundingBox, NewVec2(boundingBox.width/2, thickness/2)), NewVec2(boundingBox.width, thickness), boundingBox.rotation);
		obb2 bottomBox = NewObb2D(GetObb2DWorldPoint(boundingBox, NewVec2(boundingBox.width/2, boundingBox.height - thickness/2)), NewVec2(boundingBox.width, thickness), boundingBox.rotation);
		obb2 leftBox   = NewObb2D(GetObb2DWorldPoint(boundingBox, NewVec2(thickness/2, boundingBox.height/2)), NewVec2(thickness, boundingBox.height), boundingBox.rotation);
		obb2 rightBox  = NewObb2D(GetObb2DWorldPoint(boundingBox, NewVec2(boundingBox.width - thickness/2, boundingBox.height/2)), NewVec2(thickness, boundingBox.height), boundingBox.rotation);
		RcDrawObb2(topBox,    color);
		RcDrawObb2(bottomBox, color);
		RcDrawObb2(leftBox,   color);
		RcDrawObb2(rightBox,  color);
	}
}
//TODO: Add textured version of this function
void RcDrawRoundedObb2(obb2 boundingBox, r32 cornerRadius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.shaders->roundedCorners); }
	RcSetShiftVec(boundingBox.size);
	RcSetCircleRadius(cornerRadius);
	
	RcDrawObb2(boundingBox, color);
	
	RcSetShiftVec(oldShiftVec);
	RcSetCircleRadius(oldCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}

//NOTE: rotation is applied after flipX/flipY
void RcDrawSheetFrame(v2i frame, rec rectangle, Color_t color, bool flipX = false, bool flipY = false, Dir2_t rotation = Dir2_Down)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	rec sourceRec = GetSpriteSheetFrameSourceRec(sheet, frame);
	if (flipX) { sourceRec.x += sourceRec.width; sourceRec.width = -sourceRec.width; }
	if (flipY) { sourceRec.y += sourceRec.height; sourceRec.height = -sourceRec.height; }
	if (rotation != Dir2_Down)
	{
		obb2 boundingBox = NewObb2D(rectangle.topLeft + rectangle.size/2, rectangle.size, 0.0f);
		if (rotation == Dir2_Left || rotation == Dir2_Right) { SWAP_VARIABLES(r32, boundingBox.width, boundingBox.height); }
		if (rotation == Dir2_Left)       { boundingBox.rotation = HalfPi32;       }
		else if (rotation == Dir2_Up)    { boundingBox.rotation = Pi32;           }
		else if (rotation == Dir2_Right) { boundingBox.rotation = ThreeHalfsPi32; }
		else { AssertMsg(false, "Rotation is an unsupported value"); }
		RcBindTexture1(&sheet->texture);
		RcDrawTexturedObb2(boundingBox, color, sourceRec);
	}
	else
	{
		RcBindTexture1(&sheet->texture);
		RcDrawTexturedPartRectangle(rectangle, color, sourceRec);
	}
}
void RcDrawSheetFrame(MyStr_t frameName, rec rectangle, Color_t color, bool flipX = false, bool flipY = false, Dir2_t rotation = Dir2_Down)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	SpriteSheetFrame_t* sheetFrame = TryGetSpriteSheetFrame(sheet, frameName);
	if (sheetFrame != nullptr)
	{
		RcDrawSheetFrame(sheetFrame->gridPos, rectangle, color, flipX, flipY, rotation);
	}
	else
	{
		RcDrawRectangle(rectangle, color);
	}
}

void RcDrawSheetFrame(v2i frame, obb2 boundingBox, Color_t color, bool flipX = false, bool flipY = false)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	rec sourceRec = GetSpriteSheetFrameSourceRec(sheet, frame);
	if (flipX) { sourceRec.x += sourceRec.width; sourceRec.width = -sourceRec.width; }
	if (flipY) { sourceRec.y += sourceRec.height; sourceRec.height = -sourceRec.height; }
	RcBindTexture1(&sheet->texture);
	RcDrawTexturedObb2(boundingBox, color, sourceRec);
}
void RcDrawSheetFrame(MyStr_t frameName, obb2 boundingBox, Color_t color, bool flipX = false, bool flipY = false)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	SpriteSheetFrame_t* sheetFrame = TryGetSpriteSheetFrame(sheet, frameName);
	if (sheetFrame != nullptr)
	{
		RcDrawSheetFrame(sheetFrame->gridPos, boundingBox, color, flipX, flipY);
	}
	else
	{
		RcDrawObb2(boundingBox, color);
	}
}

void RcDrawEquilTriangleFrom(v2 base, r32 direction, r32 height, Color_t color)
{
	r32 triangleHeight = SqrtR32(3)/2;
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4RotateZ(direction));
	Mat4Transform(worldMatrix, Mat4Scale2(height/triangleHeight, height/triangleHeight));
	Mat4Transform(worldMatrix, Mat4Translate3(base.x, base.y, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->equilTriangleBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawEquilTriangleTo(v2 point, r32 direction, r32 height, Color_t color)
{
	RcDrawEquilTriangleFrom(point - Vec2FromAngle(direction, height), direction, height, color);
}
void RcDrawEquilTriangleFromTo(v2 base, v2 tip, Color_t color)
{
	RcDrawEquilTriangleFrom(base, AngleFromVec2(tip - base), Vec2Length(tip - base), color);
}

void RcDrawPoint(v2 point, r32 size, Color_t color)
{
	rec pointRec = NewRecCentered(point, size, size);
	RcDrawRectangle(pointRec, color);
}
void RcDrawLine(v2 start, v2 end, r32 thickness, Color_t color)
{
	obb2 lineRec = Obb2Line(start, end, thickness);
	RcDrawObb2(lineRec, color);
}
void RcDrawLine(v2 start, v2 end, r32 thickness, Colorf_t colorf)
{
	obb2 lineRec = Obb2Line(start, end, thickness);
	RcDrawObb2(lineRec, colorf);
}

void RcDrawBezier3WithLines(v2 start, v2 control, v2 end, r32 thickness, Color_t color, u32 numVerts)
{
	for (u32 vIndex = 0; vIndex < numVerts; vIndex++)
	{
		r32 t1 = (r32)vIndex * (1.0f/numVerts);
		r32 t2 = (r32)(vIndex+1) * (1.0f/numVerts);
		v2 pos1 = BezierCurve3(start, control, end, t1);
		v2 pos2 = BezierCurve3(start, control, end, t2);
		RcDrawLine(pos1, pos2, thickness, color);
	}
}
void RcDrawBezier4WithLines(v2 start, v2 control1, v2 control2, v2 end, r32 thickness, Color_t color, u32 numVerts)
{
	for (u32 vIndex = 0; vIndex < numVerts; vIndex++)
	{
		r32 t1 = (r32)vIndex * (1.0f/numVerts);
		r32 t2 = (r32)(vIndex+1) * (1.0f/numVerts);
		v2 pos1 = BezierCurve4(start, control1, control2, end, t1);
		v2 pos2 = BezierCurve4(start, control1, control2, end, t2);
		RcDrawLine(pos1, pos2, thickness, color);
	}
}

void RcDrawBezier3WithShader(v2 start, v2 control, v2 end, r32 thickness, Color_t color, bool bindShader = true)
{
	v2 min = NewVec2(MinR32(start.x, control.x, end.x), MinR32(start.y, control.y, end.y));
	v2 max = NewVec2(MaxR32(start.x, control.x, end.x), MaxR32(start.y, control.y, end.y));
	rec bounds = NewRecBetween(min, max);
	bounds = RecInflate(bounds, thickness, thickness);
	bounds = RecSquarify(bounds);
	// RecAlign(&bounds);
	
	Shader_t* oldShader = rc->state.boundShader;
	if (bindShader) { RcBindShader(&pig->resources.shaders->bezier3); }
	RcSetDynamicUniformVec2("StartPos", Vec2Divide((start - bounds.topLeft), bounds.size));
	RcSetDynamicUniformVec2("Control", Vec2Divide((control - bounds.topLeft), bounds.size));
	RcSetDynamicUniformVec2("EndPos",   Vec2Divide((end - bounds.topLeft), bounds.size));
	RcSetDynamicUniformR32("Thickness", (thickness / bounds.width));
	
	RcDrawRectangle(bounds, color);
	
	if (bindShader) { RcBindShader(oldShader); }
	
	// RcDrawRectangleOutline(bounds, MonokaiRed, 1);
}
void RcDrawBezier4WithShader(v2 start, v2 control1, v2 control2, v2 end, r32 thickness, Color_t color, bool bindShader = true)
{
	v2 min = NewVec2(MinR32(start.x, control1.x, control2.x, end.x), MinR32(start.y, control1.y, control2.y, end.y));
	v2 max = NewVec2(MaxR32(start.x, control1.x, control2.x, end.x), MaxR32(start.y, control1.y, control2.y, end.y));
	rec bounds = NewRecBetween(min, max);
	bounds = RecInflate(bounds, thickness, thickness);
	bounds = RecSquarify(bounds);
	// RecAlign(&bounds);
	
	Shader_t* oldShader = rc->state.boundShader;
	if (bindShader) { RcBindShader(&pig->resources.shaders->bezier4); }
	RcSetDynamicUniformVec2("StartPos", Vec2Divide((start - bounds.topLeft), bounds.size));
	RcSetDynamicUniformVec2("Control1", Vec2Divide((control1 - bounds.topLeft), bounds.size));
	RcSetDynamicUniformVec2("Control2", Vec2Divide((control2 - bounds.topLeft), bounds.size));
	RcSetDynamicUniformVec2("EndPos",   Vec2Divide((end - bounds.topLeft), bounds.size));
	RcSetDynamicUniformR32("Thickness", (thickness / bounds.width));
	
	RcDrawRectangle(bounds, color);
	
	if (bindShader) { RcBindShader(oldShader); }
	
	// RcDrawRectangleOutline(bounds, MonokaiRed, 1);
}
void RcDrawEllipseArcWithShader(v2 center, v2 radius, r32 axisAngle, r32 startAngle, r32 angleDelta, r32 thickness, Color_t color, bool bindShader = true)
{
	rec bounds = NewRecCentered(center, Vec2Fill(MaxR32(radius.x, radius.y) * 2));
	bounds = RecInflate(bounds, thickness, thickness);
	// RecAlign(&bounds);
	
	Shader_t* oldShader = rc->state.boundShader;
	if (bindShader) { RcBindShader(&pig->resources.shaders->ellipseArc); }
	RcSetDynamicUniformVec2("Center", Vec2_Half);
	RcSetDynamicUniformVec2("Radius", (radius / bounds.width));
	RcSetDynamicUniformR32("Rotation", axisAngle);
	RcSetDynamicUniformVec2("AngleRange", NewVec2(startAngle, angleDelta));
	RcSetDynamicUniformR32("Thickness", (thickness / bounds.width));
	
	RcDrawRectangle(bounds, color);
	
	if (bindShader) { RcBindShader(oldShader); }
	
	// RcDrawRectangleOutline(bounds, MonokaiRed, 1);
}

void RcDrawBezierPathWithShader(const BezierPath_t* path, r32 thickness, Color_t color, v2 offset, v2 scale, bool bindShader = true)
{
	NotNull(path);
	
	Shader_t* oldShader = rc->state.boundShader;
	if (bindShader) { RcBindShader(&pig->resources.shaders->bezier4); }
	
	v2 currentPos = Vec2_Zero;
	VarArrayLoop(&path->parts, pIndex)
	{
		VarArrayLoopGet(BezierPathPart_t, part, &path->parts, pIndex);
		if (part->detached) { currentPos = part->startPos; }
		
		switch (part->type)
		{
			case BezierPathPartType_Line:
			{
				//TODO: We should probably just add a code path for lines drawing rather than forcing ourselves to consider lines as curves
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				v2 midpoint = (startPos + endPos) / 2;
				if (rc->state.boundShader != &pig->resources.shaders->bezier3) { RcBindShader(&pig->resources.shaders->bezier3); }
				RcDrawBezier3WithShader(startPos, midpoint, endPos, thickness, color, false);
			} break;
			case BezierPathPartType_Curve3:
			{
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 control = offset + Vec2Multiply(part->control1, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				if (rc->state.boundShader != &pig->resources.shaders->bezier3) { RcBindShader(&pig->resources.shaders->bezier3); }
				RcDrawBezier3WithShader(startPos, control, endPos, thickness, color, false);
			} break;
			case BezierPathPartType_Curve4:
			{
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 control1 = offset + Vec2Multiply(part->control1, scale);
				v2 control2 = offset + Vec2Multiply(part->control2, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				if (rc->state.boundShader != &pig->resources.shaders->bezier4) { RcBindShader(&pig->resources.shaders->bezier4); }
				RcDrawBezier4WithShader(startPos, control1, control2, endPos, thickness, color, false);
			} break;
			case BezierPathPartType_EllipseArc:
			{
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				//TODO: Handle non-uniform scaling for radius!
				v2 radius = Vec2Multiply(part->radius, scale);
				v2 arcCenter = Vec2_Zero;
				r32 arcAngleStart = 0;
				r32 arcAngleDelta = 0;
				if (GetEllipseArcCurveCenterAndAngles(startPos, radius, part->axisAngle, part->arcFlags, endPos, &arcCenter, &arcAngleStart, &arcAngleDelta))
				{
					if (rc->state.boundShader != &pig->resources.shaders->ellipseArc) { RcBindShader(&pig->resources.shaders->ellipseArc); }
					RcDrawEllipseArcWithShader(arcCenter, radius, part->axisAngle, arcAngleStart, arcAngleDelta, thickness, color, false);
				}
				else
				{
					//draw a line
					v2 midpoint = (currentPos + part->endPos) / 2;
					if (rc->state.boundShader != &pig->resources.shaders->bezier3) { RcBindShader(&pig->resources.shaders->bezier3); }
					RcDrawBezier3WithShader(currentPos + offset, midpoint + offset, part->endPos + offset, thickness, color, false);
				}
			} break;
			default: Unimplemented(); break;
		}
		
		currentPos = part->endPos;
	}
	
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawPieChart(u64 numPiePieces, r64* piePiecePercentages, rec rectangle, Color_t tintColor)
{
	Assert(piePiecePercentages != nullptr || numPiePieces == 0);
	
	//TODO: This is a perfect scenario to utilize a context push and pop. A lot of values are being used and need to be reset when we are done
	Shader_t* oldShader = rc->state.boundShader;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldCircleInnerRadius = rc->state.circleInnerRadius;
	if (rc->state.boundShader != &pig->resources.shaders->pieChart) { RcBindShader(&pig->resources.shaders->pieChart); }
	r64 chunkBasePercentage = 0.0;
	for (u64 pieChunkIndex = 0; pieChunkIndex*ShaderUniform_NumGenericValues < numPiePieces; pieChunkIndex++)
	{
		r64 thisChunkPercentage = 0.0;
		for (u8 vIndex = 0; vIndex < ShaderUniform_NumGenericValues; vIndex++)
		{
			u64 pieceIndex = (pieChunkIndex*ShaderUniform_NumGenericValues + vIndex);
			if (pieceIndex < numPiePieces)
			{
				RcSetValue(vIndex, (r32)piePiecePercentages[pieceIndex]);
				thisChunkPercentage += piePiecePercentages[pieceIndex];
			}
			else
			{
				RcSetValue(vIndex, 0.0f);
			}
		}
		
		RcSetCircleInnerRadius((r32)chunkBasePercentage);
		RcSetCircleRadius(1.0f);
		
		RcDrawRectangle(rectangle, tintColor);
		
		chunkBasePercentage += thisChunkPercentage;
	}
	RcSetCircleRadius(oldCircleRadius);
	RcSetCircleInnerRadius(oldCircleInnerRadius);
	for (u8 vIndex = 0; vIndex < ShaderUniform_NumGenericValues; vIndex++) { RcSetValue(vIndex, 0.0f); }
	if (oldShader != &pig->resources.shaders->pieChart) { RcBindShader(oldShader); }
}

void RcDrawPieChartTest(rec rectangle)
{
	static u64 randValues[8] = { 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000 };
	static bool randValuesDir[8] = { false, true, false, true, false, true, false, true };
	u64 randValuesTotal = 0;
	for (u64 vIndex = 0; vIndex < ArrayCount(randValues); vIndex++)
	{
		if (RandU32(0, 100) < 2) { randValuesDir[vIndex] = !randValuesDir[vIndex]; }
		if (randValues[vIndex] > 10000 && randValuesDir[vIndex]) { randValuesDir[vIndex] = false; }
		if (randValues[vIndex] < 2 && !randValuesDir[vIndex]) { randValuesDir[vIndex] = true; }
		randValues[vIndex] += (randValuesDir[vIndex] ? 5 : -5);
		randValuesTotal += randValues[vIndex];
	}
	r64 randValuePercentages[8];
	for (u64 vIndex = 0; vIndex < ArrayCount(randValues); vIndex++)
	{
		randValuePercentages[vIndex] = (r32)randValues[vIndex] / (r32)randValuesTotal;
	}
	RcDrawPieChart(ArrayCount(randValuePercentages), &randValuePercentages[0], rectangle + NewVec2(0, 8), Grey11);
	RcDrawPieChart(ArrayCount(randValuePercentages), &randValuePercentages[0], rectangle, White);
}

void RcDrawVectorImgShape(VectorImgShape_t* shape, Color_t color)
{
	UNUSED(color); //TODO: Remove this once we use color
	NotNull(shape);
	switch (shape->type)
	{
		case VectorImgShapeType_Circle:
		{
			//TODO: Handle ellipses!
			if (shape->fill.type == VectorImgFillType_Solid)
			{
				RcDrawCircle(shape->circle.center, shape->circle.radius, shape->fill.color);
			}
			if (shape->stroke.type == VectorImgStrokeType_Solid)
			{
				TempPushMark();
				BezierPath_t circlePath;
				GenerateBezierPathForCircle(TempArena, shape->circle.center, shape->circle.radius, &circlePath);
				RcDrawBezierPathWithShader(&circlePath, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
				TempPopMark();
			}
		} break;
		case VectorImgShapeType_Rectangle:
		{
			r32 cornerRadius = MaxR32(shape->rectangle.roundedRadius.x, shape->rectangle.roundedRadius.y);
			if (shape->fill.type == VectorImgFillType_Solid)
			{
				if (shape->rectangle.rotation == 0)
				{
					rec renderRectangle = NewRecCentered(shape->rectangle.center, shape->rectangle.size);
					if (cornerRadius > 0)
					{
						RcDrawRoundedRectangle(renderRectangle, cornerRadius, shape->fill.color);
					}
					else
					{
						RcDrawRectangle(renderRectangle, shape->fill.color);
					}
				}
				else
				{
					if (cornerRadius > 0)
					{
						RcDrawRoundedObb2(shape->rectangle.value, cornerRadius, shape->fill.color);
					}
					else
					{
						RcDrawObb2(shape->rectangle.value, shape->fill.color);
					}
				}
			}
			if (shape->stroke.type == VectorImgStrokeType_Solid)
			{
				TempPushMark();
				if (shape->rectangle.rotation == 0)
				{
					rec renderRectangle = NewRecCentered(shape->rectangle.center, shape->rectangle.size);
					BezierPath_t rectanglePath;
					GenerateBezierPathForRoundedRectangle(TempArena, renderRectangle, cornerRadius, &rectanglePath);
					RcDrawBezierPathWithShader(&rectanglePath, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
				}
				else
				{
					BezierPath_t boundingBoxPath;
					GenerateBezierPathForRoundedObb2(TempArena, shape->rectangle.value, cornerRadius, &boundingBoxPath);
					RcDrawBezierPathWithShader(&boundingBoxPath, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
				}
				TempPopMark();
			}
		} break;
		case VectorImgShapeType_Path:
		{
			if (shape->fill.type == VectorImgFillType_Solid)
			{
				if (!shape->path.vertBufferUpToDate && !shape->path.vertBufferGenFailed)
				{
					shape->path.vertBufferGenFailed = !GenerateVertBufferForVectorImgPathShape(shape, SVG_TRIANGULATION_CURVE_VERT_COUNT);
					if (shape->path.vertBufferGenFailed)
					{
						PrintLine_W("Failed to triangulate VectorImgShape \"%.*s\"", shape->idStr.length, shape->idStr.pntr);
					}
				}
				if (shape->path.vertBufferUpToDate)
				{
					RcSetWorldMatrix(Mat4Translate3(0, 0, RcGetRealDepth()));
					// RcSetColor1((GetNumSubPathsInBezierPath(&shape->path.value) > 1) ? ColorLerp(MonokaiRed, MonokaiBlue, Oscillate(0, 1, 1000)) : shape->fill.color);
					RcSetColor1(shape->fill.color);
					RcBindTexture1(&rc->dotTexture);
					RcSetSourceRec1(Rec_Default);
					RcBindVertBuffer(&shape->path.vertBuffer);
					RcDrawBuffer(VertBufferPrimitive_Triangles);
				}
			}
			if (shape->stroke.type == VectorImgStrokeType_Solid)
			{
				RcDrawBezierPathWithShader(&shape->path.value, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
			}
		} break;
		default: DebugAssert(false); break;
	}
}
void RcDrawVectorImgPartInRec(VectorImgPart_t* part, rec rectangle, Color_t color, bool drawChildren = true)
{
	UNUSED(drawChildren); //TODO: Remove me once we use drawChildren
	NotNull(part);
	
	v2 scale = Vec2Divide(rectangle.size, part->bounds.size);
	mat4 augmentMatrix = Mat4_Identity;
	Mat4Transform(augmentMatrix, Mat4Translate2(-part->bounds.topLeft));
	Mat4Transform(augmentMatrix, Mat4Scale2(scale));
	Mat4Transform(augmentMatrix, Mat4Translate2(rectangle.topLeft));
	mat4 oldAugmentMatrix = rc->state.augmentMatrix;
	RcSetAugmentMatrix(augmentMatrix);
	
	VectorImgLoop_t loop = CreateVectorImgLoop(part);
	while (VectorImgLoop(&loop))
	{
		NotNull(loop.shape);
		RcDrawVectorImgShape(loop.shape, color);
	}
	
	RcSetAugmentMatrix(oldAugmentMatrix);
}
void RcDrawVectorImgPartInObb(VectorImgPart_t* part, obb2 boundingBox, Color_t color, bool drawChildren = true)
{
	UNUSED(drawChildren); //TODO: Remove me once we use drawChildren
	NotNull(part);
	
	v2 scale = Vec2Divide(boundingBox.size, part->bounds.size);
	mat4 augmentMatrix = Mat4_Identity;
	Mat4Transform(augmentMatrix, Mat4Translate2(-(part->bounds.topLeft + part->bounds.size/2)));
	Mat4Transform(augmentMatrix, Mat4RotateZ(boundingBox.rotation));
	Mat4Transform(augmentMatrix, Mat4Scale2(scale));
	Mat4Transform(augmentMatrix, Mat4Translate2(boundingBox.center));
	mat4 oldAugmentMatrix = rc->state.augmentMatrix;
	RcSetAugmentMatrix(augmentMatrix);
	
	VectorImgLoop_t loop = CreateVectorImgLoop(part);
	while (VectorImgLoop(&loop))
	{
		NotNull(loop.shape);
		RcDrawVectorImgShape(loop.shape, color);
	}
	
	RcSetAugmentMatrix(oldAugmentMatrix);
}
void RcDrawVectorImgPartAt(VectorImgPart_t* part, v2 position, r32 rotation, r32 scale, Color_t color, bool drawChildren = true)
{
	obb2 drawBox = NewObb2D(position, part->bounds.size * scale, rotation);
	drawBox.center -= Vec2Rotate(part->origin * scale, rotation);
	RcDrawVectorImgPartInObb(part, drawBox, color, drawChildren);
}

//TODO: Make a textured version of this!
void RcDrawRightAngleTriangle(rec rectangle, Dir2Ex_t direction, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate2(-Vec2_Half));
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4RotateZ(GetDir2ExAngleR32(direction) - (5 * QuarterPi32)));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x + rectangle.width/2, rectangle.y + rectangle.height/2, RcGetRealDepth()));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->rightAngleTriBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

//NOTE: offset goes down-left when + for descending and down-right when + for ascending
void RcDrawDiagonalStripeInRec(rec rectangle, bool descending, r32 thickness, Color_t color, r32 offset = 0.0f)
{
	v2 lineStart = rectangle.topLeft;
	v2 lineEnd = rectangle.topLeft + rectangle.size;
	if (!descending) { lineStart.y += rectangle.height; lineEnd.y -= rectangle.height; }
	v2 offsetVec = SqrtR32(2) * NewVec2(offset, offset);
	if (descending) { offsetVec = Vec2PerpRight(offsetVec); }
	lineStart += offsetVec;
	lineEnd += offsetVec;
	rec oldViewport = rc->state.viewportRec;
	RcSetViewport(RecOverlap(rectangle, oldViewport));
	RcDrawLine(lineStart, lineEnd, thickness, color);
	RcSetViewport(oldViewport);
}

void RcDrawFoldedRectangle(rec rectangle, r32 foldSize, u8 foldedCorners, Color_t color)
{
	rec centerRec = RecDeflate(rectangle, foldSize, foldSize);
	RcDrawRectangle(centerRec, color);
	for (u8 sideIndex = 0; sideIndex < Dir2_Count; sideIndex++)
	{
		Dir2_t side = Dir2FromIndex(sideIndex);
		rec sideRec = rectangle;
		if (side == Dir2_Left || side == Dir2_Right) { sideRec.width = foldSize; }
		if (side == Dir2_Up   || side == Dir2_Down)  { sideRec.height = foldSize; }
		if (side == Dir2_Right) { sideRec.x += rectangle.width  - foldSize; }
		if (side == Dir2_Down)  { sideRec.y += rectangle.height - foldSize; }
		if (side == Dir2_Up && IsFlagSet(foldedCorners, Dir2Ex_TopLeft))  { sideRec.x += foldSize; sideRec.width -= foldSize; }
		if (side == Dir2_Up && IsFlagSet(foldedCorners, Dir2Ex_TopRight)) { sideRec.width -= foldSize; }
		if (side == Dir2_Down && IsFlagSet(foldedCorners, Dir2Ex_BottomLeft))  { sideRec.x += foldSize; sideRec.width -= foldSize; }
		if (side == Dir2_Down && IsFlagSet(foldedCorners, Dir2Ex_BottomRight)) { sideRec.width -= foldSize; }
		//NOTE: We don't actually need this double cover, right?
		if (side == Dir2_Left || side == Dir2_Right) { sideRec.y += foldSize; sideRec.height -= foldSize*2; }
		// if (side == Dir2_Left && IsFlagSet(foldedCorners, Dir2Ex_TopLeft))    { sideRec.y += foldSize; sideRec.height -= foldSize; }
		// if (side == Dir2_Left && IsFlagSet(foldedCorners, Dir2Ex_BottomLeft)) { sideRec.height -= foldSize; }
		// if (side == Dir2_Right && IsFlagSet(foldedCorners, Dir2Ex_TopRight))    { sideRec.y += foldSize; sideRec.height -= foldSize; }
		// if (side == Dir2_Right && IsFlagSet(foldedCorners, Dir2Ex_BottomRight)) { sideRec.height -= foldSize; }
		RcDrawRectangle(sideRec, color);
	}
	for (u8 cornerIndex = 0; cornerIndex < 4; cornerIndex++)
	{
		Dir2Ex_t corner = Dir2ExFromIndex(4 + cornerIndex);
		if (IsFlagSet(foldedCorners, corner))
		{
			rec cornerRec = NewRec(rectangle.topLeft, foldSize, foldSize);
			if (corner == Dir2Ex_TopRight || corner == Dir2Ex_BottomRight) { cornerRec.x += rectangle.width - foldSize; }
			if (corner == Dir2Ex_BottomLeft || corner == Dir2Ex_BottomRight) { cornerRec.y += rectangle.height - foldSize; }
			// RcDrawRectangle(cornerRec, ColorComplementary(color));
			RcDrawRightAngleTriangle(cornerRec, Dir2ExOpposite(corner), color);
		}
	}
}

void RcDrawFoldedRectangleOutline(rec rectangle, r32 foldSize, u8 foldedCorners, r32 thickness, Color_t color, bool outsideRectangle = false)
{
	for (u8 sideIndex = 0; sideIndex < Dir2_Count; sideIndex++)
	{
		Dir2_t side = Dir2FromIndex(sideIndex);
		rec sideRec = rectangle;
		if (side == Dir2_Left || side == Dir2_Right) { sideRec.width = thickness; }
		if (side == Dir2_Up   || side == Dir2_Down)  { sideRec.height = thickness; }
		if (side == Dir2_Right) { sideRec.x += rectangle.width  - thickness; }
		if (side == Dir2_Down)  { sideRec.y += rectangle.height - thickness; }
		if (outsideRectangle && side == Dir2_Left)  { sideRec.x -= thickness; }
		if (outsideRectangle && side == Dir2_Right) { sideRec.x += thickness; }
		if (outsideRectangle && side == Dir2_Up)    { sideRec.y -= thickness; }
		if (outsideRectangle && side == Dir2_Down)  { sideRec.y += thickness; }
		if (side == Dir2_Left)
		{
			if (IsFlagSet(foldedCorners, Dir2Ex_BottomLeft)) { sideRec.height -= foldSize; }
			else if (!outsideRectangle) { sideRec.height -= thickness; }
			if (IsFlagSet(foldedCorners, Dir2Ex_TopLeft)) { sideRec.y += foldSize; sideRec.height -= foldSize; }
			else if (!outsideRectangle) { sideRec.y += thickness; sideRec.height -= thickness; }
		}
		if (side == Dir2_Right)
		{
			if (IsFlagSet(foldedCorners, Dir2Ex_BottomRight)) { sideRec.height -= foldSize; }
			else if (!outsideRectangle) { sideRec.height -= thickness; }
			if (IsFlagSet(foldedCorners, Dir2Ex_TopRight)) { sideRec.y += foldSize; sideRec.height -= foldSize; }
			else if (!outsideRectangle) { sideRec.y += thickness; sideRec.height -= thickness; }
		}
		if (side == Dir2_Up)
		{
			if (IsFlagSet(foldedCorners, Dir2Ex_TopRight)) { sideRec.width -= foldSize; }
			else if (outsideRectangle) { sideRec.width += thickness; }
			if (IsFlagSet(foldedCorners, Dir2Ex_TopLeft)) { sideRec.x += foldSize; sideRec.width -= foldSize; }
			else if (outsideRectangle) { sideRec.x -= thickness; sideRec.width += thickness; }
		}
		if (side == Dir2_Down)
		{
			if (IsFlagSet(foldedCorners, Dir2Ex_BottomRight)) { sideRec.width -= foldSize; }
			else if (outsideRectangle) { sideRec.width += thickness; }
			if (IsFlagSet(foldedCorners, Dir2Ex_BottomLeft)) { sideRec.x += foldSize; sideRec.width -= foldSize; }
			else if (outsideRectangle) { sideRec.x -= thickness; sideRec.width += thickness; }
		}
		RcDrawRectangle(sideRec, color);
	}
	for (u8 cornerIndex = 0; cornerIndex < 4; cornerIndex++)
	{
		Dir2Ex_t corner = Dir2ExFromIndex(4 + cornerIndex);
		if (IsFlagSet(foldedCorners, corner))
		{
			rec cornerRec = NewRec(rectangle.topLeft, foldSize, foldSize);
			if (corner == Dir2Ex_TopRight || corner == Dir2Ex_BottomRight) { cornerRec.x += rectangle.width - foldSize; }
			if (corner == Dir2Ex_BottomLeft || corner == Dir2Ex_BottomRight) { cornerRec.y += rectangle.height - foldSize; }
			r32 diagThickness = SinR32(QuarterPi32) * thickness;
			bool descending = (corner == Dir2Ex_BottomLeft || corner == Dir2Ex_TopRight);
			r32 diagonalOffset = (diagThickness/4) * ((corner == Dir2Ex_TopLeft || corner == Dir2Ex_TopRight) ? 1.0f : -1.0f);
			if (outsideRectangle)
			{
				// diagonalOffset = -diagonalOffset;
				cornerRec.width += thickness;
				cornerRec.height += thickness;
				if (corner == Dir2Ex_TopLeft || corner == Dir2Ex_BottomLeft) { cornerRec.x -= thickness; }
				if (corner == Dir2Ex_TopLeft || corner == Dir2Ex_TopRight) { cornerRec.y -= thickness; }
			}
			RcDrawDiagonalStripeInRec(cornerRec, descending, diagThickness, color, diagonalOffset);
		}
	}
}

#if CONVEX_POLYGON2D_SHADER_SUPPORT
//Polygon should be in clockwise order
void RcDrawConvexPolygonWithShader(Color_t color, u64 numVertices, const v2* vertices, bool bindShader = true)
{
	NotNull(vertices);
	if (numVertices < 3) { return; }
	Assert(numVertices <= ShaderUniform_NumPolygonPlanes);
	
	Shader_t* oldShader = rc->state.boundShader;
	if (bindShader) { RcBindShader(&pig->resources.shaders->convexPolygon2D); }
	
	rec polygonBounds = Rec_Zero;
	v4 planes[ShaderUniform_NumPolygonPlanes];
	for (u64 vIndex = 0; vIndex < numVertices; vIndex++)
	{
		v2 vert = vertices[vIndex];
		v2 previousVert = vertices[(vIndex+1) % numVertices];
		v2 planeDirVec = Vec2PerpRight(Vec2Normalize(vert - previousVert));
		planes[vIndex].x = vertices[vIndex].x;
		planes[vIndex].y = vertices[vIndex].y;
		planes[vIndex].z = planeDirVec.x;
		planes[vIndex].w = planeDirVec.y;
		if (vIndex == 0) { polygonBounds = NewRec(vert, 0, 0); }
		else { polygonBounds = RecExpandToVec2(polygonBounds, vert); }
	}
	polygonBounds = RecSquarify(polygonBounds);
	v2 boundsCenter = polygonBounds.topLeft + polygonBounds.size/2;
	
	RcSetCount((i32)numVertices);
	for (u64 vIndex = 0; vIndex < numVertices; vIndex++)
	{
		v2 planeDirVec = Vec2Normalize(NewVec2(planes[vIndex].z, planes[vIndex].w));
		r32 planeDir = AngleFromVec2(planeDirVec);
		v2 planeVertVec = NewVec2(planes[vIndex].x, planes[vIndex].y) - boundsCenter;
		r32 planeDistance = Vec2Dot(planeVertVec, planeDirVec);
		planeDistance = planeDistance / (polygonBounds.width/2);
		RcSetPolygonPlane(vIndex, planeDir, planeDistance);
	}
	RcDrawRectangle(polygonBounds, color);
	
	if (bindShader) { RcBindShader(oldShader); }
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2)
{
	v2 localVertsArray[3];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	RcDrawConvexPolygonWithShader(color, 3, &localVertsArray[0]);
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2, v2 vert3)
{
	v2 localVertsArray[4];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	localVertsArray[3] = vert3;
	RcDrawConvexPolygonWithShader(color, 4, &localVertsArray[0]);
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2, v2 vert3, v2 vert4)
{
	v2 localVertsArray[5];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	localVertsArray[3] = vert3;
	localVertsArray[4] = vert4;
	RcDrawConvexPolygonWithShader(color, 5, &localVertsArray[0]);
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2, v2 vert3, v2 vert4, v2 vert5)
{
	v2 localVertsArray[6];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	localVertsArray[3] = vert3;
	localVertsArray[4] = vert4;
	localVertsArray[5] = vert5;
	RcDrawConvexPolygonWithShader(color, 6, &localVertsArray[0]);
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2, v2 vert3, v2 vert4, v2 vert5, v2 vert6)
{
	v2 localVertsArray[7];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	localVertsArray[3] = vert3;
	localVertsArray[4] = vert4;
	localVertsArray[5] = vert5;
	localVertsArray[6] = vert6;
	RcDrawConvexPolygonWithShader(color, 7, &localVertsArray[0]);
}
void RcDrawConvexPolygonWithShader(Color_t color, v2 vert0, v2 vert1, v2 vert2, v2 vert3, v2 vert4, v2 vert5, v2 vert6, v2 vert7)
{
	v2 localVertsArray[8];
	localVertsArray[0] = vert0;
	localVertsArray[1] = vert1;
	localVertsArray[2] = vert2;
	localVertsArray[3] = vert3;
	localVertsArray[4] = vert4;
	localVertsArray[5] = vert5;
	localVertsArray[6] = vert6;
	localVertsArray[7] = vert7;
	RcDrawConvexPolygonWithShader(color, 8, &localVertsArray[0]);
}
#endif

void RcDrawQuarterCircleFoldedRec(rec rectangle, r32 circleRadius, r32 circleMargin, r32 borderThickness, r32 circleBorderThickness, Color_t fillColor, Color_t circleFillColor, Color_t borderColor, Color_t circleBorderColor)
{
	//NOTE: This takes 31 draw calls to perform. May not be the cheapest, all things considered
	rec oldViewport = rc->state.viewportRec;
	Shader_t* oldShader = rc->state.boundShader;
	r32 foldSize = circleRadius + circleMargin + borderThickness;
	
	if (fillColor.a > 0)
	{
		rec innerRec1 = NewRec(rectangle.x + foldSize, rectangle.y + borderThickness, rectangle.width - 2*foldSize, rectangle.height - 2*borderThickness);
		rec innerRec2 = NewRec(rectangle.x + borderThickness, rectangle.y + foldSize, foldSize - borderThickness, rectangle.height - 2*foldSize);
		rec innerRec3 = NewRec(rectangle.x + rectangle.width - foldSize, rectangle.y + foldSize, foldSize - borderThickness, rectangle.height - 2*foldSize);
		RcDrawRectangle(innerRec1, fillColor);
		RcDrawRectangle(innerRec2, fillColor);
		RcDrawRectangle(innerRec3, fillColor);
	}
	
	if (borderThickness > 0 && borderColor.a > 0)
	{
		rec leftBorderRec = NewRec(rectangle.x, rectangle.y + foldSize, borderThickness, rectangle.height - 2*foldSize);
		rec topBorderRec = NewRec(rectangle.x + foldSize, rectangle.y, rectangle.width - 2*foldSize, borderThickness);
		rec rightBorderRec = NewRec(rectangle.x + rectangle.width - borderThickness, rectangle.y + foldSize, borderThickness, rectangle.height - 2*foldSize);
		rec bottomBorderRec = NewRec(rectangle.x + foldSize, rectangle.y + rectangle.height - borderThickness, rectangle.width - 2*foldSize, borderThickness);
		RcDrawRectangle(leftBorderRec, borderColor);
		RcDrawRectangle(topBorderRec, borderColor);
		RcDrawRectangle(rightBorderRec, borderColor);
		RcDrawRectangle(bottomBorderRec, borderColor);
	}
	
	if (circleBorderThickness > 0 && circleBorderColor.a > 0)
	{
		rec quarterCircleTopLeftRec1 = NewRec(rectangle.x, rectangle.y, circleRadius - circleBorderThickness, circleBorderThickness);
		rec quarterCircleTopLeftRec2 = NewRec(rectangle.x, rectangle.y, circleBorderThickness, circleRadius - circleBorderThickness);
		rec quarterCircleTopRightRec1 = NewRec(rectangle.x + rectangle.width - (circleRadius - circleBorderThickness), rectangle.y, circleRadius - circleBorderThickness, circleBorderThickness);
		rec quarterCircleTopRightRec2 = NewRec(rectangle.x + rectangle.width - circleBorderThickness, rectangle.y, circleBorderThickness, circleRadius - circleBorderThickness);
		rec quarterCircleBottomLeftRec1 = NewRec(rectangle.x, rectangle.y + rectangle.height - circleBorderThickness, circleRadius - circleBorderThickness, circleBorderThickness);
		rec quarterCircleBottomLeftRec2 = NewRec(rectangle.x, rectangle.y + rectangle.height - (circleRadius - circleBorderThickness), circleBorderThickness, circleRadius - circleBorderThickness);
		rec quarterCircleBottomRightRec1 = NewRec(rectangle.x + rectangle.width - (circleRadius - circleBorderThickness), rectangle.y + rectangle.height - circleBorderThickness, circleRadius - circleBorderThickness, circleBorderThickness);
		rec quarterCircleBottomRightRec2 = NewRec(rectangle.x + rectangle.width - circleBorderThickness, rectangle.y + rectangle.height - (circleRadius - circleBorderThickness), circleBorderThickness, circleRadius - circleBorderThickness);
		RcDrawRectangle(quarterCircleTopLeftRec1, circleBorderColor);
		RcDrawRectangle(quarterCircleTopLeftRec2, circleBorderColor);
		RcDrawRectangle(quarterCircleTopRightRec1, circleBorderColor);
		RcDrawRectangle(quarterCircleTopRightRec2, circleBorderColor);
		RcDrawRectangle(quarterCircleBottomLeftRec1, circleBorderColor);
		RcDrawRectangle(quarterCircleBottomLeftRec2, circleBorderColor);
		RcDrawRectangle(quarterCircleBottomRightRec1, circleBorderColor);
		RcDrawRectangle(quarterCircleBottomRightRec2, circleBorderColor);
	}
	
	RcBindShader(&pig->resources.shaders->main2D);
	RcSetViewport(RecDeflate(rectangle, circleBorderThickness, circleBorderThickness));
	if (circleFillColor.a > 0)
	{
		RcDrawCircle(NewVec2(rectangle.x + circleBorderThickness, rectangle.y + circleBorderThickness),                                      circleRadius - 2*circleBorderThickness, circleFillColor, false);
		RcDrawCircle(NewVec2(rectangle.x + rectangle.width - circleBorderThickness, rectangle.y + circleBorderThickness),                    circleRadius - 2*circleBorderThickness, circleFillColor, false);
		RcDrawCircle(NewVec2(rectangle.x + circleBorderThickness, rectangle.y + rectangle.height - circleBorderThickness),                   circleRadius - 2*circleBorderThickness, circleFillColor, false);
		RcDrawCircle(NewVec2(rectangle.x + rectangle.width - circleBorderThickness, rectangle.y + rectangle.height - circleBorderThickness), circleRadius - 2*circleBorderThickness, circleFillColor, false);
	}
	RcSetViewport(rectangle);
	if (fillColor.a > 0)
	{
		RcDrawInverseCircle(NewVec2(rectangle.x, rectangle.y),                                      foldSize, fillColor, false);
		RcDrawInverseCircle(NewVec2(rectangle.x + rectangle.width, rectangle.y),                    foldSize, fillColor, false);
		RcDrawInverseCircle(NewVec2(rectangle.x, rectangle.y + rectangle.height),                   foldSize, fillColor, false);
		RcDrawInverseCircle(NewVec2(rectangle.x + rectangle.width, rectangle.y + rectangle.height), foldSize, fillColor, false);
	}
	if (circleBorderThickness > 0 && circleBorderColor.a > 0)
	{
		RcDrawDonut(NewVec2(rectangle.x, rectangle.y),                                      circleRadius - circleBorderThickness*2, circleRadius, circleBorderColor, false);
		RcDrawDonut(NewVec2(rectangle.x + rectangle.width, rectangle.y),                    circleRadius - circleBorderThickness*2, circleRadius, circleBorderColor, false);
		RcDrawDonut(NewVec2(rectangle.x, rectangle.y + rectangle.height),                   circleRadius - circleBorderThickness*2, circleRadius, circleBorderColor, false);
		RcDrawDonut(NewVec2(rectangle.x + rectangle.width, rectangle.y + rectangle.height), circleRadius - circleBorderThickness*2, circleRadius, circleBorderColor, false);
	}
	if (borderThickness > 0 && borderColor.a > 0)
	{
		RcDrawDonut(NewVec2(rectangle.x, rectangle.y),                                      circleRadius + circleMargin, circleRadius + circleMargin + borderThickness + 0.5f, borderColor, false);
		RcDrawDonut(NewVec2(rectangle.x + rectangle.width, rectangle.y),                    circleRadius + circleMargin, circleRadius + circleMargin + borderThickness + 0.5f, borderColor, false);
		RcDrawDonut(NewVec2(rectangle.x, rectangle.y + rectangle.height),                   circleRadius + circleMargin, circleRadius + circleMargin + borderThickness + 0.5f, borderColor, false);
		RcDrawDonut(NewVec2(rectangle.x + rectangle.width, rectangle.y + rectangle.height), circleRadius + circleMargin, circleRadius + circleMargin + borderThickness + 0.5f, borderColor, false);
	}
	RcBindShader(oldShader);
	
	RcSetViewport(oldViewport);
}

void RcDrawTiledUiRec(v2i baseFrame, rec drawRec, Color_t color, r32 scale = 1.0f)
{
	NotNull(rc->state.boundSpriteSheet);
	
	v2 tileSize = ToVec2(rc->state.boundSpriteSheet->frameSize) * scale;
	if (tileSize.width <= 0) { tileSize.width = 1; }
	if (tileSize.height <= 0) { tileSize.height = 1; }
	v2 centerSize = NewVec2(drawRec.width - tileSize.width*2, drawRec.height - tileSize.height*2);
	
	for (r32 yOffset = 0; yOffset < centerSize.height; yOffset += tileSize.height)
	{
		for (r32 xOffset = 0; xOffset < centerSize.width; xOffset += tileSize.width)
		{
			rec tileRec = NewRec(drawRec.x + tileSize.width + xOffset, drawRec.y + tileSize.height + yOffset, tileSize.width, tileSize.height);
			RcDrawSheetFrame(baseFrame + NewVec2i(1, 1), tileRec, color);
		}
	}
	
	for (r32 xOffset = 0; xOffset < centerSize.width; xOffset += tileSize.width)
	{
		rec topRec = NewRec(drawRec.x + tileSize.width + xOffset, drawRec.y, tileSize.width, tileSize.height);
		rec bottomRec = NewRec(drawRec.x + tileSize.width + xOffset, drawRec.y + (drawRec.height - tileSize.height), tileSize.width, tileSize.height);
		RcDrawSheetFrame(baseFrame + NewVec2i(1, 0), topRec, color);
		RcDrawSheetFrame(baseFrame + NewVec2i(1, 2), bottomRec, color);
	}
	
	for (r32 yOffset = 0; yOffset < centerSize.height; yOffset += tileSize.height)
	{
		rec leftRec = NewRec(drawRec.x, drawRec.y + tileSize.height + yOffset, tileSize.width, tileSize.height);
		rec rightRec = NewRec(drawRec.x + (drawRec.width - tileSize.width), drawRec.y + tileSize.height + yOffset, tileSize.width, tileSize.height);
		RcDrawSheetFrame(baseFrame + NewVec2i(0, 1), leftRec, color);
		RcDrawSheetFrame(baseFrame + NewVec2i(2, 1), rightRec, color);
	}
	
	rec topLeftRec = NewRec(drawRec.x, drawRec.y, tileSize.width, tileSize.height);
	rec topRightRec = NewRec(drawRec.x + (drawRec.width - tileSize.width), drawRec.y, tileSize.width, tileSize.height);
	rec bottomLeftRec = NewRec(drawRec.x, drawRec.y + (drawRec.height - tileSize.height), tileSize.width, tileSize.height);
	rec bottomRightRec = NewRec(drawRec.x + (drawRec.width - tileSize.width), drawRec.y + (drawRec.height - tileSize.height), tileSize.width, tileSize.height);
	RcDrawSheetFrame(baseFrame + NewVec2i(0, 0), topLeftRec,     color);
	RcDrawSheetFrame(baseFrame + NewVec2i(2, 0), topRightRec,    color);
	RcDrawSheetFrame(baseFrame + NewVec2i(0, 2), bottomLeftRec,  color);
	RcDrawSheetFrame(baseFrame + NewVec2i(2, 2), bottomRightRec, color);
}

// +--------------------------------------------------------------+
// |                    Post Processing Chain                     |
// +--------------------------------------------------------------+
FrameBuffer_t* RenderToPostProcessingChainInput(PostProcessingChain_t* chain, u64 inputIndex)
{
	NotNull(chain);
	NotNull(chain->allocArena);
	FrameBuffer_t* targetBuffer = PostProcessingChainAddInputBuffer(chain, inputIndex);
	Assert(targetBuffer->isValid);
	RcBindFrameBuffer(targetBuffer);
	return targetBuffer;
}

// Call this once for first pass, up to 2 input buffers can be used to feed the shader (bind shader first)
void PostProcessingChainFirstPassPntrs(PostProcessingChain_t* chain, Color_t clearColor, Color_t color, FrameBuffer_t* firstInputBuffer, FrameBuffer_t* secondInputBuffer = nullptr)
{
	NotNull(chain);
	NotNull(firstInputBuffer);
	chain->passIndex = 0;
	RcBindFrameBuffer(nullptr);
	PrepareFrameBufferTexture(firstInputBuffer);
	if (secondInputBuffer != nullptr) { PrepareFrameBufferTexture(secondInputBuffer); }
	RcBindTexture1(&firstInputBuffer->outTexture);
	if (secondInputBuffer != nullptr)
	{
		RcBindTexture2(&secondInputBuffer->outTexture);
		RcSetSourceRec2(NewRec(Vec2_Zero, secondInputBuffer->outTexture.size));
	}
	RcBindFrameBuffer(&chain->mainBuffer);
	RcSetViewport(NewRec(Vec2_Zero, ToVec2(chain->mainBuffer.size)));
	RcClearColor(clearColor);
	RcClearDepth(1.0f);
	RcDrawTexturedRectangle(NewRec(Vec2_Zero, ToVec2(chain->size)), color);
	chain->passIndex++;
}
void PostProcessingChainFirstPass(PostProcessingChain_t* chain, Color_t clearColor, Color_t color, u64 inputIndex1, u64 inputIndex2 = UINT64_MAX)
{
	NotNull(chain);
	PostProcessingChainFirstPassPntrs(
		chain,
		clearColor,
		color,
		GetPostProcessingChainInputBuffer(chain, inputIndex1),
		(inputIndex2 < chain->inputBuffers.length ? GetPostProcessingChainInputBuffer(chain, inputIndex2) : nullptr)
	);
}

// Call this for subsequent passes. Texture1 is filled with result from previous pass and second input buffer is optional (bind shader first)
void PostProcessingChainNextPassPntr(PostProcessingChain_t* chain, Color_t clearColor, Color_t color, FrameBuffer_t* secondInputBuffer = nullptr)
{
	NotNull(chain);
	FrameBuffer_t* targetBuffer = ((chain->passIndex%2) == 0 ? &chain->mainBuffer : &chain->secondaryBuffer);
	FrameBuffer_t* sourceBuffer = ((chain->passIndex%2) == 0 ? &chain->secondaryBuffer : &chain->mainBuffer);
	RcBindFrameBuffer(nullptr);
	PrepareFrameBufferTexture(sourceBuffer);
	RcBindFrameBuffer(targetBuffer);
	RcSetViewport(NewRec(Vec2_Zero, ToVec2(targetBuffer->size)));
	RcClearColor(clearColor);
	RcClearDepth(1.0f);
	if (secondInputBuffer != nullptr) { PrepareFrameBufferTexture(secondInputBuffer); }
	RcBindTexture1(&sourceBuffer->outTexture);
	if (secondInputBuffer != nullptr)
	{
		RcBindTexture2(&secondInputBuffer->outTexture);
		RcSetSourceRec2(NewRec(Vec2_Zero, secondInputBuffer->outTexture.size));
	}
	RcDrawTexturedRectangle(NewRec(Vec2_Zero, ToVec2(chain->size)), color);
	chain->passIndex++;
}
void PostProcessingChainNextPass(PostProcessingChain_t* chain, Color_t clearColor, Color_t color, u64 inputIndex2 = UINT64_MAX)
{
	NotNull(chain);
	PostProcessingChainNextPassPntr(
		chain,
		clearColor,
		color,
		(inputIndex2 < chain->inputBuffers.length ? GetPostProcessingChainInputBuffer(chain, inputIndex2) : nullptr)
	);
}

// call this once after, bind the actual target framebuffer+shader, then draw to wherever you want using Texture1 (or look into the chain yourself and pull out and bind as needed)
void PostProcessingChainEnd(PostProcessingChain_t* chain, FrameBuffer_t* outputBuffer, Shader_t* outputShader, bool bindTexture = true)
{
	NotNull(chain);
	FrameBuffer_t* sourceBuffer = ((chain->passIndex%2) == 0 ? &chain->secondaryBuffer : &chain->mainBuffer);
	RcBindFrameBuffer(nullptr);
	PrepareFrameBufferTexture(sourceBuffer);
	RcBindFrameBuffer(outputBuffer);
	RcBindShader(outputShader);
	RcSetViewport(NewRec(Vec2_Zero, (outputBuffer != nullptr ? ToVec2(outputBuffer->size) : ScreenSize)));
	if (bindTexture)
	{
		RcBindTexture1(&sourceBuffer->outTexture);
	}
}

void BindPostProcessingChainInputTexture(PostProcessingChain_t* chain, u64 inputIndex, bool bindToTexture2 = false)
{
	NotNull(chain);
	Assert(inputIndex < chain->inputBuffers.length);
	FrameBuffer_t* inputBuffer = BktArrayGet(&chain->inputBuffers, FrameBuffer_t, inputIndex);
	Assert(inputBuffer->isValid);
	PrepareFrameBufferTexture(inputBuffer);
	if (bindToTexture2) { RcBindTexture2(&inputBuffer->outTexture); }
	else { RcBindTexture1(&inputBuffer->outTexture); }
}

void DoubleBindPostProcessingChainInputTexture(PostProcessingChain_t* chain, u64 inputIndex1, u64 inputIndex2)
{
	NotNull(chain);
	Assert(inputIndex1 < chain->inputBuffers.length);
	Assert(inputIndex2 < chain->inputBuffers.length);
	FrameBuffer_t* inputBuffer1 = BktArrayGet(&chain->inputBuffers, FrameBuffer_t, inputIndex1);
	Assert(inputBuffer1->isValid);
	FrameBuffer_t* inputBuffer2 = BktArrayGet(&chain->inputBuffers, FrameBuffer_t, inputIndex2);
	Assert(inputBuffer2->isValid);
	PrepareFrameBufferTexture(inputBuffer1);
	PrepareFrameBufferTexture(inputBuffer2);
	RcBindTexture1(&inputBuffer1->outTexture);
	RcBindTexture2(&inputBuffer2->outTexture);
}
