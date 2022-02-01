/*
File:   win32_render_basic.cpp
Author: Taylor Robbins
Date:   10\05\2021
Description: 
	** Holds a bunch of functions that help us perform basic rendering operations in the platform layer.
	** This is mostly used to render the loading screen and debug overlays that are provided by the platform layer
*/

#define WIN32_TAB_WIDTH     4 //spaces

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
#define FRAGMENT_VERTEX_SPLIT_STR "FRAGMENT_SHADER"
u64 Win32_FindFragmentShaderSplit(MyStr_t shaderCode)
{
	NotNullStr(&shaderCode);
	if (shaderCode.length == 0) { return 0; }
	
	//Find the split string
	u64 splitStrLength = MyStrLength64(FRAGMENT_VERTEX_SPLIT_STR);
	bool foundSplitStr = false;
	u64 splitStrIndex = 0;
	for (u64 cIndex = 0; cIndex+splitStrLength < shaderCode.length; cIndex++)
	{
		if (MyStrCompare(&shaderCode.chars[cIndex], FRAGMENT_VERTEX_SPLIT_STR, splitStrLength) == 0)
		{
			foundSplitStr = true;
			splitStrIndex = cIndex;
			break;
		}
	}
	if (!foundSplitStr) { return 0; }
	
	//Go back to previous line break
	bool foundLineBreak = false;
	for (u64 cIndex = splitStrIndex; cIndex > 0; cIndex--)
	{
		if (shaderCode.chars[cIndex] == '\n' || shaderCode.chars[cIndex] == '\r')
		{
			splitStrIndex = cIndex+1;
			foundLineBreak = true;
			break;
		}
	}
	if (!foundLineBreak) { return 0; }
	
	return splitStrIndex;
}

// +--------------------------------------------------------------+
// |                       OpenGL Functions                       |
// +--------------------------------------------------------------+
#if OPENGL_SUPPORTED

const char* Win32_GetOpenGlErrorStr(GLenum glError, bool printUnkown = false)
{
	switch (glError)
	{
		case GL_NO_ERROR:                      return "NO_ERROR";
		case GL_INVALID_ENUM:                  return "INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "INVALID_OPERATION";
		case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW";
		case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
		case GL_CONTEXT_LOST:                  return "GL_CONTEXT_LOST";
		case GL_TABLE_TOO_LARGE:               return "GL_TABLE_TOO_LARGE";
		default: const char* result = (printUnkown ? TempPrint("[0x%08X]", glError) : "Unknown"); NotNull(result); return result;
	}
}

#define Win32_AssertNoOpenGlError(functionName) do                                                          \
{                                                                                                           \
	GLenum glError = glGetError();                                                                          \
	if (glError != GL_NO_ERROR)                                                                             \
	{                                                                                                       \
		PrintLine_E("Got OpenGL Error from %s: %s", #functionName, Win32_GetOpenGlErrorStr(glError, true)); \
	}                                                                                                       \
	Assert(glError == GL_NO_ERROR);                                                                         \
} while(0)

Win32_VertexArrayObject_t Win32_CreateVertexArrayObject_OpenGL(u64 windowId)
{
	Win32_VertexArrayObject_t result = {};
	result.windowId = windowId;
	glGenVertexArrays(1, &result.glId); Win32_AssertNoOpenGlError(glGenVertexArrays);
	glBindVertexArray(result.glId); Win32_AssertNoOpenGlError(glBindVertexArray);
	return result;
}

Win32_Texture_t Win32_CreateTexture_OpenGL(const PlatImageData_t* imageData, bool pixelated, bool repeating)
{
	AssertSingleThreaded();
	NotNull(imageData);
	Assert(imageData->width > 0 && imageData->height > 0);
	NotNull(imageData->data8);
	Win32_Texture_t result = {};
	
	glGenTextures(1, &result.glId); Win32_AssertNoOpenGlError(glGenTextures);
	glBindTexture(GL_TEXTURE_2D, result.glId); Win32_AssertNoOpenGlError(glBindTexture);
	
	GLenum dataFormat = GL_RGBA;
	GLenum internalFormat = GL_RGBA;
	if (imageData->pixelSize == 1)
	{
		dataFormat = GL_RED;
		internalFormat = GL_RED;
	}
	glTexImage2D(
		GL_TEXTURE_2D,      //bound texture type
		0,                  //image level
		internalFormat,     //internal format
		imageData->width,   //image width
		imageData->height,  //image height
		0,                  //border
		dataFormat,         //format
		GL_UNSIGNED_BYTE,   //type
		imageData->data8    //data
	);
	Win32_AssertNoOpenGlError(glTexImage2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR)); Win32_AssertNoOpenGlError(glTexParameteri);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR)); Win32_AssertNoOpenGlError(glTexParameteri);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE)); Win32_AssertNoOpenGlError(glTexParameteri);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE)); Win32_AssertNoOpenGlError(glTexParameteri);
	glGenerateMipmap(GL_TEXTURE_2D); Win32_AssertNoOpenGlError(glGenerateMipmap);
	
	result.isValid = true;
	result.size = imageData->size;
	result.singleChannel = (imageData->pixelSize == 1);
	return result;
}

