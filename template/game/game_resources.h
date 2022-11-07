/*
File:   game_resources.h
Author: Taylor Robbins
Date:   01\31\2022
Description:
	** Holds the list of resources we want the resource system to manage and provide us access to
*/

#ifndef _GAME_RESOURCES_H
#define _GAME_RESOURCES_H

#define RESOURCES_NUM_TEXTURES    14
#define RESOURCES_NUM_VECTORS     2
#define RESOURCES_NUM_SHEETS      4
#define RESOURCES_NUM_SHADERS     11
#define RESOURCES_NUM_FONTS       4
#define RESOURCES_NUM_SOUNDS      3
#define RESOURCES_NUM_MUSICS      2
#define TOTAL_NUM_RESOURCES       (RESOURCES_NUM_TEXTURES + RESOURCES_NUM_VECTORS + RESOURCES_NUM_SHEETS + RESOURCES_NUM_SHADERS + RESOURCES_NUM_FONTS + RESOURCES_NUM_SOUNDS + RESOURCES_NUM_MUSICS)

#define RESOURCE_FOLDER_FONTS          "Resources/Fonts"
#define RESOURCE_FOLDER_MUSIC          "Resources/Music"
#define RESOURCE_FOLDER_MODELS         "Resources/Models"
#define RESOURCE_FOLDER_TEXT           "Resources/Text"
#define RESOURCE_FOLDER_SHADERS        "Resources/Shaders"
#define RESOURCE_FOLDER_SHEETS         "Resources/Sheets"
#define RESOURCE_FOLDER_SOUNDS         "Resources/Sounds"
#define RESOURCE_FOLDER_SPRITES        "Resources/Sprites"
#define RESOURCE_FOLDER_TEXTURES       "Resources/Textures"
#define RESOURCE_FOLDER_VECTOR         "Resources/Vector"

#define PARTICLES_FILE_PATH            RESOURCE_FOLDER_TEXT "/particles.txt"

// +--------------------------------------------------------------+
// |                        Resource Lists                        |
// +--------------------------------------------------------------+
START_PACK()
union ATTR_PACKED ResourceTextures_t
{
	Texture_t items[RESOURCES_NUM_TEXTURES];
	struct
	{
		Texture_t piggybank;
		Texture_t piggyLoading;
		Texture_t piggyBlob;
		Texture_t gifRecording;
		Texture_t madeInPigEngine;
		Texture_t alpha;
		Texture_t defaultPink;
		Texture_t defaultBlue;
		Texture_t defaultGreen;
		Texture_t defaultPurple;
		Texture_t defaultRed;
		Texture_t defaultYellow;
		Texture_t blueGradientBack;
		
		Texture_t mainMenuBackground;
	};
};
union ATTR_PACKED ResourceVectors_t
{
	VectorImg_t items[RESOURCES_NUM_VECTORS];
	struct
	{
		VectorImg_t test;
		VectorImg_t pig;
	};
};
union ATTR_PACKED ResourceSheets_t
{
	SpriteSheet_t items[RESOURCES_NUM_SHEETS];
	struct
	{
		SpriteSheet_t pigAnim;
		SpriteSheet_t vectorIcons64;
		SpriteSheet_t controllerBtns;
		SpriteSheet_t parts;
	};
};
union ATTR_PACKED ResourceShaders_t
{
	Shader_t items[RESOURCES_NUM_SHADERS];
	struct
	{
		Shader_t main2D;
		Shader_t main2DArray;
		Shader_t main3D;
		Shader_t main3DArray;
		Shader_t gradient2D;
		Shader_t roundedCorners;
		Shader_t pieChart;
		Shader_t bezier3;
		Shader_t bezier4;
		Shader_t ellipseArc;
		
