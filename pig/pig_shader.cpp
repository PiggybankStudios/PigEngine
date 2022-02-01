/*
File:   pig_shader.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description: 
	** Holds functions that help us load shaders from disk and compile or manipulate them
*/

#define FRAGMENT_VERTEX_DIVIDER_STR "FRAGMENT_SHADER"
u64 FindFragmentShaderDivider(MyStr_t shaderCode)
{
	NotNullStr(&shaderCode);
	if (shaderCode.length == 0) { return 0; }
	
	//Find the split string
	u64 dividerStrLength = MyStrLength64(FRAGMENT_VERTEX_DIVIDER_STR);
	bool foundDividerStr = false;
	u64 dividerStrIndex = 0;
	for (u64 cIndex = 0; cIndex+dividerStrLength < shaderCode.length; cIndex++)
	{
		if (MyStrCompare(&shaderCode.chars[cIndex], FRAGMENT_VERTEX_DIVIDER_STR, dividerStrLength) == 0)
		{
			foundDividerStr = true;
			dividerStrIndex = cIndex;
			break;
		}
	}
	if (!foundDividerStr) { return 0; }
	
	//Go back to previous line break
	bool foundLineBreak = false;
	for (u64 cIndex = dividerStrIndex; cIndex > 0; cIndex--)
	{
		if (shaderCode.chars[cIndex] == '\n' || shaderCode.chars[cIndex] == '\r')
		{
			dividerStrIndex = cIndex+1;
			foundLineBreak = true;
			break;
		}
	}
	if (!foundLineBreak) { return 0; }
	
	return dividerStrIndex;
}

void DestroyShader(Shader_t* shader)
{
	AssertSingleThreaded();
	NotNull(shader);
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			if (shader->glId != 0)
			{
				glDeleteProgram(shader->glId);
				AssertNoOpenGlError();
			}
			if (shader->glVertId != 0)
			{
				glDeleteShader(shader->glVertId);
				AssertNoOpenGlError();
			}
			if (shader->glFragId != 0)
			{
				glDeleteShader(shader->glFragId);
				AssertNoOpenGlError();
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in DestroyShader"); break;
	}
	if (shader->vertexLog.pntr != nullptr)
	{
		NotNull(shader->allocArena);
		FreeString(shader->allocArena, &shader->vertexLog);
	}
	if (shader->fragmentLog.pntr != nullptr)
	{
		NotNull(shader->allocArena);
		FreeString(shader->allocArena, &shader->fragmentLog);
	}
	if (shader->linkLog.pntr != nullptr)
	{
		NotNull(shader->allocArena);
		FreeString(shader->allocArena, &shader->linkLog);
	}
	AssertIf(shader->dynamicUniforms.length > 0, shader->allocArena != nullptr);
	VarArrayLoop(&shader->dynamicUniforms, uIndex)
	{
		VarArrayLoopGet(ShaderDynamicUniform_t, uniform, &shader->dynamicUniforms, uIndex);
		FreeString(shader->allocArena, &uniform->name);
	}
	ClearPointer(shader);
}

