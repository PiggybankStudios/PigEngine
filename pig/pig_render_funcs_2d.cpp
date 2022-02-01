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
	vertices[0].position = NewVec3(positions[0].x, positions[0].y, rc->state.depth); vertices[0].color = colorVec; vertices[0].texCoord = NewVec2(0, 0);
	vertices[1].position = NewVec3(positions[1].x, positions[1].y, rc->state.depth); vertices[1].color = colorVec; vertices[1].texCoord = NewVec2(1, 0);
	vertices[2].position = NewVec3(positions[2].x, positions[2].y, rc->state.depth); vertices[2].color = colorVec; vertices[2].texCoord = NewVec2(0, 1);
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

void RcDrawRectangle(rec rectangle, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale2(rectangle.size));
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, rc->state.depth));
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
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, rc->state.depth));
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
	Mat4Transform(worldMatrix, Mat4Translate3(rectangle.x, rectangle.y, rc->state.depth));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

//TODO: Add options for rotating and flipping x/y?
void RcDrawSheetFrame(v2i frame, rec rectangle, Color_t color)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	rec sourceRec = NewRec(
		ToVec2(Vec2iMultiply(frame, sheet->frameSize + sheet->padding) + sheet->padding),
		ToVec2(sheet->frameSize)
	);
	RcBindTexture1(&sheet->texture);
	RcDrawTexturedPartRectangle(rectangle, color, sourceRec);
}
void RcDrawSheetFrame(MyStr_t frameName, rec rectangle, Color_t color)
{
	NotNull(rc->state.boundSpriteSheet);
	SpriteSheet_t* sheet = rc->state.boundSpriteSheet;
	v2i frameGridPos = GetSpriteSheetFrame(sheet, frameName);
	rec sourceRec = NewRec(
		ToVec2(Vec2iMultiply(frameGridPos, sheet->frameSize + sheet->padding) + sheet->padding),
		ToVec2(sheet->frameSize)
	);
	RcBindTexture1(&sheet->texture);
	RcDrawTexturedPartRectangle(rectangle, color, sourceRec);
}

void RcDrawGradientRectangle(rec rectangle, bool horizontal, Color_t colorLeftOrTop, Color_t colorRightOrBottom, bool bindShader = true)
{
	Shader_t* oldShader = rc->state.boundShader;
	Color_t oldColor2 = rc->state.color2;
	if (bindShader) { RcBindShader(&pig->resources.gradientShader2D); }
	RcBindTexture1(horizontal ? &rc->gradientHorizontal : &rc->gradientVertical);
	RcSetColor2(colorRightOrBottom);
	RcDrawTexturedRectangle(rectangle, colorLeftOrTop);
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawRoundedRectangle(rec rectangle, r32 cornerRadius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.roundedCornersShader); }
	RcSetShiftVec(rectangle.size);
	RcSetCircleRadius(cornerRadius);
	
	RcDrawRectangle(rectangle, color);
	
	RcSetShiftVec(oldShiftVec);
	RcSetCircleRadius(oldCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawCircle(v2 center, r32 radius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	r32 oldInnerCircleRadius = rc->state.circleInnerRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.mainShader2D); }
	RcSetCircleRadius(1.0f);
	RcSetCircleInnerRadius(0.0f);
	
	RcDrawRectangle(NewRecCentered(center, Vec2Fill(radius*2)), color);
	
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
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.x, boundingBox.y, rc->state.depth));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->squareBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
//TODO: Add textured version of this function
void RcDrawRoundedObb2(obb2 boundingBox, r32 cornerRadius, Color_t color, bool bindShader = true)
{
	NotNull(rc);
	
	Shader_t* oldShader = rc->state.boundShader;
	v2 oldShiftVec = rc->state.shiftVec;
	r32 oldCircleRadius = rc->state.circleRadius;
	
	if (bindShader) { RcBindShader(&pig->resources.roundedCornersShader); }
	RcSetShiftVec(boundingBox.size);
	RcSetCircleRadius(cornerRadius);
	
	RcDrawObb2(boundingBox, color);
	
	RcSetShiftVec(oldShiftVec);
	RcSetCircleRadius(oldCircleRadius);
	if (bindShader) { RcBindShader(oldShader); }
}

void RcDrawEquilTriangleFrom(v2 base, r32 direction, r32 height, Color_t color)
{
	r32 triangleHeight = SqrtR32(3)/2;
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4RotateZ(direction));
	Mat4Transform(worldMatrix, Mat4Scale2(height/triangleHeight, height/triangleHeight));
	Mat4Transform(worldMatrix, Mat4Translate3(base.x, base.y, rc->state.depth));
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
	if (bindShader) { RcBindShader(&pig->resources.bezier3Shader); }
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
	if (bindShader) { RcBindShader(&pig->resources.bezier4Shader); }
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
	if (bindShader) { RcBindShader(&pig->resources.ellipseArcShader); }
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
	if (bindShader) { RcBindShader(&pig->resources.bezier4Shader); }
	
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
				if (rc->state.boundShader != &pig->resources.bezier3Shader) { RcBindShader(&pig->resources.bezier3Shader); }
				RcDrawBezier3WithShader(startPos, midpoint, endPos, thickness, color, false);
			} break;
			case BezierPathPartType_Curve3:
			{
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 control = offset + Vec2Multiply(part->control1, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				if (rc->state.boundShader != &pig->resources.bezier3Shader) { RcBindShader(&pig->resources.bezier3Shader); }
				RcDrawBezier3WithShader(startPos, control, endPos, thickness, color, false);
			} break;
			case BezierPathPartType_Curve4:
			{
				v2 startPos = offset + Vec2Multiply(currentPos, scale);
				v2 control1 = offset + Vec2Multiply(part->control1, scale);
				v2 control2 = offset + Vec2Multiply(part->control2, scale);
				v2 endPos = offset + Vec2Multiply(part->endPos, scale);
				if (rc->state.boundShader != &pig->resources.bezier4Shader) { RcBindShader(&pig->resources.bezier4Shader); }
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
					if (rc->state.boundShader != &pig->resources.ellipseArcShader) { RcBindShader(&pig->resources.ellipseArcShader); }
					RcDrawEllipseArcWithShader(arcCenter, radius, part->axisAngle, arcAngleStart, arcAngleDelta, thickness, color, false);
				}
				else
				{
					//draw a line
					v2 midpoint = (currentPos + part->endPos) / 2;
					if (rc->state.boundShader != &pig->resources.bezier3Shader) { RcBindShader(&pig->resources.bezier3Shader); }
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
	if (rc->state.boundShader != &pig->resources.pieChartShader) { RcBindShader(&pig->resources.pieChartShader); }
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
	if (oldShader != &pig->resources.pieChartShader) { RcBindShader(oldShader); }
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
					RcSetWorldMatrix(Mat4Translate3(0, 0, rc->state.depth));
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
