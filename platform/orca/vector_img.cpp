/*
File:   vector_img.cpp
Author: Taylor Robbins
Date:   09\29\2024
Description: 
	** Holds the functions that help us load a .svg file into a VectorImg_t struct
	** using nanosvg to do the parsing work
*/

Color_t NewColorSvg(u32 svgPackedColor)
{
	return NewColor(
		(u8)((svgPackedColor >> 0) & 0xFF),
		(u8)((svgPackedColor >> 8) & 0xFF),
		(u8)((svgPackedColor >> 16) & 0xFF),
		(u8)((svgPackedColor >> 24) & 0xFF)
	);
}

bool TryLoadVectorImgFromPath(MyStr_t filePath, MemArena_t* memArena, VectorImg_t* imageOut)
{
	NotNull(memArena);
	NotNull(imageOut);
	OC_ArenaScope_t scratch = OC_ScratchBegin();
	NSVGimage* nsvg = nullptr;
	
	// Open, read and parse the file
	{
		OC_File_t svgFile = OC_FileOpen(filePath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
		AssertMsg(!OC_FileIsNil(svgFile), "Failed to open svg");
		
		u64 svgFileSize = OC_FileSize(svgFile);
		AssertMsg(svgFileSize > 0, "SVG file failed to open or is empty!");
		// PrintLine_I("svg file is %llu bytes", svgFileSize);
		
		MyStr_t svgFileContents = NewStr(svgFileSize, OC_ArenaPushArray(scratch.arena, char, svgFileSize+1));
		AssertMsg(svgFileContents.chars != nullptr, "Failed to allocate space for svg file");
		u64 readResult = OC_FileRead(svgFile, svgFileSize, svgFileContents.chars);
		//TODO: Assert on readResult?
		svgFileContents.chars[svgFileContents.length] = '\0';
		
		nsvg = nsvgParse(svgFileContents.chars, "px", 96);
		AssertMsg(nsvg != nullptr, "Failed to parse svg image");
		OC_FileClose(svgFile);
	}
	
	u64 numShapes = 0;
	NSVGshape* nshape = nsvg->shapes;
	while (nshape != nullptr) { numShapes++; nshape = nshape->next; }
	
	ClearPointer(imageOut);
	imageOut->allocArena = memArena;
	CreateVarArray(&imageOut->shapes, memArena, sizeof(VectorShape_t), numShapes);
	
	nshape = nsvg->shapes;
	while (nshape != nullptr)
	{
		VectorShape_t* shape = VarArrayAdd(&imageOut->shapes, VectorShape_t);
		NotNull(shape);
		ClearPointer(shape);
		
		// TODO: NSVGshape has the following information we may want to add here:
		//      char id[64];				// Optional 'id' attr of the shape or its group
		//      float strokeDashOffset;		// Stroke dash offset (scaled).
		//      float strokeDashArray[8];	// Stroke dash array (scaled).
		//      char strokeDashCount;		// Number of dash values in dash array.
		//      char strokeLineJoin;		// Stroke join type.
		//      char strokeLineCap;			// Stroke cap type.
		//      char fillGradient[64];		// Optional 'id' of fill gradient
		//      char strokeGradient[64];	// Optional 'id' of stroke gradient
		//      float xform[6];				// Root transformation for fill/stroke gradient
		
		Assert(BufferIsNullTerminated(ArrayCount(nshape->id), &nshape->id[0]));
		MyStr_t idStr = NewStr(&nshape->id[0]);
		shape->name = AllocString(memArena, &idStr);
		shape->visible = IsFlagSet(nshape->flags, NSVG_FLAGS_VISIBLE);
		shape->opacity = 1.0f; //Is this somewhere in the structure?
		shape->bounds = NewRec(nshape->bounds[0], nshape->bounds[1], nshape->bounds[2], nshape->bounds[3]); //TODO: Correct order? Are the values width and height?
		shape->fillType = VectorFillType_None;
		if (nshape->fill.type == NSVG_PAINT_COLOR) { shape->fillType = VectorFillType_Solid; }
		//TODO: Handle NSVG_PAINT_LINEAR_GRADIENT
		//TODO: Handle NSVG_PAINT_RADIAL_GRADIENT
		shape->fillColor = NewColorSvg(nshape->fill.color);
		shape->fillOddSections = (nshape->fillRule == NSVG_FILLRULE_NONZERO);
		shape->strokeType = VectorStrokeType_None;
		if (nshape->stroke.type == NSVG_PAINT_COLOR) { shape->strokeType = VectorStrokeType_Solid; }
		//TODO: Handle NSVG_PAINT_LINEAR_GRADIENT
		//TODO: Handle NSVG_PAINT_RADIAL_GRADIENT
		shape->strokeColor = NewColorSvg(nshape->stroke.color);
		shape->strokeWidth = nshape->strokeWidth;
		shape->miterLimit = nshape->miterLimit;
		
		u64 numPaths = 0;
		NSVGpath* npath = nshape->paths;
		while (npath != nullptr) { numPaths++; npath = npath->next; }
		CreateVarArray(&shape->paths, memArena, sizeof(VectorPath_t), numPaths);
		
		npath = nshape->paths;
		while (npath != nullptr)
		{
			VectorPath_t* path = VarArrayAdd(&shape->paths, VectorPath_t);
			NotNull(path);
			ClearPointer(path);
			u64 numEdges = (npath->npts > 0) ? (npath->npts-1) / 3 : 0;
			CreateVarArray(&path->edges, memArena, sizeof(VectorEdge_t), numEdges);
			for (u64 eIndex = 0; eIndex < numEdges; eIndex++)
			{
				VectorEdge_t* edge = VarArrayAdd(&path->edges, VectorEdge_t);
				NotNull(edge);
				ClearPointer(edge);
				edge->start    = NewVec2(npath->pts[eIndex*2*3 + 0], npath->pts[eIndex*2*3 + 1]);
				edge->control1 = NewVec2(npath->pts[eIndex*2*3 + 2], npath->pts[eIndex*2*3 + 3]);
				edge->control2 = NewVec2(npath->pts[eIndex*2*3 + 4], npath->pts[eIndex*2*3 + 5]);
				edge->end      = NewVec2(npath->pts[eIndex*2*3 + 6], npath->pts[eIndex*2*3 + 7]);
			}
			npath = npath->next;
		}
		
		nshape = nshape->next;
	}
	
	OC_ScratchEnd(scratch);
	
	//TODO: Turn the assertions into false returns!
	return true;
}

void DebugPrintVectorImg(const VectorImg_t* image, DbgLevel_t dbgLevel)
{
	PrintLineAt(dbgLevel, "%llu shape%s:", image->shapes.length, Plural(image->shapes.length, "s"));
	VarArrayLoop(&image->shapes, sIndex)
	{
		VarArrayLoopGet(VectorShape_t, shape, &image->shapes, sIndex);
		PrintLineAt(dbgLevel, "\tShape[%llu]: \"%s\" %llu path%s", sIndex, shape->name.chars, shape->paths.length, Plural(shape->paths.length, "s"));
		VarArrayLoop(&shape->paths, pIndex)
		{
			VarArrayLoopGet(VectorPath_t, path, &shape->paths, pIndex);
			PrintLineAt(dbgLevel, "\t\tPath[%llu]: %llu edge%s", pIndex, path->edges.length, Plural(path->edges.length, "s"));
			VarArrayLoop(&path->edges, eIndex)
			{
				VarArrayLoopGet(VectorEdge_t, edge, &path->edges, eIndex);
				PrintLineAt(dbgLevel, "\t\t\tEdge[%llu]: (%g,%g) (%g,%g) (%g,%g) (%g,%g)",
					pIndex,
					edge->start.x, edge->start.y,
					edge->control1.x, edge->control1.y,
					edge->control2.x, edge->control2.y,
					edge->end.x, edge->end.y
				);
				
			}
		}
	}
}

void TraceVectorPath(const VectorPath_t* path, v2 position, v2 scale)
{
	VarArrayLoop(&path->edges, eIndex)
	{
		VarArrayLoopGet(VectorEdge_t, edge, &path->edges, eIndex);
		v2 start    = position + Vec2Multiply(edge->start,    scale);
		v2 control1 = position + Vec2Multiply(edge->control1, scale);
		v2 control2 = position + Vec2Multiply(edge->control2, scale);
		v2 end      = position + Vec2Multiply(edge->end,      scale);
		if (eIndex == 0) { OC_MoveTo(start); }
		if (start == control1) { OC_QuadraticTo(control2, end); }
		else if (control2 == end) { OC_QuadraticTo(control1, end); }
		else { OC_CubicTo(control1, control2, end); }
	}
	OC_ClosePath();
}

void RenderVectorImg(const VectorImg_t* image, v2 position, v2 scale, Color_t color)
{
	VarArrayLoop(&image->shapes, sIndex)
	{
		VarArrayLoopGet(VectorShape_t, shape, &image->shapes, sIndex);
		VarArrayLoop(&shape->paths, pIndex)
		{
			VarArrayLoopGet(VectorPath_t, path, &shape->paths, pIndex);
			
			if (shape->fillType == VectorFillType_Solid)
			{
				TraceVectorPath(path, position, scale);
				OC_SetImage(OC_ImageNil());
				OC_SetColor(shape->fillColor);
				OC_Fill();
			}
			
			if (shape->strokeType == VectorStrokeType_Solid)
			{
				TraceVectorPath(path, position, scale);
				OC_SetImage(OC_ImageNil());
				OC_SetColor(shape->strokeColor);
				OC_SetWidth(shape->strokeWidth);
				OC_SetMaxJointExcursion(shape->miterLimit);
				OC_Stroke();
			}
		}
	}
}