//TODO: Add support for some sort of loading log that stores debug output for this process in case of weirder errors
bool CreateShader(MemArena_t* memArena, Shader_t* shaderOut, MyStr_t shaderCode, VertexType_t vertexType, u32 requiredUniforms)
{
	AssertSingleThreaded();
	NotNull(memArena);
	NotNull(shaderOut);
	NotNullStr(&shaderCode);
	Assert(vertexType != VertexType_None);
	ClearPointer(shaderOut);
	if (shaderCode.length == 0) { shaderOut->error = ShaderError_Empty; return false; }
	shaderOut->allocArena = memArena;
	CreateVarArray(&shaderOut->dynamicUniforms, memArena, sizeof(ShaderDynamicUniform_t));
	
	if (memArena != GetTempArena()) { TempPushMark(); }
	
	const char* errorStr = nullptr;
	#if OPENGL_SUPPORTED
	#define CreateShader_CheckOpenGlError(apiCallStr)                                            \
	{                                                                                            \
		errorStr = CheckOpenGlError(true);                                                       \
		if (errorStr != nullptr)                                                                 \
		{                                                                                        \
			shaderOut->apiErrorStr = PrintInArenaStr(memArena, apiCallStr " error: ", errorStr); \
			shaderOut->error = ShaderError_ApiError;                                             \
		}                                                                                        \
	} if (errorStr != nullptr)
	#endif
	
	bool result = false;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			GLboolean shaderCompileSupported = GL_FALSE;
			GLint compileStatus = GL_FALSE;
			int infoLogLength = 0;
			
			// +====================================+
			// | Check if Compilation is Supported  |
			// +====================================+
			glGetBooleanv(GL_SHADER_COMPILER, &shaderCompileSupported);
			CreateShader_CheckOpenGlError("glGetBooleanv(GL_SHADER_COMPILER)")
			{
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (shaderCompileSupported == GL_FALSE)
			{
				shaderOut->error = ShaderError_CompilationNotSupported;
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// | Find Divider and Split Code  |
			// +==============================+
			u64 splitIndex = FindFragmentShaderDivider(shaderCode);
			if (splitIndex == 0)
			{
				shaderOut->error = ShaderError_MissingDivider;
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			MyStr_t vertexCode = StrSubstring(&shaderCode, 0, splitIndex);
			MyStr_t fragmentCode = StrSubstring(&shaderCode, splitIndex, shaderCode.length);
			if (vertexCode.length == 0 || fragmentCode.length == 0)
			{
				shaderOut->error = ShaderError_MissingDivider;
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			//TODO: Re-enable me when we have a proper log to put information into
			// PrintLine_D("%u char vertex, %u char fragment", vertexCode.length, fragmentCode.length);
			
			// +==============================+
			// |     Create Vertex Shader     |
			// +==============================+
			GLuint vertId = glCreateShader(GL_VERTEX_SHADER);
			CreateShader_CheckOpenGlError("glCreateShader(GL_VERTEX_SHADER)")
			{
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (vertId == 0)
			{
				shaderOut->error = ShaderError_VertexCreationFailed;
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |    Compile Vertex Shader     |
			// +==============================+
			GLchar* glVertexCodePntr = (GLchar*)vertexCode.pntr;
			GLint glVertexCodeLength = (GLint)vertexCode.length;
			glShaderSource(vertId, 1, &glVertexCodePntr, &glVertexCodeLength);
			CreateShader_CheckOpenGlError("glShaderSource(vertId)")
			{
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glCompileShader(vertId);
			CreateShader_CheckOpenGlError("glCompileShader(vertId)")
			{
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==================================+
			// | Check Vertex Compilation Status  |
			// +==================================+
			glGetShaderiv(vertId, GL_COMPILE_STATUS, &compileStatus);
			CreateShader_CheckOpenGlError("glGetShaderiv(vertId, GL_COMPILE_STATUS)")
			{
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glGetShaderiv(vertId, GL_INFO_LOG_LENGTH, &infoLogLength);
			CreateShader_CheckOpenGlError("glGetShaderiv(vertId, GL_INFO_LOG_LENGTH)")
			{
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (infoLogLength < 0)
			{
				PrintLine_W("Warning: GL_INFO_LOG_LENGTH came back as negative: %d", infoLogLength);
				infoLogLength = 0;
			}
			
			if (compileStatus != GL_TRUE)
			{
				PrintLine_E("Vertex compilation failed with %d byte info log", infoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (infoLogLength > 3)
			{
				shaderOut->vertexLog.pntr = AllocArray(memArena, char, (u64)infoLogLength+1);
				if (shaderOut->vertexLog.pntr != nullptr)
				{
					shaderOut->vertexLog.length = (u64)infoLogLength;
					glGetShaderInfoLog(vertId, infoLogLength, NULL, shaderOut->vertexLog.pntr);
					CreateShader_CheckOpenGlError("glGetShaderInfoLog(vertId)")
					{
						glDeleteShader(vertId);
						if (memArena != GetTempArena()) { TempPopMark(); }
						return false;
					}
					shaderOut->vertexLog.pntr[shaderOut->vertexLog.length] = '\0';
					PrintLineAt((compileStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Vertex Log:\n%.*s", shaderOut->vertexLog.length, shaderOut->vertexLog.pntr);
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte vertex shader log!", infoLogLength); }
			}
			if (compileStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_VertexCompileFailed;
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |    Create Fragment Shader    |
			// +==============================+
			GLuint fragId = glCreateShader(GL_FRAGMENT_SHADER);
			CreateShader_CheckOpenGlError("glCreateShader(GL_FRAGMENT_SHADER)")
			{
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (fragId == 0)
			{
				shaderOut->error = ShaderError_FragmentCreationFailed;
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |   Compile Fragment Shader    |
			// +==============================+
			GLchar* glFragmentCodePntr = (GLchar*)fragmentCode.pntr;
			GLint glFragmentCodeLength = (GLint)fragmentCode.length;
			glShaderSource(fragId, 1, &glFragmentCodePntr, &glFragmentCodeLength);
			CreateShader_CheckOpenGlError("glShaderSource(fragId)")
			{
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glCompileShader(fragId);
			CreateShader_CheckOpenGlError("glCompileShader(fragId)")
			{
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +====================================+
			// | Check Fragment Compilation Status  |
			// +====================================+
			glGetShaderiv(fragId, GL_COMPILE_STATUS, &compileStatus);
			CreateShader_CheckOpenGlError("glGetShaderiv(fragId, GL_COMPILE_STATUS)")
			{
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glGetShaderiv(fragId, GL_INFO_LOG_LENGTH, &infoLogLength);
			CreateShader_CheckOpenGlError("glGetShaderiv(fragId, GL_INFO_LOG_LENGTH)")
			{
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (infoLogLength < 0)
			{
				PrintLine_W("Warning: GL_INFO_LOG_LENGTH came back as negative: %d", infoLogLength);
				infoLogLength = 0;
			}
			
			if (compileStatus != GL_TRUE)
			{
				PrintLine_E("Fragment compilation failed with %d byte info log", infoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (infoLogLength > 3)
			{
				shaderOut->fragmentLog.pntr = AllocArray(memArena, char, (u64)infoLogLength+1);
				if (shaderOut->fragmentLog.pntr != nullptr)
				{
					shaderOut->fragmentLog.length = (u64)infoLogLength;
					glGetShaderInfoLog(fragId, infoLogLength, NULL, shaderOut->fragmentLog.pntr);
					CreateShader_CheckOpenGlError("glGetShaderInfoLog(fragId)")
					{
						glDeleteShader(fragId);
						glDeleteShader(vertId);
						if (memArena != GetTempArena()) { TempPopMark(); }
						return false;
					}
					shaderOut->fragmentLog.pntr[shaderOut->fragmentLog.length] = '\0';
					PrintLineAt((compileStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Fragment Log:\n%.*s", shaderOut->fragmentLog.length, shaderOut->fragmentLog.pntr);
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte fragment shader log!", infoLogLength); }
			}
			if (compileStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_FragmentCompileFailed;
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |    Create Shader Program     |
			// +==============================+
			GLuint programId = glCreateProgram();
			CreateShader_CheckOpenGlError("glCreateProgram()")
			{
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			if (programId == 0)
			{
				shaderOut->error = ShaderError_ShaderCreationFailed;
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |   Attach and Link Shaders    |
			// +==============================+
			glAttachShader(programId, vertId);
			CreateShader_CheckOpenGlError("glAttachShader(vertId)")
			{
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glAttachShader(programId, fragId);
			CreateShader_CheckOpenGlError("glAttachShader(fragId)")
			{
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glLinkProgram(programId);
			CreateShader_CheckOpenGlError("glLinkProgram(programId)")
			{
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |      Check Link Status       |
			// +==============================+
			glGetProgramiv(programId, GL_LINK_STATUS, &compileStatus);
			CreateShader_CheckOpenGlError("glGetProgramiv(programId, GL_LINK_STATUS)")
			{
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
			CreateShader_CheckOpenGlError("glGetProgramiv(programId, GL_INFO_LOG_LENGTH)")
			{
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			if (compileStatus != GL_TRUE)
			{
				PrintLine_E("Shader linking failed with %d byte info log", infoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (infoLogLength > 3)
			{
				shaderOut->linkLog.pntr = AllocArray(memArena, char, (u64)infoLogLength+1);
				if (shaderOut->linkLog.pntr != nullptr)
				{
					shaderOut->linkLog.length = (u64)infoLogLength;
					glGetProgramInfoLog(programId, infoLogLength, NULL, shaderOut->linkLog.pntr);
					CreateShader_CheckOpenGlError("glGetProgramInfoLog(programId)")
					{
						glDeleteProgram(programId);
						glDeleteShader(fragId);
						glDeleteShader(vertId);
						if (memArena != GetTempArena()) { TempPopMark(); }
						return false;
					}
					shaderOut->linkLog.pntr[shaderOut->linkLog.length] = '\0';
					PrintLineAt((compileStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Link Log:\n%.*s", shaderOut->linkLog.length, shaderOut->linkLog.pntr);
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte link log!", infoLogLength); }
			}
			if (compileStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_LinkingFailed;
				glDeleteProgram(programId);
				glDeleteShader(fragId);
				glDeleteShader(vertId);
				if (memArena != GetTempArena()) { TempPopMark(); }
				return false;
			}
			
			// +==============================+
			// |   Find Attribute Locations   |
			// +==============================+
			#define CreateShader_FindAttribute(attribIndexVar, attributeName, vertexTypeBit) do             \
			{                                                                                               \
				attribIndexVar = glGetAttribLocation(programId, attributeName);                             \
				CreateShader_CheckOpenGlError("glGetAttribLocation(programId, \"" attributeName "\")")      \
				{                                                                                           \
					glDeleteProgram(programId);                                                             \
					glDeleteShader(fragId);                                                                 \
					glDeleteShader(vertId);                                                                 \
					if (memArena != GetTempArena()) { TempPopMark(); }                                      \
					return false;                                                                           \
				}                                                                                           \
				if (attribIndexVar >= 0)                                                                    \
				{                                                                                           \
					shaderOut->vertexType = (VertexType_t)(((u32)shaderOut->vertexType) | (vertexTypeBit)); \
				}                                                                                           \
				else                                                                                        \
				{                                                                                           \
					PrintLine_W("Warning: Shader is missing " attributeName " attribute");                  \
					shaderOut->error = ShaderError_MissingAttribute;                                        \
				}                                                                                           \
			} while(0)
			if (IsFlagSet(vertexType, VertexType_PositionBit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.position, "inPosition", VertexType_PositionBit);
			}
			else { shaderOut->attribLocations.gl.position = -1; }
			if (IsFlagSet(vertexType, VertexType_Color1Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.color1, "inColor1", VertexType_Color1Bit);
			}
			else { shaderOut->attribLocations.gl.color1 = -1; }
			if (IsFlagSet(vertexType, VertexType_Color2Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.color2, "inColor2", VertexType_Color2Bit);
			}
			else { shaderOut->attribLocations.gl.color2 = -1; }
			if (IsFlagSet(vertexType, VertexType_TexCoord1Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.texCoord1, "inTexCoord1", VertexType_TexCoord1Bit);
			}
			else { shaderOut->attribLocations.gl.texCoord1 = -1; }
			if (IsFlagSet(vertexType, VertexType_TexCoord2Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.texCoord2, "inTexCoord2", VertexType_TexCoord2Bit);
			}
			else { shaderOut->attribLocations.gl.texCoord2 = -1; }
			if (IsFlagSet(vertexType, VertexType_Normal1Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.normal1, "inNormal1", VertexType_Normal1Bit);
			}
			else { shaderOut->attribLocations.gl.normal1 = -1; }
			if (IsFlagSet(vertexType, VertexType_Normal2Bit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.normal2, "inNormal2", VertexType_Normal2Bit);
			}
			else { shaderOut->attribLocations.gl.normal2 = -1; }
			if (IsFlagSet(vertexType, VertexType_TangentBit))
			{
				CreateShader_FindAttribute(shaderOut->attribLocations.gl.tangent, "inTangent", VertexType_TangentBit);
			}
			else { shaderOut->attribLocations.gl.tangent = -1; }
			#undef CreateShader_FindAttribute
			
			// +==============================+
			// |    Find Uniform Locations    |
			// +==============================+
			#define CreateShader_FindUniform(uniformIndexVar, uniformName, uniformBit) do                        \
			{                                                                                                    \
				(uniformIndexVar) = glGetUniformLocation(programId, uniformName);                                \
				CreateShader_CheckOpenGlError("glGetUniformLocation(programId, \"" uniformName "\"")             \
				{                                                                                                \
					glDeleteProgram(programId);                                                                  \
					glDeleteShader(fragId);                                                                      \
					glDeleteShader(vertId);                                                                      \
					if (memArena != GetTempArena()) { TempPopMark(); }                                           \
					return false;                                                                                \
				}                                                                                                \
				if ((uniformIndexVar) >= 0)                                                                      \
				{                                                                                                \
					FlagSet(shaderOut->uniformFlags, (uniformBit));                                              \
				}                                                                                                \
				else if (IsFlagSet(requiredUniforms, (uniformBit)))                                              \
				{                                                                                                \
					if (shaderOut->error == ShaderError_None) { shaderOut->error = ShaderError_MissingUniform; } \
					PrintLine_W("Warning: Shader is missing \"" uniformName "\" required uniform");              \
				}                                                                                                \
			} while(0)
			CreateShader_FindUniform(shaderOut->glLocations.worldMatrix,       "WorldMatrix",       ShaderUniform_WorldMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.viewMatrix,        "ViewMatrix",        ShaderUniform_ViewMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.projectionMatrix,  "ProjectionMatrix",  ShaderUniform_ProjectionMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.cameraPosition,    "CameraPosition",    ShaderUniform_CameraPosition);
			CreateShader_FindUniform(shaderOut->glLocations.texture1,          "Texture1",          ShaderUniform_Texture1);
			CreateShader_FindUniform(shaderOut->glLocations.texture1Size,      "Texture1Size",      ShaderUniform_Texture1Size);
			CreateShader_FindUniform(shaderOut->glLocations.texture2,          "Texture2",          ShaderUniform_Texture2);
			CreateShader_FindUniform(shaderOut->glLocations.texture2Size,      "Texture2Size",      ShaderUniform_Texture2Size);
			CreateShader_FindUniform(shaderOut->glLocations.sourceRec1,        "SourceRec1",        ShaderUniform_SourceRec1);
			CreateShader_FindUniform(shaderOut->glLocations.sourceRec2,        "SourceRec2",        ShaderUniform_SourceRec2);
			CreateShader_FindUniform(shaderOut->glLocations.maskRectangle,     "MaskRectangle",     ShaderUniform_MaskRectangle);
			CreateShader_FindUniform(shaderOut->glLocations.shiftVec,          "ShiftVec",          ShaderUniform_ShiftVec);
			CreateShader_FindUniform(shaderOut->glLocations.color1,            "Color1",            ShaderUniform_Color1);
			CreateShader_FindUniform(shaderOut->glLocations.color2,            "Color2",            ShaderUniform_Color2);
			CreateShader_FindUniform(shaderOut->glLocations.replaceColors,     "ReplaceColors",     ShaderUniform_ReplaceColors);
			CreateShader_FindUniform(shaderOut->glLocations.time,              "Time",              ShaderUniform_Time);
			CreateShader_FindUniform(shaderOut->glLocations.brightness,        "Brightness",        ShaderUniform_Brightness);
			CreateShader_FindUniform(shaderOut->glLocations.saturation,        "Saturation",        ShaderUniform_Saturation);
			CreateShader_FindUniform(shaderOut->glLocations.circleRadius,      "CircleRadius",      ShaderUniform_CircleRadius);
			CreateShader_FindUniform(shaderOut->glLocations.circleInnerRadius, "CircleInnerRadius", ShaderUniform_CircleInnerRadius);
			DebugAssert(ShaderUniform_NumGenericValues == 8); //Update this code if the number of generic attributes change
			CreateShader_FindUniform(shaderOut->glLocations.value[0],          "Value0",            ShaderUniform_Value0);
			CreateShader_FindUniform(shaderOut->glLocations.value[1],          "Value1",            ShaderUniform_Value1);
			CreateShader_FindUniform(shaderOut->glLocations.value[2],          "Value2",            ShaderUniform_Value2);
			CreateShader_FindUniform(shaderOut->glLocations.value[3],          "Value3",            ShaderUniform_Value3);
			CreateShader_FindUniform(shaderOut->glLocations.value[4],          "Value4",            ShaderUniform_Value4);
			CreateShader_FindUniform(shaderOut->glLocations.value[5],          "Value5",            ShaderUniform_Value5);
			CreateShader_FindUniform(shaderOut->glLocations.value[6],          "Value6",            ShaderUniform_Value6);
			CreateShader_FindUniform(shaderOut->glLocations.value[7],          "Value7",            ShaderUniform_Value7);
			#undef CreateShader_FindUniform
			
			shaderOut->glId = programId;
			shaderOut->glVertId = vertId;
			shaderOut->glFragId = fragId;
			//vertexType bits set by CreateShader_FindAttribute
			//uniformBits set by CreateShader_FindUniform
			shaderOut->requiredUniforms = requiredUniforms;
			shaderOut->id = pig->nextShaderId;
			pig->nextShaderId++;
			shaderOut->isValid = true;
			result = (shaderOut->error == ShaderError_None);
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			shaderOut->error = ShaderError_UnsupportedApi;
			return false;
		} break;
	}
	
	if (memArena != GetTempArena()) { TempPopMark(); }
	
	#undef CreateShader_CheckOpenGlError
	return result;
}

bool LoadShader(MemArena_t* memArena, Shader_t* shaderOut, MyStr_t filePath, VertexType_t vertexType, u32 requiredUniforms)
{
	AssertSingleThreaded();
	NotNull(plat);
	NotNull(memArena);
	NotNull(shaderOut);
	NotEmptyStr(&filePath);
	Assert(vertexType != VertexType_None);
	ClearPointer(shaderOut);
	
	PlatFileContents_t shaderFile;
	if (!plat->ReadFileContents(filePath, &shaderFile))
	{
		shaderOut->error = ShaderError_CouldntOpenFile;
		return false;
	}
	
	bool result = CreateShader(memArena, shaderOut, NewStr(shaderFile.size, shaderFile.chars), vertexType, requiredUniforms);
	AssertIf(result == false, shaderOut->error != ShaderError_None);
	
	plat->FreeFileContents(&shaderFile);
	return result;
}

ShaderDynamicUniform_t* GetDynamicUniform(Shader_t* shader, MyStr_t uniformName, bool searchForUniform)
{
	NotNull(shader);
	if (!shader->isValid) { return nullptr; }
	
	VarArrayLoop(&shader->dynamicUniforms, uIndex)
	{
		VarArrayLoopGet(ShaderDynamicUniform_t, uniform, &shader->dynamicUniforms, uIndex);
		if (StrEquals(uniform->name, uniformName))
		{
			return uniform;
		}
	}
	
	ShaderDynamicUniform_t* result = nullptr;
	
	if (searchForUniform)
	{
		switch (pig->renderApi)
		{
			// +==============================+
			// |            OpenGL            |
			// +==============================+
			#if OPENGL_SUPPORTED
			case RenderApi_OpenGL:
			{
				NotNull(shader->allocArena);
				Assert(IsStrNullTerminated(&uniformName));
				GLint uniformLocation = glGetUniformLocation(shader->glId, uniformName.pntr);
				AssertNoOpenGlError();
				if (uniformLocation >= 0)
				{
					ShaderDynamicUniform_t* newUniform = VarArrayAdd(&shader->dynamicUniforms, ShaderDynamicUniform_t);
					NotNull(newUniform);
					ClearPointer(newUniform);
					newUniform->uniformIndex = (shader->dynamicUniforms.length-1);
					newUniform->name = AllocString(shader->allocArena, &uniformName);
					newUniform->glLocation = uniformLocation;
					result = newUniform;
				}
			} break;
			#endif
			
			// +==============================+
			// |       Unsupported API        |
			// +==============================+
			default:
			{
				DebugAssert(false);
			} break;
		}
	}
	
	return result;
}
