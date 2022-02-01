/*
File:   pig_none_app_state.cpp
Author: Taylor Robbins
Date:   01\19\2022
Description: 
	** This is the special AppState that is provided by the engine
	** and serves as a dumping ground for all of our tests when working
	** on the engine, rather than a game
*/

#define PHYSICS_ENABLED false

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void ReloadWhompsFortress()
{
	MyStr_t filePath = NewStr(RESOURCE_FOLDER_MODELS "/WF.obj");
	PlatFileContents_t objFile;
	if (!plat->ReadFileContents(filePath, &objFile))
	{
		DebugAssert(false);
		return;
	}
	
	TempPushMark();
	ProcessLog_t objParseLog;
	CreateProcessLog(&objParseLog, Kilobytes(16), TempArena, mainHeap);
	SetProcessLogFilePath(&objParseLog, filePath);
	
	ObjModelData_t objData = {};
	PerfTime_t parseStartTime = plat->GetPerfTime();
	if (TryDeserObjFile(NewStr(objFile.length, objFile.chars), &objParseLog, &objData, mainHeap))
	{
		PerfTime_t parseEndTime = plat->GetPerfTime();
		ReverseObjModelDataTriangleOrder(&objData);
		PrintLine_I("Parsed obj file successfully! (%.2lfms)", plat->GetPerfTimeDiff(&parseStartTime, &parseEndTime));
		DestroyModel(&noneState->whompsFortressModel);
		noneState->whompsFortressModel = CreateModelFromObjModelData(&objData, mainHeap);
		PrintLine_D("There are %llu parts in the model", noneState->whompsFortressModel.parts.length);
		FreeObjModelData(&objData);
	}
	else
	{
		PrintLine_E("Failed to parse obj file: %s", GetTryDeserializeObjFileErrorStr((TryDeserializeObjFileError_t)objParseLog.errorCode));
	}
	if (objParseLog.hadWarnings || objParseLog.hadErrors) { DumpProcessLog(&objParseLog, "OBJ Parse Log", DbgLevel_Warning); }
	TempPopMark();
	FreeProcessLog(&objParseLog);
}

void ReloadTestSvg()
{
	PlatFileContents_t svgFile = {};
	if (!plat->ReadFileContents(NewStr(RESOURCE_FOLDER_VECTOR "/test.svg"), &svgFile))
	{
		NotifyWrite_E("Failed to open svg file");
		return;
	}
	
	TempPushMark();
	ProcessLog_t svgDeserLog = {};
	CreateProcessLog(&svgDeserLog, Kilobytes(16), TempArena, mainHeap);
	
	SvgData_t svgData = {};
	
	if (TryDeserSvgFile(NewStr(svgFile.length, svgFile.chars), &svgDeserLog, &svgData, mainHeap))
	{
		NotifyWrite_I("Deserialized svg file successfully!");
		
		#if 0
		DumpProcessLog(&svgDeserLog, "SVG Parse Log");
		PrintLine_O("Found %llu groups in svg file:", svgData.groups.length);
		VarArrayLoop(&svgData.groups, gIndex)
		{
			VarArrayLoopGet(SvgGroup_t, group, &svgData.groups, gIndex);
			PrintLine_N("  Group[%llu]: \"%.*s\" (%llu shape%s, id \"%.*s\")",
				gIndex,
				group->label.length, group->label.pntr,
				group->shapes.length, (group->shapes.length == 1) ? "" : "s",
				group->idStr.length, group->idStr.pntr
			);
			VarArrayLoop(&group->shapes, sIndex)
			{
				VarArrayLoopGet(SvgShape_t, shape, &group->shapes, sIndex);
				PrintLine_I("    Shape[%llu]: %s (id \"%.*s\")", sIndex, GetSvgShapeTypeStr(shape->type), shape->idStr.length, shape->idStr.pntr);
				if (shape->type == SvgShapeType_Circle)
				{
					PrintLine_I("      Center: (%g, %g)", shape->circle.center.x, shape->circle.center.y);
					PrintLine_I("      Radius: %g", shape->circle.radius);
					PrintLine_I("      Color:  (%02X, %02X, %02X, %02X)", shape->fill.color.r, shape->fill.color.g, shape->fill.color.b, shape->fill.color.a);
					PrintLine_I("      Stroke: %s (%02X, %02X, %02X, %02X)", GetSvgStrokeStyleStr(shape->stroke.style), shape->stroke.color.r, shape->stroke.color.g, shape->stroke.color.b, shape->stroke.color.a);
				}
				else if (shape->type == SvgShapeType_Path)
				{
					PrintLine_I("      Parts:       %llu", shape->path.value.parts.length);
					PrintLine_I("      SubPaths:    %llu", GetNumSubPathsInBezierPath(&shape->path.value));
					PrintLine_I("      NumVertices: %llu", GetNumVerticesInBezierPath(&shape->path.value));
				}
			}
		}
		#endif
		
		FreeSvgData(&noneState->testSvg);
		MyMemCopy(&noneState->testSvg, &svgData, sizeof(SvgData_t));
	}
	else
	{
		NotifyPrint_E("Failed to deserialize svg file: %s%s",
			((svgDeserLog.errorCode == TryDeserSvgFileError_XmlParsingError) ? "" : GetTryDeserSvgFileErrorStr((TryDeserSvgFileError_t)svgDeserLog.errorCode)),
			((svgDeserLog.errorCode == TryDeserSvgFileError_XmlParsingError) ? GetXmlParsingErrorStr(svgDeserLog.xmlParsingError) : "")
		);
	}
	
	if (svgDeserLog.hadWarnings || svgDeserLog.hadErrors) { DumpProcessLog(&svgDeserLog, "SVG Parse Log", DbgLevel_Warning); }
	
	FreeProcessLog(&svgDeserLog);
	plat->FreeFileContents(&svgFile);
	TempPopMark();
}

void* Q3AllocCallback(i32 size)
{
	return AllocMem(mainHeap, size);
}
void Q3FreeCallback(void* memory)
{
	FreeMem(mainHeap, memory);
}