Win32_Texture_t Win32_LoadTexture_OpenGL(MyStr_t filePath, bool pixelated, bool repeating)
{
	AssertSingleThreaded();
	NotEmptyStr(&filePath);
	Win32_Texture_t result = {};
	PlatFileContents_t imageFile = {};
	PlatImageData_t imageData = {};
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, false);
	
	if (!Win32_ReadFileContents(filePath, &imageFile))
	{
		PrintLine_E("Failed to open image file at \"%.*s\"", fullPath.length, fullPath.pntr);
		TempPopMark();
		return result;
	}
	
	if (!Win32_TryParseImageFile(&imageFile, 4, &imageData))
	{
		PrintLine_E("Failed to parse image file as an image using stb_image at \"%.*s\"!", fullPath.length, fullPath.pntr);
		Win32_FreeFileContents(&imageFile);
		TempPopMark();
		return result;
	}
	
	result = Win32_CreateTexture_OpenGL(&imageData, pixelated, repeating);
	
	Win32_FreeImageData(&imageData);
	Win32_FreeFileContents(&imageFile);
	
	TempPopMark();
	
	return result;
}

Win32_Shader_t Win32_LoadShader_OpenGL(MyStr_t filePath)
{
	AssertSingleThreaded();
	NotEmptyStr(&filePath);
	Win32_Shader_t result = {};
	PlatFileContents_t shaderFile = {};
	GLint compiled;
	int logLength;
	UNUSED(logLength); //TODO: Remove me!
	
	TempPushMark();
	MyStr_t fullPath = Win32_GetFullPath(GetTempArena(), filePath, false);
	
	if (!Win32_ReadFileContents(filePath, &shaderFile))
	{
		PrintLine_E("Failed to open shader file at \"%.*s\"", fullPath.length, fullPath.pntr);
		TempPopMark();
		return result;
	}
	
	//TODO: Should we check this?
	// glGetBooleanv(GL_SHADER_COMPILER, &shaderCompileSupported);
	
	u64 splitIndex = Win32_FindFragmentShaderSplit(NewStr(shaderFile.size, shaderFile.chars));
	if (splitIndex == 0 || splitIndex >= shaderFile.size)
	{
		PrintLine_E("Couldn't find the split between vertex and fragment shader in \"%.*s\"", fullPath.length, fullPath.pntr);
		Win32_FreeFileContents(&shaderFile);
		TempPopMark();
		return result;
	}
	
	//TODO: Looks like we can pass a length to glShaderSource so maybe this isn't needed?
	shaderFile.chars[splitIndex-1] = '\0'; //put a null-term char at the end of the first section (This should be overwriting a \n char)
	const char* vertexShaderCodePntr = &shaderFile.chars[0];
	const char* fragmentShaderCodePntr = &shaderFile.chars[splitIndex];
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER); Win32_AssertNoOpenGlError(glCreateShader);
	glShaderSource(
		result.vertId,                               //shader
		1,                                           //count
		(const GLchar* const*)&vertexShaderCodePntr, //strings
		nullptr                                      //lengths
	);
	Win32_AssertNoOpenGlError(glShaderSource);
	glCompileShader(result.vertId); Win32_AssertNoOpenGlError(glCompileShader);
	glGetShaderiv(result.vertId, GL_COMPILE_STATUS, &compiled);
	//TODO: Check for compilation errors by retrieving the log!
	// glGetShaderiv(result.vertId, GL_INFO_LOG_LENGTH, &logLength);
	if (compiled != GL_TRUE)
	{
		PrintLine_E("Vertex shader failed to compile in \"%.*s\"", fullPath.length, fullPath.pntr);
		glDeleteShader(result.vertId); Win32_AssertNoOpenGlError(glDeleteShader);
		Win32_FreeFileContents(&shaderFile);
		TempPopMark();
		return result;
	}
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER); Win32_AssertNoOpenGlError(glCreateShader);
	glShaderSource(
		result.fragId,                                 //shader
		1,                                             //count
		(const GLchar* const*)&fragmentShaderCodePntr, //strings
		nullptr                                        //lengths
	);
	Win32_AssertNoOpenGlError(glShaderSource);
	glCompileShader(result.fragId); Win32_AssertNoOpenGlError(glCompileShader);
	glGetShaderiv(result.fragId, GL_COMPILE_STATUS, &compiled);
	//TODO: Check for compilation errors by retrieving the log!
	// glGetShaderiv(result.fragId, GL_INFO_LOG_LENGTH, &logLength);
	if (compiled != GL_TRUE)
	{
		PrintLine_E("Fragment shader failed to compile in \"%.*s\"", fullPath.length, fullPath.pntr);
		glDeleteShader(result.vertId); Win32_AssertNoOpenGlError(glDeleteShader);
		glDeleteShader(result.fragId); Win32_AssertNoOpenGlError(glDeleteShader);
		Win32_FreeFileContents(&shaderFile);
		TempPopMark();
		return result;
	}
	
	Win32_FreeFileContents(&shaderFile);
	
	result.glId = glCreateProgram(); Win32_AssertNoOpenGlError(glCreateProgram);
	glAttachShader(result.glId, result.fragId); Win32_AssertNoOpenGlError(glAttachShader);
	glAttachShader(result.glId, result.vertId); Win32_AssertNoOpenGlError(glAttachShader);
	glLinkProgram(result.glId); Win32_AssertNoOpenGlError(glLinkProgram);
	glGetProgramiv(result.glId, GL_LINK_STATUS, &compiled);
	//TODO: Check for link errors by retrieving the log!
	// glGetProgramiv(result.glId, GL_INFO_LOG_LENGTH, &logLength);
	if (compiled != GL_TRUE)
	{
		PrintLine_E("Shader failed to link in \"%.*s\"", fullPath.length, fullPath.pntr);
		glDeleteProgram(result.glId); Win32_AssertNoOpenGlError(glDeleteShader);
		glDeleteShader(result.vertId); Win32_AssertNoOpenGlError(glDeleteShader);
		glDeleteShader(result.fragId); Win32_AssertNoOpenGlError(glDeleteShader);
		TempPopMark();
		return result;
	}
	
	result.attribLocations.gl.position = glGetAttribLocation(result.glId, "inPosition"); Win32_AssertNoOpenGlError(glGetAttribLocation);
	result.attribLocations.gl.color    = glGetAttribLocation(result.glId, "inColor");    Win32_AssertNoOpenGlError(glGetAttribLocation);
	result.attribLocations.gl.texCoord = glGetAttribLocation(result.glId, "inTexCoord"); Win32_AssertNoOpenGlError(glGetAttribLocation);
	
	result.locations.worldMatrix      = glGetUniformLocation(result.glId, "WorldMatrix");      Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.viewMatrix       = glGetUniformLocation(result.glId, "ViewMatrix");       Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.projectionMatrix = glGetUniformLocation(result.glId, "ProjectionMatrix"); Win32_AssertNoOpenGlError(glGetUniformLocation);
	
	result.locations.texture              = glGetUniformLocation(result.glId, "Texture");              Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.textureSize          = glGetUniformLocation(result.glId, "TextureSize");          Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.singleChannelTexture = glGetUniformLocation(result.glId, "SingleChannelTexture"); Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.sourceRectangle      = glGetUniformLocation(result.glId, "SourceRectangle");      Win32_AssertNoOpenGlError(glGetUniformLocation);
	
	result.locations.primaryColor     = glGetUniformLocation(result.glId, "PrimaryColor");     Win32_AssertNoOpenGlError(glGetUniformLocation);
	result.locations.secondaryColor   = glGetUniformLocation(result.glId, "SecondaryColor");   Win32_AssertNoOpenGlError(glGetUniformLocation);
	
	for (u8 vIndex = 0; vIndex < ArrayCount(result.locations.value); vIndex++)
	{
		result.locations.value[vIndex] = glGetUniformLocation(result.glId, TempPrint("Value%u", vIndex)); Win32_AssertNoOpenGlError(glGetUniformLocation);
	}
	
	result.isValid = true;
	
	TempPopMark();
	
	return result;
}

