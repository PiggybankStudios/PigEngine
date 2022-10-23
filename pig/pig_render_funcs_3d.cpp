/*
File:   pig_render_funcs_3d.cpp
Author: Taylor Robbins
Date:   10\31\2021
Description: 
	** Holds a bunch of functions that help us render simple primitives and execute common render paths
*/

// void RcDrawPlane(v3 normal, r32 offset, v2 size, Color_t color)
// {
// }

void RcDrawTriangle(v3* positions, Color_t color)
{
	NotNull(positions);
	RcSetWorldMatrix(Mat4_Identity);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	v3 triNormal = Vec3Normalize(Vec3Cross(positions[2] - positions[0], positions[1] - positions[0]));
	v4 colorVec = ToVec4(color);
	Vertex3D_t vertices[3];
	vertices[0].position = positions[0]; vertices[0].color = colorVec; vertices[0].texCoord = NewVec2(0, 0); vertices[0].normal = triNormal;
	vertices[1].position = positions[1]; vertices[1].color = colorVec; vertices[1].texCoord = NewVec2(1, 0); vertices[1].normal = triNormal;
	vertices[2].position = positions[2]; vertices[2].color = colorVec; vertices[2].texCoord = NewVec2(0, 1); vertices[2].normal = triNormal;
	ChangeVertBufferVertices3D(&rc->scratchBuffer3D, 0, 3, &vertices[0]);
	RcBindVertBuffer(&rc->scratchBuffer3D);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0, 3);
}
void RcDrawTriangle(v3 p1, v3 p2, v3 p3, Color_t color)
{
	v3 positions[3];
	positions[0] = p1;
	positions[1] = p2;
	positions[2] = p3;
	RcDrawTriangle(&positions[0], color);
}

void RcDrawTexturedTriangle(v3* positions, v2* texCoords, Color_t color)
{
	NotNull(positions);
	NotNull(texCoords);
	NotNull(rc->state.boundTexture1);
	RcSetWorldMatrix(Mat4_Identity);
	RcSetColor1(color);
	v3 triNormal = Vec3Normalize(Vec3Cross(positions[2] - positions[0], positions[1] - positions[0]));
	v4 colorVec = ToVec4(color);
	Vertex3D_t vertices[3];
	vertices[0].position = positions[0]; vertices[0].color = colorVec; vertices[0].texCoord = texCoords[0]; vertices[0].normal = triNormal;
	vertices[1].position = positions[1]; vertices[1].color = colorVec; vertices[1].texCoord = texCoords[1]; vertices[1].normal = triNormal;
	vertices[2].position = positions[2]; vertices[2].color = colorVec; vertices[2].texCoord = texCoords[2]; vertices[2].normal = triNormal;
	ChangeVertBufferVertices3D(&rc->scratchBuffer3D, 0, 3, &vertices[0]);
	RcBindVertBuffer(&rc->scratchBuffer3D);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0, 3);
}
void RcDrawTexturedTriangle(v3 p1, v3 p2, v3 p3, v2 tc1, v2 tc2, v2 tc3, Color_t color)
{
	v3 positions[3];
	positions[0] = p1;
	positions[1] = p2;
	positions[2] = p3;
	v2 texCoords[3];
	texCoords[0] = tc1;
	texCoords[1] = tc2;
	texCoords[2] = tc3;
	RcDrawTexturedTriangle(&positions[0], &texCoords[0], color);
}