q3Body* AddTestRigidbodyBox(v3 position, v3 size, v3 velocity, quat rotation, bool staticBody)
{
	q3BodyDef bodyDefinition;
	bodyDefinition.position.Set(position.x, position.y, position.z);
	bodyDefinition.bodyType = (staticBody ? eStaticBody : eDynamicBody);
	bodyDefinition.axis.Set(rotation.axis.x, rotation.axis.y, rotation.axis.z);
	bodyDefinition.angle = rotation.angle;
	bodyDefinition.linearVelocity.Set(velocity.x, velocity.y, velocity.z);
	q3Body* newBody = noneState->physScene.CreateBody(bodyDefinition);
	
	q3BoxDef boxDefinition;
	boxDefinition.SetRestitution(0);
	q3Transform boxTransform;
	q3Identity(boxTransform);
	boxDefinition.Set(boxTransform, q3Vec3(size.x, size.y, size.z));
	newBody->AddBox(boxDefinition);
	
	return newBody;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializeNoneAppState()
{
	NotNull(noneState);
	ClearPointer(noneState);
	
	CreateVarArray(&noneState->testTriVerts, mainHeap, sizeof(v2));
	CreateVarArray(&noneState->testTriTriangles, mainHeap, sizeof(Triangle2D_t));
	
	for (u64 bIndex = 0; bIndex < ArrayCount(noneState->testBoxes); bIndex++)
	{
		noneState->testBoxes[bIndex].size = Vec3Fill(RandR32(0.2f, 3.0f));
		noneState->testBoxes[bIndex].y = 0;
		noneState->testBoxes[bIndex].x = RandR32(-10, 10);
		noneState->testBoxes[bIndex].z = RandR32(-10, 10);
	}
	
	noneState->ellipseCurve = {};
	noneState->ellipseCurve.type = BezierPathPartType_EllipseArc;
	noneState->ellipseCurve.startPos = NewVec2(800, 800);
	noneState->ellipseCurve.endPos = NewVec2(1000, 800);
	noneState->ellipseCurve.radius = NewVec2(150, 100);
	noneState->ellipseCurve.axisAngle = 0.0f;
	noneState->ellipseCurve.arcFlags = BezierPathPartArcFlag_None;
	
	// +==============================+
	// |         Init Physics         |
	// +==============================+
	#if PHYSICS_ENABLED
	{
		q3AllocFunc = Q3AllocCallback;
		q3FreeFunc = Q3FreeCallback;
		InPlaceNew(q3Scene, &noneState->physScene, (r32)PIG_PHYS_FRAME_TIME_SEC);
		noneState->physAccumulator = 0.0f;
		
		AddTestRigidbodyBox(NewVec3(0, -20, 0), NewVec3(100, 2, 100), Vec3_Zero, Quat_Identity, true);
		AddTestRigidbodyBox(NewVec3(-1, 1, -1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 0, 1, -1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 1, 1, -1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3(-1, 1,  0), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 0, 1,  0), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 1, 1,  0), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3(-1, 1,  1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 0, 1,  1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
		AddTestRigidbodyBox(NewVec3( 1, 1,  1), NewVec3(1, 1, 1), Vec3_Zero, Quat_Identity, false);
	}
	#endif
	
	noneState->initialized = true;
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializeNoneAppState()
{
	NotNull(noneState);
	
	//TODO: Implement me!
	
	ClearPointer(noneState);
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateNoneAppState()
{
	NotNull(noneState);
	
	// +==============================+
	// |  Sprite Sheet Editor Hotkey  |
	// +==============================+
	if (KeyPressed(Key_F6))
	{
		HandleKeyExtended(Key_F6);
		pig->changeAppStateRequested = true;
		pig->newAppState = AppState_SpriteSheetEditor;
	}
	
	// +==============================+
	// |        Update Physics        |
	// +==============================+
	#if PHYSICS_ENABLED
	{
		PerfTime_t physicsStartTime = plat->GetPerfTime();
		noneState->physAccumulator += ElapsedMs;
		while (noneState->physAccumulator >= (PIG_PHYS_FRAME_TIME_SEC * 1000.0))
		{
			noneState->physScene.Step();
			noneState->physAccumulator -= (PIG_PHYS_FRAME_TIME_SEC * 1000.0);
		}
		PerfTime_t physicsEndTime = plat->GetPerfTime();
		pig->physicsSimTimeLastFrame = plat->GetPerfTimeDiff(&physicsStartTime, &physicsEndTime);
	}
	#endif
	
	// +==============================+
	// |    Update Camera Movement    |
	// +==============================+
	v3 cameraWalkForward = Vec3_Zero;
	v3 cameraForward = Vec3_Zero;
	{
		r32 moveSpeed = KeyDown(Key_Shift) ? 0.20f : 0.02f; //meters/frame
		r32 rotationSpeed = ToRadians32(2); //radians/frame
		r32 maxUpDownAngle = HalfPi32 - ToRadians32(1);
		if (KeyDown(Key_Right))
		{
			noneState->cameraRotation.x = AngleFixR32(noneState->cameraRotation.x + rotationSpeed);
		}
		if (KeyDown(Key_Left))
		{
			noneState->cameraRotation.x = AngleFixR32(noneState->cameraRotation.x - rotationSpeed);
		}
		if (KeyDown(Key_Up))
		{
			noneState->cameraRotation.y += rotationSpeed;
			if (noneState->cameraRotation.y > maxUpDownAngle) { noneState->cameraRotation.y = maxUpDownAngle; }
		}
		if (KeyDown(Key_Down))
		{
			noneState->cameraRotation.y -= rotationSpeed;
			if (noneState->cameraRotation.y < -maxUpDownAngle) { noneState->cameraRotation.y = -maxUpDownAngle; }
		}
		cameraWalkForward = NewVec3(CosR32(noneState->cameraRotation.x), 0, SinR32(noneState->cameraRotation.x));
		cameraForward = Vec3From2Angles(noneState->cameraRotation.x, noneState->cameraRotation.y);
		if (KeyDown(Key_W))
		{
			noneState->cameraPos += cameraWalkForward * moveSpeed;
		}
		if (KeyDown(Key_S))
		{
			noneState->cameraPos -= cameraWalkForward * moveSpeed;
		}
		if (KeyDown(Key_D))
		{
			noneState->cameraPos += NewVec3(-cameraWalkForward.z, cameraWalkForward.y, cameraWalkForward.x) * moveSpeed;
		}
		if (KeyDown(Key_A))
		{
			noneState->cameraPos += NewVec3(cameraWalkForward.z, cameraWalkForward.y, -cameraWalkForward.x) * moveSpeed;
		}
		if (KeyDown(Key_E))
		{
			noneState->cameraPos.y += moveSpeed;
		}
		if (KeyDown(Key_Q))
		{
			noneState->cameraPos.y -= moveSpeed;
		}
	}
	
	// +==============================+
	// |            Key_R             |
	// +==============================+
	if (KeyPressed(Key_R))
	{
		// RcLoadBasicResources();
		// Pig_LoadAllTextures();
		Pig_LoadAllVectorImgs();
		// Pig_LoadAllFonts();
		// Pig_LoadAllShaders();
		// LoadTexture(mainHeap, &noneState->skyboxTexture, NewStr("Resources/Textures/skybox.png"), true, true);
		
		// ReloadWhompsFortress();
		// ReloadTestSvg();
		
		#if 0
		CreateRandomSeries(&noneState->series1, RandomSeriesType_LinearCongruential32);
		CreateRandomSeries(&noneState->series2, RandomSeriesType_PermutedCongruential64);
		SeedRandomSeriesU64(&noneState->series1, ProgramTime);
		SeedRandomSeriesU64(&noneState->series2, ProgramTime);
		
		noneState->randIndex = 0;
		MyMemSet(&noneState->randomV2[0],        0x00, sizeof(v2)  * ArrayCount(noneState->randomV2));
		MyMemSet(&noneState->randomR32[0],       0x00, sizeof(r32) * ArrayCount(noneState->randomR32));
		MyMemSet(&noneState->randomU8Counts[0],  0x00, sizeof(u32) * ArrayCount(noneState->randomU8Counts));
		MyMemSet(&noneState->randomU16Counts[0], 0x00, sizeof(u32) * ArrayCount(noneState->randomU16Counts));
		#endif
	}
	
	// +==============================+
	// |          Key_Enter           |
	// +==============================+
	if (KeyPressed(Key_Enter))
	{
		HandleKeyExtended(Key_Enter);
		// PrintLine_D("The debug console has %u lines, %u/%u bytes filled (%.1f%%)%s",
		// 	pig->debugConsole.fifo.numLines,
		// 	pig->debugConsole.fifo.used, pig->debugConsole.fifo.bufferSize,
		// 	(r32)pig->debugConsole.fifo.used / (r32)pig->debugConsole.fifo.bufferSize * 100.0f,
		// 	(pig->debugConsole.hasFifo ? "" : " NO FIFO")
		// );
		
		// PigPerfGraphMark();
		
		// PrintStructureOfVectorImg(&pig->resources.pigVector);
		
		{
			VectorImgLoop_t loop = CreateVectorImgLoop(&pig->resources.pigVector.root, true, true, true);
			while (VectorImgLoop(&loop))
			{
				TempPushMark();
				MyStr_t indentStr = StringRepeat(TempArena, "--", loop.partDepth);
				if (loop.shape != nullptr)
				{
					if (loop.shape->type == VectorImgShapeType_Circle)
					{
						PrintLine_I("%s--Circle[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f) radius %.1f (%.1f, %.1f, %.1f, %.1f)",
							indentStr.pntr, loop.shape->index,
							loop.shape->name.length, loop.shape->name.pntr,
							loop.shape->idStr.length, loop.shape->idStr.pntr,
							loop.shape->circle.center.x, loop.shape->circle.center.y, loop.shape->circle.radius,
							loop.shape->bounds.x, loop.shape->bounds.y, loop.shape->bounds.width, loop.shape->bounds.height
						);
					}
					else if (loop.shape->type == VectorImgShapeType_Rectangle)
					{
						PrintLine_I("%s--Rec[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f) size %.1fx%.1f rot %.1fdeg rounded (%.1f, %.1f) (%.1f, %.1f, %.1f, %.1f)",
							indentStr.pntr, loop.shape->index,
							loop.shape->name.length, loop.shape->name.pntr,
							loop.shape->idStr.length, loop.shape->idStr.pntr,
							loop.shape->rectangle.center.x, loop.shape->rectangle.center.y,
							loop.shape->rectangle.size.width, loop.shape->rectangle.size.height,
							ToDegrees32(loop.shape->rectangle.rotation),
							loop.shape->rectangle.roundedRadius.x, loop.shape->rectangle.roundedRadius.y,
							loop.shape->bounds.x, loop.shape->bounds.y, loop.shape->bounds.width, loop.shape->bounds.height
						);
					}
					else
					{
						PrintLine_I("%s--%s[%llu]: \"%.*s\" ID: \"%.*s\" (%.1f, %.1f, %.1f, %.1f)",
							indentStr.pntr, GetVectorImgShapeTypeStr(loop.shape->type), loop.shape->index,
							loop.shape->name.length, loop.shape->name.pntr,
							loop.shape->idStr.length, loop.shape->idStr.pntr,
							loop.shape->bounds.x, loop.shape->bounds.y, loop.shape->bounds.width, loop.shape->bounds.height
						);
					}
				}
				else
				{
					NotNull(loop.part);
					PrintLine_I("%sPart[%llu,%llu]: \"%.*s\" ID: \"%.*s\" has %llu shape%s %llu child%s (%.1f, %.1f, %.1f, %.1f) (%.1f, %.1f):",
						indentStr.pntr, loop.part->index, loop.part->interleaveIndex,
						loop.part->name.length, loop.part->name.pntr,
						loop.part->idStr.length, loop.part->idStr.pntr,
						loop.part->shapes.length, (loop.part->shapes.length == 1) ? "" : "s",
						loop.part->children.length, (loop.part->children.length == 1) ? "" : "ren",
						loop.part->bounds.x, loop.part->bounds.y, loop.part->bounds.width, loop.part->bounds.height,
						loop.part->origin.x, loop.part->origin.y
					);
				}
				TempPopMark();
			}
		}
		
		#if 0
		if (noneState->testSvg.groups.length > 0)
		{
			if (noneState->testSvgVertices != nullptr) { FreeMem(mainHeap, noneState->testSvgVertices, sizeof(v2) * noneState->testSvgNumVertices); }
			noneState->testSvgNumVertices = 0;
			noneState->testSvgVertices = nullptr;
			if (noneState->testSvgTriangles != nullptr) { FreeMem(mainHeap, noneState->testSvgTriangles, sizeof(Triangle2D_t) * noneState->testSvgNumTriangles); }
			noneState->testSvgNumTriangles = 0;
			noneState->testSvgTriangles = nullptr;
			
			SvgShape_t* targetShape = nullptr;
			VarArrayLoop(&noneState->testSvg.groups, gIndex)
			{
				VarArrayLoopGet(SvgGroup_t, group, &noneState->testSvg.groups, gIndex);
				VarArrayLoop(&group->shapes, sIndex)
				{
					VarArrayLoopGet(SvgShape_t, shape, &group->shapes, sIndex);
					if (shape->type == SvgShapeType_Path && shape->fill.style == SvgFillStyle_Solid)
					{
						targetShape = shape;
						break;
					}
				}
			}
			
			if (targetShape != nullptr)
			{
				PrintLine_D("Found shape \"%.*s\"", targetShape->idStr.length, targetShape->idStr.pntr);
			}
		}
		#endif
		
		#if 0
		SpriteSheet_t* spriteSheet = &pig->resources.vectorIcons64;
		VarArrayLoop(&spriteSheet->frames, fIndex)
		{
			VarArrayLoopGet(SpriteSheetFrame_t, frame, &spriteSheet->frames, fIndex);
			PrintLine_I("Frame[%d, %d]: \"%.*s\"", frame->gridPos.x, frame->gridPos.y, frame->name.length, frame->name.pntr);
			VarArrayLoop(&frame->points, pIndex)
			{
				VarArrayLoopGet(SpriteSheetFramePoint_t, point, &frame->points, pIndex);
				PrintLine_I("  Point[%llu]: (%f, %f) \"%.*s\"", pIndex, point->point.x, point->point.y, point->name.length, point->name.pntr);
			}
		}
		#endif
		
		#if 0
		u8 randomLine = (u8)RandU32(0, 7);
		switch (randomLine)
		{
			case 0: PrintLine_Dx((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Debug");   break;
			case 1: PrintLine_Rx((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Regular"); break;
			case 2: PrintLine_Ix((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Info");    break;
			case 3: PrintLine_Nx((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Notify");  break;
			case 4: PrintLine_Ox((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Other");   break;
			case 5: PrintLine_Wx((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Warning"); break;
			case 6: PrintLine_Ex((RandU32(0, 100) >= 50) ? DbgFlag_Inverted : 0x00, "This is a test of DbgLevel_Error");   break;
			default: Assert(false); break;
		}
		#endif
		
		#if 0
		u8 randomLine = (u8)RandU32(0, 8);
		switch (randomLine)
		{
			case 0: NotifyWrite_D("This is a test of DbgLevel_Debug");   break;
			case 1: NotifyWrite_R("This is a test of DbgLevel_Regular"); break;
			case 2: NotifyWrite_I("This is a test of DbgLevel_Info");    break;
			case 3: NotifyWrite_N("This is a test of DbgLevel_Notify");  break;
			case 4: NotifyWrite_O("This is a test of DbgLevel_Other");   break;
			case 5: NotifyWrite_W("This is a test of DbgLevel_Warning"); break;
			case 6: NotifyWrite_E("This is a test of DbgLevel_Error");   break;
			case 7: NotifyWrite_I("This is a \bcomplicated\b notification. And a really long one too! Hopefully this will wrap onto multiple lines and everything will be nice and dandy sweet, you know?\nAlso I forced a line break there. Hope you \adon't mind\a :P"); break;
			default: Assert(false); break;
		}
		#endif
		
		#if 0
		for (u64 repeatIndex = 0; repeatIndex < 100; repeatIndex++)
		{
			// if (noneState->randIndex < ArrayCount(noneState->randomV2))
			{
				// noneState->randomV2[noneState->randIndex] = NewVec2(GetRandR32(&noneState->series2), GetRandR32(&noneState->series2));
				
				// r32 angle = GetRandR32(&noneState->series2, 0, TwoPi32);
				// r32 distance = SqrtR32(GetRandR32(&noneState->series2, 0, 0.1f));
				// noneState->randomV2[noneState->randIndex] = Vec2_Half + NewVec2(CosR32(angle) * distance, SinR32(angle) * distance);
				
				u8 randomU8 = GetRandU8(&noneState->series2, 0, 131);
				// if (randomU8 >= 128) { MyDebugBreak(); }
				IncrementU32(noneState->randomU8Counts[randomU8]);
				noneState->randIndex++;
			}
		}
		#endif
	}
	
	// +==============================+
	// |            Key_F             |
	// +==============================+
	if (KeyPressedRepeating(Key_F, 1000, 33))
	{
		HandleKey(Key_F);
		// PigPerfGraphMark();
		
		Assert(false);
		
		#if 0
		r64 averageTimeElapsed = 0;
		const u64 numIterations = 10;
		for (u64 iIndex = 0; iIndex < numIterations; iIndex++)
		{
			PerfTime_t startTime = plat->GetPerfTime();
			ReloadWhompsFortress();
			PerfTime_t endTime = plat->GetPerfTime();
			averageTimeElapsed += plat->GetPerfTimeDiff(&startTime, &endTime);
		}
		if (KeyDownRaw(Key_Shift)) { PrintLine_D("Average: %lfms", averageTimeElapsed / numIterations); }
		#endif
	}
	
	// +==============================+
	// |          Play Notes          |
	// +==============================+
	if (false)
	{
		#if 0
		if (KeyPressed(Key_Z)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_A3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_X)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_B3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_C)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_C4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_V)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_D4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_B)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_E4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_N)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_F4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_M)) { PlaySineNote(SoundInstanceHandle_Empty, 0.3f, FREQUENCY_G4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		#elif 1
		if (KeyPressed(Key_Z)) { PlaySawNote(0.3f, FREQUENCY_A3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_X)) { PlaySawNote(0.3f, FREQUENCY_B3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_C)) { PlaySawNote(0.3f, FREQUENCY_C4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_V)) { PlaySawNote(0.3f, FREQUENCY_D4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_B)) { PlaySawNote(0.3f, FREQUENCY_E4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_N)) { PlaySawNote(0.3f, FREQUENCY_F4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_M)) { PlaySawNote(0.3f, FREQUENCY_G4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		#else
		if (KeyPressed(Key_Z)) { PlaySquareNote(0.3f, FREQUENCY_A3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_X)) { PlaySquareNote(0.3f, FREQUENCY_B3, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_C)) { PlaySquareNote(0.3f, FREQUENCY_C4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_V)) { PlaySquareNote(0.3f, FREQUENCY_D4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_B)) { PlaySquareNote(0.3f, FREQUENCY_E4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_N)) { PlaySquareNote(0.3f, FREQUENCY_F4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		if (KeyPressed(Key_M)) { PlaySquareNote(0.3f, FREQUENCY_G4, 400, 200, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut); }
		#endif
	}
	
	// +==============================+
	// |   Handle Left Mouse Button   |
	// +==============================+
	bool retriangulate = false;
	if (MousePressedAndHandle(MouseBtn_Left))
	{
		noneState->testPos0 = noneState->cameraPos;
		#if PHYSICS_ENABLED
		AddTestRigidbodyBox(noneState->cameraPos + cameraForward*1, NewVec3(1, 1, 1), cameraForward*10, Quat_Identity, false);
		#endif
		
		#if 1
		v2* newTestTriPos = VarArrayAdd(&noneState->testTriVerts, v2);
		NotNull(newTestTriPos);
		*newTestTriPos = MousePos;
		if (noneState->testTriVerts.length >= 3) { retriangulate = true; }
		#endif
		
		noneState->ellipseCurve.startPos = MousePos;
	}
	if (MouseDownAndHandle(MouseBtn_Right))
	{
		noneState->testPos1 = noneState->cameraPos;
		noneState->ellipseCurve.endPos = MousePos;
	}
	if (KeyPressed(Key_0))
	{
		HandleKey(Key_0);
		static bool tempBool = false;
		if (tempBool)
		{
			noneState->ellipseCurve.startPos = NewVec2(640.95313f, 97.895837f); //NewVec2(800, 800);
			noneState->ellipseCurve.endPos = NewVec2(600.60417f, 131.6302f); //NewVec2(1000, 800);
			noneState->ellipseCurve.radius = NewVec2(40.348957f, 33.734373f); //NewVec2(150, 100);
			noneState->ellipseCurve.axisAngle = 0.0f;
			noneState->ellipseCurve.arcFlags = BezierPathPartArcFlag_Sweep;
		}
		else
		{
			noneState->ellipseCurve.startPos = NewVec2(600.60417f, 131.6302f); //NewVec2(800, 800);
			noneState->ellipseCurve.endPos = NewVec2(560.25521f, 97.895837f); //NewVec2(1000, 800);
			noneState->ellipseCurve.radius = NewVec2(40.348957f, 33.734373f); //NewVec2(150, 100);
			noneState->ellipseCurve.axisAngle = 0.0f;
			noneState->ellipseCurve.arcFlags = BezierPathPartArcFlag_Sweep;
		}
		tempBool = !tempBool;
	}
	if (KeyDown(Key_Plus))
	{
		HandleKey(Key_Plus);
		if (KeyDownRaw(Key_Shift))
		{
			noneState->ellipseCurve.radius.x += 0.8f;
		}
		else
		{
			noneState->ellipseCurve.axisAngle = ModR32(noneState->ellipseCurve.axisAngle + 0.1f, TwoPi32);
		}
	}
	if (KeyDown(Key_Minus))
	{
		HandleKey(Key_Minus);
		if (KeyDownRaw(Key_Shift))
		{
			noneState->ellipseCurve.radius.x -= 0.8f;
		}
		else
		{
			noneState->ellipseCurve.axisAngle = ModR32(noneState->ellipseCurve.axisAngle + TwoPi32 - 0.1f, TwoPi32);
		}
	}
	
	// +==============================+
	// |            Key_T             |
	// +==============================+
	if (KeyPressed(Key_T))
	{
		HandleKeyExtended(Key_T);
		retriangulate = true;
	}
	
	// +==============================+
	// |            Key_P             |
	// +==============================+
	if (KeyPressed(Key_P))
	{
		HandleKeyExtended(Key_P);
		
		SvgShape_t* targetShape = nullptr;
		VarArrayLoop(&noneState->testSvg.groups, gIndex)
		{
			VarArrayLoopGet(SvgGroup_t, group, &noneState->testSvg.groups, gIndex);
			VarArrayLoop(&group->shapes, sIndex)
			{
				VarArrayLoopGet(SvgShape_t, shape, &group->shapes, sIndex);
				if (shape->type == SvgShapeType_Path && shape->fill.style == SvgFillStyle_Solid && shape->path.vertBufferGenFailed /*&& StrEquals(shape->idStr, "path4526")*/)
				{
					targetShape = shape;
					break;
				}
			}
		}
		
		if (targetShape != nullptr)
		{
			// u64 GetNumSubPathsInBezierPath(const BezierPath_t* path)
			PrintLine_I("Found a failed generation path: \"%.*s\" (Has %llu sub paths)", targetShape->idStr.length, targetShape->idStr.pntr, GetNumSubPathsInBezierPath(&targetShape->path.value));
			u64 numGeneratedVerts = 0;
			v2* generatedVerts = nullptr;
			GenerateVertBufferForSvgPathShape(targetShape, SVG_TRIANGULATION_CURVE_VERT_COUNT, &generatedVerts, &numGeneratedVerts);
			if (generatedVerts != nullptr)
			{
				PrintLine_I("Filling with %llu generated vertices", numGeneratedVerts);
				rec generatedVertsRec = Rec_Zero;
				for (u64 vIndex = 0; vIndex < numGeneratedVerts; vIndex++)
				{
					if (vIndex == 0) { generatedVertsRec = NewRec(generatedVerts[vIndex], Vec2_Zero); }
					else { generatedVertsRec = RecBoth(generatedVertsRec, NewRec(generatedVerts[vIndex], Vec2_Zero)); }
				}
				VarArrayClear(&noneState->testTriVerts);
				generatedVertsRec = RecInflate(generatedVertsRec, 2, 2);
				r32 displayScale = MinR32(ScreenSize.width / generatedVertsRec.width, ScreenSize.height / generatedVertsRec.height);
				for (u64 vIndex = 0; vIndex < numGeneratedVerts; vIndex++)
				{
					v2* newVertSpace = VarArrayAdd(&noneState->testTriVerts, v2);
					NotNull(newVertSpace);
					*newVertSpace = (generatedVerts[vIndex] - generatedVertsRec.topLeft) * displayScale;
				}
				retriangulate = true;
			}
		}
	}
	
	// +==============================+
	// |  Handle Test Triangulation   |
	// +==============================+
	if (retriangulate && noneState->testTriVerts.length >= 3)
	{
		VarArrayClear(&noneState->testTriTriangles);
		
		u64 numIndices = 0;
		u64* indices = Triangulate2DEarClip(TempArena, TempArena, noneState->testTriVerts.length, VarArrayGetHard(&noneState->testTriVerts, 0, v2), &numIndices, true);
		if (indices != nullptr)
		{
			Assert((numIndices % 3) == 0);
			for (u64 iIndex = 0; iIndex+3 <= numIndices; iIndex += 3)
			{
				Triangle2D_t* newTriangle = VarArrayAdd(&noneState->testTriTriangles, Triangle2D_t);
				NotNull(newTriangle);
				ClearPointer(newTriangle);
				newTriangle->vert0 = *VarArrayGetHard(&noneState->testTriVerts, indices[iIndex + 0], v2);
				newTriangle->vert1 = *VarArrayGetHard(&noneState->testTriVerts, indices[iIndex + 1], v2);
				newTriangle->vert2 = *VarArrayGetHard(&noneState->testTriVerts, indices[iIndex + 2], v2);
			}
		}
		else
		{
			NotifyPrint_E("Failed to triangulate %llu vertices", noneState->testTriVerts.length);
		}
	}
	
	// +==============================+
	// |        Key_Backspace         |
	// +==============================+
	if (KeyPressed(Key_Backspace))
	{
		HandleKeyExtended(Key_Backspace);
		VarArrayClear(&noneState->testTriVerts);
		VarArrayClear(&noneState->testTriTriangles);
	}
	
	// +==============================+
	// | Handle Paste Points to Plot  |
	// +==============================+
	if (KeyPressed(Key_V) && KeyDownRaw(Key_Control))
	{
		HandleKeyExtended(Key_V);
		TempPushMark();
		MyStr_t clipboardStr = plat->PasteTextFromClipboard(TempArena);
		if (clipboardStr.length > 0)
		{
			u64 numValidPoints = 0;
			{
				LineParser_t parser = NewLineParser(clipboardStr);
				MyStr_t line = MyStr_Empty;
				while (LineParserGetLine(&parser, &line))
				{
					if (TryParseV2(line, nullptr))
					{
						numValidPoints++;
					}
				}
			}
			
			if (numValidPoints > 0)
			{
				if (noneState->plottedPoints != nullptr)
				{
					FreeMem(mainHeap, noneState->plottedPoints, sizeof(v2) * noneState->numPlottedPoints);
					noneState->plottedPoints = nullptr;
					noneState->numPlottedPoints = 0;
				}
				
				noneState->numPlottedPoints = numValidPoints;
				noneState->plottedPoints = AllocArray(mainHeap, v2, noneState->numPlottedPoints);
				NotNull(noneState->plottedPoints);
				
				LineParser_t parser = NewLineParser(clipboardStr);
				MyStr_t line = MyStr_Empty;
				u64 pointIndex = 0;
				while (LineParserGetLine(&parser, &line))
				{
					v2 newPoint = Vec2_Zero;
					if (TryParseV2(line, &newPoint))
					{
						Assert(pointIndex < noneState->numPlottedPoints);
						noneState->plottedPoints[pointIndex] = newPoint;
						pointIndex++;
					}
				}
				Assert(pointIndex == noneState->numPlottedPoints);
				
				NotifyPrint_I("Plotting %llu point%s...", noneState->numPlottedPoints, (noneState->numPlottedPoints == 1) ? "" : "s");
			}
			else
			{
				NotifyWrite_W("No points found in the clipboard to plot");
			}
		}
		else
		{
			NotifyWrite_W("No text found in the clipboard");
		}
		TempPopMark();
	}
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderNoneAppState()
{
	NotNull(noneState);
	
	RcBegin(pig->currentWindow, &pig->resources.mainShader3D, MonokaiLightBlue);
	
	// +==============================+
	// |         3D Rendering         |
	// +==============================+
	mat4 projectionMatrix = Mat4Perspective(HalfPi32, ScreenSize.width / ScreenSize.height, 0.01f, 200.0f);
	RcSetProjectionMatrix(projectionMatrix);
	mat4 viewMatrix = Mat4LookAt(noneState->cameraPos, noneState->cameraPos + Vec3From2Angles(noneState->cameraRotation.x, noneState->cameraRotation.y), Vec3_Up);
	RcSetViewMatrix(viewMatrix);
	
	// +==============================+
	// |        Unlit Geometry        |
	// +==============================+
	RcBindTexture1(&pig->resources.skyboxTexture);
	RcDrawSkybox(NewBoxCentered(noneState->cameraPos, Vec3Fill(150)), White);
	// RcDrawSkybox(NewBoxCentered(noneState->testPos0, Vec3Fill(10)), White);
	
	#if 0
	u64 numRings = 5;
	if (KeyDownRaw(Key_Space)) { numRings = (u64)RoundR32i(Oscillate(1, 10, 5000)); }
	PrimitiveIndexedVerts_t sphereVerts = GenerateVertsForSphere(NewSphere(0, 0, 0, 10), numRings, 10, false, TempArena);
	v3 previousVert = Vec3_Zero;
	for (u64 vIndex = 0; vIndex < sphereVerts.numVertices; vIndex++)
	{
		RcDrawPoint3D(sphereVerts.vertices[vIndex].position, 0.1f, PureRed);
		if (vIndex > 0)
		{
			// RcDrawLine3D(previousVert, sphereVerts.vertices[vIndex].position, 0.5f, White);
		}
		previousVert = sphereVerts.vertices[vIndex].position;
	}
	for (u64 iIndex = 0; iIndex+3 <= sphereVerts.numIndices; iIndex += 3)
	{
		PrimitiveIndex3D_t* index0 = &sphereVerts.indices[iIndex + 0];
		PrimitiveIndex3D_t* index1 = &sphereVerts.indices[iIndex + 1];
		PrimitiveIndex3D_t* index2 = &sphereVerts.indices[iIndex + 2];
		Color_t colorLight = ((iIndex%6) >= 3) ? PalGreen : PalBlue;
		Color_t colorDark = ((iIndex%6) >= 3) ? PalGreenDarker : PalBlueDarker;
		Color_t colorNormal = ((iIndex%6) >= 3) ? PalYellow : PalOrange;
		// if (index0->index != index1->index)
		// {
		// 	bool highlighted = ((ProgramTime % 3000) >= 0 && (ProgramTime % 3000) < 1000);
		// 	RcDrawLine3D(sphereVerts.vertices[index0->index].position, sphereVerts.vertices[index1->index].position, highlighted ? 10.0f : 1.0f, highlighted ? colorLight : colorDark);
		// }
		// if (index1->index != index2->index)
		// {
		// 	bool highlighted = ((ProgramTime % 3000) >= 1000 && (ProgramTime % 3000) < 2000);
		// 	RcDrawLine3D(sphereVerts.vertices[index1->index].position, sphereVerts.vertices[index2->index].position, highlighted ? 10.0f : 1.0f, highlighted ? colorLight : colorDark);
		// }
		// if (index2->index != index0->index)
		// {
		// 	bool highlighted = ((ProgramTime % 3000) >= 2000 && (ProgramTime % 3000) < 3000);
		// 	RcDrawLine3D(sphereVerts.vertices[index2->index].position, sphereVerts.vertices[index0->index].position, highlighted ? 10.0f : 1.0f, highlighted ? colorLight : colorDark);
		// }
		if (index0->index != index1->index)
		{
			RcDrawTriangle(sphereVerts.vertices[index0->index].position, sphereVerts.vertices[index1->index].position, sphereVerts.vertices[index2->index].position, colorLight);
		}
		RcDrawLine3D(sphereVerts.vertices[index0->index].position, sphereVerts.vertices[index0->index].position + index0->normal * 2, 1, colorNormal);
		RcDrawLine3D(sphereVerts.vertices[index1->index].position, sphereVerts.vertices[index1->index].position + index1->normal * 2, 1, colorNormal);
		RcDrawLine3D(sphereVerts.vertices[index2->index].position, sphereVerts.vertices[index2->index].position + index2->normal * 2, 1, colorNormal);
	}
	#endif
	
	// +==============================+
	// |         Lit Geometry         |
	// +==============================+
	RcBindShader(&pig->resources.phongShader);
	RcSetDynamicUniformVec3("LightDirection", NewVec3(-0.363696f, 0.72739f, -0.58191f));
	RcSetDynamicUniformVec3("AmbientColor", ToVec3(White) * 0.2f);
	RcSetDynamicUniformVec3("DiffuseColor", ToVec3(White) * 0.8f);
	RcSetDynamicUniformVec3("SpecularColor", ToVec3(White) * 0.5f);
	RcSetDynamicUniformR32("SpecularExponent", 32);
	// RcSetCameraPosition(noneState->cameraPos + NewVec3(Oscillate(0, 10, 1000), 0, 0));
	RcSetCameraPosition(noneState->cameraPos);
	
	SphereQuality_t sphereQuality = (SphereQuality_t)ClampI32(RoundR32i(Oscillate(0, SphereQuality_NumQualities, 5000)), 0, (i32)SphereQuality_NumQualities-1);
	RcBindTexture1(&rc->invalidTexture);
	RcDrawTexturedSphere(NewVec3(20 + Oscillate(0, 60, 10000, 0),    0 + Oscillate(0, 60, 10000, 2500 + 0),    0), Oscillate(3, 16, 10000, 0),    PalRed,    sphereQuality);
	RcDrawTexturedSphere(NewVec3(20 + Oscillate(0, 60, 10000, 2000), 0 + Oscillate(0, 60, 10000, 2500 + 2000), 0), Oscillate(3, 16, 10000, 2000), PalOrange, sphereQuality);
	RcDrawTexturedSphere(NewVec3(20 + Oscillate(0, 60, 10000, 4000), 0 + Oscillate(0, 60, 10000, 2500 + 4000), 0), Oscillate(3, 16, 10000, 4000), PalYellow, sphereQuality);
	RcDrawTexturedSphere(NewVec3(20 + Oscillate(0, 60, 10000, 6000), 0 + Oscillate(0, 60, 10000, 2500 + 6000), 0), Oscillate(3, 16, 10000, 6000), PalGreen,  sphereQuality);
	RcDrawTexturedSphere(NewVec3(20 + Oscillate(0, 60, 10000, 8000), 0 + Oscillate(0, 60, 10000, 2500 + 8000), 0), Oscillate(3, 16, 10000, 8000), PalBlue,   sphereQuality);
	
	#if 0
	for (u64 bIndex = 0; bIndex < ArrayCount(noneState->testBoxes); bIndex++)
	{
		obb3 drawBox = ToObb3D(noneState->testBoxes[bIndex]);
		drawBox.rotation = QuatLocalRot(drawBox.rotation, ToVec3(AxisFromIndex(bIndex%3)), Oscillate(0, HalfPi32, 5000));
		drawBox.rotation = QuatLocalRot(drawBox.rotation, ToVec3(AxisFromIndex((bIndex+1)%3)), Oscillate(0, ThirdPi32, 5000, 2500));
		RcBindTexture1(&rc->invalidTexture);
		// RcDrawTexturedBox(noneState->testBoxes[bIndex], GetPredefPalColorByIndex(bIndex));
		RcDrawTexturedObb3D(drawBox, GetPredefPalColorByIndex(bIndex));
	}
	#endif
	
	RcBindModel(&noneState->whompsFortressModel);
	static u64 partIndex = 0;
	if (KeyPressed(Key_PageUp))
	{
		if (partIndex+1 < noneState->whompsFortressModel.parts.length) { partIndex++; }
	}
	if (KeyPressed(Key_PageDown))
	{
		if (partIndex > 0) { partIndex--; }
	}
	if (partIndex > 0)
	{
		RcDrawModelParts(partIndex, 1, Vec3_Zero, Quat_Identity, Vec3Fill(0.02f), White);
	}
	else
	{
		RcDrawModelSimple(NewVec3(10, 0, 0), NewQuat(Vec3_Up, HalfPi32), Vec3Fill(0.02f), White);
		// RcBindTexture1(&rc->invalidTexture);
		// RcBindTexture1(&rc->dotTexture);
		// RcDrawModelCustomTexture(NewVec3(10, 0, 0), NewQuat(Vec3_Up, HalfPi32), Vec3Fill(0.02f), White, NewRec(Vec2_Zero, rc->invalidTexture.size));
	}
	
	#if PHYSICS_ENABLED
	noneState->physScene.Render((q3Render*)&pig->physRenderer);
	#endif
	
	// RcDrawLine3D(noneState->testPos0, pig->testPos1, 0.1f, PalPurple);
	
	// +==============================+
	// |         2D Rendering         |
	// +==============================+
	RcBindShader(&pig->resources.mainShader2D);
	RcSetViewport(NewRec(Vec2_Zero, pig->currentWindow->input.renderResolution));
	RcSetViewMatrix(Mat4_Identity);
	RcClearDepth(1.0f);
	
	// rec logoRec = NewRec(200, 200, ToVec2(pig->resources.pigAnimSheet.frameSize)*2);
	// v2i logoFrame = FrameIndexToSheetIndex(AnimateAndPauseByU64(ProgramTime, 0, pig->resources.pigAnimSheet.frames.length, 800, 1000, 0, true), pig->resources.pigAnimSheet.numFrames);
	// RcBindSpriteSheet(&pig->resources.pigAnimSheet);
	// RcDrawSheetFrame(logoFrame, logoRec, White);
	
	// +==============================+
	// |          Render Svg          |
	// +==============================+
	VarArrayLoop(&noneState->testSvg.groups, gIndex)
	{
		VarArrayLoopGet(SvgGroup_t, group, &noneState->testSvg.groups, gIndex);
		VarArrayLoop(&group->shapes, sIndex)
		{
			VarArrayLoopGet(SvgShape_t, shape, &group->shapes, sIndex);
			// if (!StrEquals(shape->idStr, "path4443")) { continue; } //TODO: Remove me!
			switch (shape->type)
			{
				case SvgShapeType_Circle:
				{
					if (shape->fill.style == SvgFillStyle_Solid)
					{
						RcDrawCircle(shape->circle.center, shape->circle.radius, shape->fill.color, false);
					}
					if (shape->stroke.style == SvgStrokeStyle_Solid)
					{
						TempPushMark();
						BezierPath_t circlePath;
						GenerateBezierPathForCircle(TempArena, shape->circle.center, shape->circle.radius, &circlePath);
						RcDrawBezierPathWithShader(&circlePath, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
						TempPopMark();
					}
				} break;
				case SvgShapeType_Rectangle:
				{
					r32 cornerRadius = MaxR32(shape->rectangle.roundedRadius.x, shape->rectangle.roundedRadius.y);
					if (shape->fill.style == SvgFillStyle_Solid)
					{
						if (shape->rectangle.rotation == 0)
						{
							rec rectangle = NewRecCentered(shape->rectangle.center, shape->rectangle.size);
							if (cornerRadius > 0)
							{
								RcDrawRoundedRectangle(rectangle, cornerRadius, shape->fill.color);
							}
							else
							{
								RcDrawRectangle(rectangle, shape->fill.color);
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
					if (shape->stroke.style == SvgStrokeStyle_Solid)
					{
						TempPushMark();
						if (shape->rectangle.rotation == 0)
						{
							rec rectangle = NewRecCentered(shape->rectangle.center, shape->rectangle.size);
							BezierPath_t rectanglePath;
							GenerateBezierPathForRoundedRectangle(TempArena, rectangle, cornerRadius, &rectanglePath);
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
				case SvgShapeType_Path:
				{
					if (shape->fill.style == SvgFillStyle_Solid)
					{
						if (!shape->path.vertBufferUpToDate && !shape->path.vertBufferGenFailed)
						{
							// AssertIf(StrEquals(shape->idStr, "path4526"), false); //TODO: Remove me!
							shape->path.vertBufferGenFailed = !GenerateVertBufferForSvgPathShape(shape, SVG_TRIANGULATION_CURVE_VERT_COUNT);
							// AssertIf(StrEquals(shape->idStr, "path4526"), !shape->path.vertBufferGenFailed); //TODO: Remove me!
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
					if (shape->stroke.style == SvgStrokeStyle_Solid)
					{
						RcDrawBezierPathWithShader(&shape->path.value, shape->stroke.thickness, shape->stroke.color, Vec2_Zero, Vec2_One);
					}
				} break;
			}
		}
	}
	
	#if 0
	// +==============================+
	// |     Render Bezier Paths      |
	// +==============================+
	{
		TempPushMark();
		
		r32 lerpValue = (MousePos.x / ScreenSize.width);
		
		BezierPath_t circlePath;
		GenerateBezierPathForCircle(TempArena, MousePos, LerpR32(40, 160, lerpValue), &circlePath);
		RcDrawBezierPathWithShader(&circlePath, 4, MonokaiOrange, Vec2_Zero, Vec2_One);
		
		BezierPath_t roundedRecPath;
		GenerateBezierPathForRoundedRectangle(TempArena, NewRecCentered(MousePos, Vec2Fill(LerpR32(20, 120, lerpValue))), LerpR32(5, 15, lerpValue), &roundedRecPath);
		RcDrawBezierPathWithShader(&roundedRecPath, 4, MonokaiRed, Vec2_Zero, Vec2_One);
		
		TempPopMark();
	}
	#endif
	
	// +==============================+
	// |      Render VectorImg_t      |
	// +==============================+
	// u64 AnimateU64(u64 min, u64 max, u64 periodMs, u64 offset = 0)
	// VectorImgPart_t* wowPart = FindVectorImgPartByName(&pig->resources.pigVector.root, NewStr("Nose"));
	// VectorImgPart_t* wowPart = FindVectorImgPartByName(&pig->resources.pigVector.root, NewStr("Tests"));
	// VectorImgPart_t* wowPart = FindVectorImgPartByName(&pig->resources.pigVector.root, NewStr("LeftEar"));
	u64 showingIndex = AnimateU64(0, pig->resources.pigVector.root.children.length, 20000);
	VectorImgPart_t* wowPart = VarArrayGetHard(&pig->resources.pigVector.root.children, showingIndex, VectorImgPart_t);
	if (wowPart != nullptr)
	{
		#if 0
		RcBindFont(&pig->resources.pixelFont, SelectDefaultFontFace());
		rec drawRec = NewRec(MousePos, pig->resources.pigVector.root.bounds.size * Oscillate(4.00f, 5.0f, 10000));
		// rec drawRec = NewRec(MousePos, 400, 400);
		RcDrawVectorImgPartInRec(&pig->resources.pigVector.root, drawRec, White);
		RcDrawRectangleOutline(drawRec, PureYellow, 1);
		
		rec bounds = pig->resources.pigVector.root.bounds;
		v2 scale = Vec2Divide(drawRec.size, bounds.size);
		v2 gridSize = Vec2Fill(10);
		v2 gridBaseOffset = Vec2Multiply(Vec2Floor(Vec2Divide(bounds.topLeft, gridSize)), gridSize);
		// v2 gridBaseDiff = (bounds.topLeft - gridBaseOffset);
		for (r32 yOffset = gridSize.y; gridBaseOffset.y + yOffset < bounds.y + bounds.height; yOffset += gridSize.y)
		{
			r32 screenY = drawRec.y + ((gridBaseOffset.y + yOffset - bounds.y) * scale.y);
			rec rulerRec = NewRec(drawRec.x, screenY, drawRec.width, 1);
			RcDrawRectangle(rulerRec, MonokaiYellow);
			RcDrawTextPrint(rulerRec.topLeft + NewVec2(2, -2), MonokaiWhite, "%.0f", gridBaseOffset.y + yOffset);
		}
		u64 rulerIndex = 0;
		for (r32 xOffset = gridSize.x; gridBaseOffset.x + xOffset < bounds.x + bounds.width; xOffset += gridSize.x)
		{
			r32 screenX = drawRec.x + ((gridBaseOffset.x + xOffset - bounds.x) * scale.x);
			rec rulerRec = NewRec(screenX, drawRec.y, 1, drawRec.height);
			RcDrawRectangle(rulerRec, MonokaiYellow);
			RcDrawTextPrint(rulerRec.topLeft + NewVec2(2, rulerIndex * RcGetLineHeight()/2), MonokaiWhite, "%.0f", gridBaseOffset.x + xOffset);
			rulerIndex++;
		}
		#else
		RcDrawVectorImgPartAt(wowPart, MousePos, Oscillate(0, TwoPi32, 10000), Oscillate(0.25f, 1, 5000), White);
		#endif
	}
	
	// +==============================+
	// |    Render Plotted Points     |
	// +==============================+
	if (noneState->numPlottedPoints > 0 && noneState->plottedPoints != nullptr)
	{
		RcBindFont(&pig->resources.pixelFont, SelectDefaultFontFace());
		rec bounds = Rec_Zero;
		for (u64 pIndex = 0; pIndex < noneState->numPlottedPoints; pIndex++)
		{
			if (pIndex == 0) { bounds = NewRec(noneState->plottedPoints[pIndex], Vec2_Zero); }
			else { bounds = RecExpandToVec2(bounds, noneState->plottedPoints[pIndex]); }
		}
		v2 offset = NewVec2(20, 20);
		r32 scale = 1.0f;
		while (bounds.width > 0 && bounds.height > 0 && bounds.width * (scale+1) < ScreenSize.width*0.75f && bounds.height * (scale+1) < ScreenSize.height*0.75f)
		{
			scale += 1.0f;
		}
		
		for (u64 pIndex = 0; pIndex < noneState->numPlottedPoints; pIndex++)
		{
			v2 currentPoint = offset + (noneState->plottedPoints[pIndex] - bounds.topLeft) * scale;
			v2 nextPoint = offset + (noneState->plottedPoints[(pIndex+1) % noneState->numPlottedPoints] - bounds.topLeft) * scale;
			RcDrawLine(currentPoint, nextPoint, 2, (pIndex == noneState->numPlottedPoints-1) ? White : GetPredefPalColorByIndex(pIndex));
		}
		v2 prevPoint = noneState->plottedPoints[noneState->numPlottedPoints-1];
		u64 numPointsInARow = 0;
		for (u64 pIndex = 0; pIndex < noneState->numPlottedPoints; pIndex++)
		{
			v2 currentPoint = offset + (noneState->plottedPoints[pIndex] - bounds.topLeft) * scale;
			bool mouseIsClose = (Vec2Length(MousePos - currentPoint) < 10);
			RcDrawPoint(currentPoint, 5, MonokaiRed);
			if (Vec2BasicallyEqual(prevPoint, noneState->plottedPoints[pIndex]))
			{
				numPointsInARow++;
			}
			else if (numPointsInARow > 0)
			{
				RcDrawTextPrint(offset + (prevPoint - bounds.topLeft)*scale + NewVec2(4, -2), MonokaiRed, "x%llu", numPointsInARow+1);
				numPointsInARow = 0;
			}
			if (mouseIsClose)
			{
				v2 textPos = currentPoint + NewVec2(4, -2 + numPointsInARow*RcGetLineHeight());
				Vec2Align(&textPos);
				RcDrawTextPrint(textPos, MonokaiWhite, "[%llu] (%g, %g)", pIndex, noneState->plottedPoints[pIndex].x, noneState->plottedPoints[pIndex].y);
			}
			prevPoint = noneState->plottedPoints[pIndex];
		}
		if (numPointsInARow > 0)
		{
			RcDrawTextPrint(offset + (prevPoint - bounds.topLeft)*scale + NewVec2(4, -2), MonokaiRed, "x%llu", numPointsInARow+1);
		}
		rec outlineRec = RecInflate(NewRec(offset, bounds.size * scale), 5, 5);
		RcDrawRectangleOutline(outlineRec, MonokaiYellow, 1, true);
		RcDrawTextPrint(NewVec2(outlineRec.x, outlineRec.y + outlineRec.height + RcGetLineHeight()), MonokaiYellow,
			"x[%g, %g] y[%g, %g] %gx%g",
			bounds.x, bounds.x + bounds.width,
			bounds.y, bounds.y + bounds.height,
			bounds.width, bounds.height
		);
	}
	
	#if 1
	{
		// +==============================+
		// |  Render Test Triangulation   |
		// +==============================+
		RcBindFont(&pig->resources.debugFont, SelectFontFace(18));
		
		VarArrayLoop(&noneState->testTriTriangles, tIndex)
		{
			VarArrayLoopGet(Triangle2D_t, triangle, &noneState->testTriTriangles, tIndex);
			Color_t triColor = GetPredefPalColorByIndex(tIndex);
			RcDrawTriangle2D(triangle->vert0, triangle->vert1, triangle->vert2, triColor);
		}
		bool foundHoverTri = false;
		for (u64 tIndex = noneState->testTriTriangles.length; tIndex > 0; tIndex--)
		{
			VarArrayLoopGet(Triangle2D_t, triangle, &noneState->testTriTriangles, tIndex-1);
			Color_t triColor = GetPredefPalColorByIndex(tIndex-1);
			Color_t textColor = ColorComplementary(triColor);
			if (!foundHoverTri && IsInsideTriangle(triangle->vert0, triangle->vert1, triangle->vert2, MousePos))
			{
				foundHoverTri = true;
				textColor = MonokaiYellow;
				RcDrawLine(triangle->vert0, triangle->vert1, 2, MonokaiYellow);
				RcDrawLine(triangle->vert1, triangle->vert2, 2, MonokaiYellow);
				RcDrawLine(triangle->vert2, triangle->vert0, 2, MonokaiYellow);
			}
			v2 textPos = (triangle->vert0 + triangle->vert1 + triangle->vert2) / 3;
			Vec2Align(&textPos);
			RcDrawTextPrintEx(textPos, textColor, TextAlignment_Center, 0, "\b%llu", tIndex-1);
		}
		if (!foundHoverTri)
		{
			VarArrayLoop(&noneState->testTriVerts, vIndex)
			{
				VarArrayLoopGet(v2, vertPntr, &noneState->testTriVerts, vIndex);
				VarArrayLoopGet(v2, nextVertPntr, &noneState->testTriVerts, ((vIndex+1) % noneState->testTriVerts.length));
				RcDrawLine(*vertPntr, *nextVertPntr, 2, MonokaiYellow);
			}
		}
		VarArrayLoop(&noneState->testTriVerts, vIndex)
		{
			VarArrayLoopGet(v2, vertPntr, &noneState->testTriVerts, vIndex);
			RcDrawPoint(*vertPntr, 3, MonokaiOrange);
			if (KeyDownRaw(Key_Shift)) { RcDrawTextPrint(*vertPntr, MonokaiOrange, "%llu", vIndex); }
		}
		
		if (noneState->testTriVerts.length >= 3)
		{
			v2* verts = VarArrayGetHard(&noneState->testTriVerts, 0, v2);
			bool isClockwise = IsPolygonClockwise(noneState->testTriVerts.length, verts);
			RcDrawTextPrint(verts[0], MonokaiWhite, "%s", isClockwise ? "Clockwise" : "Anti-Clockwise");
		}
	}
	#endif
	
	#if 0
	{
		// +==============================+
		// |  Render Test Ellipse Curve   |
		// +==============================+
		RcDrawPoint(noneState->ellipseCurve.startPos, 3, MonokaiRed);
		RcDrawPoint(noneState->ellipseCurve.endPos, 3, MonokaiMagenta);
		v2 arcCenter = (noneState->ellipseCurve.startPos + noneState->ellipseCurve.endPos) / 2;
		r32 arcAngleStart = 0.0f;
		r32 arcAngleDelta = 0.0f;
		FlagSetTo(noneState->ellipseCurve.arcFlags, BezierPathPartArcFlag_Sweep, KeyDownRaw(Key_Shift));
		FlagSetTo(noneState->ellipseCurve.arcFlags, BezierPathPartArcFlag_Large, KeyDownRaw(Key_Control));
		if (GetEllipseArcCurveCenterAndAngles(noneState->ellipseCurve.startPos, noneState->ellipseCurve.radius, noneState->ellipseCurve.axisAngle, noneState->ellipseCurve.arcFlags, noneState->ellipseCurve.endPos, &arcCenter, &arcAngleStart, &arcAngleDelta))
		{
			// RcDrawEllipseArcWithShader(arcCenter, noneState->ellipseCurve.radius, noneState->ellipseCurve.axisAngle, arcAngleStart, arcAngleDelta, 4, MonokaiOrange);
			RcDrawPoint(arcCenter, 3, MonokaiYellow);
			const u64 numArcPoints = 20;
			for (u64 pIndex = 0; pIndex < numArcPoints; pIndex++)
			{
				r32 time0 = ((r32)(pIndex + 0) / (r32)numArcPoints);
				r32 time1 = ((r32)(pIndex + 1) / (r32)numArcPoints);
				v2 point0 = EllipseArcCurve(arcCenter, noneState->ellipseCurve.radius, noneState->ellipseCurve.axisAngle, arcAngleStart, arcAngleDelta, time0);
				v2 point1 = EllipseArcCurve(arcCenter, noneState->ellipseCurve.radius, noneState->ellipseCurve.axisAngle, arcAngleStart, arcAngleDelta, time1);
				RcDrawLine(point0, point1, 1, ColorLerp(MonokaiPurple, MonokaiBlue, time0));
			}
		}
		RcDrawLine(arcCenter, arcCenter + Vec2FromAngle(noneState->ellipseCurve.axisAngle) * noneState->ellipseCurve.radius.x, 1, MonokaiYellow);
		RcDrawLine(arcCenter, arcCenter + Vec2PerpLeft(Vec2FromAngle(noneState->ellipseCurve.axisAngle)) * noneState->ellipseCurve.radius.y, 1, MonokaiYellow);
		
		v2 angleVecsCenter = NewVec2(1000, 300);
		u64 rotationProgramTime = ProgramTime;
		if (KeyDownRaw(Key_Space)) { rotationProgramTime = pigIn->keyStates[Key_Space].lastChangeTime; }
		r32 testBaseAngle = AnimateBy(rotationProgramTime, 0, TwoPi32, 10000);
		v2 testBaseVec = Vec2FromAngle(testBaseAngle);
		r32 testMouseAngle = AngleFixR32(AtanR32(MousePos.y - angleVecsCenter.y, MousePos.x - angleVecsCenter.x));
		v2 testMouseVec = Vec2FromAngle(testMouseAngle);
		RcDrawLine(angleVecsCenter, angleVecsCenter + testBaseVec * 100, 1, MonokaiYellow);
		RcDrawLine(angleVecsCenter, angleVecsCenter + testMouseVec * 100, 1, MonokaiYellow);
		r32 angleBetween = Vec2AngleBetween(testBaseVec, testMouseVec);
		r32 dotProduct = Vec2Dot(testBaseVec, testMouseVec);
		RcDrawTextPrint(angleVecsCenter + NewVec2(50, 50), MonokaiWhite,
			"(%g, %g) (%g, %g)\n%.3f (%.2f degrees)\nDot: %.3f",
			testBaseVec.x, testBaseVec.y,
			testMouseVec.x, testMouseVec.y,
			angleBetween, ToDegrees32(angleBetween), dotProduct
		);
	}
	#endif
	
	#if 0
	const u64 numEllipseVerts = 20;
	const v2 ellipseStart = NewVec2(600, 200);
	const v2 ellipseRadius = NewVec2(100, 75);
	const v2 ellipseEnd = NewVec2(Oscillate(750, 800, 8000), 200);
	const r32 ellipseAngle = 0; //Oscillate(-QuarterPi32, QuarterPi32, 8000);
	const u8 ellipseFlags = BezierPathPartArcFlag_None;
	for (u64 tIndex = 0; tIndex < numEllipseVerts; tIndex++)
	{
		r32 time0 = ((r32)(tIndex + 0) / (r32)numEllipseVerts);
		r32 time1 = ((r32)(tIndex + 1) / (r32)numEllipseVerts);
		v2 point0 = EllipseArcCurve(ellipseStart, ellipseRadius, ellipseAngle, ellipseFlags, ellipseEnd, time0);
		v2 point1 = EllipseArcCurve(ellipseStart, ellipseRadius, ellipseAngle, ellipseFlags, ellipseEnd, time1);
		RcDrawLine(point0, point1, 2, MonokaiOrange);
	}
	for (u64 tIndex = 0; tIndex < numEllipseVerts; tIndex++)
	{
		r32 time = ((r32)tIndex / (r32)numEllipseVerts);
		v2 point = EllipseArcCurve(ellipseStart, ellipseRadius, ellipseAngle, ellipseFlags, ellipseEnd, time);
		RcDrawPoint(point, 4, MonokaiRed);
	}
	RcDrawPoint(EllipseArcCurve(ellipseStart, ellipseRadius, ellipseAngle, ellipseFlags, ellipseEnd, 1.0f), 4, MonokaiRed);
	RcDrawPoint(ellipseStart, 8, MonokaiBlue);
	RcDrawPoint(ellipseEnd, 8, MonokaiBlue);
	#endif
	
	#if 0
	// +==============================+
	// |   Render Triangulated Path   |
	// +==============================+
	if (noneState->testSvgNumTriangles > 0)
	{
		NotNull(noneState->testSvgTriangles);
		for (u64 tIndex = 0; tIndex < noneState->testSvgNumTriangles; tIndex++)
		{
			Triangle2D_t triangle = noneState->testSvgTriangles[tIndex];
			RcDrawTriangle2D(triangle.vert0, triangle.vert1, triangle.vert2, GetPredefPalColorByIndex(tIndex));
		}
	}
	if (noneState->testSvgNumVertices > 0)
	{
		NotNull(noneState->testSvgVertices);
		v2 prevVert = Vec2_Zero;
		for (u64 vIndex = 0; vIndex < noneState->testSvgNumVertices; vIndex++)
		{
			v2 vertex1 = noneState->testSvgVertices[vIndex];
			v2 vertex2 = noneState->testSvgVertices[(vIndex+1) % noneState->testSvgNumVertices];
			RcDrawLine(vertex1, vertex2, 2.0f, MonokaiLightBlue);
		}
	}
	#endif
	
	// RcBindShader(&pig->resources.bezier4Shader);
	// RcSetDynamicUniformVec2("StartPos", NewVec2(0.1f, 0.2f));
	// RcSetDynamicUniformVec2("Control1", NewVec2(0.6f, 0.2f));
	// RcSetDynamicUniformVec2("Control2", NewVec2(0.2f, 0.7f));
	// RcSetDynamicUniformVec2("EndPos",   NewVec2(0.9f, 0.9f));
	// RcSetDynamicUniformR32("Thickness", 2 / 800.0f);
	// RcDrawRectangle(NewRec(100, 100, 800, 800), White);
	// RcBindShader(&pig->resources.mainShader2D);
	
	// +==============================+
	// |      Render Color Tests      |
	// +==============================+
	#if 0
	for (u64 cIndex = 0; cIndex < NUM_PREDEF_PAL_COLORS; cIndex++)
	{
		Color_t palColor = GetPredefPalColorByIndex(cIndex);
		// for (u64 sIndex = 0; sIndex < 100; sIndex++)
		// {
		// 	r32 saturation = (1.0f - sIndex * 0.01f);
		// 	Color_t color = ColorDesaturate(palColor, saturation);
		// 	RcDrawRectangle(NewRec(10.0f*sIndex, 15.0f*cIndex, 10, 15), color);
		// }
		RcDrawRectangle(NewRec(0, 15.0f*cIndex, 100, 15), palColor);
		RcDrawRectangle(NewRec(100, 15.0f*cIndex, 100, 15), ColorComplementary(palColor));
		RcDrawRectangle(NewRec(200, 15.0f*cIndex, 100, 15), ColorComplementaryOld(palColor));
	}
	#endif
	
	#if 0
	// +==============================+
	// |     Render Random Tests      |
	// +==============================+
	{
		// rec randomRec = NewRec(10, 10, 800, 800);
		// RcDrawRectangle(randomRec, Black);
		// RcDrawRectangleOutline(randomRec, White, 1, true);
		// for (u64 rIndex = 0; rIndex < noneState->randIndex; rIndex++)
		// {
		// 	v2 randomPos = randomRec.topLeft + Vec2Multiply(noneState->randomV2[rIndex], randomRec.size);
		// 	RcDrawRectangle(NewRec(randomPos, 2, 2), PureRed);
		// }
		
		u32 countTotals = 0;
		for (u32 vIndex = 0; vIndex < 256; vIndex++)
		{
			u32 u8ValueCount = noneState->randomU8Counts[vIndex];
			countTotals += u8ValueCount;
		}
		for (u32 vIndex = 0; vIndex < 256; vIndex++)
		{
			u32 u8ValueCount = noneState->randomU8Counts[vIndex];
			rec barRec = NewRec(vIndex * 5.0f, 0, 5, ((r32)u8ValueCount / (r32)(countTotals/64)) * ScreenSize.height);
			RecLayoutTopOf(&barRec, ScreenSize.height);
			RcDrawRectangle(barRec, PalYellow);
		}
	}
	#endif
	
	// plat->DebugReadout(NewStr("We are running! :D"), White, 1.0f);
	// plat->DebugReadout(NewStr("These are debug output lines"), Black, 1.0f);
	// if (wIndex == 0)
	// {
	// 	plat->DebugReadout(NewStr("And they can be colored"), PalYellow, 1.0f);
	// }
	// else
	// {
	// 	plat->DebugReadout(NewStr("And they can be different for each window"), PalRed, 1.0f);
	// }
	
	#if 0
	{
		v2 drawPos = NewVec2(600, 80);
		r32 lineHeight = 0;
		for (u64 fontIndex = 0; fontIndex < RESOURCES_NUM_FONTS; fontIndex++)
		{
			Font_t* font = &pig->resources.fonts[fontIndex];
			ResourceFontMetaInfo_t fontMetaInfo = {};
			VarArrayLoop(&font->faces, fIndex)
			{
				VarArrayLoopGet(FontFace_t, face, &font->faces, fIndex);
				VarArrayLoop(&face->bakes, bIndex)
				{
					VarArrayLoopGet(FontBake_t, bake, &face->bakes, bIndex);
					rec bakeRec = NewRec(drawPos, bake->bitmap.size);
					if (bakeRec.height > lineHeight) { lineHeight = bakeRec.height; }
					Color_t bakeColor = Black;
					Color_t outlineColor = Transparent;
					r32 outlineThickness = 1;
					if (fIndex == font->defaultFaceIndex)
					{
						outlineColor = IsFlagSet(face->flags, FontFaceFlag_IsDefault) ? White : Grey9;
					}
					RcDrawRectangle(bakeRec, bakeColor);
					if (outlineColor.a > 0)
					{
						RcDrawRectangleOutline(bakeRec, outlineColor, outlineThickness, true);
					}
					RcBindTexture1(&bake->bitmap);
					RcDrawTexturedRectangle(bakeRec, White);
					drawPos.x += bakeRec.width + 10;
				}
				drawPos.y += lineHeight + 10;
				drawPos.x = 600;
				lineHeight = 0;
			}
		}
		
		RcBindFont(&pig->resources.debugFont, SelectFontFace(36));
		RcDrawText("Hiragana: \bあなた\bはどうですか Katakana: テイラー ロビンズ", NewVec2(200, 20), White);
		RcDrawText("Hey this is \f\asome \breal\b\a\f text \frendering!\nIn \bConsolas\b\f none\fthe\fless :O", NewVec2(200, 60), White, TextAlignment_Left, MaxR32(0, MousePos.x - 200));
		// RcDrawText("\bHello World!\nTest", NewVec2(200, 60), White, TextAlignment_Left, MaxR32(0, MousePos.x - 200));
		RcDrawRectangleOutline(rc->flowInfo.logicalRec, White, 3, true);
		RcDrawRectangleOutline(rc->flowInfo.renderRec, PalRed, 2, true);
		// RcDrawRectangle(NewRecCentered(rc->flowInfo.endPos, 2, 2), PalRed);
		RcDrawText(
			TempPrintStr("numLines: %u numNewLineCharacters: %u numLineWraps: %u\n"
			"numCharactersRendered: %u numInvalidCharacters: %u numControlCharacters: %u numWhitespaceCharacters: %u",
			rc->flowInfo.numLines, rc->flowInfo.numNewLineCharacters, rc->flowInfo.numLineWraps,
			rc->flowInfo.numCharactersRendered, rc->flowInfo.numInvalidCharacters, rc->flowInfo.numControlCharacters, rc->flowInfo.numWhitespaceCharacters),
			NewVec2(200, 500), White
		);
		// RcDrawText("if (something.everything == true) { int myVar = 100; }", NewVec2(600, 160), White, TextAlignment_Right, MaxR32(0, 600 - MousePos.x));
	}
	#endif
	
	#if 0
	{
		// FontFace_t* face = VarArrayGet(&pig->resources.pixelFont8.faces, 0, FontFace_t);
		// FontBake_t* bake = VarArrayGet(&face->bakes, 0, FontBake_t);
		// FontRange_t* range = VarArrayGet(&bake->ranges, 0, FontRange_t);
		// FontCharInfo_t* charInfo = VarArrayGet(&range->charInfos, 0, FontCharInfo_t);
		// r32 bakeScale = 2.0f;
		// rec bakeRec = NewRec(200, 300, bake->bitmap.size * bakeScale);
		// rec charRec = NewRec(bakeRec.topLeft + (ToVec2(charInfo->sourceRec.topLeft) * bakeScale), ToVec2(charInfo->sourceSize) * bakeScale);
		// RcBindTexture1(&bake->bitmap);
		// RcDrawTexturedRectangle(bakeRec, White);
		// RcDrawRectangleOutline(charRec, MonokaiPurple, 1, true);
		
		RcBindFont(&pig->resources.pixelFont, SelectFontFace(8), 2.0f);
		// RcBindFont(&pig->resources.debugFont, SelectDefaultFontFace());
		// RcBindFont(&pig->resources.largeFont, SelectDefaultFontFace());
		bool doBold = (OscillateSaw(0, 1, 1000) > 0.5f);
		// RcDrawTextPrint(NewVec2(200, 200), MonokaiWhite, "%sHello World%s!", doBold ? "\b" : "", doBold ? "\b" : "");
		// RcDrawTextPrint(NewVec2(200, 200), MonokaiWhite, "%sABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz!%s", doBold ? "\b" : "", doBold ? "\b" : "");
		// RcDrawTextPrint(NewVec2(200, 200), MonokaiWhite, "%sif (&var != \"true\") { *var += -123; } //comment's%s", doBold ? "\b" : "", doBold ? "\b" : "");
		// RcDrawTextPrint(NewVec2(200, 200), MonokaiWhite, "%sABC abc !@#$%%^&*()_+-=~`{}[]:\";'<>,./?|\\%s", doBold ? "\b" : "", doBold ? "\b" : "");
		
		u64 mostRecentKeyTime = 0;
		Key_t mostRecentKey = Key_NumKeys;
		for (u64 keyIndex = 0; keyIndex < Key_NumKeys; keyIndex++)
		{
			u64 keyTime = TimeSince(pigIn->keyStates[keyIndex].lastChangeTime);
			if (pigIn->keyStates[keyIndex].lastChangeTime != 0 && (mostRecentKey == Key_NumKeys || keyTime < mostRecentKeyTime))
			{
				mostRecentKey = (Key_t)keyIndex;
				mostRecentKeyTime = keyTime;
			}
		}
		u64 mostRecentMouseTime = 0;
		MouseBtn_t mostRecentMouseBtn = MouseBtn_NumBtns;
		for (u64 mouseIndex = 0; mouseIndex < MouseBtn_NumBtns; mouseIndex++)
		{
			u64 mouseTime = TimeSince(pigIn->mouseBtnStates[mouseIndex].lastChangeTime);
			if (pigIn->mouseBtnStates[mouseIndex].lastChangeTime != 0 && (mostRecentMouseBtn == MouseBtn_NumBtns || mouseTime < mostRecentMouseTime))
			{
				mostRecentMouseBtn = (MouseBtn_t)mouseIndex;
				mostRecentMouseTime = mouseTime;
			}
		}
		u64 mostRecentControllerTime = 0;
		u8 mostRecentControllerIndex = 0;
		ControllerBtn_t mostRecentControllerBtn = ControllerBtn_NumBtns;
		for (u8 controllerIndex = 0; controllerIndex < MAX_NUM_CONTROLLERS; controllerIndex++)
		{
			const PlatControllerState_t* controller = &pigIn->controllerStates[controllerIndex];
			if (controller->connected)
			{
				for (u64 btnIndex = 0; btnIndex < ControllerBtn_NumBtns; btnIndex++)
				{
					u64 btnTime = TimeSince(controller->btnStates[btnIndex].lastChangeTime);
					if (controller->btnStates[btnIndex].lastChangeTime != 0 && (mostRecentControllerBtn == ControllerBtn_NumBtns || btnTime < mostRecentControllerTime))
					{
						mostRecentControllerIndex = controllerIndex;
						mostRecentControllerBtn = (ControllerBtn_t)btnIndex;
						mostRecentControllerTime = btnTime;
					}
				}
			}
		}
		RcDrawTextPrint(NewVec2(200, 202), Black, "Here is a unicode character %s and %s and %s. And some " INPUT_DISP_CHAR_F1_STR " stuff after", GetDisplayCharStrForKey(mostRecentKey), GetDisplayCharStrForMouseBtn(mostRecentMouseBtn), GetDisplayCharStrForControllerBtn(pigIn->controllerStates[mostRecentControllerIndex].type, mostRecentControllerBtn));
		RcDrawTextPrint(NewVec2(200, 200), MonokaiWhite, "Here is a unicode character %s and %s and %s. And some " INPUT_DISP_CHAR_F1_STR " stuff after", GetDisplayCharStrForKey(mostRecentKey), GetDisplayCharStrForMouseBtn(mostRecentMouseBtn), GetDisplayCharStrForControllerBtn(pigIn->controllerStates[mostRecentControllerIndex].type, mostRecentControllerBtn));
	}
	#endif
	
	// RcDrawRectangle(testRec, IsMouseOverNamed("testRec") ? PalYellow : PalRed);
	
	// RcDrawLine(pigIn->scrollValue*10, MousePos, 5.0f, PalPurple);
	// RcDrawPoint(pigIn->scrollValue*10, 5, PalPink);
	// RcDrawPoint(MousePos, 5, PalPinkLighter);
	// RcDrawPoint((pigIn->scrollValue*10 + MousePos) / 2, 5, PalPinkDarker);
}