Win32_VertBuffer_t Win32_CreateVertBuffer_OpenGL(bool dynamic, u64 numVertices, const Win32_Vertex_t* vertices)
{
	AssertSingleThreaded();
	Assert(numVertices > 0);
	NotNull(vertices);
	Win32_VertBuffer_t result = {};
	result.isDynamic = dynamic;
	result.numVertices = numVertices;
	
	glGenBuffers(1, &result.glId); Win32_AssertNoOpenGlError(glGenBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, result.glId); Win32_AssertNoOpenGlError(glBindBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Win32_Vertex_t) * numVertices, vertices, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)); Win32_AssertNoOpenGlError(glBufferData);
	
	result.isValid = true;
	return result;
}

void Win32_BindVertexArrayObject_OpenGL(Win32_VertexArrayObject_t* vao)
{
	NotNull(vao);
	glBindVertexArray(vao->glId); Win32_AssertNoOpenGlError(glBindVertexArray);
	Platform->boundVao = vao;
	
}
void Win32_BindShader_OpenGL(Win32_Shader_t* shader)
{
	AssertSingleThreaded();
	NotNull(shader);
	Assert(shader->isValid);
	glUseProgram(shader->glId);
	Platform->boundShader = shader;
	
	NotNull(Platform->boundVao);
	if (!Platform->boundVao->boundOnce)
	{
		if (shader->attribLocations.gl.position >= 0) { glEnableVertexAttribArray(shader->attribLocations.gl.position); Win32_AssertNoOpenGlError(glEnableVertexAttribArray); }
		if (shader->attribLocations.gl.color    >= 0) { glEnableVertexAttribArray(shader->attribLocations.gl.color);    Win32_AssertNoOpenGlError(glEnableVertexAttribArray); }
		if (shader->attribLocations.gl.texCoord >= 0) { glEnableVertexAttribArray(shader->attribLocations.gl.texCoord); Win32_AssertNoOpenGlError(glEnableVertexAttribArray); }
		Platform->boundVao->boundOnce = true;
	}
}
void Win32_BindVertBuffer_OpenGL(Win32_VertBuffer_t* buffer)
{
	AssertSingleThreaded();
	NotNull(buffer);
	Assert(buffer->isValid);
	NotNull(Platform->boundShader);
	
	NotNull(Platform->boundVao);
	glBindVertexArray(Platform->boundVao->glId); Win32_AssertNoOpenGlError(glBindVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->glId); Win32_AssertNoOpenGlError(glBindBuffer);
	
	Assert(Platform->boundShader->attribLocations.gl.position >= 0);
	glVertexAttribPointer(Platform->boundShader->attribLocations.gl.position, 3, GL_FLOAT, GL_FALSE, sizeof(Win32_Vertex_t), (void*)0); Win32_AssertNoOpenGlError(glVertexAttribPointer);
	if (Platform->boundShader->attribLocations.gl.color >= 0)
	{
		glVertexAttribPointer(Platform->boundShader->attribLocations.gl.color, 4, GL_FLOAT, GL_FALSE, sizeof(Win32_Vertex_t), (void*)sizeof(v3)); Win32_AssertNoOpenGlError(glVertexAttribPointer);
	}
	if (Platform->boundShader->attribLocations.gl.texCoord >= 0)
	{
		glVertexAttribPointer(Platform->boundShader->attribLocations.gl.texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Win32_Vertex_t), (void*)(sizeof(v3)+sizeof(v4))); Win32_AssertNoOpenGlError(glVertexAttribPointer);
	}
	
	Platform->boundBuffer = buffer;
}
void Win32_BindTexture_OpenGL(Win32_Texture_t* texture)
{
	AssertSingleThreaded();
	NotNull(texture);
	Assert(texture->isValid);
	NotNull(Platform->boundShader);
	
	glActiveTexture(GL_TEXTURE0); Win32_AssertNoOpenGlError(glActiveTexture);
	glBindTexture(GL_TEXTURE_2D, texture->glId); Win32_AssertNoOpenGlError(glBindTexture);
	if (Platform->boundShader != nullptr)
	{
		glUniform1i(Platform->boundShader->locations.texture, 0); Win32_AssertNoOpenGlError(glUniform1i);
		glUniform2f(Platform->boundShader->locations.textureSize, (r32)texture->width, (r32)texture->height); Win32_AssertNoOpenGlError(glUniform2f);
		glUniform1i(Platform->boundShader->locations.singleChannelTexture, (GLint)(texture->singleChannel ? 1 : 0)); Win32_AssertNoOpenGlError(glUniform1i);
	}
	
	Platform->boundTexture = texture;
}
void Win32_SetWorldMatrix_OpenGL(mat4 worldMatrix)
{
	NotNull(Platform->boundShader);
	glUniformMatrix4fv(Platform->boundShader->locations.worldMatrix, 1, GL_FALSE, &worldMatrix.values[0][0]);
	Platform->worldMatrix = worldMatrix;
}
void Win32_SetViewMatrix_OpenGL(mat4 viewMatrix)
{
	NotNull(Platform->boundShader);
	glUniformMatrix4fv(Platform->boundShader->locations.viewMatrix, 1, GL_FALSE, &viewMatrix.values[0][0]);
	Platform->viewMatrix = viewMatrix;
}
void Win32_SetProjectionMatrix_OpenGL(mat4 projectionMatrix)
{
	NotNull(Platform->boundShader);
	glUniformMatrix4fv(Platform->boundShader->locations.projectionMatrix, 1, GL_FALSE, &projectionMatrix.values[0][0]);
	Platform->projectionMatrix = projectionMatrix;
}

