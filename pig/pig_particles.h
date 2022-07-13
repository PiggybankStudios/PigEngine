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

#endif //  _PIG_PARTICLES_H