		Shader_t testShader;
	};
};
union ATTR_PACKED ResourceFonts_t
{
	Font_t items[RESOURCES_NUM_FONTS];
	struct
	{
		Font_t debug;
		Font_t pixel;
		Font_t large;
		Font_t title;
	};
};
union ATTR_PACKED ResourceSounds_t
{
	Sound_t items[RESOURCES_NUM_SOUNDS];
	struct
	{
		Sound_t oink;
		Sound_t click1;
		Sound_t notification;
	};
};
union ATTR_PACKED ResourceMusics_t
{
	Sound_t items[RESOURCES_NUM_MUSICS];
	struct
	{
		Sound_t test;
		Sound_t mainMenu;
	};
};
END_PACK()

// +--------------------------------------------------------------+
// |                      Texture Meta Info                       |
// +--------------------------------------------------------------+
const char* Resources_GetPathForTexture(u64 textureIndex, ResourceTextureMetaInfo_t* metaInfo)
{
	NotNull(metaInfo);
	ClearPointer(metaInfo);
	metaInfo->pixelated = false;
	metaInfo->repeating = true;
	metaInfo->numLayers = 0;
	#define NORMAL_TEXTURE_META_INFO(isPixelated, isRepeating, folder, filename) metaInfo->pixelated = (isPixelated); metaInfo->repeating = (isRepeating); return RESOURCE_FOLDER_##folder "/" filename;
	switch (textureIndex)
	{
		//                            pixelated repeating     folder               filename
		case 0:  NORMAL_TEXTURE_META_INFO(true,  false, SPRITES,  "piggybank.png");             //| piggybank        |
		case 1:  NORMAL_TEXTURE_META_INFO(true,  false, SPRITES,  "loading_image.png");         //| piggyLoading     |
		case 2:  NORMAL_TEXTURE_META_INFO(true,  false, SPRITES,  "piggyblob.png");             //| piggyBlob        |
		case 3:  NORMAL_TEXTURE_META_INFO(true,  false, SPRITES,  "gif_recording.png");         //| gifRecording     |
		case 4:  NORMAL_TEXTURE_META_INFO(true,  false, SPRITES,  "made_in_pig_engine.png");    //| madeInPigEngine  |
		case 5:  NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "alpha.png");                 //| alpha            |
		case 6:  NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid.png");               //| defaultPink      |
		case 7:  NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid_blue.png");          //| defaultBlue      |
		case 8:  NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid_green.png");         //| defaultGreen     |
		case 9:  NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid_purple.png");        //| defaultPurple    |
		case 10: NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid_red.png");           //| defaultRed       |
		case 11: NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "invalid_yellow.png");        //| defaultYellow    |
		case 12: NORMAL_TEXTURE_META_INFO(true,  true,  TEXTURES, "blue_gradient_back.png");    //| blueGradientBack |
		
		case 13: NORMAL_TEXTURE_META_INFO(false, false, SPRITES,  "main_menu.png");             //| mainMenuBackground    |
		
		default: DebugAssert(false); return nullptr;
	}
	#undef NORMAL_TEXTURE_META_INFO
}

// +--------------------------------------------------------------+
// |                     VectorImg Meta Info                      |
// +--------------------------------------------------------------+
const char* Resources_GetPathForVectorImg(u64 vectorImgIndex)
{
	switch (vectorImgIndex)
	{
		case 0: return RESOURCE_FOLDER_VECTOR  "/test.svg";             //| test           |
		case 1: return RESOURCE_FOLDER_VECTOR  "/pig.svg";              //| pig            |
		case 2: return RESOURCE_FOLDER_VECTOR  "/pig_parts_title.svg";  //| pigPartsTitle  |
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                    Sprite Sheet Meta Info                    |
// +--------------------------------------------------------------+
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
		// +==============================+
		// |           pigAnim            |
		// +==============================+
		case 0:
		{
			metaInfo->numFrames = NewVec2i(4, 1);
			return RESOURCE_FOLDER_SHEETS "/pig_anim.png";
		} break;
		// +==============================+
		// |        vectorIcons64         |
		// +==============================+
		case 1:
		{
			metaInfo->numFrames = NewVec2i(4, 4);
			metaInfo->metaFilePath = NewStr(RESOURCE_FOLDER_SHEETS "/vector_icons64.meta");
			metaInfo->pixelated = false;
			return RESOURCE_FOLDER_SHEETS "/vector_icons64.png";
		} break;
		// +==============================+
		// |        controllerBtns        |
		// +==============================+
		case 2:
		{
			metaInfo->numFrames = NewVec2i(2, 2);
			metaInfo->pixelated = false;
			return RESOURCE_FOLDER_SHEETS "/controller_btns.png";
		} break;
		// +==============================+
		// |            parts             |
		// +==============================+
		case 3:
		{
			metaInfo->numFrames = NewVec2i(32, 32);
			metaInfo->padding = NewVec2i(1, 1);
			metaInfo->pixelated = true;
			return RESOURCE_FOLDER_SHEETS "/parts.png";
		} break;
		default: Assert(false); return nullptr;
	}
}
const char* GetResourceSheetName(u64 sheetIndex)
{
	switch (sheetIndex)
	{
		case 0: return "pigAnim";
		case 1: return "vectorIcons64";
		case 2: return "controllerBtns";
		case 3: return "parts";
		default: return "Unknown";
	}
}