void Win32_SetShaderValue_OpenGL(u8 index, r32 value)
{
	NotNull(Platform->boundShader);
	Assert(index < ArrayCount(Platform->boundShader->locations.value));
	glUniform1f(Platform->boundShader->locations.value[index], value);
}

void Win32_DrawBuffer_OpenGL(rec sourceRectangle, Color_t color, Color_t secondaryColor)
{
	NotNull(Platform->boundShader);
	NotNull(Platform->boundBuffer);
	NotNull(Platform->boundTexture);
	
	glUniform4f(Platform->boundShader->locations.sourceRectangle, sourceRectangle.x, sourceRectangle.y, sourceRectangle.width, sourceRectangle.height); Win32_AssertNoOpenGlError(glUniform4f); Win32_AssertNoOpenGlError(glUniform4f);
	
	glUniform4f(Platform->boundShader->locations.primaryColor, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f); Win32_AssertNoOpenGlError(glUniform4f);
	glUniform4f(Platform->boundShader->locations.secondaryColor, secondaryColor.r / 255.0f, secondaryColor.g / 255.0f, secondaryColor.b / 255.0f, secondaryColor.a / 255.0f); Win32_AssertNoOpenGlError(glUniform4f);
	
	//TODO: Should we assert that numVertices is less than GLsizei max?
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)Platform->boundBuffer->numVertices); Win32_AssertNoOpenGlError(glDrawArrays);
}

void Win32_InitBasicRendering_OpenGL()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); Win32_AssertNoOpenGlError(glBlendFunc);
	glEnable(GL_BLEND); Win32_AssertNoOpenGlError(glEnable);
	glEnable(GL_DEPTH_TEST); Win32_AssertNoOpenGlError(glEnable);
	glDepthFunc(GL_LEQUAL); Win32_AssertNoOpenGlError(glDepthFunc);
	glEnable(GL_ALPHA_TEST); Win32_AssertNoOpenGlError(glEnable);
	glAlphaFunc(GL_GEQUAL, 0.01f); Win32_AssertNoOpenGlError(glAlphaFunc);
	glDisable(GL_CULL_FACE); Win32_AssertNoOpenGlError(glAlphaFunc);
	glFrontFace(GL_CW); Win32_AssertNoOpenGlError(glAlphaFunc);
}

