/*
File:   pig_particles_collection.cpp
Author: Taylor Robbins
Date:   07\14\2022
Description: 
	** Holds functions that help us use the PartsPrefabCollection_t that we loaded from a
	** file in order to spawn particles
*/

// +--------------------------------------------------------------+
// |                     Parts Param Helpers                      |
// +--------------------------------------------------------------+
i32 PartsParamI32ChooseValue(RandomSeries_t* random, const PartsParamI32_t* param)
{
	if (param->minValue != param->maxValue)
	{
		//TODO: Take param->distroFunc into account
		return GetRandI32(random, param->minValue, param->maxValue+1);
	}
	return param->minValue;
}
r32 PartsParamR32ChooseValue(RandomSeries_t* random, const PartsParamR32_t* param)
{
	if (!BasicallyEqualR32(param->minValue, param->maxValue))
	{
		//TODO: Take param->distroFunc into account
		return GetRandR32(random, param->minValue, param->maxValue);
	}
	return param->minValue;
}
v2 PartsParamVec2ChooseValue(RandomSeries_t* random, const PartsParamVec2_t* param)
{
	if (!Vec2BasicallyEqual(param->minValue, param->maxValue))
	{
		//TODO: Take param->distroFunc into account
		if (param->isPolar)
		{
			r32 direction = GetRandR32(random, param->minValue.x, param->maxValue.x);
			r32 speed = GetRandR32(random, param->minValue.y, param->maxValue.y);
			return Vec2FromAngle(direction, speed);
		}
		else
		{
			return NewVec2(GetRandR32(random, param->minValue.x, param->maxValue.x), GetRandR32(random, param->minValue.y, param->maxValue.y));
		}
	}
	return param->minValue;
}
v2i PartsParamVec2iChooseValue(RandomSeries_t* random, const PartsParamVec2i_t* param)
{
	if (param->minValue != param->maxValue)
	{
		//TODO: Take param->distroFunc into account
		return NewVec2i(GetRandI32(random, param->minValue.x, param->maxValue.x+1), GetRandI32(random, param->minValue.y, param->maxValue.y+1));
	}
	return param->minValue;
}
Color_t PartsParamColorChooseValue(RandomSeries_t* random, const PartsParamColor_t* param)
{
	if (param->minColor != param->maxColor || param->pickFromList)
	{
		//TODO: Take param->distroFunc into account
		if (param->pickFromList)
		{
			return param->possibleColors[GetRandI32(random, 0, (i32)param->numPossibleColors)];
		}
		else
		{
			return ColorLerp(param->minColor, param->maxColor, GetRandR32(random, 0, 1));
		}
	}
	return param->minColor;
}

// +--------------------------------------------------------------+
// |                        Prefab Helpers                        |
// +--------------------------------------------------------------+
PartsPrefab_t* FindPartsPrefab(PartsPrefabCollection_t* collection, MyStr_t name, u64* prefabIndexOut = nullptr)
{
	NotNull(collection);
	NotNullStr(&name);
	VarArrayLoop(&collection->prefabs, pIndex)
	{
		VarArrayLoopGet(PartsPrefab_t, prefab, &collection->prefabs, pIndex);
		if (StrEqualsIgnoreCase(prefab->name, name))
		{
			if (prefabIndexOut != nullptr) { *prefabIndexOut = pIndex; }
			return prefab;
		}
	}
	return nullptr;
}
PartsPrefab_t* FindPartsPrefab(PartsPrefabCollection_t* collection, const char* nameNullTerm)
{
	return FindPartsPrefab(collection, NewStr(nameNullTerm));
}
const PartsPrefab_t* FindPartsPrefab(const PartsPrefabCollection_t* collection, MyStr_t name) //const variant
{
	return (const PartsPrefab_t*)FindPartsPrefab((PartsPrefabCollection_t*)collection, name);
}
const PartsPrefab_t* FindPartsPrefab(const PartsPrefabCollection_t* collection, const char* nameNullTerm) //const variant
{
	return (const PartsPrefab_t*)FindPartsPrefab((PartsPrefabCollection_t*)collection, NewStr(nameNullTerm));
}

