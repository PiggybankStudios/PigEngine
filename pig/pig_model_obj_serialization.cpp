/*
File:   pig_model_obj_serialization.cpp
Author: Taylor Robbins
Date:   01\10\2022
Description: 
	** Holds functions that help us parse the .obj and ,mtl file format
*/

//TODO: Should we check for naming conflicts? Like two materials/objects/groups with the same name?

enum TryDeserializeObjFileError_t
{
	DeserObjFileError_None = 0,
	DeserObjFileError_MissingFile,
	DeserObjFileError_EmptyFile,
	DeserObjFileError_LinePieceCountIsWrong,
	DeserObjFileError_IntParseFailure,
	DeserObjFileError_FloatParseFailure,
	DeserObjFileError_IndexOutOfBounds,
	DeserObjFileError_FaceVertIndexCountIsWrong,
	DeserObjFileError_CouldntFindMaterialLibrary,
	DeserObjFileError_MaterialAttribBeforeNew,
	DeserObjFileError_UnknownMaterial,
	DeserObjFileError_FacesBeforeObjectStart,
	DeserObjFileError_FacesBeforeMaterialSelect,
	DeserObjFileError_NumErrors,
};

const char* GetTryDeserializeObjFileErrorStr(TryDeserializeObjFileError_t error)
{
	switch (error)
	{
		case DeserObjFileError_None:                       return "None";
		case DeserObjFileError_MissingFile:                return "MissingFile";
		case DeserObjFileError_EmptyFile:                  return "EmptyFile";
		case DeserObjFileError_LinePieceCountIsWrong:      return "LinePieceCountIsWrong";
		case DeserObjFileError_IntParseFailure:            return "IntParseFailure";
		case DeserObjFileError_FloatParseFailure:          return "FloatParseFailure";
		case DeserObjFileError_IndexOutOfBounds:           return "IndexOutOfBounds";
		case DeserObjFileError_FaceVertIndexCountIsWrong:  return "FaceVertIndexCountIsWrong";
		case DeserObjFileError_CouldntFindMaterialLibrary: return "CouldntFindMaterialLibrary";
		case DeserObjFileError_MaterialAttribBeforeNew:    return "MaterialAttribBeforeNew";
		case DeserObjFileError_UnknownMaterial:            return "UnknownMaterial";
		case DeserObjFileError_FacesBeforeObjectStart:     return "FacesBeforeObjectStart";
		case DeserObjFileError_FacesBeforeMaterialSelect:  return "FacesBeforeMaterialSelect";
		default: return "Unknown";
	}
}

struct ObjModelDataMaterialLibrary_t
{
	MyStr_t filePath;
	u64 numMaterials;
};

struct ObjModelDataMaterial_t
{
	MyStr_t name;
	r32 specularExponent;
	r32 opticalDensity;
	r32 dissolve;
	i8 illuminationType; //TODO: What are the valid types?
	Color_t ambientColor;
	Color_t diffuseColor;
	Color_t specularColor;
	MyStr_t ambientMapPath;
	MyStr_t diffuseMapPath;
	MyStr_t specularMapPath;
};

struct ObjModelDataFace_t
{
	u64 materialIndex;
	u64 vertexIndices[3];
	u64 normalIndices[3];
	u64 texCoordIndices[3];
};

struct ObjModelDataObject_t
{
	MyStr_t name;
	VarArray_t faces; //ObjModelDataFace_t
};

struct ObjModelData_t
{
	MemArena_t* allocArena;
	VarArray_t materialLibs; //ObjModelDataMaterialLibrary_t
	VarArray_t materials; //ObjModelDataMaterial_t
	VarArray_t vertices; //v3
	VarArray_t normals; //v3
	VarArray_t texCoords; //v2
	VarArray_t objects; //ObjModelDataObject_t
};

