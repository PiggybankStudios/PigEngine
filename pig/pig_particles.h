/*
File:   pig_particles.h
Author: Taylor Robbins
Date:   07\12\2022
*/

#ifndef _PIG_PARTICLES_H
#define _PIG_PARTICLES_H

typedef enum
{
	ParticleFlag_Alive           = 0x00000001,
	ParticleFlag_ScreenSpace     = 0x00000002,
	ParticleFlag_Align           = 0x00000004,
	ParticleFlag_FlipX           = 0x00000008,
	ParticleFlag_FlipY           = 0x00000010,
	// ParticleFlag_Unused          = 0x00000020,
	// ParticleFlag_Unused          = 0x00000040,
	// ParticleFlag_Unused          = 0x00000080,
	ParticleFlag_None            = 0x00000000,
} ParticleFlags_t;

typedef enum
{
	PartMovementType_Default = 0,
	PartMovementType_None,
	PartMovementType_NumTypes,
} PartMovementType_t;
const char* GetPartMovementTypeStr(PartMovementType_t movementType)
{
	switch (movementType)
	{
		case PartMovementType_Default: return "Default";
		case PartMovementType_None: return "None";
		default: return "Unknown";
	}
}

struct Particle_t
{
	u32 flags;
	PartMovementType_t movementType;
	TextureHandle_t texture;
	SpriteSheetHandle_t sheet;
	union
	{
		struct
		{
			rec sourceRec;
		};
		struct
		{
			v2i sheetFrame;
			r32 frameTime;
			u32 numFrames;
			u32 frameOffset;
		};
	};
	v2 position;
	v2 velocity;
	v2 acceleration;
	r32 velocityDamping;
	r32 rotation;
	r32 rotationVelocity;
	r32 rotationAcceleration;
	r32 rotVelocityDamping;
	Color_t colorStart;
	Color_t colorEnd;
	r32 scaleStart;
	r32 scaleEnd;
	r32 depth;
	r32 age; //ms
	r32 lifeSpan; //ms
};

struct ParticleEngine_t
{
	MemArena_t* allocArena;
	u64 maxParticles;
	u64 numPartsAlive;
	BktArray_t particles; //Particle_t
};


// +--------------------------------------------------------------+
// |                      Deserialized Data                       |
// +--------------------------------------------------------------+
enum PartsDistroFunc_t
{
	PartsDistroFunc_Random = 0,
	PartsDistroFunc_BlueRandom,
	PartsDistroFunc_Spread,
	PartsDistroFunc_NumFuncs,
};
const char* GetPartsDistroFuncStr(PartsDistroFunc_t func)
{
	switch (func)
	{
		case PartsDistroFunc_Random:     return "Random";
		case PartsDistroFunc_BlueRandom: return "BlueRandom";
		case PartsDistroFunc_Spread:     return "Spread";
		default: return "Unknown";
	}
}

struct PartsParamR32_t
{
	PartsDistroFunc_t distroFunc;
	r32 minValue;
	r32 maxValue;
};

struct PartsParamI32_t
{
	PartsDistroFunc_t distroFunc;
	i32 minValue;
	i32 maxValue;
};

struct PartsParamVec2_t
{
	bool isPolar;
	PartsDistroFunc_t distroFunc;
	v2 minValue; //in polar mode, x is direction, y is speed
	v2 maxValue; //in polar mode, x is direction, y is speed
};

struct PartsParamVec2i_t
{
	PartsDistroFunc_t distroFunc;
	v2i minValue;
	v2i maxValue;
};

#define MAX_PARTS_PARAM_COLOR_POSSIBLE_COLORS 16
struct PartsParamColor_t
{
	bool pickFromList;
	u64 numPossibleColors;
	PartsDistroFunc_t distroFunc;
	union
	{
		Color_t possibleColors[MAX_PARTS_PARAM_COLOR_POSSIBLE_COLORS];
		struct
		{
			Color_t minColor;
			Color_t maxColor;
		};
	};
};

enum PartsTypeFlag_t
{
	PartsTypeFlag_None             = 0x00,
	PartsTypeFlag_AddMouseVelocity = 0x01,
	PartsTypeFlag_NumFlags = 2,
};
const char* GetPartsTypeFlagStr(u8 flag)
{
	switch (flag)
	{
		case PartsTypeFlag_None:             return "None";
		case PartsTypeFlag_AddMouseVelocity: return "AddMouseVelocity";
		default: return "Unknown";
	}
}

struct PartsType_t
{
	u64 id;
	MyStr_t name;
	
	u8 flags;
	SpriteSheetHandle_t sheet;
	TextureHandle_t texture;
	rec sourceRec;
	PartsParamVec2i_t frame;
	PartsParamR32_t frameTime;
	PartsParamI32_t frameCount, frameOffset;
	PartsParamR32_t lifeSpan, depth;
	PartsParamR32_t scaleStart, scaleEnd;
	PartsParamR32_t alphaStart, alphaEnd;
	PartsParamR32_t rotation, rotationVelocity, rotationAcceleration;
	PartsParamR32_t velocityDamping, rotVelocityDamping;
	PartsParamVec2_t velocity, acceleration;
	PartsParamColor_t colorStart, colorEnd;
};

enum PartsEmissionShape_t
{
	PartsEmissionShape_Point = 0,
	PartsEmissionShape_Square,
	PartsEmissionShape_Circle,
	PartsEmissionShape_Line,
	PartsEmissionShape_NumShapes,
};
const char* GetPartsEmissionShapeStr(PartsEmissionShape_t shape)
{
	switch (shape)
	{
		case PartsEmissionShape_Point:  return "Point";
		case PartsEmissionShape_Square: return "Square";
		case PartsEmissionShape_Circle: return "Circle";
		case PartsEmissionShape_Line:   return "Line";
		default: return "Unknown";
	}
}

struct PartsBurst_t
{
	u64 id;
	MyStr_t name;
	
	PartsEmissionShape_t shape;
	PartsParamI32_t count;
	PartsType_t type;
};

struct PartsPrefab_t
{
	u64 id;
	MyStr_t name;
	VarArray_t bursts; //PartsBurst_t
};

struct PartsPrefabCollection_t
{
	u64 nextTypeId;
	u64 nextBurstId;
	u64 nextPrefabId;
	
	MemArena_t* allocArena;
	VarArray_t prefabs; //PartsPrefab_t
};

#endif //  _PIG_PARTICLES_H