void RcDrawSphere(v3 center, r32 radius, Color_t color, u64 sphereQuality = SphereQuality_Default)
{
	Assert(sphereQuality < SphereQuality_NumQualities);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(radius, radius, radius));
	Mat4Transform(worldMatrix, Mat4Translate3(center));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcBindVertBuffer(&rc->sphereBuffers[sphereQuality]);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedSphere(v3 center, r32 radius, Color_t color, u64 sphereQuality = SphereQuality_Default)
{
	Assert(sphereQuality < SphereQuality_NumQualities);
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(radius, radius, radius));
	Mat4Transform(worldMatrix, Mat4Translate3(center));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcSetSourceRec1(NewRec(Vec2_Zero, rc->state.boundTexture1->size));
	RcBindVertBuffer(&rc->sphereBuffers[sphereQuality]);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedSphere(v3 center, r32 radius, Color_t color, u64 sphereQuality, rec sourceRec)
{
	Assert(sphereQuality < SphereQuality_NumQualities);
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(radius, radius, radius));
	Mat4Transform(worldMatrix, Mat4Translate3(center));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcSetSourceRec1(sourceRec);
	RcBindVertBuffer(&rc->sphereBuffers[sphereQuality]);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

void RcDrawBox(box boundingBox, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.bottomLeft));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawSkybox(box boundingBox, Color_t color)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.bottomLeft));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(NewRec(Vec2_Zero, rc->state.boundTexture1->size));
	RcSetColor1(color);
	RcBindVertBuffer(&rc->skyboxBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

void RcDrawTexturedBox(box boundingBox, Color_t color)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.bottomLeft));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(NewRec(Vec2_Zero, rc->state.boundTexture1->size));
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedBox(box boundingBox, Color_t color, rec sourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.bottomLeft));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedBoxEx(box boundingBox, Color_t color,
	rec rightSourceRec, rec topSourceRec, rec frontSourceRec,
	rec leftSourceRec, rec bottomSourceRec, rec backSourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.bottomLeft));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	
	RcSetSourceRec1(topSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0*6, 6);
	RcSetSourceRec1(rightSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 1*6, 6);
	RcSetSourceRec1(frontSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 2*6, 6);
	RcSetSourceRec1(leftSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 3*6, 6);
	RcSetSourceRec1(backSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 4*6, 6);
	RcSetSourceRec1(bottomSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 5*6, 6);
}

void RcDrawObb3D(obb3 boundingBox, Color_t color)
{
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate3(-Vec3_Half));
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Quaternion(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.center));
	RcSetWorldMatrix(worldMatrix);
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedObb3D(obb3 boundingBox, Color_t color)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate3(-Vec3_Half));
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Quaternion(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.center));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(NewRec(Vec2_Zero, rc->state.boundTexture1->size));
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedObb3D(obb3 boundingBox, Color_t color, rec sourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate3(-Vec3_Half));
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Quaternion(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.center));
	RcSetWorldMatrix(worldMatrix);
	RcSetSourceRec1(sourceRec);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
void RcDrawTexturedObb3DEx(obb3 boundingBox, Color_t color,
	rec rightSourceRec, rec topSourceRec, rec frontSourceRec,
	rec leftSourceRec, rec bottomSourceRec, rec backSourceRec)
{
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate3(-Vec3_Half));
	Mat4Transform(worldMatrix, Mat4Scale3(boundingBox.size));
	Mat4Transform(worldMatrix, Mat4Quaternion(boundingBox.rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(boundingBox.center));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	
	RcSetSourceRec1(topSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 0*6, 6);
	RcSetSourceRec1(rightSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 1*6, 6);
	RcSetSourceRec1(frontSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 2*6, 6);
	RcSetSourceRec1(leftSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 3*6, 6);
	RcSetSourceRec1(backSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 4*6, 6);
	RcSetSourceRec1(bottomSourceRec);
	RcDrawBuffer(VertBufferPrimitive_Triangles, 5*6, 6);
}

