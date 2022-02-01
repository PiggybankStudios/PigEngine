/*
File:   pig_none_app_state.h
Author: Taylor Robbins
Date:   01\19\2022
*/

#ifndef _PIG_NONE_APP_STATE_H
#define _PIG_NONE_APP_STATE_H

struct NoneAppState_t
{
	bool initialized;
	
	box testBoxes[100];
	v3 cameraPos;
	v2 cameraRotation;
	v3 testPos0;
	v3 testPos1;
	v3 testPos2;
	v3 testPos3;
	
	//physics
	q3Scene physScene;
	r64 physAccumulator;
	
	RandomSeries_t series1;
	RandomSeries_t series2;
	
	u64 randIndex;
	v2 randomV2[10000];
	r32 randomR32[10000];
	u32 randomU8Counts[256];
	u32 randomU16Counts[65536];
	
	Model_t whompsFortressModel;
	
	SvgData_t testSvg;
	// u64 testSvgNumVertices;
	// v2* testSvgVertices;
	// u64 testSvgNumTriangles;
	// Triangle2D_t* testSvgTriangles;
	VarArray_t testTriVerts;
	VarArray_t testTriTriangles;
	
	BezierPathPart_t ellipseCurve;
	
	u64 numPlottedPoints;
	v2* plottedPoints;
};

#endif //  _PIG_NONE_APP_STATE_H
