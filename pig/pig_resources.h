/*
File:   pig_resources.h
Author: Taylor Robbins
Date:   11\04\2021
*/

#ifndef _PIG_RESOURCES_H
#define _PIG_RESOURCES_H

#define RESOURCES_NUM_TEXTURES    3
#define RESOURCES_NUM_VECTORS     2
#define RESOURCES_NUM_SHADERS     9
#define RESOURCES_NUM_FONTS       3
#define RESOURCES_NUM_SHEETS      4
#define TOTAL_NUM_RESOURCES       (RESOURCES_NUM_TEXTURES + RESOURCES_NUM_SHADERS + RESOURCES_NUM_FONTS + RESOURCES_NUM_SHEETS)

#define RESOURCE_FOLDER_FONTS          "Resources/Fonts"
#define RESOURCE_FOLDER_SHADERS        "Resources/Shaders"
#define RESOURCE_FOLDER_SHEETS         "Resources/Sheets"
#define RESOURCE_FOLDER_SPRITES        "Resources/Sprites"
#define RESOURCE_FOLDER_TEXTURES       "Resources/Textures"
#define RESOURCE_FOLDER_VECTOR         "Resources/Vector"
#define RESOURCE_FOLDER_MODELS         "Resources/Models"
#define RESOURCE_FOLDER_MODEL_TEXTURES "Resources/Models/Textures"

// +--------------------------------------------------------------+
// |                     Resources Structure                      |
// +--------------------------------------------------------------+
enum ResourceType_t
{
	ResourceType_None,
	ResourceType_Texture,
	ResourceType_VectorImage,
	ResourceType_Sheet,
	ResourceType_Shader,
	ResourceType_Font,
	ResourceType_NumTypes,
};
struct ResourceWatch_t
{
	ResourceType_t type;
	u64 resourceIndex;
	PlatWatchedFile_t* watchedFile;
};
struct Resources_t
{
	#if DEBUG_BUILD
	VarArray_t watches; //ResourceWatch_t
	#endif
	
	// +==============================+
	// |           Textures           |
	// +==============================+
	union
	{
		Texture_t textures[RESOURCES_NUM_TEXTURES];
		struct
		{
			Texture_t piggybankTexture;
			Texture_t boxTexture;
			Texture_t skyboxTexture;
		};
	};
	// +==============================+
	// |          VectorImgs          |
	// +==============================+
	union
	{
		VectorImg_t vectorImgs[RESOURCES_NUM_VECTORS];
		struct
		{
			VectorImg_t testVector;
			VectorImg_t pigVector;
		};
	};
	// +==============================+
	// |            Sheets            |
	// +==============================+
	union
	{
		SpriteSheet_t sheets[RESOURCES_NUM_SHEETS];
		struct
		{
			SpriteSheet_t pigAnimSheet;
			SpriteSheet_t vectorIcons64;
			SpriteSheet_t controllerBtnsSheet;
			SpriteSheet_t fontEditSheet;
		};
	};
	// +==============================+
	// |           Shaders            |
	// +==============================+
	union
	{
		Shader_t shaders[RESOURCES_NUM_SHADERS];
		struct
		{
			Shader_t mainShader2D;
			Shader_t mainShader3D;
			Shader_t phongShader;
			Shader_t gradientShader2D;
			Shader_t roundedCornersShader;
			Shader_t pieChartShader;
			Shader_t bezier3Shader;
			Shader_t bezier4Shader;
			Shader_t ellipseArcShader;
		};
	};
	// +==============================+
	// |            Fonts             |
	// +==============================+
	union
	{
		Font_t fonts[RESOURCES_NUM_FONTS];
		struct
		{
			Font_t debugFont;
			Font_t pixelFont;
			Font_t largeFont;
		};
	};
};

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
const char* GetResourceTypeStr(ResourceType_t resourceType)
{
	switch (resourceType)
	{
		case ResourceType_None:        return "None";
		case ResourceType_Texture:     return "Texture";
		case ResourceType_VectorImage: return "VectorImage";
		case ResourceType_Sheet:       return "Sheet";
		case ResourceType_Shader:      return "Shader";
		case ResourceType_Font:        return "Font";
		default: return "Unknown";
	}
}
u64 GetNumResourcesOfType(ResourceType_t resourceType)
{
	switch (resourceType)
	{
		case ResourceType_Texture: return RESOURCES_NUM_TEXTURES;
		case ResourceType_Sheet:   return RESOURCES_NUM_SHEETS;
		case ResourceType_Shader:  return RESOURCES_NUM_SHADERS;
		case ResourceType_Font:    return RESOURCES_NUM_FONTS;
		default: return 0;
	}
}