void Win32_ClearScreen_OpenGL(Color_t clearColor, r32 clearDepth)
{
	v4 colorVec = ToVec4(clearColor);
	glClearColor(colorVec.r, colorVec.g, colorVec.b, colorVec.a);
	glClearDepth(clearDepth);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Win32_SetFramebufferToWindow_OpenGL(PlatWindow_t* window)
{
	UNUSED(window); //TODO: Remove me if we actually use the window here
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif //OPENGL_SUPPORTED

// +--------------------------------------------------------------+
// |                      Routing Functions                       |
// +--------------------------------------------------------------+
Win32_VertexArrayObject_t* Win32_GetVertexArrayObj(u64 windowId)
{
	for (u64 vaoIndex = 0; vaoIndex < Platform->vertexArrayObjs.length; vaoIndex++)
	{
		Win32_VertexArrayObject_t* vao = VarArrayGet(&Platform->vertexArrayObjs, vaoIndex, Win32_VertexArrayObject_t);
		if (vao->windowId == windowId)
		{
			return vao;
		}
	}
	
	Win32_VertexArrayObject_t* newVao = VarArrayAdd(&Platform->vertexArrayObjs, Win32_VertexArrayObject_t);
	NotNull(newVao);
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: *newVao = Win32_CreateVertexArrayObject_OpenGL(windowId); return newVao; break;
		#endif
		default: return nullptr; break;
	}
}
Win32_Texture_t Win32_CreateTexture(const PlatImageData_t* imageData, bool pixelated, bool repeating)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: return Win32_CreateTexture_OpenGL(imageData, pixelated, repeating); break;
		#endif
		default:
		{
			Win32_Texture_t fakeTexture = {};
			fakeTexture.isValid = true;
			fakeTexture.size = Vec2i_One;
			return fakeTexture;
		} break;
	}
}
Win32_Texture_t Win32_LoadTexture(MyStr_t filePath, bool pixelated, bool repeating)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: return Win32_LoadTexture_OpenGL(filePath, pixelated, repeating); break;
		#endif
		default:
		{
			Win32_Texture_t fakeTexture = {};
			fakeTexture.isValid = true;
			fakeTexture.size = Vec2i_One;
			return fakeTexture;
		} break;
	}
}
Win32_Shader_t Win32_LoadShader(MyStr_t filePath)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: return Win32_LoadShader_OpenGL(filePath); break;
		#endif
		default:
		{
			Win32_Shader_t fakeShader = {};
			fakeShader.isValid = true;
			return fakeShader;
		} break;
	}
}
Win32_VertBuffer_t Win32_CreateVertBuffer(bool dynamic, u64 numVertices, const Win32_Vertex_t* vertices)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: return Win32_CreateVertBuffer_OpenGL(dynamic, numVertices, vertices); break;
		#endif
		default:
		{
			Win32_VertBuffer_t fakeBuffer = {};
			fakeBuffer.isValid = true;
			fakeBuffer.numVertices = numVertices;
			return fakeBuffer;
		} break;
	}
}
Win32_Font_t Win32_CreateFont(PlatFontData_t* fontData, bool pixelated) //not-routed
{
	NotNull(fontData);
	Win32_Font_t result = {};
	if (fontData->charData == nullptr || fontData->imageData.data8 == nullptr || fontData->ranges == nullptr)
	{
		WriteLine_E("Win32_CreatFont_OpenGL was called with invalid fontData!");
		return result;
	}
	
	result.bake = Win32_CreateTexture(&fontData->imageData, pixelated, false);
	if (!result.bake.isValid)
	{
		PrintLine_E("Failed to create (%d, %d) texture for baked font data in Win32_CreateFont", fontData->imageData.width, fontData->imageData.height);
		return result;
	}
	
	result.numChars = fontData->numChars;
	result.chars = AllocArray(&Platform->mainHeap, Win32_FontChar_t, fontData->numChars);
	NotNull(result.chars);
	NotNull(fontData->charData);
	for (u64 cIndex = 0; cIndex < fontData->numChars; cIndex++)
	{
		Win32_FontChar_t* resultChar = &result.chars[cIndex];
		PlatFontCharData_t* charData = &fontData->charData[cIndex];
		
		//TODO: Should drawRec be offset/smaller in any scenarios?
		//TODO: What is offset2 for?
		resultChar->codepoint = charData->codepoint;
		resultChar->sourceRec = charData->sourceRec;
		resultChar->logicalRec = NewRec(charData->offset, ToVec2(charData->sourceRec.size));
		resultChar->renderOffset = charData->offset;
		resultChar->advanceX = charData->advanceX;
		
		r32 charHeight = -(resultChar->logicalOffset.y);
		r32 charHeightRender = -(resultChar->renderOffset.y);
		r32 lineHeight = (r32)resultChar->logicalSize.height;
		r32 lineHeightRender = (r32)resultChar->renderSize.height;
		r32 leftOverhang = MinR32(-resultChar->logicalOffset.x, 0);
		r32 leftOverhangRender = MinR32(-resultChar->renderOffset.x, 0);
		if (result.maxHeight < charHeight) { result.maxHeight = charHeight; }
		if (result.maxHeightRender < charHeightRender) { result.maxHeightRender = charHeightRender; }
		if (result.maxLineHeight < lineHeight) { result.maxLineHeight = lineHeight; }
		if (result.maxLineHeightRender < lineHeightRender) { result.maxLineHeightRender = lineHeightRender; }
		if (result.maxLeftOverang < leftOverhang) { result.maxLeftOverang = leftOverhang; }
		if (result.maxLeftOverangRender < leftOverhangRender) { result.maxLeftOverangRender = leftOverhangRender; }
	}
	
	result.numRanges = fontData->numRanges;
	result.ranges = AllocArray(&Platform->stdHeap, PlatFontRange_t, fontData->numRanges);
	NotNull(result.ranges);
	for (u64 rIndex = 0; rIndex < result.numRanges; rIndex++)
	{
		PlatFontRange_t* fromRange = &fontData->ranges[rIndex];
		PlatFontRange_t* resultRange = &result.ranges[rIndex];
		MyMemCopy(resultRange, fromRange, sizeof(PlatFontRange_t));
		if (fromRange->codePointList != nullptr)
		{
			resultRange->codePointList = AllocArray(&Platform->stdHeap, u32, fromRange->numChars);
			NotNull(resultRange->codePointList);
			MyMemCopy(resultRange->codePointList, fromRange->codePointList, sizeof(u32) * fromRange->numChars);
		}
	}
	
	result.lineAdvance = result.maxLineHeight + 1;
	result.isValid = true;
	return result;
}
void Win32_BindVertexArrayObject(Win32_VertexArrayObject_t* vao)
{
	NotNull(vao);
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_BindVertexArrayObject_OpenGL(vao); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_BindShader(Win32_Shader_t* shader)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_BindShader_OpenGL(shader); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_BindVertBuffer(Win32_VertBuffer_t* buffer)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_BindVertBuffer_OpenGL(buffer); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_BindTexture(Win32_Texture_t* texture)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_BindTexture_OpenGL(texture); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_SetWorldMatrix(mat4 worldMatrix)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_SetWorldMatrix_OpenGL(worldMatrix); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_SetViewMatrix(mat4 viewMatrix)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_SetViewMatrix_OpenGL(viewMatrix); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_SetProjectionMatrix(mat4 projectionMatrix)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_SetProjectionMatrix_OpenGL(projectionMatrix); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_SetShaderValue(u8 index, r32 value)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_SetShaderValue_OpenGL(index, value); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_DrawBuffer(rec sourceRectangle, Color_t color, Color_t secondaryColor)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_DrawBuffer_OpenGL(sourceRectangle, color, secondaryColor); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_InitBasicRendering()
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_InitBasicRendering_OpenGL(); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_ClearScreen(Color_t clearColor, r32 clearDepth)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_ClearScreen_OpenGL(clearColor, clearDepth); break;
		#endif
		default: /* do nothing */ break;
	}
}
void Win32_SetFramebufferToWindow(PlatWindow_t* window)
{
	switch (Platform->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL: Win32_SetFramebufferToWindow_OpenGL(window); break;
		#endif
		default: /* do nothing */ break;
	}
}

