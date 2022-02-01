/*
File:   pig_model.cpp
Author: Taylor Robbins
Date:   01\11\2022
Description: 
	** Holds functions that help us manage and manipulate Model_t structures
*/

// +--------------------------------------------------------------+
// |                      Destroy and Create                      |
// +--------------------------------------------------------------+
void DestroyModel(Model_t* model)
{
	NotNull(model);
	AssertIf((model->materials.length > 0 || model->parts.length > 0), model->allocArena != nullptr);
	VarArrayLoop(&model->materials, mIndex)
	{
		VarArrayLoopGet(ModelMaterial_t, material, &model->materials, mIndex);
		FreeString(model->allocArena, &material->name);
		DestroyTexture(&material->diffuseTexture);
	}
	VarArrayLoop(&model->parts, pIndex)
	{
		VarArrayLoopGet(ModelPart_t, part, &model->parts, pIndex);
		DestroyVertBuffer(&part->buffer);
	}
	FreeVarArray(&model->materials);
	FreeVarArray(&model->parts);
	ClearPointer(model);
}

Model_t CreateModelFromObjModelData(ObjModelData_t* objData, MemArena_t* memArena)
{
	NotNull(objData);
	NotNull(memArena);
	
	u64 numParts = 0;
	{
		u64 currentMaterialIndex = 0;
		u64 startFaceIndex = 0;
		VarArrayLoop(&objData->objects, oIndex)
		{
			VarArrayLoopGet(ObjModelDataObject_t, objObject, &objData->objects, oIndex);
			VarArrayLoop(&objObject->faces, fIndex)
			{
				VarArrayLoopGet(ObjModelDataFace_t, objFace, &objObject->faces, fIndex);
				if (oIndex == 0 && fIndex == 0)
				{
					currentMaterialIndex = objFace->materialIndex;
				}
				bool endOfFaces = (fIndex+1 == objObject->faces.length);
				if (objFace->materialIndex != currentMaterialIndex || endOfFaces)
				{
					u64 endFaceIndex = (endOfFaces ? fIndex+1 : fIndex);
					if (endFaceIndex != startFaceIndex)
					{
						numParts++;
					}
					currentMaterialIndex = objFace->materialIndex;
					startFaceIndex = endFaceIndex;
				}
			}
		}
	}
	DebugAssert(numParts > 0);
	
	Model_t result = {};
	result.allocArena = memArena;
	CreateVarArray(&result.materials, memArena, sizeof(ModelMaterial_t), objData->materials.length);
	CreateVarArray(&result.parts, memArena, sizeof(ModelPart_t), numParts);
	
	VarArrayLoop(&objData->materials, mIndex)
	{
		VarArrayLoopGet(ObjModelDataMaterial_t, objMaterial, &objData->materials, mIndex);
		ModelMaterial_t* material = VarArrayAdd(&result.materials, ModelMaterial_t);
		NotNull(material);
		ClearPointer(material);
		material->materialIndex = mIndex;
		material->name = AllocString(memArena, &objMaterial->name);
		material->specularExponent = objMaterial->specularExponent;
		material->ambientColor     = objMaterial->ambientColor;
		material->diffuseColor     = objMaterial->diffuseColor;
		material->specularColor    = objMaterial->specularColor;
		if (!IsStrEmpty(objMaterial->diffuseMapPath))
		{
			MyStr_t texturePath = TempPrintStr("%s/Textures/%.*s", RESOURCE_FOLDER_MODELS, objMaterial->diffuseMapPath.length, objMaterial->diffuseMapPath.pntr);
			bool loadSuccess = LoadTexture(memArena, &material->diffuseTexture, texturePath, false, true);
			DebugAssertAndUnused(loadSuccess, loadSuccess);
		}
	}
	
	{
		u64 currentMaterialIndex = 0;
		u64 startFaceIndex = 0;
		VarArrayLoop(&objData->objects, oIndex)
		{
			VarArrayLoopGet(ObjModelDataObject_t, objObject, &objData->objects, oIndex);
			VarArrayLoop(&objObject->faces, fIndex)
			{
				VarArrayLoopGet(ObjModelDataFace_t, objFace, &objObject->faces, fIndex);
				if (oIndex == 0 && fIndex == 0)
				{
					currentMaterialIndex = objFace->materialIndex;
				}
				bool endOfFaces = (fIndex+1 == objObject->faces.length);
				if (objFace->materialIndex != currentMaterialIndex || endOfFaces)
				{
					u64 endFaceIndex = (endOfFaces ? fIndex+1 : fIndex);
					if (endFaceIndex != startFaceIndex)
					{
						ModelPart_t* newPart = VarArrayAdd(&result.parts, ModelPart_t);
						NotNull(newPart);
						ClearPointer(newPart);
						newPart->partIndex = result.parts.length-1;
						newPart->materialIndex = currentMaterialIndex;
						TempPushMark();
						u64 numVertices = (endFaceIndex - startFaceIndex) * 3;
						Vertex3D_t* vertices = AllocArray(TempArena, Vertex3D_t, numVertices);
						NotNull(vertices);
						u64 numVertsFilled = 0;
						for (u64 faceIndex = startFaceIndex; faceIndex < endFaceIndex; faceIndex++)
						{
							VarArrayLoopGet(ObjModelDataFace_t, partFace, &objObject->faces, faceIndex);
							for (u64 triVertIndex = 0; triVertIndex < 3; triVertIndex++)
							{
								vertices[(faceIndex - startFaceIndex)*3 + triVertIndex].position = *VarArrayGetHard(&objData->vertices,  partFace->vertexIndices[triVertIndex],   v3);
								vertices[(faceIndex - startFaceIndex)*3 + triVertIndex].texCoord = *VarArrayGetHard(&objData->texCoords, partFace->texCoordIndices[triVertIndex], v2);
								vertices[(faceIndex - startFaceIndex)*3 + triVertIndex].normal   = *VarArrayGetHard(&objData->normals,   partFace->normalIndices[triVertIndex],   v3);
								vertices[(faceIndex - startFaceIndex)*3 + triVertIndex].color = ToVec4(White); //TODO: Should we get the color from somewhere?
								numVertsFilled++;
							}
						}
						Assert(numVertsFilled == numVertices);
						bool createBufferSuccess = CreateVertBuffer3D(memArena, &newPart->buffer, false, numVertices, vertices, false);
						Assert(createBufferSuccess);
						TempPopMark();
					}
					currentMaterialIndex = objFace->materialIndex;
					startFaceIndex = endFaceIndex;
				}
			}
		}
	}
	
	return result;
}