// +--------------------------------------------------------------+
// |                       Shader Meta Info                       |
// +--------------------------------------------------------------+
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
		// |            main2D            |
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
		// |         main2DArray          |
		// +==============================+
		case 1:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/main2DArray.glsl"; 
		} break;
		// +==============================+
		// |            main3D            |
		// +==============================+
		case 2:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default3D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/main3D.glsl"; 
		} break;
		// +==============================+
		// |         main3DArray          |
		// +==============================+
		case 3:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
				metaInfo->vertexType = VertexType_Default3D;
			}
			return RESOURCE_FOLDER_SHADERS "/main3DArray.glsl";
		} break;
		// +==============================+
		// |          gradient2D          |
		// +==============================+
		case 4:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1|ShaderUniform_Color2);
			}
			return RESOURCE_FOLDER_SHADERS "/gradient2D.glsl"; 
		} break;
		// +==============================+
		// |        roundedCorners        |
		// +==============================+
		case 5:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_RequireTexture|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/roundedCorners.glsl"; 
		} break;
		// +==============================+
		// |           pieChart           |
		// +==============================+
		case 6:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices);
			}
			return RESOURCE_FOLDER_SHADERS "/pieChart.glsl"; 
		} break;
		// +==============================+
		// |           bezier3            |
		// +==============================+
		case 7:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/bezier3.glsl"; 
		} break;
		// +==============================+
		// |           bezier4            |
		// +==============================+
		case 8:
		{
			if (metaInfo != nullptr)
			{
				metaInfo->vertexType = VertexType_Default2D;
				metaInfo->requiredUniforms = (ShaderUniform_RequireMatrices|ShaderUniform_Color1);
			}
			return RESOURCE_FOLDER_SHADERS "/bezier4.glsl"; 
		} break;
		// +==============================+
		// |          ellipseArc          |
		// +==============================+
		case 9:
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
			metaInfo->fontName = "debugFont";
			metaInfo->requestFromPlatform = true;
			metaInfo->faces[0].name  = NewStr("default18");    metaInfo->faces[0].size  = 18; metaInfo->faces[0].bold  = false; metaInfo->faces[0].italic  = false; metaInfo->faces[0].bakeSize  = NewVec2i(256, 256); metaInfo->faces[0].includeCyrillicBake = true; metaInfo->faces[0].includeJapaneseKanaBake = true; metaInfo->faces[0].includeBtnsSheet = true;
			metaInfo->faces[1].name  = NewStr("bold18");       metaInfo->faces[1].size  = 18; metaInfo->faces[1].bold  = true;  metaInfo->faces[1].italic  = false; metaInfo->faces[1].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[2].name  = NewStr("italic18");     metaInfo->faces[2].size  = 18; metaInfo->faces[2].bold  = false; metaInfo->faces[2].italic  = true;  metaInfo->faces[2].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[3].name  = NewStr("bolditalic18"); metaInfo->faces[3].size  = 18; metaInfo->faces[3].bold  = true;  metaInfo->faces[3].italic  = true;  metaInfo->faces[3].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[4].name  = NewStr("default12");    metaInfo->faces[4].size  = 12; metaInfo->faces[4].bold  = false; metaInfo->faces[4].italic  = false; metaInfo->faces[4].bakeSize  = NewVec2i(256, 256); metaInfo->faces[4].includeCyrillicBake = true; metaInfo->faces[4].includeJapaneseKanaBake = true; metaInfo->faces[4].includeBtnsSheet = true;
			metaInfo->faces[5].name  = NewStr("bold12");       metaInfo->faces[5].size  = 12; metaInfo->faces[5].bold  = true;  metaInfo->faces[5].italic  = false; metaInfo->faces[5].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[6].name  = NewStr("italic12");     metaInfo->faces[6].size  = 12; metaInfo->faces[6].bold  = false; metaInfo->faces[6].italic  = true;  metaInfo->faces[6].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[7].name  = NewStr("bolditalic12"); metaInfo->faces[7].size  = 12; metaInfo->faces[7].bold  = true;  metaInfo->faces[7].italic  = true;  metaInfo->faces[7].bakeSize  = NewVec2i(256, 256);
			metaInfo->faces[8].name  = NewStr("default24");    metaInfo->faces[8].size  = 24; metaInfo->faces[8].bold  = false; metaInfo->faces[8].italic  = false; metaInfo->faces[8].bakeSize  = NewVec2i(512, 512); metaInfo->faces[8].includeCyrillicBake = true; metaInfo->faces[8].includeJapaneseKanaBake = true; metaInfo->faces[8].includeBtnsSheet = true;
			metaInfo->faces[9].name  = NewStr("bold24");       metaInfo->faces[9].size  = 24; metaInfo->faces[9].bold  = true;  metaInfo->faces[9].italic  = false; metaInfo->faces[9].bakeSize  = NewVec2i(512, 512);
			metaInfo->faces[10].name = NewStr("italic24");     metaInfo->faces[10].size = 24; metaInfo->faces[10].bold = false; metaInfo->faces[10].italic = true;  metaInfo->faces[10].bakeSize = NewVec2i(512, 512);
			metaInfo->faces[11].name = NewStr("bolditalic24"); metaInfo->faces[11].size = 24; metaInfo->faces[11].bold = true;  metaInfo->faces[11].italic = true;  metaInfo->faces[11].bakeSize = NewVec2i(512, 512);
			// return "Verdana";
			// return "Cascadia";
			return "Consolas";
			// return "Arial";
			// return "Yu Mincho";
		} break;
		// +==============================+
		// |          pixelFont           |
		// +==============================+
		case 1:
		{
			metaInfo->fontName = "pixelFont";
			metaInfo->faces[0].isSpriteFont    = true;
			metaInfo->faces[0].name             = NewStr("main8");
			metaInfo->faces[0].size             = 8;
			metaInfo->faces[0].bold             = false;
			metaInfo->faces[0].italic           = false;
			metaInfo->faces[0].sheetSizes[0]    = NewVec2i(16, 16);
			metaInfo->faces[0].filePaths[0]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel8.png");
			metaInfo->faces[0].metaFilePaths[0] = NewStr(RESOURCE_FOLDER_FONTS "/pixel8.meta");
			metaInfo->faces[0].paddings[0]      = Vec2i_Zero;
			metaInfo->faces[0].isPixelated[0]   = true;
			metaInfo->faces[0].scalables[0]     = false;
			metaInfo->faces[0].colored[0]       = false;
			metaInfo->faces[0].sheetSizes[1]    = NewVec2i(16, 16);
			metaInfo->faces[0].filePaths[1]     = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns_white.png");
			metaInfo->faces[0].metaFilePaths[1] = NewStr(RESOURCE_FOLDER_FONTS "/pixel8_btns.meta");
			metaInfo->faces[0].paddings[1]      = Vec2i_Zero;
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
			metaInfo->faces[1].paddings[0]      = Vec2i_Zero;
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
			metaInfo->faces[2].paddings[0]      = Vec2i_Zero;
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
			metaInfo->fontName = "largeFont";
			metaInfo->requestFromPlatform = true;
			metaInfo->faces[0].name = NewStr("default48");    metaInfo->faces[0].size = 48; metaInfo->faces[0].bold = false; metaInfo->faces[0].italic = false; metaInfo->faces[0].bakeSize = NewVec2i(1024, 1024); metaInfo->faces[0].includeCyrillicBake = true; metaInfo->faces[0].includeJapaneseKanaBake = true; metaInfo->faces[0].includeBtnsSheet = true;
			metaInfo->faces[1].name = NewStr("bold48");       metaInfo->faces[1].size = 48; metaInfo->faces[1].bold = true;  metaInfo->faces[1].italic = false; metaInfo->faces[1].bakeSize = NewVec2i(1024, 1024);
			metaInfo->faces[2].name = NewStr("italic48");     metaInfo->faces[2].size = 48; metaInfo->faces[2].bold = false; metaInfo->faces[2].italic = true;  metaInfo->faces[2].bakeSize = NewVec2i(1024, 1024);
			metaInfo->faces[3].name = NewStr("bolditalic48"); metaInfo->faces[3].size = 48; metaInfo->faces[3].bold = true;  metaInfo->faces[3].italic = true;  metaInfo->faces[3].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[4].name = NewStr("default12");    metaInfo->faces[4].size = 36; metaInfo->faces[4].bold = false; metaInfo->faces[4].italic = false; metaInfo->faces[4].bakeSize = NewVec2i(1024, 1024); metaInfo->faces[4].includeCyrillicBake = true; metaInfo->faces[4].includeJapaneseKanaBake = true; metaInfo->faces[4].includeBtnsSheet = true;
			// metaInfo->faces[5].name = NewStr("bold12");       metaInfo->faces[5].size = 36; metaInfo->faces[5].bold = true;  metaInfo->faces[5].italic = false; metaInfo->faces[5].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[6].name = NewStr("italic12");     metaInfo->faces[6].size = 36; metaInfo->faces[6].bold = false; metaInfo->faces[6].italic = true;  metaInfo->faces[6].bakeSize = NewVec2i(1024, 1024);
			// metaInfo->faces[7].name = NewStr("bolditalic12"); metaInfo->faces[7].size = 36; metaInfo->faces[7].bold = true;  metaInfo->faces[7].italic = true;  metaInfo->faces[7].bakeSize = NewVec2i(1024, 1024);
			return "Verdana";
		} break;
		// +==============================+
		// |          titleFont           |
		// +==============================+
		case 3:
		{
			metaInfo->fontName = "titleFont";
			metaInfo->requestFromPlatform = true;
			metaInfo->faces[0].name = NewStr("bold96"); metaInfo->faces[0].size = 128; metaInfo->faces[0].bold = true; metaInfo->faces[0].italic = false; metaInfo->faces[0].bakeSize = NewVec2i(1024, 1024);
			return "Verdana";
		} break;
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                       Sound Meta Info                        |
// +--------------------------------------------------------------+
const char* Resources_GetPathForSound(u64 soundIndex)
{
	switch (soundIndex)
	{
		case 0:  return RESOURCE_FOLDER_SOUNDS "/oink.ogg";         // | oink         |
		case 1:  return RESOURCE_FOLDER_SOUNDS "/click1.ogg";       // | click1       |
		case 2:  return RESOURCE_FOLDER_SOUNDS "/notification.ogg"; // | notification |
		default: DebugAssert(false); return nullptr;
	}
}

// +--------------------------------------------------------------+
// |                       Music Meta Info                        |
// +--------------------------------------------------------------+
const char* Resources_GetPathForMusic(u64 musicIndex)
{
	switch (musicIndex)
	{
		case 0: return RESOURCE_FOLDER_MUSIC "/test.ogg";      // | test     |
		case 1: return RESOURCE_FOLDER_MUSIC "/main_menu.ogg"; // | mainMenu |
		default: DebugAssert(false); return nullptr;
	}
}

#endif //  _GAME_RESOURCES_H