//NOTE: This draws an actual 3D box in the world so apparent thickness is dependent on distance from the camera
void RcDrawLine3DBox(v3 point1, v3 point2, r32 thickness, Color_t color)
{
	if (Vec3BasicallyEqual(point1, point2, 0.0001f)) { return; }
	v2 horizontalVec = Vec3_xz(point2) - Vec3_xz(point1);
	r32 distance = Vec3Length(point2 - point1);
	r32 facingRotation = AngleFromVec2(horizontalVec);
	r32 facingDot = Vec2Dot(horizontalVec, Vec2FromAngle(facingRotation));
	r32 upDownRotation = AngleFromVec2(facingDot, point2.y - point1.y);
	// plat->DebugReadout(TempPrintStr("facingRotation: %f", facingRotation), White, 1.0f);
	// plat->DebugReadout(TempPrintStr("p1: (%.1f, %.1f, %.1f)", point1.x, point1.y, point1.z), White, 1.0f);
	// plat->DebugReadout(TempPrintStr("p2: (%.1f, %.1f, %.1f)", point2.x, point2.y, point2.z), White, 1.0f);
	
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Translate3(0, -0.5f, -0.5f));
	Mat4Transform(worldMatrix, Mat4Scale3(distance, thickness, thickness));
	Mat4Transform(worldMatrix, Mat4RotateZ(upDownRotation));
	Mat4Transform(worldMatrix, Mat4RotateY(facingRotation));
	Mat4Transform(worldMatrix, Mat4Translate3(point1));
	RcSetWorldMatrix(worldMatrix);
	
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->cubeBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}

void RcDrawLine3D(v3 point1, v3 point2, r32 thickness, Color_t color)
{
	if (Vec3BasicallyEqual(point1, point2, 0.0001f)) { return; }
	v2 horizontalVec = Vec3_xz(point2) - Vec3_xz(point1);
	r32 distance = Vec3Length(point2 - point1);
	r32 facingRotation = AngleFromVec2(horizontalVec);
	r32 facingDot = Vec2Dot(horizontalVec, Vec2FromAngle(facingRotation));
	r32 upDownRotation = AngleFromVec2(facingDot, point2.y - point1.y);
	
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4ScaleX(distance));
	Mat4Transform(worldMatrix, Mat4RotateZ(upDownRotation));
	Mat4Transform(worldMatrix, Mat4RotateY(facingRotation));
	Mat4Transform(worldMatrix, Mat4Translate3(point1));
	RcSetWorldMatrix(worldMatrix);
	
	RcSetLineThickness(thickness);
	
	RcBindTexture1(&rc->dotTexture);
	RcSetSourceRec1(Rec_Default);
	RcSetColor1(color);
	RcBindVertBuffer(&rc->lineBuffer);
	RcDrawBuffer(VertBufferPrimitive_Lines);
}

void RcDrawPoint3D(v3 point, r32 radius, Color_t color, bool useBox = false, SphereQuality_t sphereQuality = SphereQuality_SuperLow)
{
	if (useBox)
	{
		RcDrawBox(NewBoxCentered(point, Vec3Fill(radius*2)), color);
	}
	else
	{
		RcDrawSphere(point, radius, color, sphereQuality);
	}
}

void RcDrawBoxWireframe(box boundingBox, Color_t color, r32 thickness)
{
	v3 min = boundingBox.bottomLeft;
	v3 max = boundingBox.bottomLeft + boundingBox.size;
	// Vertical lines
	RcDrawLine3D(NewVec3(min.x, min.y, min.z), NewVec3(min.x, max.y, min.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, min.y, min.z), NewVec3(max.x, max.y, min.z), thickness, color);
	RcDrawLine3D(NewVec3(min.x, min.y, max.z), NewVec3(min.x, max.y, max.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, min.y, max.z), NewVec3(max.x, max.y, max.z), thickness, color);
	// Bottom Loop
	RcDrawLine3D(NewVec3(min.x, min.y, min.z), NewVec3(max.x, min.y, min.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, min.y, min.z), NewVec3(max.x, min.y, max.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, min.y, max.z), NewVec3(min.x, min.y, max.z), thickness, color);
	RcDrawLine3D(NewVec3(min.x, min.y, max.z), NewVec3(min.x, min.y, min.z), thickness, color);
	// Top Loop
	RcDrawLine3D(NewVec3(min.x, max.y, min.z), NewVec3(max.x, max.y, min.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, max.y, min.z), NewVec3(max.x, max.y, max.z), thickness, color);
	RcDrawLine3D(NewVec3(max.x, max.y, max.z), NewVec3(min.x, max.y, max.z), thickness, color);
	RcDrawLine3D(NewVec3(min.x, max.y, max.z), NewVec3(min.x, max.y, min.z), thickness, color);
}

