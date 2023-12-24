/*
File:   pig_vector_img.cpp
Author: Taylor Robbins
Date:   01\27\2022
Description: 
	** Holds all the functions that help us manage and manipulate or use VectorImg_t structures
	** Vector Images are primarily sourced from svg files and hold a very similar structure to SvgData_t
	** but contain our post processed interpretations (like a proper hiercharcy and calculated bounds for each part)
*/

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
VectorImgFillType_t GetVectorImgFillTypeForSvgFillStyle(SvgFillStyle_t fillStyle)
{
	switch (fillStyle)
	{
		case SvgFillStyle_None:  return VectorImgFillType_None;
		case SvgFillStyle_Solid: return VectorImgFillType_Solid;
		default: return VectorImgFillType_None;
	}
}
VectorImgStrokeType_t GetVectorImgStrokeTypeForSvgStrokeStyle(SvgStrokeStyle_t strokeStyle)
{
	switch (strokeStyle)
	{
		case SvgStrokeStyle_None:  return VectorImgStrokeType_None;
		case SvgStrokeStyle_Solid: return VectorImgStrokeType_Solid;
		default: return VectorImgStrokeType_None;
	}
}

VectorImgPart_t* GetVectorImgPartByIndexArray(VectorImgPart_t* base, u64 numIndices, u64* indices)
{
	NotNull(base);
	AssertIf(numIndices > 0, indices != nullptr);
	VectorImgPart_t* result = base;
	for (u64 iIndex = 0; iIndex < numIndices; iIndex++)
	{
		u64 nextIndex = indices[iIndex];
		if (nextIndex >= result->children.length) { return nullptr; }
		result = VarArrayGetHard(&result->children, nextIndex, VectorImgPart_t);
	}
	return result;
}

// +--------------------------------------------------------------+
// |                           Destroy                            |
// +--------------------------------------------------------------+
void DestroyVectorImgShape(VectorImg_t* image, VectorImgPart_t* part, VectorImgShape_t* shape)
{
	NotNull(image);
	NotNull(part);
	NotNull(shape);
	FreeString(image->allocArena, &shape->idStr);
	FreeString(image->allocArena, &shape->name);
}
void DestroyVectorImgPart(VectorImg_t* image, VectorImgPart_t* part)
{
	NotNull(image);
	NotNull(part);
	FreeString(image->allocArena, &part->idStr);
	FreeString(image->allocArena, &part->name);
	VarArrayLoop(&part->shapes, sIndex)
	{
		VarArrayLoopGet(VectorImgShape_t, shape, &part->shapes, sIndex);
		DestroyVectorImgShape(image, part, shape);
	}
	FreeVarArray(&part->shapes);
	VarArrayLoop(&part->children, cIndex)
	{
		VarArrayLoopGet(VectorImgPart_t, childPart, &part->children, cIndex);
		DestroyVectorImgPart(image, childPart);
	}
	FreeVarArray(&part->children);
	ClearPointer(part);
}
void DestroyVectorImg(VectorImg_t* image)
{
	NotNull(image);
	DestroyVectorImgPart(image, &image->root);
	VectorImgError_t error          = image->error;
	XmlParsingError_t xmlError      = image->xmlError;
	TryDeserSvgFileError_t svgError = image->svgError;
	ClearPointer(image);
	image->error    = error;
	image->xmlError = xmlError;
	image->svgError = svgError;
}