void FreeObjModelData(ObjModelData_t* objFile)
{
	NotNull(objFile);
	VarArrayLoop(&objFile->materialLibs, lIndex)
	{
		VarArrayLoopGet(ObjModelDataMaterialLibrary_t, matLib, &objFile->materialLibs, lIndex);
		FreeString(objFile->allocArena, &matLib->filePath);
	}
	VarArrayLoop(&objFile->materials, mIndex)
	{
		VarArrayLoopGet(ObjModelDataMaterial_t, material, &objFile->materials, mIndex);
		FreeString(objFile->allocArena, &material->name);
		FreeString(objFile->allocArena, &material->ambientMapPath);
		FreeString(objFile->allocArena, &material->diffuseMapPath);
		FreeString(objFile->allocArena, &material->specularMapPath);
	}
	VarArrayLoop(&objFile->objects, oIndex)
	{
		VarArrayLoopGet(ObjModelDataObject_t, object, &objFile->objects, oIndex);
		FreeString(objFile->allocArena, &object->name);
		FreeVarArray(&object->faces);
	}
	FreeVarArray(&objFile->materialLibs);
	FreeVarArray(&objFile->materials);
	FreeVarArray(&objFile->vertices);
	FreeVarArray(&objFile->normals);
	FreeVarArray(&objFile->texCoords);
	FreeVarArray(&objFile->objects);
	ClearPointer(objFile);
}