// +--------------------------------------------------------------+
// |                   Common Helper Functions                    |
// +--------------------------------------------------------------+
Win32_FontChar_t* GetFontChar(Win32_Font_t* font, u32 codepoint, u64* charIndexOut = nullptr)
{
	NotNull(font);
	if (font->numRanges == 0) { return nullptr; }
	NotNull(font->ranges);
	u64 rangeStartIndex = 0;
	for (u64 rIndex = 0; rIndex < font->numRanges; rIndex++)
	{
		PlatFontRange_t* range = &font->ranges[rIndex];
		if (range->codePointList != nullptr)
		{
			for (u64 cIndex = 0; cIndex < range->numChars; cIndex++)
			{
				if (codepoint == range->codePointList[cIndex])
				{
					u64 resultIndex = rangeStartIndex + cIndex;
					Assert(resultIndex < font->numChars);
					if (charIndexOut != nullptr) { *charIndexOut = resultIndex; }
					return &font->chars[resultIndex];
				}
			}
		}
		else if (codepoint >= range->firstCodepoint && codepoint < range->firstCodepoint + range->numChars)
		{
			u64 resultIndex = rangeStartIndex + (codepoint - range->firstCodepoint);
			Assert(resultIndex < font->numChars);
			if (charIndexOut != nullptr) { *charIndexOut = resultIndex; }
			return &font->chars[resultIndex];
		}
		rangeStartIndex += range->numChars;
	}
	return nullptr;
}

void Win32_DrawTexturedRec(rec drawRec, Color_t color, rec sourceRectangle)
{
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate2(drawRec.x, drawRec.y), //Position
		Mat4Scale2(drawRec.width, drawRec.height)); //Scale
	Win32_SetWorldMatrix(worldMatrix);
	Win32_BindVertBuffer(&Platform->squareBuffer);
	Win32_DrawBuffer(sourceRectangle, color, White);
}