// +--------------------------------------------------------------+
// |                            Create                            |
// +--------------------------------------------------------------+
void CreateVectorImgPartFromSvgGroup(VectorImg_t* image, VectorImgPart_t* partOut, VectorImgPart_t* parent, const SvgData_t* svgData, const SvgGroup_t* svgGroup, u64 groupIndex)
{
	NotNull(image);
	NotNull(image->allocArena);
	NotNull(partOut);
	NotNull(svgData);
	NotNull(svgGroup);
	
	image->totalNumParts++;
	
	u64 numChildren = 0;
	VarArrayLoop(&svgData->groups, gIndex)
	{
		VarArrayLoopGet(SvgGroup_t, svgSubGroup, &svgData->groups, gIndex);
		if (gIndex != groupIndex && svgSubGroup->parentIndex == groupIndex) { numChildren++; }
	}
	
	ClearPointer(partOut);
	partOut->id = image->nextPartId;
	image->nextPartId++;
	partOut->index = ((parent != nullptr) ? (parent->children.length-1) : 0);
	partOut->interleaveIndex = svgGroup->interleaveIndex;
	partOut->idStr = AllocString(image->allocArena, &svgGroup->idStr);
	partOut->name = AllocString(image->allocArena, &svgGroup->label);
	partOut->origin = svgGroup->origin;
	CreateVarArray(&partOut->shapes, image->allocArena, sizeof(VectorImgShape_t), svgGroup->shapes.length, false, 1);
	CreateVarArray(&partOut->children, image->allocArena, sizeof(VectorImgPart_t), numChildren, false, 1);
	// CreateVarArray(&partOut->shapes, image->allocArena, sizeof(VectorImgShape_t));
	// CreateVarArray(&partOut->children, image->allocArena, sizeof(VectorImgPart_t)); TODO: Remove these lines!
	
	bool boundsStarted = false;
	partOut->bounds = Rec_Zero;
	VarArrayLoop(&svgGroup->shapes, sIndex)
	{
		VarArrayLoopGet(SvgShape_t, svgShape, &svgGroup->shapes, sIndex);
		
		image->totalNumShapes++;
		
		VectorImgShape_t* newShape = VarArrayAdd(&partOut->shapes, VectorImgShape_t);
		NotNull(newShape);
		ClearPointer(newShape);
		newShape->id = image->nextShapeId;
		image->nextShapeId++;
		newShape->index = (partOut->shapes.length-1);
		newShape->idStr = AllocString(image->allocArena, &svgShape->idStr);
		newShape->name = AllocString(image->allocArena, &svgShape->label);
		newShape->fill.type = GetVectorImgFillTypeForSvgFillStyle(svgShape->fill.style);
		newShape->fill.color = svgShape->fill.color;
		newShape->stroke.type = GetVectorImgStrokeTypeForSvgStrokeStyle(svgShape->stroke.style);
		newShape->stroke.color = svgShape->stroke.color;
		newShape->stroke.thickness = svgShape->stroke.thickness;
		switch (svgShape->type)
		{
			case SvgShapeType_Circle:
			{
				newShape->type = VectorImgShapeType_Circle;
				newShape->circle.center = svgShape->circle.center;
				newShape->circle.radius = svgShape->circle.radius;
				newShape->bounds = NewRecCentered(newShape->circle.center, 2*newShape->circle.radius, 2*newShape->circle.radius);
			} break;
			case SvgShapeType_Rectangle:
			{
				newShape->type = VectorImgShapeType_Rectangle;
				newShape->rectangle.value = svgShape->rectangle.value;
				newShape->rectangle.roundedRadius = svgShape->rectangle.roundedRadius;
				newShape->bounds = GetObb2DAlignedBounds(newShape->rectangle.value);
			} break;
			case SvgShapeType_Path:
			{
				newShape->type = VectorImgShapeType_Path;
				CreateBezierPath(&newShape->path.value, image->allocArena, svgShape->path.value.parts.length);
				VarArrayAddVarArray(&newShape->path.value.parts, &svgShape->path.value.parts, 0);
				newShape->bounds = GetBoundsForBezierPath(&newShape->path.value);
			} break;
			default: DebugAssert(false); break;
		}
		if (newShape->origin == Vec2_Zero) { newShape->origin = newShape->bounds.topLeft + newShape->bounds.size/2; }
		if (!boundsStarted) { partOut->bounds = newShape->bounds; boundsStarted = true; }
		else { partOut->bounds = RecBoth(partOut->bounds, newShape->bounds); }
	}
	
	VarArrayLoop(&svgData->groups, gIndex)
	{
		VarArrayLoopGet(SvgGroup_t, svgSubGroup, &svgData->groups, gIndex);
		if (gIndex != groupIndex && svgSubGroup->parentIndex == groupIndex)
		{
			VectorImgPart_t* newPart = VarArrayAdd(&partOut->children, VectorImgPart_t);
			NotNull(newPart);
			CreateVectorImgPartFromSvgGroup(image, newPart, partOut, svgData, svgSubGroup, gIndex);
			if (!boundsStarted) { partOut->bounds = newPart->bounds; boundsStarted = true; }
			else { partOut->bounds = RecBoth(partOut->bounds, newPart->bounds); }
		}
	}
}
bool CreateVectorImgFromSvg(MemArena_t* memArena, VectorImg_t* imageOut, const SvgData_t* svgData)
{
	AssertSingleThreaded();
	NotNull(memArena);
	NotNull(imageOut);
	NotNull(svgData);
	Assert(memArena != TempArena);
	
	ClearPointer(imageOut);
	imageOut->allocArena = memArena;
	imageOut->totalNumParts = 0;
	imageOut->totalNumShapes = 0;
	imageOut->nextPartId = 1;
	imageOut->nextShapeId = 1;
	imageOut->id = pig->nextVectorImgId;
	pig->nextVectorImgId++;
	
	if (svgData->groups.length == 0)
	{
		imageOut->error = VectorImgError_NoGroups;
		DestroyVectorImg(imageOut);
		return false;
	}
	
	SvgGroup_t* mainGroup = VarArrayGetHard(&svgData->groups, 0, SvgGroup_t);
	CreateVectorImgPartFromSvgGroup(imageOut, &imageOut->root, nullptr, svgData, mainGroup, 0);
	if (imageOut->totalNumParts != svgData->groups.length)
	{
		NotifyPrint_W("Not all groups had parents in SVG Data in CreateVectorImgFromSvg (%llu/%llu groups walked)", imageOut->totalNumParts, svgData->groups.length);
	}
	
	imageOut->isValid = true;
	return true;
}