bool TryDeserObjMaterialLibFile(MyStr_t matLibFileContents, ProcessLog_t* log, ObjModelData_t* objData, ObjModelDataMaterialLibrary_t* matLib)
{
	NotNullStr(&matLibFileContents);
	NotNull(log);
	NotNull(objData);
	NotNull(matLib);
	NotNull(objData->allocArena);
	
	LogWriteLine_N(log, "Entering TryDeserObjMaterialLibFile...");
	SetProcessLogName(log, NewStr("DeserObjMtlFile"));
	
	matLib->numMaterials = 0;
	ObjModelDataMaterial_t* currentMaterial = nullptr;
	
	LineParser_t lineParser = NewLineParser(matLibFileContents);
	MyStr_t fileLine = MyStr_Empty;
	while (LineParserGetLine(&lineParser, &fileLine))
	{
		TrimWhitespace(&fileLine);
		if (fileLine.length == 0 || fileLine.pntr[0] == '#') //empty lines and comments have no effect
		{
			continue;
		}
		else
		{
			TempPushMark();
			u64 numLinePieces = 0;
			MyStr_t* linePieces = SplitStringBySpacesFastTemp(TempArena, fileLine, &numLinePieces);
			Assert(numLinePieces > 0);
			
			//TODO: Find out what r32 Tr     attribute is
			//TODO: Find out what v3  Tf     attribute is
			//TODO: Find out what v3  Ke     attribute is
			//TODO: Find out what str map_Ke attribute is
			
			if (StrEquals(linePieces[0], "newmtl"))
			{
				if (numLinePieces == 2)
				{
					ObjModelDataMaterial_t* newMaterial = VarArrayAdd(&objData->materials, ObjModelDataMaterial_t);
					NotNull(newMaterial);
					ClearPointer(newMaterial);
					newMaterial->name = AllocString(objData->allocArena, &linePieces[1]);
					matLib->numMaterials++;
					currentMaterial = newMaterial;
				}
				else
				{
					LogPrintLine_E(log, "newmtl on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "Ns")) //r32 specularExponent
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 2)
				{
					if (!TryParseR32(linePieces[1], &currentMaterial->specularExponent, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\" as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "Ni")) //r32 opticalDensity
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 2)
				{
					if (!TryParseR32(linePieces[1], &currentMaterial->opticalDensity, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\" as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "d")) //r32 dissolve
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 2)
				{
					if (!TryParseR32(linePieces[1], &currentMaterial->dissolve, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\" as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "illum")) //i8 illuminationType TODO: Is i8 really the correct type?
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 2)
				{
					if (!TryParseI8(linePieces[1], &currentMaterial->illuminationType, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\" as i8 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "Ka")) //v3 ambientColor
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 4)
				{
					v3 colorVec = Vec3_Zero;
					if (!TryParseR32(linePieces[1], &colorVec.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".x as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[2], &colorVec.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".y as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[2].length, linePieces[2].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[3], &colorVec.z, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".z as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[3].length, linePieces[3].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else
					{
						currentMaterial->ambientColor = ToColor(NewColorf(colorVec.x, colorVec.y, colorVec.z, 1.0f));
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "Kd")) //v3 diffuseColor
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 4)
				{
					v3 colorVec = Vec3_Zero;
					if (!TryParseR32(linePieces[1], &colorVec.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".x as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[2], &colorVec.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".y as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[2].length, linePieces[2].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[3], &colorVec.z, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".z as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[3].length, linePieces[3].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else
					{
						currentMaterial->diffuseColor = ToColor(NewColorf(colorVec.x, colorVec.y, colorVec.z, 1.0f));
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "Ks")) //v3 specularColor
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces == 4)
				{
					v3 colorVec = Vec3_Zero;
					if (!TryParseR32(linePieces[1], &colorVec.x, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".x as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[1].length, linePieces[1].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[2], &colorVec.y, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".y as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[2].length, linePieces[2].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else if (!TryParseR32(linePieces[3], &colorVec.z, &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Couldn't parse %.*s value \"%.*s\".z as r32 on line %llu for material \"%.*s\": \"%.*s\"",
							linePieces[0].length, linePieces[0].pntr,
							linePieces[3].length, linePieces[3].pntr,
							lineParser.lineIndex+1,
							currentMaterial->name.length, currentMaterial->name.pntr,
							fileLine.length, fileLine.pntr
						);
						LogExitFailure(log, DeserObjFileError_FloatParseFailure);
						TempPopMark();
						return false;
					}
					else
					{
						currentMaterial->specularColor = ToColor(NewColorf(colorVec.x, colorVec.y, colorVec.z, 1.0f));
					}
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "map_Ka")) //str ambientMapPath
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces >= 2)
				{
					MyStr_t fileName = NewStr((u64)((fileLine.pntr + fileLine.length) - linePieces[1].pntr), linePieces[1].pntr);
					currentMaterial->ambientMapPath = AllocString(objData->allocArena, &fileName);
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of >= 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "map_Kd")) //str diffuseMapPath
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces >= 2)
				{
					MyStr_t fileName = NewStr((u64)((fileLine.pntr + fileLine.length) - linePieces[1].pntr), linePieces[1].pntr);
					currentMaterial->diffuseMapPath = AllocString(objData->allocArena, &fileName);
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of >= 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else if (StrEquals(linePieces[0], "map_Ks")) //str specularMapPath
			{
				if (currentMaterial == nullptr)
				{
					LogPrintLine_E(log, "Found material attrib %.*s on line %llu before first newmtl: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_MaterialAttribBeforeNew);
					TempPopMark();
					return false;
				}
				
				if (numLinePieces >= 2)
				{
					MyStr_t fileName = NewStr((u64)((fileLine.pntr + fileLine.length) - linePieces[1].pntr), linePieces[1].pntr);
					currentMaterial->specularMapPath = AllocString(objData->allocArena, &fileName);
				}
				else
				{
					LogPrintLine_E(log, "Ns on line %llu had %llu instead of >= 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					TempPopMark();
					return false;
				}
			}
			else
			{
				//TODO: Turn this warning output back on once we've actually handled all possible values
				// LogPrintLine_D(log, "Unknown material attribute \"%.*s\" found at beginning of line %llu: \"%.*s\"", linePieces[0].length, linePieces[0].pntr, lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
			}
			
			TempPopMark();
		}
	}
	
	LogPrintLine_I(log, "Found %llu material%s in material library", matLib->numMaterials, Plural(matLib->numMaterials, "s"));
	LogExitSuccess(log);
	return true;
}

bool FindObjMaterialByName(ObjModelData_t* objData, MyStr_t materialName, u64* materialIndexOut, ObjModelDataMaterial_t** materialOut = nullptr)
{
	NotNull(objData);
	NotNullStr(&materialName);
	VarArrayLoop(&objData->materials, mIndex)
	{
		VarArrayLoopGet(ObjModelDataMaterial_t, material, &objData->materials, mIndex);
		if (StrEquals(material->name, materialName))
		{
			if (materialIndexOut != nullptr) { *materialIndexOut = mIndex; }
			if (materialOut != nullptr) { *materialOut = material; }
			return true;
		}
	}
	return false;
}

bool TryDeserObjFile(MyStr_t objFileContents, MyStr_t folderPath, ProcessLog_t* log, ObjModelData_t* objData, MemArena_t* memArena)
{
	NotNullStr(&objFileContents);
	NotNull(log);
	NotNull(objData);
	NotNull(memArena);
	NotNullStr(&folderPath);
	Assert(IsEmptyStr(folderPath) || StrEndsWith(folderPath, "/") || StrEndsWith(folderPath, "\\"));
	Assert(memArena != TempArena);
	
	ClearPointer(objData);
	objData->allocArena = memArena;
	CreateVarArray(&objData->materialLibs, memArena, sizeof(ObjModelDataMaterialLibrary_t));
	CreateVarArray(&objData->materials,    memArena, sizeof(ObjModelDataMaterial_t));
	CreateVarArray(&objData->vertices,     memArena, sizeof(v3));
	CreateVarArray(&objData->normals,      memArena, sizeof(v3));
	CreateVarArray(&objData->texCoords,    memArena, sizeof(v2));
	CreateVarArray(&objData->objects,      memArena, sizeof(ObjModelDataObject_t));
	
	LogWriteLine_N(log, "Entering TryDeserializeObjFile...");
	SetProcessLogName(log, NewStr("DeserObjFile"));
	
	if (objFileContents.length == 0) //empty file cannot be a valid obj file
	{
		LogWriteLine_E(log, "Empty file is not a valid .obj file format");
		LogExitFailure(log, DeserObjFileError_EmptyFile);
		FreeObjModelData(objData);
		return false;
	}
	
	u64 totalNumFaces = 0;
	u64 numGroupsFound = 0;
	
	ObjModelDataObject_t* currentObject = nullptr;
	bool materialSelected = false;
	u64 currentMaterialIndex = 0;
	
	LineParser_t lineParser = NewLineParser(objFileContents);
	MyStr_t fileLine = MyStr_Empty;
	while (LineParserGetLine(&lineParser, &fileLine))
	{
		TrimWhitespace(&fileLine);
		if (fileLine.length == 0 || fileLine.pntr[0] == '#') //empty lines and comments have no effect
		{
			continue;
		}
		else
		{
			TempPushMark();
			u64 numLinePieces = 0;
			MyStr_t* linePieces = SplitStringBySpacesFastTemp(TempArena, fileLine, &numLinePieces);
			Assert(numLinePieces > 0);
			
			if (StrEquals(linePieces[0], "o")) //object
			{
				if (numLinePieces != 2)
				{
					LogPrintLine_E(log, "object on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				ObjModelDataObject_t* newObject = VarArrayAdd(&objData->objects, ObjModelDataObject_t);
				NotNull(newObject);
				ClearPointer(newObject);
				newObject->name = AllocString(memArena, &linePieces[1]);
				CreateVarArray(&newObject->faces, memArena, sizeof(ObjModelDataFace_t));
				
				currentObject = newObject;
			}
			else if (StrEquals(linePieces[0], "v")) //vertex
			{
				if (numLinePieces != 4)
				{
					LogPrintLine_E(log, "Vertex on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v3 newVertValue = Vec3_Zero;
				if (!TryParseR32(linePieces[1], &newVertValue.x, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Vertex on line %llu x value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[1].length, linePieces[1].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				else if (!TryParseR32(linePieces[2], &newVertValue.y, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Vertex on line %llu y value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[2].length, linePieces[2].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				else if (!TryParseR32(linePieces[3], &newVertValue.z, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Vertex on line %llu z value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[3].length, linePieces[3].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v3* newVert = VarArrayAdd(&objData->vertices, v3);
				NotNull(newVert);
				*newVert = newVertValue;
				// PrintLine_D("Vert[%u]: (%f, %f, %f) from \"%.*s\"", objData->vertices.length-1, newVertValue.x, newVertValue.y, newVertValue.z, fileLine.length, fileLine.pntr);
			}
			else if (StrEquals(linePieces[0], "vn")) //normal
			{
				if (numLinePieces != 4)
				{
					LogPrintLine_E(log, "Normal on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v3 newNormalValue = Vec3_Zero;
				if (!TryParseR32(linePieces[1], &newNormalValue.x, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Normal on line %llu x value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[1].length, linePieces[1].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				else if (!TryParseR32(linePieces[2], &newNormalValue.y, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Normal on line %llu y value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[2].length, linePieces[2].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				else if (!TryParseR32(linePieces[3], &newNormalValue.z, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "Normal on line %llu z value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[3].length, linePieces[3].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v3* newNormal = VarArrayAdd(&objData->normals, v3);
				NotNull(newNormal);
				*newNormal = newNormalValue;
			}
			else if (StrEquals(linePieces[0], "vt")) //texCoord
			{
				if (numLinePieces != 3)
				{
					LogPrintLine_E(log, "TexCoord on line %llu had %llu instead of 4 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v2 newTexCoordValue = Vec2_Zero;
				if (!TryParseR32(linePieces[1], &newTexCoordValue.x, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "TexCoord on line %llu x value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[1].length, linePieces[1].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				else if (!TryParseR32(linePieces[2], &newTexCoordValue.y, &log->parseFailureReason))
				{
					LogPrintLine_E(log, "TexCoord on line %llu y value doesn't parse as r32 \"%.*s\" error %s: \"%.*s\"",
						lineParser.lineIndex+1,
						linePieces[2].length, linePieces[2].pntr,
						GetTryParseFailureReasonStr(log->parseFailureReason),
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_FloatParseFailure);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				v2* newTexCoord = VarArrayAdd(&objData->texCoords, v2);
				NotNull(newTexCoord);
				*newTexCoord = newTexCoordValue;
			}
			else if (StrEquals(linePieces[0], "g")) //group
			{
				numGroupsFound++;
			}
			else if (StrEquals(linePieces[0], "f")) //face
			{
				if (numLinePieces != 4)
				{
					LogPrintLine_E(log, "Found face with %llu vertices. We only support faces with 3 vertices: \"%.*s\"", numLinePieces-1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				if (currentObject == nullptr)
				{
					LogPrintLine_E(log, "Faces started being defined on line %llu before a object was started: \"%.*s\"", lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_FacesBeforeObjectStart);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				if (!materialSelected)
				{
					LogPrintLine_E(log, "Faces started being defined on line %llu before a material was selected: \"%.*s\"", lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_FacesBeforeMaterialSelect);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				ObjModelDataFace_t faceValue = {};
				for (u8 vIndex = 0; vIndex < 3; vIndex++)
				{
					TempPushMark();
					u64 numIndexPieces = 0;
					MyStr_t* indexPieces = SplitStringBySlashesFastTemp(TempArena, linePieces[1 + vIndex], &numIndexPieces);
					
					if (numIndexPieces != 3)
					{
						LogPrintLine_E(log, "Face on line %llu has vertex[%u] with %llu indices. We expected to find 3 indices: \"%.*s\"", lineParser.lineIndex+1, vIndex, numIndexPieces, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_FaceVertIndexCountIsWrong);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					
					if (!TryParseU64(indexPieces[0], &faceValue.vertexIndices[vIndex], &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] couldn't parse vertex index \"%.*s\": \"%.*s\"", lineParser.lineIndex+1, vIndex, indexPieces[0].length, indexPieces[0].pntr, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IntParseFailure);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					else if (!TryParseU64(indexPieces[1], &faceValue.texCoordIndices[vIndex], &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] couldn't parse texCoord index \"%.*s\": \"%.*s\"", lineParser.lineIndex+1, vIndex, indexPieces[1].length, indexPieces[1].pntr, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IntParseFailure);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					else if (!TryParseU64(indexPieces[2], &faceValue.normalIndices[vIndex], &log->parseFailureReason))
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] couldn't parse normal index \"%.*s\": \"%.*s\"", lineParser.lineIndex+1, vIndex, indexPieces[2].length, indexPieces[2].pntr, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IntParseFailure);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					
					if (faceValue.vertexIndices[vIndex] == 0 || faceValue.vertexIndices[vIndex] > objData->vertices.length)
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] has invalid vertex index %llu when we have %llu vertices: \"%.*s\"", lineParser.lineIndex+1, vIndex, faceValue.vertexIndices[vIndex], objData->vertices.length, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IndexOutOfBounds);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					else if (faceValue.texCoordIndices[vIndex] == 0 || faceValue.texCoordIndices[vIndex] > objData->texCoords.length)
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] has invalid texCoord index %llu when we have %llu texCoords: \"%.*s\"", lineParser.lineIndex+1, vIndex, faceValue.texCoordIndices[vIndex], objData->texCoords.length, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IndexOutOfBounds);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					else if (faceValue.normalIndices[vIndex] == 0 || faceValue.normalIndices[vIndex] > objData->normals.length)
					{
						LogPrintLine_E(log, "Face on line %llu vertex[%u] has invalid normal index %llu when we have %llu normals: \"%.*s\"", lineParser.lineIndex+1, vIndex, faceValue.normalIndices[vIndex], objData->normals.length, fileLine.length, fileLine.pntr);
						LogExitFailure(log, DeserObjFileError_IndexOutOfBounds);
						FreeObjModelData(objData);
						TempPopMark();
						TempPopMark();
						return false;
					}
					
					faceValue.materialIndex = currentMaterialIndex;
					faceValue.vertexIndices[vIndex]--;
					faceValue.normalIndices[vIndex]--;
					faceValue.texCoordIndices[vIndex]--;
					
					TempPopMark();
				}
				
				ObjModelDataFace_t* newFace = VarArrayAdd(&currentObject->faces, ObjModelDataFace_t);
				NotNull(newFace);
				MyMemCopy(newFace, &faceValue, sizeof(ObjModelDataFace_t));
				totalNumFaces++;
			}
			else if (StrEquals(linePieces[0], "s")) //smoothing group
			{
				//TODO: Add support for smoothing groups
				if (!StrEqualsIgnoreCase(linePieces[1], "off"))
				{
					LogPrintLine_W(log, "This model file has smoothing group \"%.*s\" which we don't support yet", linePieces[1].length, linePieces[1].chars);
					log->hadWarnings = true;
				}
			}
			else if (StrEquals(linePieces[0], "mtllib")) //material library
			{
				if (numLinePieces < 2)
				{
					LogPrintLine_E(log, "mtllib on line %llu has no filePath following it: \"%.*s\"", lineParser.lineIndex+1, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				MyStr_t fileName = NewStr((u64)((fileLine.pntr + fileLine.length) - linePieces[1].pntr), linePieces[1].pntr);
				//TODO: Should we check to make sure the file path is relative? (Doesn't start with C:/ or a /)
				// PrintLine_D("The obj file wants us to use a material library \"%.*s\"", fileName.length, fileName.pntr);
				TempPushMark();
				MyStr_t filePath = TempPrintStr("%.*s%.*s", folderPath.length, folderPath.chars, fileName.length, fileName.pntr);
				PlatFileContents_t matLibFile = {};
				if (!plat->ReadFileContents(filePath, &matLibFile))
				{
					LogPrintLine_E(log, "On line %llu couldn't find material library file at \"%.*s\": \"%.*s\"", lineParser.lineIndex+1, filePath.length, filePath.pntr, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_CouldntFindMaterialLibrary);
					FreeObjModelData(objData);
					TempPopMark();
					TempPopMark();
					return false;
				}
				
				MyStr_t oldFilePath = AllocString(TempArena, &log->filePath);
				MyStr_t oldProcessName = AllocString(TempArena, &log->processName);
				SetProcessLogFilePath(log, filePath);
				
				ObjModelDataMaterialLibrary_t* newMaterialLib = VarArrayAdd(&objData->materialLibs, ObjModelDataMaterialLibrary_t);
				NotNull(newMaterialLib);
				ClearPointer(newMaterialLib);
				newMaterialLib->filePath = AllocString(memArena, &log->filePath);
				if (!TryDeserObjMaterialLibFile(NewStr(matLibFile.length, matLibFile.chars), log, objData, newMaterialLib))
				{
					Assert(log->hadErrors == true);
					FreeObjModelData(objData);
					TempPopMark();
					TempPopMark();
					plat->FreeFileContents(&matLibFile);
					return false;
				}
				
				SetProcessLogFilePath(log, oldFilePath);
				SetProcessLogName(log, oldProcessName);
				TempPopMark();
				plat->FreeFileContents(&matLibFile);
			}
			else if (StrEquals(linePieces[0], "usemtl")) //material
			{
				if (numLinePieces != 2)
				{
					LogPrintLine_E(log, "usemtl on line %llu had %llu instead of 2 parts: \"%.*s\"",
						lineParser.lineIndex+1,
						numLinePieces,
						fileLine.length, fileLine.pntr
					);
					LogExitFailure(log, DeserObjFileError_LinePieceCountIsWrong);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
				
				u64 materialIndex = 0;
				if (FindObjMaterialByName(objData, linePieces[1], &materialIndex))
				{
					materialSelected = true;
					currentMaterialIndex = materialIndex;
				}
				else
				{
					LogPrintLine_E(log, "usemtl on line %llu has unknown material name \"%.*s\": \"%.*s\"", lineParser.lineIndex+1, linePieces[1].length, linePieces[1].pntr, fileLine.length, fileLine.pntr);
					LogExitFailure(log, DeserObjFileError_UnknownMaterial);
					FreeObjModelData(objData);
					TempPopMark();
					return false;
				}
			}
			else
			{
				LogPrintLine_W(log, "WARNING: Unknown command string \"%.*s\" found at beginning of line %llu: \"%.*s\"",
					linePieces[0].length, linePieces[0].pntr,
					lineParser.lineIndex+1,
					fileLine.length, fileLine.pntr
				);
				log->hadWarnings = true;
			}
			
			TempPopMark();
		}
	}
	// PrintLine_D("There were %llu line(s) in the obj file", lineParser.lineIndex);
	
	LogPrintLine_I(log, "Found %llu object%s %llu vert%s %llu normal%s %llu texCoord%s %llu group%s %llu face%s and %llu material%s",
		objData->objects.length, Plural(objData->objects.length, "s"),
		objData->vertices.length, PluralEx(objData->vertices.length, "ex", "ices"),
		objData->normals.length, Plural(objData->normals.length, "s"),
		objData->texCoords.length, Plural(objData->texCoords.length, "s"),
		numGroupsFound, Plural(numGroupsFound, "s"),
		totalNumFaces, Plural(totalNumFaces, "s"),
		objData->materials.length, Plural(objData->materials.length, "s")
	);
	
	LogExitSuccess(log);
	return true;
}

void ReverseObjModelDataTriangleOrder(ObjModelData_t* objData)
{
	NotNull(objData);
	VarArrayLoop(&objData->objects, oIndex)
	{
		VarArrayLoopGet(ObjModelDataObject_t, object, &objData->objects, oIndex);
		VarArrayLoop(&object->faces, fIndex)
		{
			VarArrayLoopGet(ObjModelDataFace_t, face, &object->faces, fIndex);
			SWAP_VARIABLES(u64, face->vertexIndices[0],   face->vertexIndices[1]);
			SWAP_VARIABLES(u64, face->texCoordIndices[0], face->texCoordIndices[1]);
			SWAP_VARIABLES(u64, face->normalIndices[0],   face->normalIndices[1]);
		}
	}
}
