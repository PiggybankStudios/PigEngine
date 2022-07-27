/*
File:   pig_particles.cpp
Author: Taylor Robbins
Date:   07\12\2022
Description: 
	** Particles are a bunch of images\shapes that each have a bunch of parameters
	** that affect their position and display parameters over some lifespan.
	** This file contains all the functions and helpers that allow us to easily
	** create and manage particle effects.
*/

void FreeParticleEngine(ParticleEngine_t* engine)
{
	NotNull(engine);
	FreeBktArray(&engine->particles);
	ClearPointer(engine);
}

void CreateParticleEngine(ParticleEngine_t* engine, MemArena_t* memArena, u64 maxParticles = 0, u64 minBucketSize = 64, u64 initialSizeRequirement = 0)
{
	NotNull2(engine, memArena);
	ClearPointer(engine);
	engine->allocArena = memArena;
	engine->maxParticles = maxParticles;
	engine->numPartsAlive = 0;
	CreateBktArray(&engine->particles, memArena, sizeof(Particle_t), minBucketSize, initialSizeRequirement);
}

void UpdateParticleEngine(ParticleEngine_t* engine, r64 elapsedMs)
{
	NotNull(engine);
	
	if (elapsedMs > 0)
	{
		BktArrayBucket_t* bucket = engine->particles.firstBucket;
		u64 pIndex = 0;
		while (bucket != nullptr && pIndex < engine->particles.length)
		{
			Particle_t* part = (Particle_t*)bucket->items;
			for (u64 bktIndex = 0; bktIndex < bucket->numItems && pIndex < engine->particles.length; bktIndex++)
			{
				if (IsFlagSet(part->flags, ParticleFlag_Alive))
				{
					part->age += (r32)elapsedMs;
					if (part->age >= part->lifeSpan)
					{
						FlagUnset(part->flags, ParticleFlag_Alive);
						engine->numPartsAlive--;
					}
					else
					{
						if (part->movementType == PartMovementType_Default)
						{
							//TODO: We should be taking elapsedMs into account here!
							part->velocity += part->acceleration;
							part->velocity = (1.0f - (part->velocityDamping / 100.0f)) * part->velocity;
							part->position += part->velocity;
							part->rotationVelocity += part->rotationAcceleration;
							part->rotationVelocity *= (1.0f - (part->rotVelocityDamping / 100.0f));
							part->rotation += part->rotationVelocity;
						}
						else if (part->movementType == PartMovementType_None) { /* do nothing */ }
						else { Unimplemented(); }
					}
				}
				pIndex++;
				part++;
			}
			bucket = bucket->next;
		}
	}
}

void RenderParticleEngine(ParticleEngine_t* engine, bool renderNormal, bool renderScreenSpace)
{
	NotNull(engine);
	r32 oldDepth = rc->state.depth;
	
	BktArrayBucket_t* bucket = engine->particles.firstBucket;
	u64 pIndex = 0;
	while (bucket != nullptr && pIndex < engine->particles.length)
	{
		Particle_t* part = (Particle_t*)bucket->items;
		for (u64 bktIndex = 0; bktIndex < bucket->numItems && pIndex < engine->particles.length; bktIndex++)
		{
			if (IsFlagSet(part->flags, ParticleFlag_Alive))
			{
				if ((renderNormal && !IsFlagSet(part->flags, ParticleFlag_ScreenSpace)) || (renderScreenSpace && IsFlagSet(part->flags, ParticleFlag_ScreenSpace)))
				{
					r32 partLifeLerp = ClampR32(part->age / part->lifeSpan, 0.0f, 1.0f);
					Color_t partColor = ColorLerp(part->colorStart, part->colorEnd, partLifeLerp);
					r32 partScale = LerpR32(part->scaleStart, part->scaleEnd, partLifeLerp);
					RcSetDepth(part->depth);
					
					Texture_t* texturePntr = GetPointer(&part->texture);
					SpriteSheet_t* sheetPntr = GetPointer(&part->sheet);
					if (texturePntr != nullptr)
					{
						obb2 partBounds = NewObb2D(part->position, texturePntr->size * partScale, part->rotation);
						RcBindTexture1(texturePntr);
						RcDrawTexturedObb2(partBounds, partColor, part->sourceRec);
					}
					else if (sheetPntr != nullptr)
					{
						i32 partFrameIndex = ((part->frameTime > 0) ? ((FloorR32i(part->age / part->frameTime) + (i32)part->frameOffset) % part->numFrames) : 0);
						i32 numFramesX = sheetPntr->numFramesX;
						v2i partFrame = NewVec2i(
							(part->sheetFrame.x + partFrameIndex) % numFramesX,
							part->sheetFrame.y + ((part->sheetFrame.x + partFrameIndex) / numFramesX)
						);
						obb2 partBounds = NewObb2D(part->position, ToVec2(sheetPntr->frameSize) * partScale, part->rotation);
						RcBindSpriteSheet(sheetPntr);
						RcDrawSheetFrame(partFrame, partBounds, partColor, IsFlagSet(part->flags, ParticleFlag_FlipX), IsFlagSet(part->flags, ParticleFlag_FlipY));
					}
					else { DebugAssertMsg(false, "Particle didn't have a sprite sheet or texture to back it!"); }
				}
			}
			pIndex++;
			part++;
		}
		bucket = bucket->next;
	}
	
	RcSetDepth(oldDepth);
}