// +--------------------------------------------------------------+
// |                             Loop                             |
// +--------------------------------------------------------------+
struct VectorImgLoop_t
{
	VectorImgPart_t* base;
	bool loopShapes;
	bool loopParts;
	bool loopParentPart;
	u64 partDepth;
	u64 shapeIndex[VECTOR_IMG_MAX_PART_TREE_DEPTH];
	u64 partIndex[VECTOR_IMG_MAX_PART_TREE_DEPTH];
	VectorImgPart_t* part;
	VectorImgShape_t* shape;
	u64 iterPartCount;
	u64 iterShapeCount;
};
VectorImgLoop_t CreateVectorImgLoop(VectorImgPart_t* part, bool loopShapes = true, bool loopParts = false, bool loopParentPart = false)
{
	VectorImgLoop_t result = {};
	result.base = part;
	result.loopShapes = loopShapes;
	result.loopParts = loopParts;
	result.loopParentPart = loopParentPart;
	result.partDepth = 0;
	result.shapeIndex[0] = 0;
	result.partIndex[0] = 0;
	result.part = part;
	result.shape = nullptr;
	result.iterPartCount = 0;
	result.iterShapeCount = 0;
	return result;
}
bool VectorImgLoop(VectorImgLoop_t* loop)
{
	NotNull(loop);
	NotNull(loop->base);
	Assert(loop->partDepth < VECTOR_IMG_MAX_PART_TREE_DEPTH);
	
	if (loop->loopParentPart)
	{
		loop->loopParentPart = false;
		loop->part = loop->base;
		loop->shape = nullptr;
		loop->iterPartCount++;
		return true;
	}
	
	while (true)
	{
		VectorImgPart_t* currentPart = GetVectorImgPartByIndexArray(loop->base, loop->partDepth, &loop->partIndex[0]);
		if (currentPart == nullptr) { return false; }
		u64 currentShapeIndex = loop->shapeIndex[loop->partDepth];
		u64 currentPartIndex = loop->partIndex[loop->partDepth];
		
		VectorImgPart_t* nextPart = VarArrayGetSoft(&currentPart->children, currentPartIndex, VectorImgPart_t);
		if (nextPart != nullptr)
		{
			DebugAssert(nextPart->interleaveIndex <= currentPart->shapes.length);
			if (nextPart->interleaveIndex <= currentShapeIndex)
			{
				Assert(loop->partDepth+1 < VECTOR_IMG_MAX_PART_TREE_DEPTH);
				loop->shapeIndex[loop->partDepth+1] = 0;
				loop->partIndex[loop->partDepth+1] = 0;
				loop->partDepth++;
				
				if (loop->loopParts)
				{
					loop->part = nextPart;
					loop->shape = nullptr;
					loop->iterPartCount++;
					return true;
				}
				else { continue; }
			}
		}
		
		VectorImgShape_t* nextShape = VarArrayGetSoft(&currentPart->shapes, currentShapeIndex, VectorImgShape_t);
		if (nextShape != nullptr)
		{
			loop->shapeIndex[loop->partDepth]++;
			if (loop->loopShapes)
			{
				loop->part = nullptr;
				loop->shape = nextShape;
				loop->iterShapeCount++;
				return true;
			}
			else { continue; }
		}
		
		if (loop->partDepth > 0)
		{
			loop->partDepth--;
			loop->partIndex[loop->partDepth]++;
			continue;
		}
		else
		{
			return false;
		}
	}
	return false; //We really shouldn't ever get here
}