void Win32_DrawBorder(rec innerRec, r32 thickness, Color_t color, rec sourceRectangle, bool includeCorners = true)
{
	rec topRec = NewRec(innerRec.x, innerRec.y - thickness, innerRec.width, thickness);
	if (includeCorners) { topRec.x -= thickness; topRec.width += thickness*2; }
	rec bottomRec = topRec;
	bottomRec.y = innerRec.y + innerRec.height;
	rec leftRec = NewRec(innerRec.x - thickness, innerRec.y, thickness, innerRec.height);
	rec rightRec = leftRec;
	rightRec.x = innerRec.x + innerRec.width;
	Win32_DrawTexturedRec(topRec, color, sourceRectangle);
	Win32_DrawTexturedRec(bottomRec, color, sourceRectangle);
	Win32_DrawTexturedRec(leftRec, color, sourceRectangle);
	Win32_DrawTexturedRec(rightRec, color, sourceRectangle);
}

void Win32_DrawString(Win32_Font_t* font, v2 startPos, r32 scale, Color_t color, MyStr_t str)
{
	NotNull(font);
	bool fontBakeBound = false;
	bool hexBakeBound = false;
	v2 position = startPos;
	u64 lineIndex = 0;
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		char c = str.pntr[cIndex];
		// +==============================+
		// |           New Line           |
		// +==============================+
		if (c == '\n')
		{
			position.x = startPos.x;
			position.y += font->lineAdvance * scale;
			lineIndex = 0;
		}
		// +==============================+
		// |             Tab              |
		// +==============================+
		else if (c == '\t')
		{
			Win32_FontChar_t* spaceChar = GetFontChar(font, (u32)' ');
			if (spaceChar != nullptr)
			{
				u64 nextTabIndex = WIN32_TAB_WIDTH - (lineIndex % WIN32_TAB_WIDTH);
				position.x += spaceChar->advanceX * nextTabIndex * scale;
				lineIndex += nextTabIndex;
			}
		}
		else
		{
			// +==============================+
			// |         UTF-8 Parse          |
			// +==============================+
			bool isUnicode = (c < 0);
			u32 codepoint = 0;
			u8 charByteLength = GetCodepointForUtf8Str(str, cIndex, &codepoint);
			if (codepoint == 0 && c != '\0') { codepoint = CharToU32(c); isUnicode = false; }
			
			// +==============================+
			// |       Known Character        |
			// +==============================+
			Win32_FontChar_t* fontChar = GetFontChar(font, codepoint);
			if (fontChar != nullptr)
			{
				if (!fontBakeBound) { Win32_BindTexture(&font->bake); fontBakeBound = true; hexBakeBound = false; }
				rec charRec = NewRec(position + fontChar->logicalOffset*scale, fontChar->logicalSize * scale);
				rec renderRec = NewRec(position + fontChar->renderOffset*scale, ToVec2(fontChar->renderSize) * scale);
				Win32_DrawTexturedRec(renderRec, color, ToRec(fontChar->sourceRec));
				//NOTE: If we need to keep track of total string size/bounds we should look at charRec not renderRec
				position.x += fontChar->advanceX * scale;
				lineIndex++;
			}
			// +==============================+
			// |          Hex Render          |
			// +==============================+
			else if (Platform->hexFont.isValid)
			{
				position.x += 2*scale;
				rec totalHexRec = NewRec(position, Vec2_Zero);
				rec whiteSourceRec = NewRec((r32)Platform->hexFont.bake.width-1, (r32)Platform->hexFont.bake.height-1, 1, 1);
				
				// +==============================+
				// |     Convert to Hex Chars     |
				// +==============================+
				char hexChars[8];
				u8 firstNonZeroNibble = 0xFF;
				for (u8 byteIndex = 0; byteIndex < 4; byteIndex++)
				{
					u8 byteValue = (u8)((codepoint & (0xFF000000UL >> (byteIndex*8))) >> ((3-byteIndex)*8));
					if (firstNonZeroNibble == 0xFF)
					{
						if (GetUpperNibble(byteValue) > 0) { firstNonZeroNibble = (byteIndex*2 + 0); }
						else if (GetLowerNibble(byteValue) > 0) { firstNonZeroNibble = (byteIndex*2 + 1); }
					}
					hexChars[byteIndex*2 + 0] = GetHexChar(GetUpperNibble(byteValue));
					hexChars[byteIndex*2 + 1] = GetHexChar(GetLowerNibble(byteValue));
				}
				if (firstNonZeroNibble == 0xFF || firstNonZeroNibble == 7) { firstNonZeroNibble = 6; }
				
				// +==============================+
				// |      Unicode U+ Prefix       |
				// +==============================+
				if (isUnicode)
				{
					Win32_FontChar_t* charU = GetFontChar(&Platform->hexFont, CharToU32('U'));
					if (charU != nullptr)
					{
						if (!hexBakeBound) { Win32_BindTexture(&Platform->hexFont.bake); fontBakeBound = false; hexBakeBound = true; }
						rec charRec = NewRec(position, ToVec2(charU->renderSize) * scale); //ignore offset
						charRec.y -= charRec.height;
						r32 extraHeight = font->maxHeight*scale - charRec.height;
						if (extraHeight < 0) { extraHeight = 0; }
						//ignore renderRec/sourceRec stuff
						Win32_DrawTexturedRec(charRec, color, ToRec(charU->sourceRec));
						totalHexRec = RecBoth(totalHexRec, charRec);
						if (extraHeight > 0)
						{
							rec extraRec = NewRec(charRec.x, charRec.y - extraHeight, charRec.width, extraHeight);
							Win32_DrawTexturedRec(extraRec, color, whiteSourceRec);
							totalHexRec = RecBoth(totalHexRec, extraRec);
						}
						position.x += charRec.width; //ignore advanceX
					}
					else { DebugAssertMsg(false, "Couldn't find 'U' char for Unknown Unicode Hex Display!"); }
					Win32_FontChar_t* charPlus = GetFontChar(&Platform->hexFont, CharToU32('+'));
					if (charPlus != nullptr)
					{
						if (!hexBakeBound) { Win32_BindTexture(&Platform->hexFont.bake); fontBakeBound = false; hexBakeBound = true; }
						rec charRec = NewRec(position, ToVec2(charPlus->renderSize) * scale); //ignore offset
						charRec.y -= charRec.height;
						r32 extraHeight = font->maxHeight*scale - charRec.height;
						if (extraHeight < 0) { extraHeight = 0; }
						r32 halfExtra = FloorR32(extraHeight/2);
						r32 otherHalfExtra = extraHeight - halfExtra;
						charRec.y -= halfExtra;
						//ignore renderRec/sourceRec stuff
						Win32_DrawTexturedRec(charRec, color, ToRec(charPlus->sourceRec));
						totalHexRec = RecBoth(totalHexRec, charRec);
						if (extraHeight > 0)
						{
							rec extraRec1 = NewRec(charRec.x, charRec.y - otherHalfExtra, charRec.width, otherHalfExtra);
							Win32_DrawTexturedRec(extraRec1, color, whiteSourceRec);
							totalHexRec = RecBoth(totalHexRec, extraRec1);
							rec extraRec2 = NewRec(charRec.x, charRec.y + charRec.height, charRec.width, halfExtra);
							Win32_DrawTexturedRec(extraRec2, color, whiteSourceRec);
							totalHexRec = RecBoth(totalHexRec, extraRec2);
						}
						position.x += charRec.width; //ignore advanceX
					}
					else { DebugAssertMsg(false, "Couldn't find +' char for Unknown Unicode Hex Display!"); }
				}
				
				// +==============================+
				// |       Render Hex Chars       |
				// +==============================+
				u8 numHexChars = (8 - firstNonZeroNibble);
				for (u64 hexIndex = 0; hexIndex < numHexChars; hexIndex++)
				{
					char hexChar = hexChars[firstNonZeroNibble + hexIndex];
					u32 hexCodepoint = (u32)hexChar;
					Win32_FontChar_t* hexFontChar = GetFontChar(&Platform->hexFont, hexCodepoint);
					if (hexFontChar != nullptr)
					{
						if (!hexBakeBound) { Win32_BindTexture(&Platform->hexFont.bake); fontBakeBound = false; hexBakeBound = true; }
						rec charRec = NewRec(position, ToVec2(hexFontChar->renderSize) * scale); //ignore offset
						charRec.y -= charRec.height;
						r32 extraHeight = font->maxHeight*scale - charRec.height;
						if (extraHeight < 0) { extraHeight = 0; }
						if ((hexIndex%2) == 0)
						{
							charRec.y -= extraHeight;
						}
						//ignore renderRec/sourceRec stuff
						Win32_DrawTexturedRec(charRec, color, ToRec(hexFontChar->sourceRec));
						totalHexRec = RecBoth(totalHexRec, charRec);
						if (extraHeight > 0)
						{
							rec extraRec = NewRec(charRec.x, charRec.y, charRec.width, extraHeight);
							if ((hexIndex%2) == 0)
							{
								extraRec.y += charRec.height;
							}
							else
							{
								extraRec.y -= extraHeight;
							}
							Win32_DrawTexturedRec(extraRec, color, whiteSourceRec);
							totalHexRec = RecBoth(totalHexRec, extraRec);
						}
						position.x += charRec.width; //ignore advanceX
						if (hexIndex+1 == numHexChars)
						{
							position.x += 2 * scale;
						}
					}
					else { PrintLine_W("Couldn't find hex char 0x%02X", hexCodepoint); DebugAssertMsg(false, "Couldn't find hex char in hexFont even though hexFont is valid!"); }
				}
				
				// +==============================+
				// |        Render Border         |
				// +==============================+
				if (totalHexRec.width > 0 && totalHexRec.height > 0)
				{
					Win32_DrawBorder(totalHexRec, 1 * scale, color, whiteSourceRec, false);
				}
				lineIndex++;
			}
			if (charByteLength > 1) { cIndex += charByteLength-1; }
		}
	}
}

void Win32_SetupViewportForWindow(PlatWindow_t* window)
{
	NotNull(window);
	glViewport(0, 0, window->input.contextResolution.width, window->input.contextResolution.height);
	mat4 projectionMatrix = Mat4_Identity;
	Mat4Transform(projectionMatrix, Mat4Scale2(2.0f / window->input.renderResolution.width, -2.0f / window->input.renderResolution.height));
	Mat4Transform(projectionMatrix, Mat4Translate2(-1, 1));
	Win32_SetProjectionMatrix(projectionMatrix);
	Win32_SetViewMatrix(Mat4_Identity);
	Win32_SetWorldMatrix(Mat4_Identity);
}