void FreeCopiedPartsPrefab(MemArena_t* allocArena, PartsPrefab_t* prefab)
{
	NotNull2(allocArena, prefab);
	VarArrayLoop(&prefab->bursts, bIndex)
	{
		VarArrayLoopGet(PartsBurst_t, burst, &prefab->bursts, bIndex);
		FreeString(allocArena, &burst->name);
		FreeString(allocArena, &burst->type.name);
	}
	FreeVarArray(&prefab->bursts);
	FreeString(allocArena, &prefab->name);
	FreeMem(allocArena, prefab, sizeof(PartsPrefab_t));
}

PartsPrefab_t* CopyPartsPrefab(MemArena_t* memArena, const PartsPrefab_t* prefab)
{
	NotNull2(memArena, prefab);
	
	PartsPrefab_t* result = AllocStruct(memArena, PartsPrefab_t);
	NotNull(result);
	MyMemCopy(result, prefab, sizeof(PartsPrefab_t));
	
	result->name = AllocString(memArena, &result->name);
	NotNullStr(&result->name);
	CreateVarArray(&result->bursts, memArena, sizeof(PartsBurst_t), prefab->bursts.length);
	
	VarArrayLoop(&prefab->bursts, bIndex)
	{
		VarArrayLoopGet(PartsBurst_t, burst, &prefab->bursts, bIndex);
		PartsBurst_t* newBurst = VarArrayAdd(&result->bursts, PartsBurst_t);
		NotNull(newBurst);
		MyMemCopy(newBurst, burst, sizeof(PartsBurst_t));
		newBurst->name = AllocString(memArena, &newBurst->name);
		NotNullStr(&newBurst->name);
		newBurst->type.name = AllocString(memArena, &newBurst->type.name);
		NotNullStr(&newBurst->type.name);
	}
	
	return result;
}

// +--------------------------------------------------------------+
// |                              Do                              |
// +--------------------------------------------------------------+
#define DO_PARTS_PREFAB_CALLBACK_DEF(functionName) void functionName(const PartsPrefab_t* prefab, const PartsBurst_t* burst, u64 burstIndex, u64 burstPartIndex, Particle_t* newParticle, void* context)
typedef DO_PARTS_PREFAB_CALLBACK_DEF(DoPartsPrefabCallback_f);