// +--------------------------------------------------------------+
// |                        Find Functions                        |
// +--------------------------------------------------------------+
VectorImgPart_t* FindVectorImgPartByName(VectorImgPart_t* base, MyStr_t targetName, bool ignoreCase = true, bool giveBaseIfNameMatches = false)
{
	VectorImgLoop_t loop = CreateVectorImgLoop(base, false, true, giveBaseIfNameMatches);
	while (VectorImgLoop(&loop))
	{
		NotNull(loop.part);
		if (ignoreCase && StrEqualsIgnoreCase(loop.part->name, targetName))
		{
			return loop.part;
		}
		else if (!ignoreCase && StrEquals(loop.part->name, targetName))
		{
			return loop.part;
		}
	}
	return nullptr;
}

// +--------------------------------------------------------------+
// |                   Triangulate Path Shapes                    |
// +--------------------------------------------------------------+
bool GenerateVertBufferForVectorImgPathShape(VectorImgShape_t* shape, u64 numCurveVertices, v2** debugVerticesOut = nullptr, u64* debugNumVerticesOut = nullptr, bool ignoreFailures = false)
{
	NotNull(shape);
	Assert(shape->type == VectorImgShapeType_Path);
	Assert(numCurveVertices > 0);
	
	u64 numPositionsNeeded = 0;
	u64 numSubParts = 0;
	VarArrayLoop(&shape->path.value.parts, pIndex)
	{
		VarArrayLoopGet(BezierPathPart_t, pathPart, &shape->path.value.parts, pIndex);
		if (pathPart->detached)
		{
			numPositionsNeeded++;
			numSubParts++;
		}
		if (pathPart->type == BezierPathPartType_Curve3 ||
			pathPart->type == BezierPathPartType_Curve4 ||
			pathPart->type == BezierPathPartType_EllipseArc)
		{
			numPositionsNeeded += numCurveVertices;
		}
		else if (pathPart->type == BezierPathPartType_Line)
		{
			numPositionsNeeded += 1;
		}
		else { Assert(false); }
		if (pIndex == shape->path.value.parts.length-1 && shape->path.value.isClosedLoop) { numPositionsNeeded--; }
	}
	Assert(numSubParts > 0);
	Assert(numPositionsNeeded > 0);
	
	TempPushMark();
	v2* vertPositions = AllocArray(TempArena, v2, numPositionsNeeded);
	NotNull(vertPositions);
	u64* subPartStartIndices = AllocArray(TempArena, u64, numSubParts);
	NotNull(subPartStartIndices);
	u64* subPartVertCounts = AllocArray(TempArena, u64, numSubParts);
	NotNull(subPartVertCounts);
	
	u64 vpIndex = 0;
	u64 subPartIndex = 0;
	v2 currentPos = Vec2_Zero;
	VarArrayLoop(&shape->path.value.parts, pIndex)
	{
		VarArrayLoopGet(BezierPathPart_t, pathPart, &shape->path.value.parts, pIndex);
		bool addLastVertex = !(pIndex == shape->path.value.parts.length-1 && shape->path.value.isClosedLoop);
		if (pathPart->detached)
		{
			Assert(subPartIndex < numSubParts);
			subPartStartIndices[subPartIndex] = vpIndex;
			subPartVertCounts[subPartIndex] = 0;
			subPartIndex++;
			currentPos = pathPart->startPos;
			
			Assert(subPartIndex > 0);
			Assert(vpIndex < numPositionsNeeded);
			vertPositions[vpIndex] = (pathPart->detached ? pathPart->startPos : currentPos);
			vpIndex++;
			subPartVertCounts[subPartIndex-1]++;
		}
		if (pathPart->type == BezierPathPartType_Curve3)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = BezierCurve3(currentPos, pathPart->control1, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_Curve4)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = BezierCurve4(currentPos, pathPart->control1, pathPart->control2, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_EllipseArc)
		{
			for (u64 cvIndex = 0; cvIndex < (addLastVertex ? numCurveVertices : numCurveVertices-1); cvIndex++)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				r32 time = (r32)(cvIndex+1) / (r32)numCurveVertices;
				vertPositions[vpIndex] = EllipseArcCurveStartEnd(currentPos, pathPart->radius, pathPart->axisAngle, pathPart->arcFlags, pathPart->endPos, time);
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else if (pathPart->type == BezierPathPartType_Line)
		{
			if (addLastVertex)
			{
				Assert(subPartIndex > 0);
				Assert(vpIndex < numPositionsNeeded);
				vertPositions[vpIndex] = pathPart->endPos;
				vpIndex++;
				subPartVertCounts[subPartIndex-1]++;
			}
		}
		else { Assert(false); }
		currentPos = pathPart->endPos;
	}
	Assert(vpIndex == numPositionsNeeded);
	Assert(subPartIndex == numSubParts);
	
	if (debugVerticesOut != nullptr)
	{
		*debugVerticesOut = AllocArray(mainHeap, v2, numPositionsNeeded);
		MyMemCopy(*debugVerticesOut, vertPositions, sizeof(v2) * numPositionsNeeded);
		*debugNumVerticesOut = numPositionsNeeded;
	}
	
	u64 numTotalIndicesNeeded = 0;
	for (u64 subIndex = 0; subIndex < numSubParts; subIndex++)
	{
		u64 subPartStartIndex = subPartStartIndices[subIndex];
		Assert(subPartStartIndex+subPartVertCounts[subIndex] <= numPositionsNeeded);
		v2* subPartVertices = &vertPositions[subPartStartIndex];
		
		u64 uniqeVertCount = RemoveColinearAndDuplicateVertices(subPartVertCounts[subIndex], subPartVertices);
		subPartVertCounts[subIndex] = uniqeVertCount;
		u64 subPartVertCount = subPartVertCounts[subIndex];
		
		u64 triangulationNumIndices = 0;
		Triangulate2DEarClip(nullptr, nullptr, subPartVertCount, subPartVertices, &triangulationNumIndices, ignoreFailures);
		if (triangulationNumIndices == 0)
		{
			WriteLine_E("Triangulate2DEarClip failed! (early)");
			TempPopMark();
			return false;
		}
		numTotalIndicesNeeded += triangulationNumIndices;
	}
	Assert(numTotalIndicesNeeded > 0);
	
	u64* allIndices = AllocArray(TempArena, u64, numTotalIndicesNeeded);
	NotNull(allIndices);
	
	u64 allIndicesIndex = 0;
	for (u64 subIndex = 0; subIndex < numSubParts; subIndex++)
	{
		TempPushMark();
		u64 subPartStartIndex = subPartStartIndices[subIndex];
		u64 subPartVertCount = subPartVertCounts[subIndex];
		Assert(subPartStartIndex+subPartVertCount <= numPositionsNeeded);
		v2* subPartVertices = &vertPositions[subPartStartIndex];
		bool isClockwise = IsPolygonClockwise(subPartVertCount, subPartVertices);
		if (!isClockwise)
		{
			//reverse vertices
			for (u64 reverseIndex = 0; reverseIndex < subPartVertCount/2; reverseIndex++)
			{
				v2* vertLeft = &subPartVertices[reverseIndex];
				v2* vertRight = &subPartVertices[subPartVertCount-1 - reverseIndex];
				SWAP_POINTERS(v2, vertLeft, vertRight);
			}
		}
		u64 triangulationNumIndices = 0;
		u64* triangulationIndices = Triangulate2DEarClip(TempArena, TempArena, subPartVertCount, subPartVertices, &triangulationNumIndices, ignoreFailures);
		if (triangulationIndices == nullptr)
		{
			PrintLine_E("Triangulate2DEarClip failed on shape \"%.*s\" ID \"%.*s\"!", StrPrint(shape->name), StrPrint(shape->idStr));
			for (u64 vIndex = 0; vIndex < subPartVertCount; vIndex++)
			{
				PrintLine_W("(%g, %g)", subPartVertices[vIndex].x, subPartVertices[vIndex].y);
			}
			TempPopMark();
			TempPopMark();
			return false;
		}
		Assert(allIndicesIndex + triangulationNumIndices <= numTotalIndicesNeeded);
		for (u64 triIndicesIndex = 0; triIndicesIndex < triangulationNumIndices; triIndicesIndex++) { triangulationIndices[triIndicesIndex] += subPartStartIndex; }
		MyMemCopy(&allIndices[allIndicesIndex], triangulationIndices, sizeof(u64) * triangulationNumIndices);
		allIndicesIndex += triangulationNumIndices;
		TempPopMark();
	}
	AssertIf(!ignoreFailures, allIndicesIndex == numTotalIndicesNeeded);
	
	Assert((numTotalIndicesNeeded % 3) == 0);
	Vertex2D_t* vertices = AllocArray(TempArena, Vertex2D_t, numTotalIndicesNeeded);
	NotNull(vertices);
	for (u64 iIndex = 0; iIndex < numTotalIndicesNeeded; iIndex++)
	{
		u64 vertPosIndex = allIndices[iIndex + 0];
		Assert(vertPosIndex < numPositionsNeeded);
		vertices[iIndex].position = NewVec3(vertPositions[vertPosIndex].x, vertPositions[vertPosIndex].y, 0);
		vertices[iIndex].color = Vec4_One;
		vertices[iIndex].texCoord = Vec2_Zero; //TODO: Implement me!
	}
	
	DestroyVertBuffer(&shape->path.vertBuffer);
	shape->path.vertBufferUpToDate = false;
	
	if (!CreateVertBuffer2D(mainHeap, &shape->path.vertBuffer, false, numTotalIndicesNeeded, vertices, false))
	{
		WriteLine_E("CreateVertBuffer2D failed!");
		TempPopMark();
		return false;
	}
	shape->path.vertBufferUpToDate = true;
	
	TempPopMark();
	
	return true;
}

// +--------------------------------------------------------------+
// |                       Other Functions                        |
// +--------------------------------------------------------------+
void PrintStructureOfVectorImgPartRecurse(const VectorImg_t* image, const VectorImgPart_t* part, u64 indentLevel)
{
	TempPushMark();
	MyStr_t indentStr = StringRepeat(TempArena, "--", indentLevel);
	VarArrayLoop(&part->shapes, sIndex)
	{
		VarArrayLoopGet(VectorImgShape_t, shape, &part->shapes, sIndex);
		if (shape->type == VectorImgShapeType_Circle)
		{
			PrintLine_I("%s--Circle[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f) radius %.1f",
				indentStr.pntr, shape->index,
				StrPrint(shape->name),
				StrPrint(shape->idStr),
				shape->circle.center.x, shape->circle.center.y, shape->circle.radius
			);
		}
		else if (shape->type == VectorImgShapeType_Rectangle)
		{
			PrintLine_I("%s--Rec[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f) size %.1fx%.1f rot %.1fdeg rounded (%.1f, %.1f)",
				indentStr.pntr, shape->index,
				StrPrint(shape->name),
				StrPrint(shape->idStr),
				shape->rectangle.center.x, shape->rectangle.center.y,
				shape->rectangle.size.width, shape->rectangle.size.height,
				ToDegrees32(shape->rectangle.rotation),
				shape->rectangle.roundedRadius.x, shape->rectangle.roundedRadius.y
			);
		}
		else
		{
			PrintLine_I("%s--%s[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f, %.1f, %.1f)",
				indentStr.pntr, GetVectorImgShapeTypeStr(shape->type), shape->index,
				StrPrint(shape->name),
				StrPrint(shape->idStr),
				shape->bounds.x, shape->bounds.y, shape->bounds.width, shape->bounds.height
			);
		}
	}
	VarArrayLoop(&part->children, cIndex)
	{
		VarArrayLoopGet(VectorImgPart_t, child, &part->children, cIndex);
		PrintStructureOfVectorImgPartRecurse(image, child, indentLevel+1);
	}
	TempPopMark();
}
void PrintStructureOfVectorImg(const VectorImg_t* image)
{
	PrintLine_N("Vector Image has %llu part%s and %llu shape%s:",
		image->totalNumParts, (image->totalNumParts == 1) ? "" : "s",
		image->totalNumShapes, (image->totalNumShapes == 1) ? "" : "s"
	);
	PrintStructureOfVectorImgPartRecurse(image, &image->root, 0);
}