void ParticleEngineClear(ParticleEngine_t* engine)
{
	NotNull(engine);
	for (u64 pIndex = 0; pIndex < engine->particles.length; pIndex++)
	{
		Particle_t* part = BktArrayGet(&engine->particles, Particle_t, pIndex);
		FlagUnset(part->flags, ParticleFlag_Alive);
	}
	engine->numPartsAlive = 0;
}

Particle_t* AllocParticle_(ParticleEngine_t* engine, u8 flags)
{
	NotNull(engine);
	
	Particle_t* result = nullptr;
	for (u64 pIndex = 0; pIndex < engine->particles.length; pIndex++)
	{
		Particle_t* part = BktArrayGet(&engine->particles, Particle_t, pIndex);
		if (!IsFlagSet(part->flags, ParticleFlag_Alive))
		{
			result = part;
			break;
		}
	}
	
	if (engine->maxParticles != 0 && engine->particles.length >= engine->maxParticles && result == nullptr)
	{
		return nullptr;
	}
	
	if (result == nullptr)
	{
		result = BktArrayAdd(&engine->particles, Particle_t);
		if (result == nullptr) { return result; }
	}
	
	ClearPointer(result);
	result->flags = (flags | ParticleFlag_Alive);
	result->age = 0;
	
	engine->numPartsAlive++;
	return result;
}
//Leave lifeSpan=0 to have it auto-calculate based on frameTime and numFrames
Particle_t* AllocParticle(ParticleEngine_t* engine, u8 flags, const SpriteSheet_t* sheet,
	v2i sheetFrame, r32 frameTime, u32 numFrames, u32 frameOffset,
	r32 depth, r32 lifeSpan,
	v2 position, v2 velocity, v2 acceleration, r32 velocityDamping,
	r32 rotation, r32 rotationVelocity, r32 rotationAcceleration, r32 rotVelocityDamping,
	r32 scaleStart, r32 scaleEnd,
	Color_t colorStart, Color_t colorEnd)
{
	NotNull(sheet);
	
	Particle_t* result = AllocParticle_(engine, flags);
	if (result == nullptr) { return result; }
	
	if (lifeSpan == 0) { lifeSpan = frameTime * numFrames; }
	result->sheet                = GetSpriteSheetHandle(sheet);
	result->sheetFrame           = sheetFrame;
	result->frameTime            = frameTime;
	result->numFrames            = numFrames;
	result->frameOffset          = frameOffset;
	result->position             = position;
	result->velocity             = velocity;
	result->acceleration         = acceleration;
	result->rotation             = rotation;
	result->rotationVelocity     = rotationVelocity;
	result->rotationAcceleration = rotationAcceleration;
	result->velocityDamping      = velocityDamping;
	result->rotVelocityDamping   = rotVelocityDamping;
	result->scaleStart           = scaleStart;
	result->scaleEnd             = scaleEnd;
	result->colorStart           = colorStart;
	result->colorEnd             = colorEnd;
	result->depth                = depth;
	result->lifeSpan             = lifeSpan;
	result->movementType         = PartMovementType_Default;
	
	return result;
}
Particle_t* AllocParticle(ParticleEngine_t* engine, u8 flags, const Texture_t* texture,
	rec sourceRec,
	r32 depth, r32 lifeSpan,
	v2 position, v2 velocity, v2 acceleration, r32 velocityDamping,
	r32 rotation, r32 rotationVelocity, r32 rotationAcceleration, r32 rotVelocityDamping,
	r32 scaleStart, r32 scaleEnd,
	Color_t colorStart, Color_t colorEnd)
{
	NotNull(texture);
	
	Particle_t* result = AllocParticle_(engine, flags);
	if (result == nullptr) { return result; }
	
	if (RecBasicallyEqual(sourceRec, Rec_Zero)) { sourceRec = NewRec(Vec2_Zero, texture->size); }
	result->texture              = GetTextureHandle(texture);
	result->sourceRec            = sourceRec;
	result->position             = position;
	result->velocity             = velocity;
	result->acceleration         = acceleration;
	result->rotation             = rotation;
	result->rotationVelocity     = rotationVelocity;
	result->rotationAcceleration = rotationAcceleration;
	result->velocityDamping      = velocityDamping;
	result->rotVelocityDamping   = rotVelocityDamping;
	result->scaleStart           = scaleStart;
	result->scaleEnd             = scaleEnd;
	result->colorStart           = colorStart;
	result->colorEnd             = colorEnd;
	result->depth                = depth;
	result->lifeSpan             = lifeSpan;
	result->movementType         = PartMovementType_Default;
	
	return result;
}
