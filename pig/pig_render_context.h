/*
File:   pig_render_context.h
Author: Taylor Robbins
Date:   12\30\2021
*/

#ifndef _PIG_RENDER_CONTEXT_H
#define _PIG_RENDER_CONTEXT_H

#define RC_SCRATCH_BUFFER_SIZE   32 //vertices

struct RenderContextState_t
{
	bool faceCulling;
	bool depthTestEnabled;
	PolygonFillMode_t fillMode;
	r32 lineThickness;
	
	FrameBuffer_t* boundFrameBuffer;
	Shader_t* boundShader;
	VertexArrayObject_t* boundVao;
	Texture_t* boundTexture1;
	Texture_t* boundTexture2;
	SpriteSheet_t* boundSpriteSheet;
	const VertBuffer_t* boundBuffer;
	Model_t* boundModel;
	
	const Font_t* boundFont;
	FontFaceSelector_t faceSelector;
	r32 fontScale;
	FontFlowCallbacks_t flowCallbacks;
	
	rec viewportRec;
	
	mat4 worldMatrix;
	mat4 augmentMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	v3 cameraPosition;
	r32 depth;
	
	rec sourceRec1;
	rec sourceRec2;
	rec maskRectangle;
	v2 shiftVec;
	
	Color_t color1;
	Colorf_t color1f;
	Color_t color2;
	Colorf_t color2f;
	Color_t replaceColors[16];
	Colorf_t replaceColorsf[16];
	
	r32 time;
	i32 count;
	r32 brightness;
	r32 saturation;
	r32 circleRadius;
	r32 circleInnerRadius;
	
	r32 values[ShaderUniform_NumGenericValues];
	
	v2 polygonPlanes[ShaderUniform_NumPolygonPlanes];
};

enum SphereQuality_t
{
	SphereQuality_SuperLow = 0,
	SphereQuality_Low,
	SphereQuality_Default,
	SphereQuality_High,
	SphereQuality_NumQualities,
};
#define RC_SPHERE_BUFFER_RING_COUNTS    { 3, 5, 12, 18 }
#define RC_SPHERE_BUFFER_SEGMENT_COUNTS { 5, 7, 20, 37 }

struct RenderContext_t
{
	const PlatWindow_t* currentWindow;
	
	Texture_t dotTexture;
	Texture_t invalidTexture;
	Texture_t gradientHorizontal;
	Texture_t gradientVertical;
	VertBuffer_t lineBuffer;
	VertBuffer_t squareBuffer;
	VertBuffer_t rightAngleTriBuffer;
	VertBuffer_t equilTriangleBuffer;
	VertBuffer_t voxelOrtho2D;
	VertBuffer_t cubeBuffer;
	VertBuffer_t skyboxBuffer;
	VertBuffer_t scratchBuffer2D; //RC_SCRATCH_BUFFER_SIZE vertices
	VertBuffer_t scratchBuffer3D; //RC_SCRATCH_BUFFER_SIZE vertices
	VertBuffer_t sphereBuffers[SphereQuality_NumQualities];
	
	VarArray_t vertexArrayObjs; //VertexArrayObject_t TODO: Maybe we should make this a linked list so that pointers don't go invalid?
	RenderContextState_t state; //TODO: Add support for pushing/popping the state
	
	FontFlowInfo_t flowInfo;
};

#endif //  _PIG_RENDER_CONTEXT_H