u64 DoPartsPrefab(ParticleEngine_t* engine, RandomSeries_t* random, const PartsPrefab_t* prefab, v2 position, v2 spawnScale, r32 scale, r32 rotation, v2 addVelocity, DoPartsPrefabCallback_f* callback = nullptr, void* userPntr = nullptr)
{
	NotNull2(engine, random);
	if (prefab == nullptr) { return 0; }
	
	u64 blueNoiseSeed = GetRandU64(random);
	
	u64 numPartsSpawned = 0;
	VarArrayLoop(&prefab->bursts, bIndex)
	{
		VarArrayLoopGet(PartsBurst_t, burst, &prefab->bursts, bIndex);
		PartsType_t* type = &burst->type;
		
		u64 partCount = PartsParamI32ChooseValue(random, &burst->count);
		
		for (u64 pIndex = 0; pIndex < partCount; pIndex++)
		{
			v2 partPos = Vec2_Zero;
			switch (burst->shape)
			{
				case PartsEmissionShape_Point: partPos = Vec2_Zero; break;
				case PartsEmissionShape_Square:
				{
					rec spawnRec = NewRecCentered(Vec2_Zero, spawnScale);
					partPos = GetBlueNoisePosInRec(blueNoiseSeed, spawnRec, partCount, pIndex);
				} break;
				case PartsEmissionShape_Circle:
				{
					r32 angle = GetRandR32(random, 0, TwoPi32);
					r32 radius = GetRandR32(random, 0, 1.0f);
					radius = SqrtR32(radius);
					partPos = Vec2Multiply(Vec2FromAngle(angle, radius)/2, spawnScale);
				} break;
				case PartsEmissionShape_Line:
				{
					partPos = NewVec2(GetRandR32(random, 0, spawnScale.x), 0);
				} break;
				default: DebugAssert(false); break;
			}
			partPos = Vec2Rotate(partPos, rotation);
			partPos += position;
			
			Texture_t* texturePntr = GetPointer(&type->texture);
			SpriteSheet_t* sheetPntr = GetPointer(&type->sheet);
			
			Particle_t* newPart = nullptr;
			if (texturePntr != nullptr)
			{
				newPart = AllocParticle(engine,
					type->flags, //flags
					texturePntr, //sheet
					type->sourceRec, //sourceRec
					PartsParamR32ChooseValue(random, &type->depth), //depth
					PartsParamR32ChooseValue(random, &type->lifeSpan), //lifeSpan
					partPos, //position
					Vec2Rotate(PartsParamVec2ChooseValue(random, &type->velocity), rotation) + addVelocity, //velocity
					Vec2Rotate(PartsParamVec2ChooseValue(random, &type->acceleration), rotation), //acceleration
					PartsParamR32ChooseValue(random, &type->velocityDamping), //velocityDamping
					PartsParamR32ChooseValue(random, &type->rotation) + rotation, //rotation
					PartsParamR32ChooseValue(random, &type->rotationVelocity), //rotationVelocity
					PartsParamR32ChooseValue(random, &type->rotationAcceleration), //rotationAcceleration
					PartsParamR32ChooseValue(random, &type->rotVelocityDamping), //rotVelocityDamping
					PartsParamR32ChooseValue(random, &type->scaleStart) * scale, //scaleStart
					PartsParamR32ChooseValue(random, &type->scaleEnd) * scale, //scaleEnd
					ColorTransparent(PartsParamColorChooseValue(random, &type->colorStart), PartsParamR32ChooseValue(random, &type->alphaStart)), //colorStart
					ColorTransparent(PartsParamColorChooseValue(random, &type->colorEnd), PartsParamR32ChooseValue(random, &type->alphaEnd)) //colorEnd
				);
			}
			else if (sheetPntr != nullptr)
			{
				newPart = AllocParticle(engine,
					type->flags, //flags
					sheetPntr, //sheet
					PartsParamVec2iChooseValue(random, &type->frame), //sheetFrame
					PartsParamR32ChooseValue(random, &type->frameTime), //frameTime
					PartsParamI32ChooseValue(random, &type->frameCount), //numFrames
					PartsParamI32ChooseValue(random, &type->frameOffset), //frameOffset
					PartsParamR32ChooseValue(random, &type->depth), //depth
					PartsParamR32ChooseValue(random, &type->lifeSpan), //lifeSpan
					partPos, //position
					Vec2Rotate(PartsParamVec2ChooseValue(random, &type->velocity), rotation) + addVelocity, //velocity
					Vec2Rotate(PartsParamVec2ChooseValue(random, &type->acceleration), rotation), //acceleration
					PartsParamR32ChooseValue(random, &type->velocityDamping), //velocityDamping
					PartsParamR32ChooseValue(random, &type->rotation) + rotation, //rotation
					PartsParamR32ChooseValue(random, &type->rotationVelocity), //rotationVelocity
					PartsParamR32ChooseValue(random, &type->rotationAcceleration), //rotationAcceleration
					PartsParamR32ChooseValue(random, &type->rotVelocityDamping), //rotVelocityDamping
					PartsParamR32ChooseValue(random, &type->scaleStart) * scale, //scaleStart
					PartsParamR32ChooseValue(random, &type->scaleEnd) * scale, //scaleEnd
					ColorTransparent(PartsParamColorChooseValue(random, &type->colorStart), PartsParamR32ChooseValue(random, &type->alphaStart)), //colorStart
					ColorTransparent(PartsParamColorChooseValue(random, &type->colorEnd), PartsParamR32ChooseValue(random, &type->alphaEnd)) //colorEnd
				);
			}
			else
			{
				PrintLine_W("Warning: Particle type \"%.*s\" doesn't have sheet or texture defined for it!", type->name.length, type->name.pntr);
			}
			
			if (newPart != nullptr)
			{
				if (callback != nullptr)
				{
					callback(prefab, burst, bIndex, pIndex, newPart, userPntr);
				}
				numPartsSpawned++;
			}
		}
	}
	
	return numPartsSpawned;
}
