/*
File:   pig_model_armature.cpp
Author: Taylor Robbins
Date:   08\29\2023
Description: 
	** Holds the functions that operate on ModelArmature_t and related types
*/

void DestroyModelArmatureBone(ModelArmature_t* armature, ModelArmatureBone_t* bone)
{
	NotNull2(armature, bone);
	FreeString(armature->allocArena, &bone->name);
	VarArrayLoop(&bone->partNames, nIndex)
	{
		VarArrayLoopGet(MyStr_t, partName, &bone->partNames, nIndex);
		FreeString(armature->allocArena, partName);
	}
	FreeVarArray(&bone->partNames);
	ClearPointer(bone);
}
void DestroyModelArmature(ModelArmature_t* armature)
{
	NotNull(armature);
	if (armature->allocArena != nullptr)
	{
		FreeString(armature->allocArena, &armature->name);
		VarArrayLoop(&armature->bones, bIndex)
		{
			VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
			DestroyModelArmatureBone(armature, bone);
		}
		FreeVarArray(&armature->bones);
	}
	ClearPointer(armature);
}

void CreateModelArmature(ModelArmature_t* armature, MemArena_t* memArena, u64 numBonesExpected = 0)
{
	NotNull(armature);
	ClearPointer(armature);
	armature->allocArena = memArena;
	armature->nextBoneId = 1;
	armature->name = MyStr_Empty;
	CreateVarArray(&armature->bones, memArena, sizeof(ModelArmatureBone_t), numBonesExpected);
}

ModelArmatureBone_t* FindArmatureBoneByName(ModelArmature_t* armature, MyStr_t boneName, bool ignoreCase = true)
{
	VarArrayLoop(&armature->bones, bIndex)
	{
		VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
		if ((ignoreCase && StrEqualsIgnoreCase(bone->name, boneName)) ||
			(!ignoreCase && StrEquals(bone->name, boneName)))
		{
			return bone;
		}
	}
	return nullptr;
}
const ModelArmatureBone_t* FindArmatureBoneByName(const ModelArmature_t* armature, MyStr_t boneName, bool ignoreCase = true) //const-variant
{
	return (const ModelArmatureBone_t*)FindArmatureBoneByName((ModelArmature_t*)armature, boneName, ignoreCase);
}

ModelArmatureBone_t* FindArmatureBoneById(ModelArmature_t* armature, u64 id)
{
	VarArrayLoop(&armature->bones, bIndex)
	{
		VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
		if (bone->id == id) { return bone; }
	}
	return nullptr;
}
const ModelArmatureBone_t* FindArmatureBoneById(const ModelArmature_t* armature, u64 id) //const-variant
{
	return (const ModelArmatureBone_t*)FindArmatureBoneById((ModelArmature_t*)armature, id);
}

ModelArmatureBone_t* FindArmatureBoneForPartName(ModelArmature_t* armature, MyStr_t partName, bool ignoreCase = true)
{
	VarArrayLoop(&armature->bones, bIndex)
	{
		VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
		VarArrayLoop(&bone->partNames, nIndex)
		{
			VarArrayLoopGet(MyStr_t, namePntr, &bone->partNames, nIndex);
			if ((ignoreCase && StrEqualsIgnoreCase(*namePntr, partName)) ||
				(!ignoreCase && StrEquals(*namePntr, partName)))
			{
				return bone;
			}
		}
	}
	return nullptr;
}
const ModelArmatureBone_t* FindArmatureBoneForPartName(const ModelArmature_t* armature, MyStr_t partName, bool ignoreCase = true) //const-variant
{
	return (const ModelArmatureBone_t*)FindArmatureBoneForPartName((ModelArmature_t*)armature, partName, ignoreCase);
}

void ModelArmatureClearBonePntrs(ModelArmature_t* armature)
{
	armature->rootBone = nullptr;
	VarArrayLoop(&armature->bones, bIndex)
	{
		VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
		bone->parentBone = nullptr;
	}
}
void ModelArmatureUpdateBonePntrs(ModelArmature_t* armature)
{
	armature->rootBone = FindArmatureBoneById(armature, armature->rootBoneId);
	VarArrayLoop(&armature->bones, bIndex)
	{
		VarArrayLoopGet(ModelArmatureBone_t, bone, &armature->bones, bIndex);
		bone->parentBone = FindArmatureBoneById(armature, bone->parentBoneId);
	}
}

ModelArmatureBone_t* AddModelArmatureBone(ModelArmature_t* armature, MyStr_t name)
{
	NotNull(armature);
	NotNull(armature->allocArena);
	ModelArmatureBone_t* result = VarArrayAdd(&armature->bones, ModelArmatureBone_t);
	if (result == nullptr) { return result; }
	ClearPointer(result);
	result->id = armature->nextBoneId;
	armature->nextBoneId++;
	result->name = AllocString(armature->allocArena, &name);
	CreateVarArray(&result->partNames, armature->allocArena, sizeof(MyStr_t));
	ModelArmatureClearBonePntrs(armature);
	return result;
}

void CopyModelArmature(ModelArmature_t* target, const ModelArmature_t* source, MemArena_t* memArena)
{
	NotNull3(target, source, memArena);
	CreateModelArmature(target, memArena, source->bones.length);
	target->name = AllocString(memArena, &source->name);
	VarArrayLoop(&source->bones, bIndex)
	{
		const VarArrayLoopGet(ModelArmatureBone_t, sourceBone, &source->bones, bIndex);
		ModelArmatureBone_t* targetBone = VarArrayAdd(&target->bones, ModelArmatureBone_t);
		NotNull(targetBone);
		MyMemCopy(targetBone, sourceBone, sizeof(ModelArmatureBone_t));
		targetBone->name = AllocString(memArena, &sourceBone->name);
		targetBone->parentBone = nullptr;
		CreateVarArray(&targetBone->partNames, memArena, sizeof(MyStr_t), sourceBone->partNames.length);
		VarArrayLoop(&sourceBone->partNames, nIndex)
		{
			const VarArrayLoopGet(MyStr_t, sourcePartName, &sourceBone->partNames, nIndex);
			MyStr_t* targetPartName = VarArrayAdd(&targetBone->partNames, MyStr_t);
			NotNull(targetPartName);
			*targetPartName = AllocString(memArena, sourcePartName);
		}
	}
	target->nextBoneId = source->nextBoneId;
	target->rootBoneId = source->rootBoneId;
	ModelArmatureUpdateBonePntrs(target);
}

// Returns true if all parts had a bone to drive them
bool AddArmatureToModel(Model_t* model, const ModelArmature_t* armature, ProcessLog_t* log = nullptr)
{
	NotNull2(model, armature);
	NotNull(model->allocArena);
	Assert(!model->hasArmature);
	CopyModelArmature(&model->armature, armature, model->allocArena);
	
	bool foundBonesForAllParts = true;
	VarArrayLoop(&model->parts, pIndex)
	{
		VarArrayLoopGet(ModelPart_t, modelPart, &model->parts, pIndex);
		ModelArmatureBone_t* bone = FindArmatureBoneForPartName(&model->armature, modelPart->name);
		if (bone != nullptr)
		{
			modelPart->boneId = bone->id;
		}
		else
		{
			foundBonesForAllParts = false;
			if (log != nullptr)
			{
				LogPrintLine_W(log, "Couldn't find bone for model part \"%.*s\" (%llu bones in armature)", StrPrint(modelPart->name), model->armature.bones.length);
				log->hadWarnings = true;
			}
		}
	}
	model->hasArmature = true;
	
	return foundBonesForAllParts;
}