// +--------------------------------------------------------------+
// |                      Texture Meta Info                       |
// +--------------------------------------------------------------+
const char* Resources_GetPathForTexture(u64 textureIndex)
{
	switch (textureIndex)
	{
		case 0: return RESOURCE_FOLDER_SPRITES  "/piggybank.png"; //| piggybankTexture |
		case 1: return RESOURCE_FOLDER_TEXTURES "/box.png";       //| boxTexture       |
		case 2: return RESOURCE_FOLDER_TEXTURES "/skybox.png";    //| skyboxTexture    |
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                     VectorImg Meta Info                      |
// +--------------------------------------------------------------+
const char* Resources_GetPathForVectorImg(u64 vectorImgIndex)
{
	switch (vectorImgIndex)
	{
		case 0: return RESOURCE_FOLDER_VECTOR  "/test.svg"; //| testVector |
		case 1: return RESOURCE_FOLDER_VECTOR  "/pig.svg"; //| pigVector |
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                    Sprite Sheet Meta Info                    |
// +--------------------------------------------------------------+
struct ResourceSheetMetaInfo_t
{
	v2i numFrames;
	v2i padding;
	bool pixelated;
	MyStr_t metaFilePath;
};
const char* Resources_GetPathForSheet(u64 sheetIndex, ResourceSheetMetaInfo_t* metaInfo)
{
	NotNull(metaInfo);
	ClearPointer(metaInfo);
	metaInfo->numFrames = NewVec2i(1, 1);
	metaInfo->padding = NewVec2i(0, 0);
	metaInfo->pixelated = true;
	metaInfo->metaFilePath = MyStr_Empty;
	switch (sheetIndex)
	{
		case 0: metaInfo->numFrames = NewVec2i(4, 1); return RESOURCE_FOLDER_SHEETS "/pig_anim.png"; break; //pigAnimSheet
		case 1: metaInfo->numFrames = NewVec2i(4, 4); metaInfo->metaFilePath = NewStr(RESOURCE_FOLDER_SHEETS "/vector_icons64.meta"); metaInfo->pixelated = false; return RESOURCE_FOLDER_SHEETS "/vector_icons64.png"; break; //vectorIcons64
		case 2: metaInfo->numFrames = NewVec2i(2, 2); metaInfo->pixelated = false; return RESOURCE_FOLDER_SHEETS "/controller_btns.png"; break; //controllerBtnsSheet
		case 3: metaInfo->numFrames = NewVec2i(16, 16); metaInfo->metaFilePath = NewStr(RESOURCE_FOLDER_FONTS "/btns_light.meta"); return RESOURCE_FOLDER_FONTS "/btns_light.png"; break; //fontEditSheet
		default: Assert(false); return nullptr;
	}
}
const char* GetResourceSheetName(u64 sheetIndex)
{
	switch (sheetIndex)
	{
		case 0: return "pigAnimSheet";
		case 1: return "vectorIcons64";
		case 2: return "controllerBtnsSheet";
		case 3: return "fontEditSheet";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                       Shader Meta Info                       |
// +--------------------------------------------------------------+
struct ResourceShaderMetaInfo_t
{
	VertexType_t vertexType;
	u32 requiredUniforms;
};
const char* Resources_GetPathForShader(u64 shaderIndex, ResourceShaderMetaInfo_t* metaInfo = nullptr)
{
	if (metaInfo != nullptr)
	{
		ClearPointer(metaInfo);
		metaInfo->vertexType = VertexType_Default3D;
		metaInfo->requiredUniforms = ShaderUniform_None;
	}
	switch (shaderIndex)
	{
		// +==============================+
		// |         mainShader2D         |
		// +==============================+
		case 0:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/main2D.glsl"; 
		} break;
		// +==============================+
		// |         mainShader3D         |
		// +==============================+
		case 1:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default3D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/main3D.glsl"; 
		} break;
		// +==============================+
		// |         phongShader          |
		// +==============================+
		case 2:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default3D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/phong.glsl"; 
		} break;
		// +==============================+
		// |       gradientShader2D       |
		// +==============================+
		case 3:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1|ShaderUniform_Color2);
			}
			return RESOURCE_FOLDER_SHADERS "/gradient2D.glsl"; 
		} break;
		// +==============================+
		// |     roundedCornersShader     |
		// +==============================+
		case 4:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/roundedCorners.glsl"; 
		} break;
		// +==============================+
		// |        pieChartShader        |
		// +==============================+
		case 5:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices);
			}
			return RESOURCE_FOLDER_SHADERS "/pieChart.glsl"; 
		} break;
		// +==============================+
		// |        bezier3Shader         |
		// +==============================+
		case 6:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/bezier3.glsl"; 
		} break;
		// +==============================+
		// |        bezier4Shader         |
		// +==============================+
		case 7:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/bezier4.glsl"; 
		} break;
		// +==============================+
		// |       ellipseArcShader       |
		// +==============================+
		case 8:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/ellipseArc.glsl"; 
		} break;
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                        Font Meta Info                        |
// +--------------------------------------------------------------+
#define MAX_NUM_RESOURCE_FONT_FACES 8
#define MAX_NUM_RESOURCE_FONT_BAKES 4