void RcApplyModelMaterial(ModelMaterial_t* material, bool changeTexture = true)
{
	if (changeTexture)
	{
		RcBindTexture1(&material->diffuseTexture);
		RcSetSourceRec1(NewRec(Vec2_Zero, material->diffuseTexture.size));
	}
	RcSetDynamicUniformVec3("AmbientColor",  ToVec3(material->ambientColor));
	RcSetDynamicUniformVec3("DiffuseColor",  ToVec3(material->diffuseColor));
	RcSetDynamicUniformVec3("SpecularColor", ToVec3(material->specularColor));
	RcSetDynamicUniformR32("SpecularExponent", material->specularExponent);
}

void RcDrawModelSimple(v3 position, quat rotation, v3 scale, Color_t color)
{
	NotNull(rc->state.boundModel);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(scale));
	Mat4Transform(worldMatrix, Mat4Quaternion(rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(position));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	VarArrayLoop(&rc->state.boundModel->parts, pIndex)
	{
		VarArrayLoopGet(ModelPart_t, part, &rc->state.boundModel->parts, pIndex);
		if (part->buffer.isValid)
		{
			ModelMaterial_t* material = VarArrayGetHard(&rc->state.boundModel->materials, part->materialIndex, ModelMaterial_t);
			RcApplyModelMaterial(material);
			RcBindVertBuffer(&part->buffer);
			RcDrawBuffer(VertBufferPrimitive_Triangles);
		}
	}
}
void RcDrawModelParts(u64 startPartIndex, u64 numParts, v3 position, quat rotation, v3 scale, Color_t color)
{
	NotNull(rc->state.boundModel);
	Assert(startPartIndex + numParts <= rc->state.boundModel->parts.length);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(scale));
	Mat4Transform(worldMatrix, Mat4Quaternion(rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(position));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	for (u64 pIndex = startPartIndex; pIndex < startPartIndex + numParts; pIndex++)
	{
		VarArrayLoopGet(ModelPart_t, part, &rc->state.boundModel->parts, pIndex);
		if (part->buffer.isValid)
		{
			ModelMaterial_t* material = VarArrayGetHard(&rc->state.boundModel->materials, part->materialIndex, ModelMaterial_t);
			RcApplyModelMaterial(material);
			RcBindVertBuffer(&part->buffer);
			RcDrawBuffer(VertBufferPrimitive_Triangles);
		}
	}
}

void RcDrawModelCustomTexture(v3 position, quat rotation, v3 scale, Color_t color, rec sourceRec)
{
	NotNull(rc->state.boundModel);
	NotNull(rc->state.boundTexture1);
	mat4 worldMatrix = Mat4_Identity;
	Mat4Transform(worldMatrix, Mat4Scale3(scale));
	Mat4Transform(worldMatrix, Mat4Quaternion(rotation));
	Mat4Transform(worldMatrix, Mat4Translate3(position));
	RcSetWorldMatrix(worldMatrix);
	RcSetColor1(color);
	RcSetSourceRec1(sourceRec);
	VarArrayLoop(&rc->state.boundModel->parts, pIndex)
	{
		VarArrayLoopGet(ModelPart_t, part, &rc->state.boundModel->parts, pIndex);
		if (part->buffer.isValid)
		{
			ModelMaterial_t* material = VarArrayGetHard(&rc->state.boundModel->materials, part->materialIndex, ModelMaterial_t);
			RcApplyModelMaterial(material, false);
			RcBindVertBuffer(&part->buffer);
			RcDrawBuffer(VertBufferPrimitive_Triangles);
		}
	}
}