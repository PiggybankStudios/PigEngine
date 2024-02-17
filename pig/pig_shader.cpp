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
	FreeVarArray(&shader->dynamicUniforms);
	AssertIf(shader->vertexCodePieces.length > 0, shader->allocArena != nullptr);
	VarArrayLoop(&shader->vertexCodePieces, pIndex)
	{
		VarArrayLoopGet(MyStr_t, codePiece, &shader->vertexCodePieces, pIndex);
		FreeString(shader->allocArena, codePiece);
	}
	FreeVarArray(&shader->vertexCodePieces);
	AssertIf(shader->fragmentCodePieces.length > 0, shader->allocArena != nullptr);
	VarArrayLoop(&shader->fragmentCodePieces, pIndex)
	{
		VarArrayLoopGet(MyStr_t, codePiece, &shader->fragmentCodePieces, pIndex);
		FreeString(shader->allocArena, codePiece);
	}
	FreeVarArray(&shader->fragmentCodePieces);
	ClearPointer(shader);
}

void CreateShaderMultiPieceStart(MemArena_t* memArena, Shader_t* shaderOut, VertexType_t vertexType, u32 requiredUniforms, u64 numVertexPieces = 0, u64 numFragmentPieces = 0)
{
	AssertSingleThreaded();
	NotNull(memArena);
	NotNull(shaderOut);
	Assert(vertexType != VertexType_None);
	ClearPointer(shaderOut);
	shaderOut->allocArena = memArena;
	shaderOut->error = ShaderError_None;
	shaderOut->vertexType = vertexType;
	shaderOut->requiredUniforms = requiredUniforms;
	CreateVarArray(&shaderOut->dynamicUniforms, memArena, sizeof(ShaderDynamicUniform_t));
	
	CreateVarArray(&shaderOut->vertexCodePieces, memArena, sizeof(MyStr_t), numVertexPieces);
	CreateVarArray(&shaderOut->fragmentCodePieces, memArena, sizeof(MyStr_t), numFragmentPieces);
	
	const char* errorStr = nullptr;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			#define CreateShader_CheckOpenGlError(apiCallStr)                                                         \
			{                                                                                                         \
				errorStr = CheckOpenGlError(true);                                                                    \
				if (errorStr != nullptr)                                                                              \
				{                                                                                                     \
					shaderOut->apiErrorStr = PrintInArenaStr(shaderOut->allocArena, apiCallStr " error: ", errorStr); \
					shaderOut->error = ShaderError_ApiError;                                                          \
				}                                                                                                     \
			} if (errorStr != nullptr)
			
			// +====================================+
			// | Check if Compilation is Supported  |
			// +====================================+
			GLboolean shaderCompileSupported = GL_FALSE;
			glGetBooleanv(GL_SHADER_COMPILER, &shaderCompileSupported);
			CreateShader_CheckOpenGlError("glGetBooleanv(GL_SHADER_COMPILER)") { return; }
			if (shaderCompileSupported == GL_FALSE) { shaderOut->error = ShaderError_CompilationNotSupported; return; }
			
			// +==============================+
			// |     Create Vertex Shader     |
			// +==============================+
			shaderOut->glVertId = glCreateShader(GL_VERTEX_SHADER);
			CreateShader_CheckOpenGlError("glCreateShader(GL_VERTEX_SHADER)") { return; }
			if (shaderOut->glVertId == 0) { shaderOut->error = ShaderError_VertexCreationFailed; return; }
			
			// +==============================+
			// |    Create Fragment Shader    |
			// +==============================+
			shaderOut->glFragId = glCreateShader(GL_FRAGMENT_SHADER);
			CreateShader_CheckOpenGlError("glCreateShader(GL_FRAGMENT_SHADER)") { return; }
			if (shaderOut->glFragId == 0) { shaderOut->error = ShaderError_FragmentCreationFailed; return; }
			
			// +==============================+
			// |    Create Shader Program     |
			// +==============================+
			shaderOut->glId = glCreateProgram();
			CreateShader_CheckOpenGlError("glCreateProgram()") { return; }
			if (shaderOut->glId == 0) { shaderOut->error = ShaderError_ShaderCreationFailed; return; }
			
			#undef CreateShader_CheckOpenGlError
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			shaderOut->error = ShaderError_UnsupportedApi;
		} break;
	}
}
void CreateShaderMultiPieceVertex(Shader_t* shaderOut, MyStr_t vertexPiece)
{
	AssertSingleThreaded();
	NotNull(shaderOut);
	NotNull(shaderOut->allocArena);
	NotNullStr(&vertexPiece);
	if (shaderOut->error != ShaderError_None) { return; }
	MyStr_t* newPiecePntr = VarArrayAdd(&shaderOut->vertexCodePieces, MyStr_t);
	NotNull(newPiecePntr);
	*newPiecePntr = AllocString(shaderOut->allocArena, &vertexPiece);
	NotNullStr(newPiecePntr);
}
void CreateShaderMultiPieceFragment(Shader_t* shaderOut, MyStr_t fragmentPiece)
{
	AssertSingleThreaded();
	NotNull(shaderOut);
	NotNull(shaderOut->allocArena);
	NotNullStr(&fragmentPiece);
	if (shaderOut->error != ShaderError_None) { return; }
	MyStr_t* newPiecePntr = VarArrayAdd(&shaderOut->fragmentCodePieces, MyStr_t);
	NotNull(newPiecePntr);
	*newPiecePntr = AllocString(shaderOut->allocArena, &fragmentPiece);
	NotNullStr(newPiecePntr);
}
bool CreateShaderMultiPieceEnd(Shader_t* shaderOut)
{
	AssertSingleThreaded();
	NotNull(shaderOut);
	NotNull(shaderOut->allocArena);
	if (shaderOut->error != ShaderError_None) { return false; }
	
	bool isSlugShader = IsFlagSet(shaderOut->vertexType, VertexType_SlugBit);
	bool result = false;
	const char* errorStr = nullptr;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			#define CreateShader_CheckOpenGlError(apiCallStr)                                                         \
			{                                                                                                         \
				errorStr = CheckOpenGlError(true);                                                                    \
				if (errorStr != nullptr)                                                                              \
				{                                                                                                     \
					shaderOut->apiErrorStr = PrintInArenaStr(shaderOut->allocArena, apiCallStr " error: ", errorStr); \
					shaderOut->error = ShaderError_ApiError;                                                          \
				}                                                                                                     \
			} if (errorStr != nullptr)
			
			if (shaderOut->vertexCodePieces.length == 0) { shaderOut->error = ShaderError_NoVertShaderPieces; return false; }
			if (shaderOut->fragmentCodePieces.length == 0) { shaderOut->error = ShaderError_NoFragShaderPieces; return false; }
			
			// +==============================+
			// |    Compile Vertex Shader     |
			// +==============================+
			GLchar** glVertexCodePiecePntrs = AllocArray(TempArena, GLchar*, shaderOut->vertexCodePieces.length);
			NotNull(glVertexCodePiecePntrs);
			GLint* glVertexCodePieceLengths = AllocArray(TempArena, GLint, shaderOut->vertexCodePieces.length);
			NotNull(glVertexCodePieceLengths);
			u64 totalVertexCodeLength = 0;
			VarArrayLoop(&shaderOut->vertexCodePieces, pIndex)
			{
				VarArrayLoopGet(MyStr_t, codePiece, &shaderOut->vertexCodePieces, pIndex);
				glVertexCodePiecePntrs[pIndex] = (GLchar*)codePiece->pntr;
				glVertexCodePieceLengths[pIndex] = (GLint)codePiece->length;
				totalVertexCodeLength += codePiece->length;
			}
			if (totalVertexCodeLength == 0) { shaderOut->error = ShaderError_Empty; return false; }
			
			glShaderSource(shaderOut->glVertId, (GLsizei)shaderOut->vertexCodePieces.length, glVertexCodePiecePntrs, glVertexCodePieceLengths);
			CreateShader_CheckOpenGlError("glShaderSource(vertId)") { return false; }
			glCompileShader(shaderOut->glVertId);
			CreateShader_CheckOpenGlError("glCompileShader(vertId)") { return false; }
			
			// +==================================+
			// | Check Vertex Compilation Status  |
			// +==================================+
			GLint vertCompileStatus = 0;
			GLint vertInfoLogLength = 0;
			glGetShaderiv(shaderOut->glVertId, GL_COMPILE_STATUS, &vertCompileStatus);
			CreateShader_CheckOpenGlError("glGetShaderiv(vertId, GL_COMPILE_STATUS)") { return false; }
			glGetShaderiv(shaderOut->glVertId, GL_INFO_LOG_LENGTH, &vertInfoLogLength);
			CreateShader_CheckOpenGlError("glGetShaderiv(vertId, GL_INFO_LOG_LENGTH)") { return false; }
			
			if (vertInfoLogLength < 0)
			{
				PrintLine_W("Warning: GL_INFO_LOG_LENGTH came back as negative: %d", vertInfoLogLength);
				vertInfoLogLength = 0;
			}
			if (vertCompileStatus != GL_TRUE)
			{
				PrintLine_E("Vertex compilation failed with %d byte info log", vertInfoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (vertInfoLogLength > 3)
			{
				shaderOut->vertexLog.pntr = AllocArray(shaderOut->allocArena, char, (u64)vertInfoLogLength+1);
				if (shaderOut->vertexLog.pntr != nullptr)
				{
					shaderOut->vertexLog.length = (u64)vertInfoLogLength;
					glGetShaderInfoLog(shaderOut->glVertId, vertInfoLogLength, NULL, shaderOut->vertexLog.pntr);
					CreateShader_CheckOpenGlError("glGetShaderInfoLog(vertId)") { return false; }
					shaderOut->vertexLog.pntr[shaderOut->vertexLog.length] = '\0';
					PrintLineAt((vertCompileStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Vertex Log:\n%.*s", StrPrint(shaderOut->vertexLog));
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte vertex shader log!", vertInfoLogLength); }
			}
			if (vertCompileStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_VertexCompileFailed;
				return false;
			}
			
			// +==============================+
			// |   Compile Fragment Shader    |
			// +==============================+
			GLchar** glFragmentCodePiecePntrs = AllocArray(TempArena, GLchar*, shaderOut->fragmentCodePieces.length);
			NotNull(glFragmentCodePiecePntrs);
			GLint* glFragmentCodePieceLengths = AllocArray(TempArena, GLint, shaderOut->fragmentCodePieces.length);
			NotNull(glFragmentCodePieceLengths);
			u64 totalFragmentCodeLength = 0;
			VarArrayLoop(&shaderOut->fragmentCodePieces, pIndex)
			{
				VarArrayLoopGet(MyStr_t, codePiece, &shaderOut->fragmentCodePieces, pIndex);
				glFragmentCodePiecePntrs[pIndex] = (GLchar*)codePiece->pntr;
				glFragmentCodePieceLengths[pIndex] = (GLint)codePiece->length;
				totalFragmentCodeLength += codePiece->length;
			}
			if (totalFragmentCodeLength == 0) { shaderOut->error = ShaderError_Empty; return false; }
			
			glShaderSource(shaderOut->glFragId, (GLsizei)shaderOut->fragmentCodePieces.length, glFragmentCodePiecePntrs, glFragmentCodePieceLengths);
			CreateShader_CheckOpenGlError("glShaderSource(fragId)") { return false; }
			glCompileShader(shaderOut->glFragId);
			CreateShader_CheckOpenGlError("glCompileShader(fragId)") { return false; }
			
			// +==================================+
			// | Check Vertex Compilation Status  |
			// +==================================+
			GLint fragCompileStatus = 0;
			GLint fragInfoLogLength = 0;
			glGetShaderiv(shaderOut->glFragId, GL_COMPILE_STATUS, &fragCompileStatus);
			CreateShader_CheckOpenGlError("glGetShaderiv(fragId, GL_COMPILE_STATUS)") { return false; }
			glGetShaderiv(shaderOut->glFragId, GL_INFO_LOG_LENGTH, &fragInfoLogLength);
			CreateShader_CheckOpenGlError("glGetShaderiv(fragId, GL_INFO_LOG_LENGTH)") { return false; }
			
			if (fragInfoLogLength < 0)
			{
				PrintLine_W("Warning: GL_INFO_LOG_LENGTH came back as negative: %d", fragInfoLogLength);
				fragInfoLogLength = 0;
			}
			if (fragCompileStatus != GL_TRUE)
			{
				PrintLine_E("Fragment compilation failed with %d byte info log", fragInfoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (fragInfoLogLength > 3)
			{
				shaderOut->fragmentLog.pntr = AllocArray(shaderOut->allocArena, char, (u64)fragInfoLogLength+1);
				if (shaderOut->fragmentLog.pntr != nullptr)
				{
					shaderOut->fragmentLog.length = (u64)fragInfoLogLength;
					glGetShaderInfoLog(shaderOut->glFragId, fragInfoLogLength, NULL, shaderOut->fragmentLog.pntr);
					CreateShader_CheckOpenGlError("glGetShaderInfoLog(fragId)") { return false; }
					shaderOut->fragmentLog.pntr[shaderOut->fragmentLog.length] = '\0';
					PrintLineAt((fragCompileStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Fragment Log:\n%.*s", StrPrint(shaderOut->fragmentLog));
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte fragment shader log!", fragInfoLogLength); }
			}
			if (fragCompileStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_VertexCompileFailed;
				return false;
			}
			
			// +==============================+
			// |   Attach and Link Shaders    |
			// +==============================+
			glAttachShader(shaderOut->glId, shaderOut->glVertId);
			CreateShader_CheckOpenGlError("glAttachShader(vertId)") { return false; }
			glAttachShader(shaderOut->glId, shaderOut->glFragId);
			CreateShader_CheckOpenGlError("glAttachShader(fragId)") { return false; }
			glLinkProgram(shaderOut->glId);
			CreateShader_CheckOpenGlError("glLinkProgram(programId)") { return false; }
			
			// +==============================+
			// |      Check Link Status       |
			// +==============================+
			GLint linkStatus = 0;
			GLint linkInfoLogLength = 0;
			glGetProgramiv(shaderOut->glId, GL_LINK_STATUS, &linkStatus);
			CreateShader_CheckOpenGlError("glGetProgramiv(programId, GL_LINK_STATUS)") { return false; }
			glGetProgramiv(shaderOut->glId, GL_INFO_LOG_LENGTH, &linkInfoLogLength);
			CreateShader_CheckOpenGlError("glGetProgramiv(programId, GL_INFO_LOG_LENGTH)") { return false; }
			
			if (linkStatus != GL_TRUE)
			{
				PrintLine_E("Shader linking failed with %d byte info log", linkInfoLogLength);
			}
			//NOTE: We were getting 1 byte logs before so we just put a lower bound on length
			//      to make sure the log actually has useful informatin in it before printing it out
			if (linkInfoLogLength > 3)
			{
				shaderOut->linkLog.pntr = AllocArray(shaderOut->allocArena, char, (u64)linkInfoLogLength+1);
				if (shaderOut->linkLog.pntr != nullptr)
				{
					shaderOut->linkLog.length = (u64)linkInfoLogLength;
					glGetProgramInfoLog(shaderOut->glId, linkInfoLogLength, NULL, shaderOut->linkLog.pntr);
					CreateShader_CheckOpenGlError("glGetProgramInfoLog(programId)") { return false; }
					shaderOut->linkLog.pntr[shaderOut->linkLog.length] = '\0';
					PrintLineAt((linkStatus != GL_TRUE) ? DbgLevel_Error : DbgLevel_Warning, "Link Log:\n%.*s", StrPrint(shaderOut->linkLog));
				}
				else { PrintLine_W("Warning: Failed to allocate space for %d byte link log!", linkInfoLogLength); }
			}
			if (linkStatus != GL_TRUE)
			{
				shaderOut->error = ShaderError_LinkingFailed;
				return false;
			}
			
			// +==============================+
			// |   Find Attribute Locations   |
			// +==============================+
			#define CreateShader_FindAttribute(attribIndexVar, attributeName, vertexTypeBit) do                          \
			{                                                                                                            \
				attribIndexVar = glGetAttribLocation(shaderOut->glId, attributeName);                                    \
				CreateShader_CheckOpenGlError("glGetAttribLocation(programId, \"" attributeName "\")") { return false; } \
				if (attribIndexVar >= 0)                                                                                 \
				{                                                                                                        \
					shaderOut->actualVertexType = (VertexType_t)(((u32)shaderOut->actualVertexType) | (vertexTypeBit));  \
				}                                                                                                        \
				else                                                                                                     \
				{                                                                                                        \
					PrintLine_W("Warning: Shader is missing " attributeName " attribute");                               \
					shaderOut->error = ShaderError_MissingAttribute;                                                     \
				}                                                                                                        \
			} while(0)
			shaderOut->actualVertexType = VertexType_None;
			if (isSlugShader)
			{
				shaderOut->attribLocations.gl.slug.attrib0 = 0;
				shaderOut->attribLocations.gl.slug.attrib1 = 1;
				shaderOut->attribLocations.gl.slug.attrib2 = 2;
				shaderOut->attribLocations.gl.slug.attrib3 = 3;
				shaderOut->attribLocations.gl.slug.attrib4 = 4;
			}
			else
			{
				if (IsFlagSet(shaderOut->vertexType, VertexType_PositionBit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.position, "inPosition", VertexType_PositionBit);
				}
				else { shaderOut->attribLocations.gl.position = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_Color1Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.color1, "inColor1", VertexType_Color1Bit);
				}
				else { shaderOut->attribLocations.gl.color1 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_Color2Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.color2, "inColor2", VertexType_Color2Bit);
				}
				else { shaderOut->attribLocations.gl.color2 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_TexCoord1Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.texCoord1, "inTexCoord1", VertexType_TexCoord1Bit);
				}
				else { shaderOut->attribLocations.gl.texCoord1 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_TexCoord2Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.texCoord2, "inTexCoord2", VertexType_TexCoord2Bit);
				}
				else { shaderOut->attribLocations.gl.texCoord2 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_Normal1Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.normal1, "inNormal1", VertexType_Normal1Bit);
				}
				else { shaderOut->attribLocations.gl.normal1 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_Normal2Bit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.normal2, "inNormal2", VertexType_Normal2Bit);
				}
				else { shaderOut->attribLocations.gl.normal2 = -1; }
				if (IsFlagSet(shaderOut->vertexType, VertexType_TangentBit))
				{
					CreateShader_FindAttribute(shaderOut->attribLocations.gl.tangent, "inTangent", VertexType_TangentBit);
				}
				else { shaderOut->attribLocations.gl.tangent = -1; }
			}
			#undef CreateShader_FindAttribute
			
			// +==============================+
			// |    Find Uniform Locations    |
			// +==============================+
			#define CreateShader_FindUniform(uniformIndexVar, uniformName, uniformBit) do                                \
			{                                                                                                            \
				(uniformIndexVar) = glGetUniformLocation(shaderOut->glId, uniformName);                                  \
				CreateShader_CheckOpenGlError("glGetUniformLocation(programId, \"" #uniformName "\")") { return false; } \
				if ((uniformIndexVar) >= 0)                                                                              \
				{                                                                                                        \
					FlagSet(shaderOut->uniformFlags, (uniformBit));                                                      \
				}                                                                                                        \
				else if (IsFlagSet(shaderOut->requiredUniforms, (uniformBit)))                                           \
				{                                                                                                        \
					if (shaderOut->error == ShaderError_None) { shaderOut->error = ShaderError_MissingUniform; }         \
					PrintLine_W("Warning: Shader is missing \"%s\" required uniform", (uniformName));                    \
				}                                                                                                        \
			} while(0)
			// float4 slug_matrix[4]
			// float2 slug_viewport
			// const char* worldMatrixUniName = (isSlugShader ? "slug_matrix" : "WorldMatrix");
			const char* texture1UniName = (isSlugShader ? "curveTexture" : "Texture1");
			const char* texture2UniName = (isSlugShader ? "bandTexture" : "Texture2");
			DebugAssert(ShaderUniform_NumGenericValues == 8); //Update this code if the number of generic attributes change
			shaderOut->uniformFlags = ShaderUniform_None;
			CreateShader_FindUniform(shaderOut->glLocations.worldMatrix,       "WorldMatrix",       ShaderUniform_WorldMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.viewMatrix,        "ViewMatrix",        ShaderUniform_ViewMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.projectionMatrix,  "ProjectionMatrix",  ShaderUniform_ProjectionMatrix);
			CreateShader_FindUniform(shaderOut->glLocations.cameraPosition,    "CameraPosition",    ShaderUniform_CameraPosition);
			CreateShader_FindUniform(shaderOut->glLocations.playerPosition,    "PlayerPosition",    ShaderUniform_PlayerPosition);
			CreateShader_FindUniform(shaderOut->glLocations.texture1,          texture1UniName,     ShaderUniform_Texture1);
			CreateShader_FindUniform(shaderOut->glLocations.texture1Size,      "Texture1Size",      ShaderUniform_Texture1Size);
			CreateShader_FindUniform(shaderOut->glLocations.texture2,          texture2UniName,     ShaderUniform_Texture2);
			CreateShader_FindUniform(shaderOut->glLocations.texture2Size,      "Texture2Size",      ShaderUniform_Texture2Size);
			CreateShader_FindUniform(shaderOut->glLocations.sourceRec1,        "SourceRec1",        ShaderUniform_SourceRec1);
			CreateShader_FindUniform(shaderOut->glLocations.sourceRec2,        "SourceRec2",        ShaderUniform_SourceRec2);
			CreateShader_FindUniform(shaderOut->glLocations.maskRectangle,     "MaskRectangle",     ShaderUniform_MaskRectangle);
			CreateShader_FindUniform(shaderOut->glLocations.shiftVec,          "ShiftVec",          ShaderUniform_ShiftVec);
			CreateShader_FindUniform(shaderOut->glLocations.color1,            "Color1",            ShaderUniform_Color1);
			CreateShader_FindUniform(shaderOut->glLocations.color2,            "Color2",            ShaderUniform_Color2);
			CreateShader_FindUniform(shaderOut->glLocations.replaceColors,     "ReplaceColors",     ShaderUniform_ReplaceColors);
			CreateShader_FindUniform(shaderOut->glLocations.time,              "Time",              ShaderUniform_Time);
			CreateShader_FindUniform(shaderOut->glLocations.count,             "Count",             ShaderUniform_Count);
			CreateShader_FindUniform(shaderOut->glLocations.brightness,        "Brightness",        ShaderUniform_Brightness);
			CreateShader_FindUniform(shaderOut->glLocations.saturation,        "Saturation",        ShaderUniform_Saturation);
			CreateShader_FindUniform(shaderOut->glLocations.circleRadius,      "CircleRadius",      ShaderUniform_CircleRadius);
			CreateShader_FindUniform(shaderOut->glLocations.circleInnerRadius, "CircleInnerRadius", ShaderUniform_CircleInnerRadius);
			CreateShader_FindUniform(shaderOut->glLocations.value[0],          "Value0",            ShaderUniform_Value0);
			CreateShader_FindUniform(shaderOut->glLocations.value[1],          "Value1",            ShaderUniform_Value1);
			CreateShader_FindUniform(shaderOut->glLocations.value[2],          "Value2",            ShaderUniform_Value2);
			CreateShader_FindUniform(shaderOut->glLocations.value[3],          "Value3",            ShaderUniform_Value3);
			CreateShader_FindUniform(shaderOut->glLocations.value[4],          "Value4",            ShaderUniform_Value4);
			CreateShader_FindUniform(shaderOut->glLocations.value[5],          "Value5",            ShaderUniform_Value5);
			CreateShader_FindUniform(shaderOut->glLocations.value[6],          "Value6",            ShaderUniform_Value6);
			CreateShader_FindUniform(shaderOut->glLocations.value[7],          "Value7",            ShaderUniform_Value7);
			CreateShader_FindUniform(shaderOut->glLocations.polygonPlanes,     "PolygonPlanes",     ShaderUniform_PolygonPlanes);
			#undef CreateShader_FindUniform
			
			if (shaderOut->error == ShaderError_None || shaderOut->error == ShaderError_MissingAttribute || shaderOut->error == ShaderError_MissingUniform)
			{
				shaderOut->isValid = true;
				shaderOut->id = pig->nextShaderId;
				pig->nextShaderId++;
				result = true;
			}
			
			#undef CreateShader_CheckOpenGlError
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			shaderOut->error = ShaderError_UnsupportedApi;
		} break;
	}
	return result;
}

//TODO: Add support for some sort of loading log that stores debug output for this process in case of weirder errors
//NOTE: If this fails, you still need to call DestroyShader after checking error codes/strings in the structure
bool CreateShader(MemArena_t* memArena, Shader_t* shaderOut, MyStr_t shaderCode, VertexType_t vertexType, u32 requiredUniforms)
{
	NotNull(shaderOut);
	NotNullStr(&shaderCode);
	
	// +===============================+
	// | Find Vert/Frag Shader Divider |
	// +===============================+
	MyStr_t divider = NewStr("FRAGMENT_SHADER");
	u64 dividerIndex = 0;
	if (!FindSubstring(shaderCode, divider, &dividerIndex))
	{
		ClearPointer(shaderOut);
		shaderOut->error = ShaderError_MissingDivider;
		return false;
	}
	while (dividerIndex > 0 && shaderCode.pntr[dividerIndex] != '\n') { dividerIndex--; }
	
	CreateShaderMultiPieceStart(memArena, shaderOut, vertexType, requiredUniforms, 1, 1);
	CreateShaderMultiPieceVertex(shaderOut, NewStr(dividerIndex, &shaderCode.pntr[0]));
	CreateShaderMultiPieceFragment(shaderOut, NewStr(shaderCode.length - dividerIndex, &shaderCode.pntr[dividerIndex]));
	return CreateShaderMultiPieceEnd(shaderOut);
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
	if (!plat->ReadFileContents(filePath, nullptr, false, &shaderFile))
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