struct ResourceFontFaceMetaInfo_t
{
	bool isSpriteFont;
	
	//shared options
	bool bold;
	bool italic;
	MyStr_t name;
	
	//TTF Options
	i32 size;
	MyStr_t fontName;
	v2i bakeSize;
	bool includeCyrillicBake;
	bool includeJapaneseKanaBake;
	bool includeBtnsSheet;
	
	//Sprite Font options
	MyStr_t filePaths[MAX_NUM_RESOURCE_FONT_BAKES]; //overrides the const char* returned from Resources_GetPathOrNameForFont
	MyStr_t metaFilePaths[MAX_NUM_RESOURCE_FONT_BAKES];
	v2i sheetSizes[MAX_NUM_RESOURCE_FONT_BAKES];
	v2i paddings[MAX_NUM_RESOURCE_FONT_BAKES];
	bool isPixelated[MAX_NUM_RESOURCE_FONT_BAKES];
	bool scalables[MAX_NUM_RESOURCE_FONT_BAKES];
	bool colored[MAX_NUM_RESOURCE_FONT_BAKES];
};
struct ResourceFontMetaInfo_t
{
	bool requestFromPlatform;
	ResourceFontFaceMetaInfo_t faces[MAX_NUM_RESOURCE_FONT_FACES];
};
const char* Resources_GetPathOrNameForFont(u64 fontIndex, ResourceFontMetaInfo_t* metaInfo)
{
	NotNull(metaInfo);
	Assert(fontIndex < RESOURCES_NUM_FONTS);
	ClearPointer(metaInfo);
	metaInfo->requestFromPlatform = false;
	metaInfo->faces[0].size = 12;
	metaInfo->faces[0].bakeSize = NewVec2i(256, 256);
	switch (fontIndex)
	{
		// +==============================+
		// |          debugFont           |
		// +==============================+
		case 0:
		{
			metaInfo->requestFromPlatform = true;
			metaInfo->faces[0].name = NewStr("default18");    metaInfo->faces[0].size = 18; metaInfo->faces[0].bold = false; metaInfo->faces[0].italic = false; metaInfo->faces[0].bakeSize = NewVec2i(256, 256); metaInfo->faces[0].includeCyrillicBake = true; metaInfo->faces[0].includeJapaneseKanaBake = true; metaInfo->faces[0].includeBtnsSheet = true;
			metaInfo->faces[1].name = NewStr("bold18");       metaInfo->faces[1].size = 18; metaInfo->faces[1].bold = true;  metaInfo->faces[1].italic = false; metaInfo->faces[1].bakeSize = NewVec2i(256, 256);
			metaInfo->faces[2].name = NewStr("italic18");     metaInfo->faces[2].size = 18; metaInfo->faces[2].bold = false; metaInfo->faces[2].italic = true;  metaInfo->faces[2].bakeSize = NewVec2i(256, 256);
			metaInfo->faces[3].name = NewStr("bolditalic18"); metaInfo->faces[3].size = 18; metaInfo->faces[3].bold = true;  metaInfo->faces[3].italic = true;  metaInfo->faces[3].bakeSize = NewVec2i(256, 256);
			metaInfo->faces[4].name = NewStr("default12");    metaInfo->faces[4].size = 12; metaInfo->faces[4].bold = false; metaInfo->faces[4].italic = false; metaInfo->faces[4].bakeSize = NewVec2i(256, 256); metaInfo->faces[4].includeCyrillicBake = true; metaInfo->faces[4].includeJapaneseKanaBake = true; metaInfo->faces[4].includeBtnsSheet = true;
			metaInfo->faces[5].name = NewStr("bold12");       metaInfo->faces[5].size = 12; metaInfo->faces[5].bold = true;  metaInfo->faces[5].italic = false; metaInfo->faces[5].bakeSize = NewVec2i(256, 256);
			metaInfo->faces[6].name = NewStr("italic12");     metaInfo->faces[6].size = 12; metaInfo->faces[6].bold = false; metaInfo->faces[6].italic = true;  metaInfo->faces[6].bakeSize = NewVec2i(256, 256);
			metaInfo->faces[7].name = NewStr("bolditalic12"); metaInfo->faces[7].size = 12; metaInfo->faces[7].bold = true;  metaInfo->faces[7].italic = true;  metaInfo->faces[7].bakeSize = NewVec2i(256, 256);
			return "Consolas";
			// return "Arial";
			// return "Yu Mincho";
		} break;
		// +==============================+
		// |          pixelFont           |
		// +==============================+
		case 1:
		{
			metaInfo->faces[0].isSpriteFont    = true;
			metaInfo->faces[0].name             = NewStr("main8");
			metaInfo->faces[0].size             = 8;
			metaInfo->faces[0].bold             = false;
			metaInfo->faces[0].italic           = false;
			metaInfo->faces[0].sheetSizes[0]    = NewVec2i(16, 16);
			metaInfo->faces[0].filePaths[0]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel8.png");
			metaInfo->faces[0].metaFilePaths[0] = NewStr(RESOURCE_FOLDER_FONTS "/pixel8.meta");
			metaInfo->faces[0].paddings[0]      = Vec2iFill(1);
			metaInfo->faces[0].isPixelated[0]   = true;
			metaInfo->faces[0].scalables[0]     = false;
			metaInfo->faces[0].colored[0]       = false;
			metaInfo->faces[0].sheetSizes[1]    = NewVec2i(16, 16);
			metaInfo->faces[0].filePaths[1]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns_white.png");
			metaInfo->faces[0].metaFilePaths[1] = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns.meta");
			metaInfo->faces[0].paddings[1]      = Vec2iFill(1);
			metaInfo->faces[0].isPixelated[1]   = true;
			metaInfo->faces[0].scalables[1]     = false;
			metaInfo->faces[0].colored[1]       = false;
			
			metaInfo->faces[1].isSpriteFont    = true;
			metaInfo->faces[1].name             = NewStr("outline8");
			metaInfo->faces[1].size             = 8;
			metaInfo->faces[1].bold             = true;
			metaInfo->faces[1].italic           = false;
			metaInfo->faces[1].sheetSizes[0]    = NewVec2i(16, 16);
			metaInfo->faces[1].filePaths[0]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_outline.png");
			metaInfo->faces[1].metaFilePaths[0] = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_outline.meta");
			metaInfo->faces[1].paddings[0]      = Vec2iFill(1);
			metaInfo->faces[1].isPixelated[0]   = true;
			metaInfo->faces[1].scalables[0]     = false;
			metaInfo->faces[1].colored[0]       = false;
			
			metaInfo->faces[2].isSpriteFont    = true;
			metaInfo->faces[2].name             = NewStr("main16");
			metaInfo->faces[2].size             = 12;
			metaInfo->faces[2].bold             = false;
			metaInfo->faces[2].italic           = false;
			metaInfo->faces[2].sheetSizes[0]    = NewVec2i(16, 12);
			metaInfo->faces[2].filePaths[0]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel16.png");
			metaInfo->faces[2].metaFilePaths[0] = NewStr(RESOURCE_FOLDER_FONTS "/pixel16.meta");
			metaInfo->faces[2].paddings[0]      = Vec2iFill(1);
			metaInfo->faces[2].isPixelated[0]   = true;
			metaInfo->faces[2].scalables[0]     = false;
			metaInfo->faces[2].colored[0]       = false;
			
			return "pixelFont";
		} break;
		// +==============================+
		// |          largeFont           |
		// +==============================+
		case 2:
		{
			metaInfo->requestFromPlatform = true;
			metaInfo->faces[0].name = NewStr("default18");    metaInfo->faces[0].size = 48; metaInfo->faces[0].bold = false; metaInfo->faces[0].italic = false; metaInfo->faces[0].bakeSize = NewVec2i(1024, 1024); metaInfo->faces[0].includeCyrillicBake = true; metaInfo->faces[0].includeJapaneseKanaBake = true; metaInfo->faces[0].includeBtnsSheet = true;
			metaInfo->faces[1].name = NewStr("bold18");       metaInfo->faces[1].size = 48; metaInfo->faces[1].bold = true;  metaInfo->faces[1].italic = false; metaInfo->faces[1].bakeSize = NewVec2i(1024, 1024);
			metaInfo->faces[2].name = NewStr("italic18");     metaInfo->faces[2].size = 48; metaInfo->faces[2].bold = false; metaInfo->faces[2].italic = true;  metaInfo->faces[2].bakeSize = NewVec2i(1024, 1024);
			metaInfo->faces[3].name = NewStr("bolditalic18"); metaInfo->faces[3].size = 48; metaInfo->faces[3].bold = true;  metaInfo->faces[3].italic = true;  metaInfo->faces[3].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[4].name = NewStr("default12");    metaInfo->faces[4].size = 36; metaInfo->faces[4].bold = false; metaInfo->faces[4].italic = false; metaInfo->faces[4].bakeSize = NewVec2i(1024, 1024); metaInfo->faces[4].includeCyrillicBake = true; metaInfo->faces[4].includeJapaneseKanaBake = true; metaInfo->faces[4].includeBtnsSheet = true;
			// metaInfo->faces[5].name = NewStr("bold12");       metaInfo->faces[5].size = 36; metaInfo->faces[5].bold = true;  metaInfo->faces[5].italic = false; metaInfo->faces[5].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[6].name = NewStr("italic12");     metaInfo->faces[6].size = 36; metaInfo->faces[6].bold = false; metaInfo->faces[6].italic = true;  metaInfo->faces[6].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[7].name = NewStr("bolditalic12"); metaInfo->faces[7].size = 36; metaInfo->faces[7].bold = true;  metaInfo->faces[7].italic = true;  metaInfo->faces[7].bakeSize = NewVec2i(1024, 1024);
			return "Consolas";
		} break;
		default: DebugAssert(false); return nullptr;
	}
}

#endif //  _PIG_RESOURCES_H
