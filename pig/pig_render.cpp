/*
File:   pig_render.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds a bunch of code that helps us abstract our render processes
*/

// +--------------------------------------------------------------+
// |                            OpenGL                            |
// +--------------------------------------------------------------+
#if OPENGL_SUPPORTED

void RcSetFaceCulling_OpenGL(bool enabled)
{
	if (enabled)
	{
		glFrontFace(GL_CW);
		AssertNoOpenGlError();
		glEnable(GL_CULL_FACE);
		AssertNoOpenGlError();
	}
	else
	{
		glDisable(GL_CULL_FACE);
		AssertNoOpenGlError();
	}
}

void RcSetDepthTestEnabled_OpenGL(bool enabled)
{
	if (enabled) { glEnable(GL_DEPTH_TEST); }
	else { glDisable(GL_DEPTH_TEST); }
	AssertNoOpenGlError();
}

void RcSetFillMode_OpenGL(PolygonFillMode_t fillMode)
{
	GLenum glFillMode = GL_FILL;
	switch (fillMode)
	{
		case PolygonFillMode_Points:    glFillMode = GL_POINT; break;
		case PolygonFillMode_Wireframe: glFillMode = GL_LINE; break;
		case PolygonFillMode_Default:   glFillMode = GL_FILL; break;
	}
	glPolygonMode(GL_FRONT_AND_BACK, glFillMode);
	AssertNoOpenGlError();
}

#if 0
//NOTE: glLineWidth is deprecated in OpenGL 3.3+
void RcSetLineThickness_OpenGL(r32 thickness)
{
	glLineWidth(thickness);
	AssertNoOpenGlError();
}
#endif

void RcCreateVertexArrayObject_OpenGL(u64 windowId, VertexType_t vertexType, VertexArrayObject_t* vaoOut)
{
	NotNull(vaoOut);
	
	GLuint vaoId = 0;
	glGenVertexArrays(1, &vaoId); AssertNoOpenGlError();
	glBindVertexArray(vaoId); AssertNoOpenGlError();
	
	//TODO: Is there anything else we are supposed to do here?
	
	ClearPointer(vaoOut);
	vaoOut->windowId = windowId;
	vaoOut->vertexType = vertexType;
	vaoOut->glId = vaoId;
	vaoOut->boundOnce = false;
}

void RcBindVertexArrayObject_OpenGL(VertexArrayObject_t* vao)
{
	NotNull(rc->state.boundShader);
	Assert(vao->vertexType == rc->state.boundShader->actualVertexType);
	
	glBindVertexArray(vao->glId); AssertNoOpenGlError();
	if (!vao->boundOnce || true) //TODO: Remove me when we figure out why the assertion in the else statement is firing on laptop
	{
		if (IsFlagSet(vao->vertexType, VertexType_SlugBit))
		{
			glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.slug.attrib0); AssertNoOpenGlError();
			glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.slug.attrib1); AssertNoOpenGlError();
			glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.slug.attrib2); AssertNoOpenGlError();
			glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.slug.attrib3); AssertNoOpenGlError();
			glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.slug.attrib4); AssertNoOpenGlError();
		}
		else
		{
			for (u64 attIndex = 0; attIndex < MAX_NUM_VERTEX_ATTRIBUTES; attIndex++)
			{
				if (rc->state.boundShader->attribLocations.gl.values[attIndex] >= 0)
				{
					glEnableVertexAttribArray(rc->state.boundShader->attribLocations.gl.values[attIndex]); AssertNoOpenGlError();
				}
				vao->attribLocations.gl.values[attIndex] = rc->state.boundShader->attribLocations.gl.values[attIndex];
			}
		}
		vao->boundOnce = true;
	}
	else
	{
		for (u64 attIndex = 0; attIndex < MAX_NUM_VERTEX_ATTRIBUTES; attIndex++)
		{
			Assert(vao->attribLocations.gl.values[attIndex] == rc->state.boundShader->attribLocations.gl.values[attIndex]);
		}
	}
}

void RcBindFrameBuffer_OpenGL(FrameBuffer_t* buffer)
{
	if (buffer != nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->glId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->glId);
		AssertNoOpenGlError();
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		AssertNoOpenGlError();
	}
}

void RcBindShader_OpenGL(Shader_t* shader)
{
	if (shader != nullptr)
	{
		glUseProgram(shader->glId);
		AssertNoOpenGlError();
	}
	else
	{
		glUseProgram(0);
		AssertNoOpenGlError();
	}
}

void RcBindTexture1_OpenGL(Texture_t* texture)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	
	if (texture != nullptr)
	{
		Assert(texture->isValid);
		GLenum targetEnum = ((texture->numLayers > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0); AssertNoOpenGlError();
		glBindTexture(targetEnum, texture->glId); AssertNoOpenGlError();
		glUniform1i(rc->state.boundShader->glLocations.texture1, 0); AssertNoOpenGlError();
		glUniform2f(rc->state.boundShader->glLocations.texture1Size, texture->width, texture->height); AssertNoOpenGlError();
	}
	else
	{
		glActiveTexture(GL_TEXTURE0); AssertNoOpenGlError();
		glBindTexture(GL_TEXTURE_2D, 0); AssertNoOpenGlError();
		glUniform1i(rc->state.boundShader->glLocations.texture1, 0); AssertNoOpenGlError();
		glUniform2f(rc->state.boundShader->glLocations.texture1Size, 0, 0); AssertNoOpenGlError();
	}
}
void RcBindTexture2_OpenGL(Texture_t* texture)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	
	if (texture != nullptr)
	{
		Assert(texture->isValid);
		glActiveTexture(GL_TEXTURE1); AssertNoOpenGlError();
		glBindTexture(GL_TEXTURE_2D, texture->glId); AssertNoOpenGlError();
		glUniform1i(rc->state.boundShader->glLocations.texture2, 1); AssertNoOpenGlError();
		glUniform2f(rc->state.boundShader->glLocations.texture2Size, texture->width, texture->height); AssertNoOpenGlError();
	}
	else
	{
		glActiveTexture(GL_TEXTURE1); AssertNoOpenGlError();
		glBindTexture(GL_TEXTURE_2D, 0); AssertNoOpenGlError();
		glUniform1i(rc->state.boundShader->glLocations.texture2, 1); AssertNoOpenGlError();
		glUniform2f(rc->state.boundShader->glLocations.texture2Size, 0, 0); AssertNoOpenGlError();
	}
}

void RcBindVertBuffer_OpenGL(const VertBuffer_t* buffer)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	NotNull(rc->state.boundVao);
	
	if (buffer != nullptr)
	{
		NotNull(rc->state.boundVao);
		glBindVertexArray(rc->state.boundVao->glId); AssertNoOpenGlError(); //TODO: We don't really need to do this?
		glBindBuffer(GL_ARRAY_BUFFER, buffer->glId); AssertNoOpenGlError();
		if (buffer->numIndices > 0) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->glIndexId); AssertNoOpenGlError(); }
		if (IsFlagSet(rc->state.boundVao->vertexType, VertexType_SlugBit))
		{
			#if SLUG_SUPPORTED
			glVertexAttribPointer(0, 4, GL_FLOAT,         false, sizeof(Terathon::Slug::Vertex),   nullptr); AssertNoOpenGlError();
			glVertexAttribPointer(1, 4, GL_FLOAT,         false, sizeof(Terathon::Slug::Vertex), (char*)16); AssertNoOpenGlError();
			glVertexAttribPointer(2, 4, GL_FLOAT,         false, sizeof(Terathon::Slug::Vertex), (char*)32); AssertNoOpenGlError();
			glVertexAttribPointer(3, 4, GL_FLOAT,         false, sizeof(Terathon::Slug::Vertex), (char*)48); AssertNoOpenGlError();
			glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true,  sizeof(Terathon::Slug::Vertex), (char*)64); AssertNoOpenGlError();
			#else
			AssertMsg(false, "Slug support is not enabled but this vertex buffer is marked with VertexType_SlugBit");
			#endif
		}
		else
		{
			u8* attribOffset = nullptr;
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_PositionBit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_PositionBit), "Tried to bind vertex buffer with no Position attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.position, 3, GL_FLOAT, GL_FALSE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_PositionBit)) { attribOffset += sizeof(v3); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_Color1Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_Color1Bit), "Tried to bind vertex buffer with no Color1 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.color1, 4, GL_FLOAT, GL_FALSE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_Color1Bit)) { attribOffset += sizeof(v4); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_Color2Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_Color2Bit), "Tried to bind vertex buffer with no Color2 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.color2, 4, GL_FLOAT, GL_FALSE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_Color2Bit)) { attribOffset += sizeof(v4); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_TexCoord1Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_TexCoord1Bit), "Tried to bind vertex buffer with no TexCoord1 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.texCoord1, 2, GL_FLOAT, GL_FALSE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_TexCoord1Bit)) { attribOffset += sizeof(v2); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_TexCoord2Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_TexCoord2Bit), "Tried to bind vertex buffer with no TexCoord2 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.texCoord2, 2, GL_FLOAT, GL_FALSE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_TexCoord2Bit)) { attribOffset += sizeof(v2); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_Normal1Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_Normal1Bit), "Tried to bind vertex buffer with no Normal1 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.normal1, 3, GL_FLOAT, GL_TRUE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_Normal1Bit)) { attribOffset += sizeof(v3); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_Normal2Bit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_Normal2Bit), "Tried to bind vertex buffer with no Normal2 attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.normal2, 3, GL_FLOAT, GL_TRUE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_Normal2Bit)) { attribOffset += sizeof(v3); }
			if (IsFlagSet(rc->state.boundShader->actualVertexType, VertexType_TangentBit))
			{
				AssertMsg(IsFlagSet(buffer->vertexType, VertexType_TangentBit), "Tried to bind vertex buffer with no Tangent attribute to shader that requires it");
				glVertexAttribPointer(rc->state.boundShader->attribLocations.gl.tangent, 3, GL_FLOAT, GL_TRUE, (GLsizei)buffer->vertexSize, attribOffset); AssertNoOpenGlError();
			}
			if (IsFlagSet(buffer->vertexType, VertexType_TangentBit)) { attribOffset += sizeof(v3); }
		}
	}
	else
	{
		glBindVertexArray(0); AssertNoOpenGlError();
		glBindBuffer(GL_ARRAY_BUFFER, 0); AssertNoOpenGlError();
	}
}

void RcSetWorldMatrix_OpenGL(mat4 matrix)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_WorldMatrix))
	{
		Assert(rc->state.boundShader->glLocations.worldMatrix >= 0);
		glUniformMatrix4fv(rc->state.boundShader->glLocations.worldMatrix, 1, GL_FALSE, &matrix.values[0][0]);
		AssertNoOpenGlError();
	}
}
void RcSetViewMatrix_OpenGL(mat4 matrix)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_ViewMatrix))
	{
		Assert(rc->state.boundShader->glLocations.viewMatrix >= 0);
		glUniformMatrix4fv(rc->state.boundShader->glLocations.viewMatrix, 1, GL_FALSE, &matrix.values[0][0]);
		AssertNoOpenGlError();
	}
}
void RcSetProjectionMatrix_OpenGL(mat4 matrix)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_ProjectionMatrix))
	{
		Assert(rc->state.boundShader->glLocations.projectionMatrix >= 0);
		glUniformMatrix4fv(rc->state.boundShader->glLocations.projectionMatrix, 1, GL_FALSE, &matrix.values[0][0]);
		AssertNoOpenGlError();
	}
}

void RcSetCameraPosition_OpenGL(v3 cameraPos)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_CameraPosition))
	{
		Assert(rc->state.boundShader->glLocations.cameraPosition >= 0);
		glUniform3f(rc->state.boundShader->glLocations.cameraPosition, cameraPos.x, cameraPos.y, cameraPos.z);
		AssertNoOpenGlError();
	}
}
void RcSetPlayerPosition_OpenGL(v3 playerPos)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_PlayerPosition))
	{
		Assert(rc->state.boundShader->glLocations.playerPosition >= 0);
		glUniform3f(rc->state.boundShader->glLocations.playerPosition, playerPos.x, playerPos.y, playerPos.z);
		AssertNoOpenGlError();
	}
}

void RcSetViewport_OpenGL(rec viewportRec)
{
	NotNull(rc);
	NotNull(rc->currentWindow);
	
	reci openglRec = NewReci(
		RoundR32i(viewportRec.x),
		RoundR32i(viewportRec.y),
		RoundR32i(viewportRec.width), //TODO: We used to multiply width and height by GUI_SCALE here!
		RoundR32i(viewportRec.height) //TODO: We used to multiply width and height by GUI_SCALE here!
	);
	openglRec.y = (i32)(rc->currentWindow->input.contextResolution.height - (viewportRec.y + viewportRec.height));
	
	if (openglRec.width < 0) { openglRec.x += openglRec.width; openglRec.width = -openglRec.width; }
	if (openglRec.height < 0) { openglRec.y += openglRec.height; openglRec.height = -openglRec.height; }
	
	glViewport(openglRec.x, openglRec.y, openglRec.width, openglRec.height); AssertNoOpenGlError();
}

void RcSetColor1_OpenGL(Colorf_t colorf)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Color1))
	{
		Assert(rc->state.boundShader->glLocations.color1 >= 0);
		glUniform4f(rc->state.boundShader->glLocations.color1, colorf.r, colorf.g, colorf.b, colorf.a);
		AssertNoOpenGlError();
	}
}
void RcSetColor2_OpenGL(Colorf_t colorf)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Color2))
	{
		Assert(rc->state.boundShader->glLocations.color2 >= 0);
		glUniform4f(rc->state.boundShader->glLocations.color2, colorf.r, colorf.g, colorf.b, colorf.a);
		AssertNoOpenGlError();
	}
}

void RcSetSourceRec1_OpenGL(rec rectangle, bool flipped, r32 textureHeight)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_SourceRec1))
	{
		if (flipped)
		{
			rectangle.y = textureHeight - rectangle.y;
			rectangle.height = -rectangle.height;
		}
		Assert(rc->state.boundShader->glLocations.sourceRec1 >= 0);
		glUniform4f(rc->state.boundShader->glLocations.sourceRec1, rectangle.x, rectangle.y, rectangle.width, rectangle.height);
		AssertNoOpenGlError();
	}
}
void RcSetSourceRec2_OpenGL(rec rectangle, bool flipped, r32 textureHeight)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_SourceRec2))
	{
		if (flipped)
		{
			rectangle.y = textureHeight - rectangle.y;
			rectangle.height = -rectangle.height;
		}
		Assert(rc->state.boundShader->glLocations.sourceRec2 >= 0);
		glUniform4f(rc->state.boundShader->glLocations.sourceRec2, rectangle.x, rectangle.y, rectangle.width, rectangle.height);
		AssertNoOpenGlError();
	}
}

void RcSetShiftVec_OpenGL(v2 shiftVec)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_ShiftVec))
	{
		Assert(rc->state.boundShader->glLocations.shiftVec >= 0);
		glUniform2f(rc->state.boundShader->glLocations.shiftVec, shiftVec.x, shiftVec.y);
		AssertNoOpenGlError();
	}
}

void RcSetCount_OpenGL(i32 count)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Count))
	{
		Assert(rc->state.boundShader->glLocations.count >= 0);
		glUniform1i(rc->state.boundShader->glLocations.count, count);
		AssertNoOpenGlError();
	}
}

void RcSetCircleRadius_OpenGL(r32 radius)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_CircleRadius))
	{
		Assert(rc->state.boundShader->glLocations.circleRadius >= 0);
		glUniform1f(rc->state.boundShader->glLocations.circleRadius, radius);
		AssertNoOpenGlError();
	}
}
void RcSetCircleInnerRadius_OpenGL(r32 innerRadius)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_CircleInnerRadius))
	{
		Assert(rc->state.boundShader->glLocations.circleInnerRadius >= 0);
		glUniform1f(rc->state.boundShader->glLocations.circleInnerRadius, innerRadius);
		AssertNoOpenGlError();
	}
}

void RcSetTime_OpenGL(r32 time)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Time))
	{
		Assert(rc->state.boundShader->glLocations.time >= 0);
		glUniform1f(rc->state.boundShader->glLocations.time, time);
		AssertNoOpenGlError();
	}
}

void RcSetSaturation_OpenGL(r32 saturation)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Saturation))
	{
		Assert(rc->state.boundShader->glLocations.saturation >= 0);
		glUniform1f(rc->state.boundShader->glLocations.saturation, saturation);
		AssertNoOpenGlError();
	}
}

void RcSetBrightness_OpenGL(r32 brightness)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Brightness))
	{
		Assert(rc->state.boundShader->glLocations.brightness >= 0);
		glUniform1f(rc->state.boundShader->glLocations.brightness, brightness);
		AssertNoOpenGlError();
	}
}

void RcSetPolygonPlanes_OpenGL(const v2* planeValues)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	NotNull(planeValues);
	if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_PolygonPlanes))
	{
		Assert(rc->state.boundShader->glLocations.polygonPlanes >= 0);
		glUniform2fv(rc->state.boundShader->glLocations.polygonPlanes, (GLsizei)ShaderUniform_NumPolygonPlanes, (const GLfloat*)planeValues);
		AssertNoOpenGlError();
	}
}

void RcSetValue_OpenGL(u8 valueIndex, r32 value)
{
	NotNull(rc);
	NotNull(rc->state.boundShader);
	switch (valueIndex)
	{
		case 0: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value0)) { Assert(rc->state.boundShader->glLocations.value[0] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[0], value); AssertNoOpenGlError(); } break;
		case 1: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value1)) { Assert(rc->state.boundShader->glLocations.value[1] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[1], value); AssertNoOpenGlError(); } break;
		case 2: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value2)) { Assert(rc->state.boundShader->glLocations.value[2] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[2], value); AssertNoOpenGlError(); } break;
		case 3: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value3)) { Assert(rc->state.boundShader->glLocations.value[3] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[3], value); AssertNoOpenGlError(); } break;
		case 4: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value4)) { Assert(rc->state.boundShader->glLocations.value[4] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[4], value); AssertNoOpenGlError(); } break;
		case 5: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value5)) { Assert(rc->state.boundShader->glLocations.value[5] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[5], value); AssertNoOpenGlError(); } break;
		case 6: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value6)) { Assert(rc->state.boundShader->glLocations.value[6] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[6], value); AssertNoOpenGlError(); } break;
		case 7: if (IsFlagSet(rc->state.boundShader->uniformFlags, ShaderUniform_Value7)) { Assert(rc->state.boundShader->glLocations.value[7] >= 0); glUniform1f(rc->state.boundShader->glLocations.value[7], value); AssertNoOpenGlError(); } break;
		default: DebugAssert(false); break;
	}
}

void RcSetDynamicUniformR32_OpenGL(ShaderDynamicUniform_t* uniform, r32 value)
{
	NotNull(uniform);
	Assert(uniform->glLocation >= 0);
	glUniform1f(uniform->glLocation, value);
	AssertNoOpenGlError();
}
void RcSetDynamicUniformVec2_OpenGL(ShaderDynamicUniform_t* uniform, v2 value)
{
	NotNull(uniform);
	Assert(uniform->glLocation >= 0);
	glUniform2f(uniform->glLocation, value.x, value.y);
	AssertNoOpenGlError();
}
void RcSetDynamicUniformVec3_OpenGL(ShaderDynamicUniform_t* uniform, v3 value)
{
	NotNull(uniform);
	Assert(uniform->glLocation >= 0);
	glUniform3f(uniform->glLocation, value.x, value.y, value.z);
	AssertNoOpenGlError();
}
void RcSetDynamicUniformVec4_OpenGL(ShaderDynamicUniform_t* uniform, v4 value)
{
	NotNull(uniform);
	Assert(uniform->glLocation >= 0);
	glUniform4f(uniform->glLocation, value.x, value.y, value.z, value.w);
	AssertNoOpenGlError();
}
void RcSetDynamicUniformMat4_OpenGL(ShaderDynamicUniform_t* uniform, mat4 value)
{
	NotNull(uniform);
	Assert(uniform->glLocation >= 0);
	glUniformMatrix4fv(uniform->glLocation, 1, GL_FALSE, &value.values[0][0]);
	AssertNoOpenGlError();
}

void RcClearColor_OpenGL(Color_t color)
{
	Colorf_t colorf = ToColorf(color);
	glClearColor(colorf.r, colorf.g, colorf.b, colorf.a); AssertNoOpenGlError();
	glClear(GL_COLOR_BUFFER_BIT); AssertNoOpenGlError();
}

void RcClearDepth_OpenGL(r32 depth)
{
	glClearDepth(depth); AssertNoOpenGlError();
	glClear(GL_DEPTH_BUFFER_BIT); AssertNoOpenGlError();
}

void RcClearStencil_OpenGL(i32 stencilValue)
{
	glClearStencil((GLint)stencilValue); AssertNoOpenGlError();
	glClear(GL_STENCIL_BUFFER_BIT); AssertNoOpenGlError();
}

void RcDrawBuffer_OpenGL(VertBufferPrimitive_t primitive, u64 startIndex = 0, u64 numElements = 0, u64 vertexOffset = 0)
{
	//TODO: Keep track of buffer draw counts
	NotNull(rc->state.boundShader);
	NotNull(rc->state.boundBuffer);
	GLenum glPrimitive = GL_FALSE;
	switch (primitive)
	{
		case VertBufferPrimitive_Lines: glPrimitive = GL_LINES; break;
		case VertBufferPrimitive_Triangles: glPrimitive = GL_TRIANGLES; break;
		//TODO: Add other primitive types here
		default: AssertMsg(false, "Unsupported primitive type in RcDrawBuffer_OpenGL"); break;
	}
	if (rc->state.boundBuffer->numIndices > 0)
	{
		Assert(startIndex < rc->state.boundBuffer->numIndices);
		Assert(startIndex + numElements <= rc->state.boundBuffer->numIndices);
		if (numElements == 0) { numElements = rc->state.boundBuffer->numIndices - startIndex; }
		GLenum glIndexType = GL_UNSIGNED_INT;
		switch (rc->state.boundBuffer->indexSize)
		{
			case 1: glIndexType = GL_UNSIGNED_BYTE; break;
			case 2: glIndexType = GL_UNSIGNED_SHORT; break;
			case 4: glIndexType = GL_UNSIGNED_INT; break;
			//TODO: Add other index types here
			default: AssertMsg(false, "Unsupported index size in RcDrawBuffer_OpenGL"); break;
		}
		if (vertexOffset > 0)
		{
			//TODO: Make sure our u64 numElements/vertexOffset isn't too large for GLsizei and Glint
			glDrawElementsBaseVertex(glPrimitive, (GLsizei)numElements, glIndexType, (const void*)(startIndex * rc->state.boundBuffer->indexSize), (GLint)vertexOffset);
		}
		else
		{
			//TODO: Make sure our u64 numElements isn't too large for GLsizei
			glDrawElements(glPrimitive, (GLsizei)numElements, glIndexType, (const void*)(startIndex * rc->state.boundBuffer->indexSize));
		}
	}
	else
	{
		Assert(vertexOffset == 0); //This option is not needed for non-indexed vertex buffers
		Assert(startIndex < rc->state.boundBuffer->numVertices);
		Assert(startIndex + numElements <= rc->state.boundBuffer->numVertices);
		if (numElements == 0) { numElements = rc->state.boundBuffer->numVertices - startIndex; }
		//TODO: Make sure our u64 startIndex/numElements aren't too large for GLint and GLsizei
		glDrawArrays(glPrimitive, (GLint)startIndex, (GLsizei)numElements);
	}
	AssertNoOpenGlError();
}

void RcStartStencilDrawing_OpenGL()
{
	glEnable(GL_STENCIL_TEST); AssertNoOpenGlError();
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); AssertNoOpenGlError(); //don't draw to the color buffer
	glDepthMask(GL_FALSE); AssertNoOpenGlError(); //don't draw to the depth buffer
	glStencilMask(0xFF); AssertNoOpenGlError();
	glStencilFunc(GL_NEVER, 1, 0xFF); AssertNoOpenGlError();
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP); AssertNoOpenGlError();
}

void RcSetStencilPolarity_OpenGL(bool writePositiveValues)
{
	// glStencilMask(writePositiveValues ? 0xFF : 0x00); AssertNoOpenGlError();
	glStencilFunc(GL_NEVER, writePositiveValues ? 1 : 0, 0xFF); AssertNoOpenGlError();
}

void RcUseStencil_OpenGL(bool inverseMask)
{
	glEnable(GL_STENCIL_TEST); AssertNoOpenGlError();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); AssertNoOpenGlError();
	glDepthMask(GL_TRUE); AssertNoOpenGlError();
	glStencilMask(0x00); AssertNoOpenGlError();
	if (inverseMask)
	{
		glStencilFunc(GL_GREATER, 1, 0xFF); AssertNoOpenGlError();
	}
	else
	{
		glStencilFunc(GL_EQUAL, 1, 0xFF); AssertNoOpenGlError();
	}
}

void RcDisableStencil_OpenGL()
{
	glDisable(GL_STENCIL_TEST);
	AssertNoOpenGlError();
}

void RcBegin_OpenGL()
{
	NotNull(rc);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); AssertNoOpenGlError();
	glEnable(GL_BLEND); AssertNoOpenGlError();
	#if 1 //Left-Handed
	glDepthFunc(GL_GEQUAL); AssertNoOpenGlError();
	#else //Right-Handed
	glDepthFunc(GL_LEQUAL); AssertNoOpenGlError();
	#endif
	RcSetDepthTestEnabled_OpenGL(true);
	RcSetFillMode_OpenGL(PolygonFillMode_Default);
	//NOTE: GL_APHA_TEST and glAlphaFunc have been deprecated. We should use discard; in shaders instead
	// glAlphaFunc(GL_GEQUAL, 0.01f); AssertNoOpenGlError();
	// glEnable(GL_ALPHA_TEST); AssertNoOpenGlError();
	RcSetFaceCulling_OpenGL(true);
	
	
}

#endif //OPENGL_SUPPORTED

// +--------------------------------------------------------------+
// |                         General API                          |
// +--------------------------------------------------------------+
void InitRenderContext()
{
	NotNull(rc);
	CreateVarArray(&rc->vertexArrayObjs, fixedHeap, sizeof(VertexArrayObject_t));
}

void RcSetFaceCulling(bool enabled)
{
	NotNull(rc);
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetFaceCulling_OpenGL(enabled);
			rc->state.faceCulling = enabled;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetFaceCulling!"); break;
	}
}

void RcSetDepthTestEnabled(bool enabled)
{
	NotNull(rc);
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDepthTestEnabled_OpenGL(enabled);
			rc->state.depthTestEnabled = enabled;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDepthTestEnabled!"); break;
	}
}

void RcSetFillMode(PolygonFillMode_t fillMode)
{
	NotNull(rc);
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetFillMode_OpenGL(fillMode);
			rc->state.fillMode = fillMode;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDepthTestEnabled!"); break;
	}
}

#if 0
//NOTE: glLineWidth is deprecated in OpenGL 3.3+
void RcSetLineThickness(r32 thickness)
{
	NotNull(rc);
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetLineThickness_OpenGL(thickness);
			rc->state.lineThickness = thickness;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetFaceCulling!"); break;
	}
}
#endif

VertexArrayObject_t* RcGetVertexArrayObj(u64 windowId, VertexType_t vertexType)
{
	for (u64 vaoIndex = 0; vaoIndex < rc->vertexArrayObjs.length; vaoIndex++)
	{
		VertexArrayObject_t* vao = VarArrayGet(&rc->vertexArrayObjs, vaoIndex, VertexArrayObject_t);
		if (vao->windowId == windowId && vao->vertexType == vertexType)
		{
			return vao;
		}
	}
	
	VertexArrayObject_t* newVao = VarArrayAdd(&rc->vertexArrayObjs, VertexArrayObject_t);
	NotNull(newVao);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcCreateVertexArrayObject_OpenGL(windowId, vertexType, newVao);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcGetVertexArrayObj!"); break;
	}
	
	return newVao;
}

void RcBindShader(Shader_t* shader)
{
	NotNull(rc);
	if (shader != nullptr && !shader->isValid) { DebugAssertMsg(false, "Trying to bind invalid shader!"); shader = &pig->resources.shaders->main2D; }
	if (shader != nullptr && !shader->isValid) { shader = nullptr; }
	if (rc->state.boundShader == shader) { return; }
	//TODO: Track the number of shader binds
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcBindShader_OpenGL(shader);
			rc->state.boundShader = shader;
			NotNull(rc->currentWindow);
			VertexArrayObject_t* compatibleVao = RcGetVertexArrayObj(rc->currentWindow->id, shader->actualVertexType);
			NotNull(compatibleVao);
			RcBindVertexArrayObject_OpenGL(compatibleVao);
			rc->state.boundVao = compatibleVao;
			RcBindVertBuffer_OpenGL(nullptr);
			rc->state.boundBuffer = nullptr;
			RcBindTexture1_OpenGL(rc->state.boundTexture1);
			RcBindTexture2_OpenGL(rc->state.boundTexture2);
			RcSetWorldMatrix_OpenGL(Mat4Multiply(rc->state.augmentMatrix, rc->state.worldMatrix));
			RcSetViewMatrix_OpenGL(rc->state.viewMatrix);
			RcSetProjectionMatrix_OpenGL(rc->state.projectionMatrix);
			RcSetCameraPosition_OpenGL(rc->state.cameraPosition);
			RcSetPlayerPosition_OpenGL(rc->state.playerPosition);
			RcSetColor1_OpenGL(rc->state.color1f);
			RcSetColor2_OpenGL(rc->state.color2f);
			if (rc->state.boundTexture1 != nullptr) { RcSetSourceRec1_OpenGL(rc->state.sourceRec1, rc->state.boundTexture1->isFlippedY, rc->state.boundTexture1->height); }
			if (rc->state.boundTexture2 != nullptr) { RcSetSourceRec2_OpenGL(rc->state.sourceRec2, rc->state.boundTexture2->isFlippedY, rc->state.boundTexture2->height); }
			RcSetShiftVec_OpenGL(rc->state.shiftVec);
			RcSetCount_OpenGL(rc->state.count);
			RcSetCircleRadius_OpenGL(rc->state.circleRadius);
			RcSetCircleInnerRadius_OpenGL(rc->state.circleInnerRadius);
			RcSetTime_OpenGL(rc->state.time);
			RcSetSaturation_OpenGL(rc->state.saturation);
			RcSetBrightness_OpenGL(rc->state.brightness);
			RcSetPolygonPlanes_OpenGL(&rc->state.polygonPlanes[0]);
			RcSetBrightness_OpenGL(rc->state.brightness);
			for (u8 vIndex = 0; vIndex < ArrayCount(rc->state.values); vIndex++)
			{
				RcSetValue_OpenGL(vIndex, rc->state.values[vIndex]);
			}
			//TODO: Update all of the uniform values to match our current render context state
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in BindShader!"); break;
	}
}

void RcBindFrameBuffer(FrameBuffer_t* buffer, bool forceRebind = false)
{
	if (buffer != nullptr && !buffer->isValid) { DebugAssertMsg(false, "Trying to bind invalid frame buffer!"); buffer = nullptr; }
	if (!forceRebind && rc->state.boundFrameBuffer == buffer) { return; }
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcBindFrameBuffer_OpenGL(buffer);
			rc->state.boundFrameBuffer = buffer;
		} break;
		#endif
		default:
		{
			
		} break;
	}
}

void RcBindTexture1(Texture_t* texture)
{
	NotNull(rc);
	if (texture == nullptr) { texture = &rc->dotTexture; }
	if (!texture->isValid) { texture = &rc->invalidTexture; }
	//TODO: Track the number of texture binds
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			rec defaultSourceRec = NewRec(0, 0, texture->width, texture->height);
			RcBindTexture1_OpenGL(texture);
			RcSetSourceRec1_OpenGL(defaultSourceRec, texture->isFlippedY, texture->height);
			rc->state.boundTexture1 = texture;
			rc->state.sourceRec1 = defaultSourceRec;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in BindTexture1!"); break;
	}
}
void RcBindTexture2(Texture_t* texture)
{
	NotNull(rc);
	if (texture == nullptr) { texture = &rc->dotTexture; }
	if (!texture->isValid) { texture = &rc->invalidTexture; }
	//TODO: Track the number of texture binds
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			rec defaultSourceRec = NewRec(0, 0, texture->width, texture->height);
			RcBindTexture2_OpenGL(texture);
			RcSetSourceRec2_OpenGL(defaultSourceRec, texture->isFlippedY, texture->height);
			rc->state.boundTexture2 = texture;
			rc->state.sourceRec2 = defaultSourceRec;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in BindTexture2!"); break;
	}
}

void RcBindSpriteSheet(SpriteSheet_t* spriteSheet)
{
	NotNull(rc);
	rc->state.boundSpriteSheet = spriteSheet;
}

void RcBindVertBuffer(const VertBuffer_t* buffer)
{
	NotNull(rc);
	if (buffer != nullptr && !buffer->isValid) { DebugAssertMsg(false, "Trying to bind invalid vertex buffer!"); buffer = nullptr; }
	if (rc->state.boundBuffer == buffer) { return; }
	//TODO: Track the number of buffer binds
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcBindVertBuffer_OpenGL(buffer);
			rc->state.boundBuffer = buffer;
			//TODO: Update all of the uniform values to match our current render context state
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcBindVertBuffer!"); break;
	}
}

void RcBindModel(Model_t* model)
{
	NotNull(model);
	rc->state.boundModel = model;
}

void RcBindFont(const Font_t* font, FontFaceSelector_t faceSelector, r32 scale = 1)
{
	NotNull(font);
	rc->state.boundFont = font;
	rc->state.faceSelector = faceSelector;
	rc->state.fontScale = scale;
	#if DEBUG_BUILD
	const FontFace_t* testSelectResult = GetFontFace(font, faceSelector);
	DebugAssertMsg(testSelectResult != nullptr, "Incorrect faceSelector passed to RcBindFont. Does this font actually have a bake that matches what you want?");
	#endif
}
void RcBindFontDefault(const Font_t* font, r32 scale = 1)
{
	RcBindFont(font, SelectDefaultFontFace(), scale);
}
void RcSetFontScale(r32 scale)
{
	rc->state.fontScale = scale;
}
void RcSetFaceSelector(FontFaceSelector_t faceSelector)
{
	NotNull(rc->state.boundFont);
	rc->state.faceSelector = faceSelector;
	#if DEBUG_BUILD
	const FontFace_t* testSelectResult = GetFontFace(rc->state.boundFont, faceSelector);
	DebugAssertMsg(testSelectResult != nullptr, "Incorrect faceSelector passed to RcSetFaceSelector. Does this font actually have a bake that matches what you want?");
	#endif
}
void RcSetFontFlowCallbacks(const FontFlowCallbacks_t* callbacks)
{
	NotNull(callbacks);
	MyMemCopy(&rc->state.flowCallbacks, callbacks, sizeof(FontFlowCallbacks_t));
}
void RcClearFontFlowCallbacks()
{
	rc->state.flowCallbacks = {};
}

const FontFace_t* RcGetFontFace()
{
	NotNull(rc);
	NotNull(rc->state.boundFont);
	const FontFace_t* fontFace = GetFontFace(rc->state.boundFont, rc->state.faceSelector);
	return fontFace;
}
r32 RcGetLineHeight()
{
	NotNull(rc);
	NotNull(rc->state.boundFont);
	const FontFace_t* fontFace = GetFontFace(rc->state.boundFont, rc->state.faceSelector);
	if (fontFace == nullptr) { return 0; }
	return fontFace->lineHeight * rc->state.fontScale;
}
r32 RcGetMaxAscend()
{
	NotNull(rc);
	NotNull(rc->state.boundFont);
	const FontFace_t* fontFace = GetFontFace(rc->state.boundFont, rc->state.faceSelector);
	if (fontFace == nullptr) { return 0; }
	return fontFace->maxAscend * rc->state.fontScale;
}
r32 RcGetMaxDescend()
{
	NotNull(rc);
	NotNull(rc->state.boundFont);
	const FontFace_t* fontFace = GetFontFace(rc->state.boundFont, rc->state.faceSelector);
	if (fontFace == nullptr) { return 0; }
	return fontFace->maxDescend * rc->state.fontScale;
}

void RcSetAugmentMatrix(mat4 matrix)
{
	NotNull(rc);
	if (rc->state.augmentMatrix == matrix) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetWorldMatrix_OpenGL(Mat4Multiply(matrix, rc->state.worldMatrix));
			rc->state.augmentMatrix = matrix;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetWorldMatrix!"); break;
	}
}
void RcSetWorldMatrix(mat4 matrix)
{
	NotNull(rc);
	if (rc->state.worldMatrix == matrix) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetWorldMatrix_OpenGL(Mat4Multiply(rc->state.augmentMatrix, matrix));
			rc->state.worldMatrix = matrix;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetWorldMatrix!"); break;
	}
}
void RcSetViewMatrix(mat4 matrix)
{
	NotNull(rc);
	if (rc->state.viewMatrix == matrix) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetViewMatrix_OpenGL(matrix);
			rc->state.viewMatrix = matrix;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetViewMatrix!"); break;
	}
}
void RcSetProjectionMatrix(mat4 matrix)
{
	NotNull(rc);
	if (rc->state.projectionMatrix == matrix) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetProjectionMatrix_OpenGL(matrix);
			rc->state.projectionMatrix = matrix;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetProjectionMatrix!"); break;
	}
}

void RcSetCameraPosition(v3 cameraPos)
{
	NotNull(rc);
	if (rc->state.cameraPosition == cameraPos) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetCameraPosition_OpenGL(cameraPos);
			rc->state.cameraPosition = cameraPos;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetCameraPosition!"); break;
	}
}
void RcSetPlayerPosition(v3 playerPos)
{
	NotNull(rc);
	if (rc->state.playerPosition == playerPos) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetPlayerPosition_OpenGL(playerPos);
			rc->state.playerPosition = playerPos;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetPlayerPosition!"); break;
	}
}

void RcSetDepth(r32 depth)
{
	rc->state.depth = depth;
}
r32 RcGetRealDepth(r32 depth)
{
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			return (-2.0f * depth) + 1.0f;
		} break;
		#endif
		default: DebugAssertMsg(false, "Unsupported render API in RcGetRealDepth!"); return depth;
	}
}
r32 RcGetRealDepth()
{
	return RcGetRealDepth(rc->state.depth);
}

void RcSetViewport(rec viewportRec, bool fakeScreenSpaceCoordinates = true)
{
	NotNull(rc);
	NotNull(rc->currentWindow);
	
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetViewport_OpenGL(viewportRec);
			rc->state.viewportRec = viewportRec;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetViewport!"); break;
	}
	
	//NOTE: For 2D rendering it can be nice to pretend like our coordinate space hasn't changed when rendering
	//      to a sub-section of the back buffer. In order to do this we generate a projection matrix to map\
	//      normal screenspace into this smaller viewport
	if (fakeScreenSpaceCoordinates)
	{
		bool flipYAxis = true;
		mat4 projMatrix = Mat4_Identity;
		projMatrix = Mat4Scale2(2.0f/viewportRec.width, (flipYAxis ? -1 : 1) * 2.0f/viewportRec.height);
		projMatrix = Mat4Multiply(projMatrix, Mat4Translate2(-viewportRec.width/2.0f, -viewportRec.height/2.0f));
		projMatrix = Mat4Multiply(projMatrix, Mat4Translate2(-viewportRec.x, -viewportRec.y));
		RcSetProjectionMatrix(projMatrix);
	}
}

void RcSetColor1(Color_t color)
{
	NotNull(rc);
	if (rc->state.color1.value == color.value) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetColor1_OpenGL(ToColorf(color));
			rc->state.color1 = color;
			rc->state.color1f = ToColorf(color);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetColor1!"); break;
	}
}
void RcSetColor1(Colorf_t color)
{
	NotNull(rc);
	if (rc->state.color1f == color) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetColor1_OpenGL(color);
			rc->state.color1 = ToColor(color);
			rc->state.color1f = color;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetColor1!"); break;
	}
}
void RcSetColor2(Color_t color)
{
	NotNull(rc);
	if (rc->state.color2.value == color.value) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetColor2_OpenGL(ToColorf(color));
			rc->state.color2 = color;
			rc->state.color2f = ToColorf(color);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetColor2!"); break;
	}
}
void RcSetColor2(Colorf_t color)
{
	NotNull(rc);
	if (rc->state.color2f == color) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetColor2_OpenGL(color);
			rc->state.color2 = ToColor(color);
			rc->state.color2f = color;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetColor2!"); break;
	}
}

void RcSetSourceRec1(rec rectangle)
{
	NotNull(rc);
	NotNull(rc->state.boundTexture1);
	if (RecBasicallyEqual(rc->state.sourceRec1, rectangle)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetSourceRec1_OpenGL(rectangle, rc->state.boundTexture1->isFlippedY, rc->state.boundTexture1->height);
			rc->state.sourceRec1 = rectangle;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetSourceRec1!"); break;
	}
}
void RcSetSourceRec2(rec rectangle)
{
	NotNull(rc);
	NotNull(rc->state.boundTexture2);
	if (RecBasicallyEqual(rc->state.sourceRec2, rectangle)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetSourceRec2_OpenGL(rectangle, rc->state.boundTexture2->isFlippedY, rc->state.boundTexture2->height);
			rc->state.sourceRec2 = rectangle;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetSourceRec2!"); break;
	}
}

void RcSetShiftVec(v2 shiftVec)
{
	NotNull(rc);
	if (Vec2BasicallyEqual(rc->state.shiftVec, shiftVec)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetShiftVec_OpenGL(shiftVec);
			rc->state.shiftVec = shiftVec;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetShiftVec!"); break;
	}
}

void RcSetCount(i32 count)
{
	NotNull(rc);
	if (rc->state.count == count) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetCount_OpenGL(count);
			rc->state.count = count;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetColor1!"); break;
	}
}

void RcSetCircleRadius(r32 radius)
{
	NotNull(rc);
	if (BasicallyEqualR32(rc->state.circleRadius, radius)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetCircleRadius_OpenGL(radius);
			rc->state.circleRadius = radius;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetCircleRadius!"); break;
	}
}
void RcSetCircleInnerRadius(r32 innerRadius)
{
	NotNull(rc);
	if (BasicallyEqualR32(rc->state.circleInnerRadius, innerRadius)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetCircleInnerRadius_OpenGL(innerRadius);
			rc->state.circleInnerRadius = innerRadius;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetCircleInnerRadius!"); break;
	}
}

void RcSetTime(r32 time)
{
	NotNull(rc);
	if (BasicallyEqualR32(rc->state.time, time)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetTime_OpenGL(time);
			rc->state.time = time;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetSaturation!"); break;
	}
}
void RcSetSaturation(r32 saturation)
{
	NotNull(rc);
	if (BasicallyEqualR32(rc->state.saturation, saturation)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetSaturation_OpenGL(saturation);
			rc->state.saturation = saturation;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetSaturation!"); break;
	}
}
void RcSetBrightness(r32 brightness)
{
	NotNull(rc);
	if (BasicallyEqualR32(rc->state.brightness, brightness)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetBrightness_OpenGL(brightness);
			rc->state.brightness = brightness;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetSaturation!"); break;
	}
}

void RcSetPolygonPlane(u64 planeIndex, r32 direction, r32 distance)
{
	NotNull(rc);
	Assert(planeIndex < ShaderUniform_NumPolygonPlanes);
	v2 newValue = NewVec2(direction, distance);
	if (Vec2BasicallyEqual(rc->state.polygonPlanes[planeIndex], newValue)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			rc->state.polygonPlanes[planeIndex] = newValue;
			RcSetPolygonPlanes_OpenGL(&rc->state.polygonPlanes[0]);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetPolygonPlane!"); break;
	}
}

void RcSetValue(u8 valueIndex, r32 value)
{
	NotNull(rc);
	Assert(valueIndex < ArrayCount(rc->state.values));
	if (BasicallyEqualR32(rc->state.values[valueIndex], value)) { return; }
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetValue_OpenGL(valueIndex, value);
			rc->state.values[valueIndex] = value;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetValue!"); break;
	}
}

bool RcSetDynamicUniformR32(MyStr_t uniformName, r32 value)
{
	NotNullStr(&uniformName);
	ShaderDynamicUniform_t* uniform = GetDynamicUniform(rc->state.boundShader, uniformName, true);
	if (uniform == nullptr) { return false; }
	bool result = false;
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDynamicUniformR32_OpenGL(uniform, value);
			result = true;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDynamicUniformR32!"); break;
	}
	return result;
}
bool RcSetDynamicUniformVec2(MyStr_t uniformName, v2 value)
{
	NotNullStr(&uniformName);
	ShaderDynamicUniform_t* uniform = GetDynamicUniform(rc->state.boundShader, uniformName, true);
	if (uniform == nullptr) { return false; }
	bool result = false;
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDynamicUniformVec2_OpenGL(uniform, value);
			result = true;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDynamicUniformR32!"); break;
	}
	return result;
}
bool RcSetDynamicUniformVec3(MyStr_t uniformName, v3 value)
{
	NotNullStr(&uniformName);
	ShaderDynamicUniform_t* uniform = GetDynamicUniform(rc->state.boundShader, uniformName, true);
	if (uniform == nullptr) { return false; }
	bool result = false;
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDynamicUniformVec3_OpenGL(uniform, value);
			result = true;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDynamicUniformR32!"); break;
	}
	return result;
}
bool RcSetDynamicUniformVec4(MyStr_t uniformName, v4 value)
{
	NotNullStr(&uniformName);
	ShaderDynamicUniform_t* uniform = GetDynamicUniform(rc->state.boundShader, uniformName, true);
	if (uniform == nullptr) { return false; }
	bool result = false;
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDynamicUniformVec4_OpenGL(uniform, value);
			result = true;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDynamicUniformR32!"); break;
	}
	return result;
}
bool RcSetDynamicUniformMat4(MyStr_t uniformName, mat4 value)
{
	NotNullStr(&uniformName);
	ShaderDynamicUniform_t* uniform = GetDynamicUniform(rc->state.boundShader, uniformName, true);
	if (uniform == nullptr) { return false; }
	bool result = false;
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetDynamicUniformMat4_OpenGL(uniform, value);
			result = true;
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetDynamicUniformR32!"); break;
	}
	return result;
}

bool RcSetDynamicUniformR32(const char* nullTermStr, r32 value)
{
	return RcSetDynamicUniformR32(NewStr(nullTermStr), value);
}
bool RcSetDynamicUniformVec2(const char* nullTermStr, v2 value)
{
	return RcSetDynamicUniformVec2(NewStr(nullTermStr), value);
}
bool RcSetDynamicUniformVec3(const char* nullTermStr, v3 value)
{
	return RcSetDynamicUniformVec3(NewStr(nullTermStr), value);
}
bool RcSetDynamicUniformVec4(const char* nullTermStr, v4 value)
{
	return RcSetDynamicUniformVec4(NewStr(nullTermStr), value);
}
bool RcSetDynamicUniformMat4(const char* nullTermStr, mat4 value)
{
	return RcSetDynamicUniformMat4(NewStr(nullTermStr), value);
}

void RcClearColor(Color_t color)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcClearColor_OpenGL(color);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcClearColor!"); break;
	}
}
void RcClearDepth(r32 depth)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcClearDepth_OpenGL(RcGetRealDepth(depth));
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcClearDepth!"); break;
	}
}
void RcClearStencil(i32 stencilValue)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcClearStencil_OpenGL(stencilValue);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcClearStencil!"); break;
	}
}

void RcDrawBuffer(VertBufferPrimitive_t primitive, u64 startIndex = 0, u64 numElements = 0, u64 vertexOffset = 0)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcDrawBuffer_OpenGL(primitive, startIndex, numElements, vertexOffset);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcDrawBuffer!"); break;
	}
}

// +==============================+
// |           Stencil            |
// +==============================+
void RcStartStencilDrawing()
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcStartStencilDrawing_OpenGL();
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcStartStencilDrawing!"); break;
	}
}
void RcSetStencilPolarity(bool writePositiveValues)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcSetStencilPolarity_OpenGL(writePositiveValues);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcSetStencilPolarity!"); break;
	}
}
void RcUseStencil(bool inverseMask = false)
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcUseStencil_OpenGL(inverseMask);
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcUseStencil!"); break;
	}
}
void RcDisableStencil()
{
	NotNull(rc);
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcDisableStencil_OpenGL();
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcDisableStencil!"); break;
	}
}

// +==============================+
// |            Begin             |
// +==============================+
void RcBegin(const PlatWindow_t* window, FrameBuffer_t* frameBuffer, Shader_t* initialShader, Color_t clearColor, r32 clearDepth = 1.0f, i32 clearStencil = 0)
{
	NotNull(pig);
	NotNull(rc);
	NotNull(window);
	NotNull(initialShader);
	
	Pig_ChangeWindow(window);
	rc->currentWindow = window;
	
	RcBindFrameBuffer(frameBuffer, true);
	if (clearColor.a > 0) { RcClearColor(clearColor); }
	RcClearDepth(clearDepth);
	//TODO: Should we check if the current window actually has a stencil buffer?
	RcClearStencil(clearStencil);
	
	// +======================================+
	// | API Specific Initialization/Options  |
	// +======================================+
	switch (pig->renderApi)
	{
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			RcBegin_OpenGL();
		} break;
		#endif
		
		default: DebugAssertMsg(false, "Unsupported render API in RcBegin!"); break;
	}
	
	// +==============================+
	// | Set Default Values for State |
	// +==============================+
	rc->state.faceCulling = true;
	rc->state.depthTestEnabled = true;
	rc->state.fillMode = PolygonFillMode_Default;
	rc->state.lineThickness = 1.0f;
	
	rc->state.boundShader = nullptr;
	rc->state.boundVao = nullptr;
	rc->state.boundTexture1 = nullptr;
	rc->state.boundTexture2 = nullptr;
	rc->state.boundSpriteSheet = nullptr;
	rc->state.boundBuffer = nullptr;
	rc->state.boundModel = nullptr;
	
	rc->state.boundFont = nullptr;
	rc->state.faceSelector = {};
	rc->state.fontScale = 1.0f;
	rc->state.flowCallbacks = {};
	
	rc->state.worldMatrix = Mat4_Identity;
	rc->state.augmentMatrix = Mat4_Identity;
	rc->state.viewMatrix = Mat4_Identity;
	rc->state.projectionMatrix = Mat4_Identity;
	
	rc->state.cameraPosition = Vec3_Zero;
	rc->state.playerPosition = Vec3_Zero;
	
	rc->state.depth = clearDepth;
	
	rc->state.sourceRec1 = Rec_Default;
	rc->state.sourceRec2 = Rec_Default;
	rc->state.maskRectangle = Rec_Zero;
	rc->state.shiftVec = Vec2_Zero;
	rc->state.count = 0;
	
	rc->state.color1 = White;
	rc->state.color1f = ToColorf(rc->state.color1);
	rc->state.color2 = White;
	rc->state.color2f = ToColorf(rc->state.color2);
	for (u64 cIndex = 0; cIndex < ArrayCount(rc->state.replaceColors); cIndex++)
	{
		rc->state.replaceColors[cIndex] = White;
		rc->state.replaceColorsf[cIndex] = ToColorf(rc->state.replaceColors[cIndex]);
	}
	
	rc->state.time = 0.0f;
	rc->state.brightness = 1.0f;
	rc->state.saturation = 1.0f;
	rc->state.circleRadius = 0.0f;
	rc->state.circleInnerRadius = 0.0f;
	
	ClearArray(rc->state.values);
	ClearArray(rc->state.polygonPlanes);
	
	RcBindShader(initialShader);
	RcSetViewport(NewRec(Vec2_Zero, window->input.renderResolution));
}
//Overload with a PostProcessingChain_t* rather than FrameBuffer_t*
void RcBegin(const PlatWindow_t* window, PostProcessingChain_t* postProcessingChain, u64 inputIndex, Shader_t* initialShader, Color_t clearColor, r32 clearDepth = 1.0f, i32 clearStencil = 0)
{
	NotNull(postProcessingChain);
	RcBegin(window, GetPostProcessingChainInputBuffer(postProcessingChain, inputIndex), initialShader, clearColor, clearDepth, clearStencil);
}


// +==============================+
// |       Basic Resources        |
// +==============================+
void RcLoadBasicResources()
{
	Vertex3D_t lineVerts[] =
	{
		//position, color, texCoord, normal
		{ {0, 0, 0}, ToVec4(White), {0, 0}, {0, 1, 0}, }, //top-left
		{ {1, 0, 0}, ToVec4(White), {1, 0}, {0, 1, 0}, }, //top-right
	};
	if (!CreateVertBuffer3D(fixedHeap, &rc->lineBuffer, false, ArrayCount(lineVerts), lineVerts, true))
	{
		WriteLine_E("Failed to create the line vertex buffer!");
		DebugAssert(false);
	}
	
	Vertex2D_t squareVerts[] =
	{
		//position, color, texCoord
		{ {0, 0, 0}, ToVec4(White), {0, 0}, }, //top-left
		{ {1, 0, 0}, ToVec4(White), {1, 0}, }, //top-right
		{ {0, 1, 0}, ToVec4(White), {0, 1}, }, //bottom-left
		
		{ {1, 1, 0}, ToVec4(White), {1, 1}, }, //bottom-right
		{ {0, 1, 0}, ToVec4(White), {0, 1}, }, //bottom-left
		{ {1, 0, 0}, ToVec4(White), {1, 0}, }, //top-right
	};
	if (!CreateVertBuffer2D(fixedHeap, &rc->squareBuffer, false, ArrayCount(squareVerts), squareVerts, true))
	{
		WriteLine_E("Failed to create the square vertex buffer!");
		DebugAssert(false);
	}
	
	Vertex2D_t rightAngleTriVerts[] =
	{
		//position, color, texCoord
		{ {0, 0, 0}, ToVec4(White), {0, 0}, }, //top-left
		{ {1, 0, 0}, ToVec4(White), {1, 0}, }, //top-right
		{ {0, 1, 0}, ToVec4(White), {0, 1}, }, //bottom-left
	};
	if (!CreateVertBuffer2D(fixedHeap, &rc->rightAngleTriBuffer, false, ArrayCount(rightAngleTriVerts), rightAngleTriVerts, true))
	{
		WriteLine_E("Failed to create the riangle angle triangle vertex buffer!");
		DebugAssert(false);
	}
	
	Hexagon_t hexagon = NewHexagon(Vec2_Half, 0.5f, -ToRadians32(30));
	v3 hexagonVertices[HEXAGON_NUM_VERTICES];
	v3 hexagonCenter = NewVec3(0.5f, 0.5f, 0.0f);
	for (u64 vIndex = 0; vIndex < HEXAGON_NUM_VERTICES; vIndex++) { hexagonVertices[vIndex] = Vec3FromVec2(GetHexagonVertex(hexagon, vIndex), 0); }
	Vertex2D_t voxelOrthoVerts2D[6*3] = {
		//Front Face (which is facing down left)
		{ hexagonVertices[4], ToVec4(White), {0, 0} },
		{ hexagonCenter,      ToVec4(White), {1, 0} },
		{ hexagonVertices[3], ToVec4(White), {0, 1} },
		
		{ hexagonVertices[2], ToVec4(White), {1, 1} },
		{ hexagonVertices[3], ToVec4(White), {0, 1} },
		{ hexagonCenter,      ToVec4(White), {1, 0} },
		
		//Left Face (which is facing down right)
		{ hexagonCenter,      ToVec4(White), {0, 0} },
		{ hexagonVertices[0], ToVec4(White), {1, 0} },
		{ hexagonVertices[2], ToVec4(White), {0, 1} },
		
		{ hexagonVertices[1], ToVec4(White), {1, 1} },
		{ hexagonVertices[2], ToVec4(White), {0, 1} },
		{ hexagonVertices[0], ToVec4(White), {1, 0} },
		
		//Top Face (which is facing up)
		{ hexagonCenter,      ToVec4(White), {0, 0} },
		{ hexagonVertices[4], ToVec4(White), {1, 0} },
		{ hexagonVertices[0], ToVec4(White), {0, 1} },
		
		{ hexagonVertices[5], ToVec4(White), {1, 1} },
		{ hexagonVertices[0], ToVec4(White), {0, 1} },
		{ hexagonVertices[4], ToVec4(White), {1, 0} },
	};
	if (!CreateVertBuffer2D(mainHeap, &rc->voxelOrtho2D, false, ArrayCount(voxelOrthoVerts2D), &voxelOrthoVerts2D[0], true))
	{
		WriteLine_E("Failed to create voxelOrtho2D vertex buffer!");
		DebugAssert(false);
	}
	
	//cubeBuffer
	{
		PrimitiveIndexedVerts_t primVerts = GenerateVertsForBox(NewBox(0, 0, 0, 1, 1, 1), &pig->largeAllocHeap);
		NotNull(primVerts.vertices);
		NotNull(primVerts.indices);
		// InvertPrimitiveVerts(&primVerts);
		if (!CreateVertBufferFromIndexedPrimitiveVerts3D(fixedHeap, &rc->cubeBuffer, false, &primVerts, White, true))
		{
			WriteLine_E("Failed to create the cube vertex buffer!");
			DebugAssert(false);
		}
		FreePrimitiveIndexedVerts(&primVerts);
	}
	
	//skyboxBuffer
	{
		PrimitiveIndexedVerts_t primVerts = GenerateVertsForBox(NewBox(0, 0, 0, 1, 1, 1), &pig->largeAllocHeap);
		NotNull(primVerts.vertices);
		NotNull(primVerts.indices);
		InvertPrimitiveVerts(&primVerts);
		for (u64 iIndex = 0; iIndex < primVerts.numIndices; iIndex++)
		{
			PrimitiveIndex3D_t* index = &primVerts.indices[iIndex];
			rec faceSourceRec = Rec_Default;
			bool flipYAxis = false;
			bool flipXAxis = true;
			switch (index->faceIndex)
			{
				case 0: faceSourceRec = NewRec( 4.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //top
				case 1: faceSourceRec = NewRec( 0.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //right
				case 2: faceSourceRec = NewRec( 8.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //front
				case 3: faceSourceRec = NewRec( 2.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //left
				case 4: faceSourceRec = NewRec(10.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //back
				case 5: faceSourceRec = NewRec( 6.5f + 1.0f, 0.0f, -1.0f, 1.0f); break; //bottom
				default: Assert(index->faceIndex < 6); break;
			}
			if (flipXAxis) { index->texCoord.x = (1 - index->texCoord.x); }
			if (flipYAxis) { index->texCoord.y = (1 - index->texCoord.y); }
			index->texCoord = faceSourceRec.topLeft + Vec2Multiply(index->texCoord, faceSourceRec.size);
		}
		if (!CreateVertBufferFromIndexedPrimitiveVerts3D(fixedHeap, &rc->skyboxBuffer, false, &primVerts, White, true))
		{
			WriteLine_E("Failed to create the skybox vertex buffer!");
			DebugAssert(false);
		}
		FreePrimitiveIndexedVerts(&primVerts);
	}
	
	//sphereBuffer
	u64 sphereRingCounts[]    = RC_SPHERE_BUFFER_RING_COUNTS;
	u64 sphereSegmentCounts[] = RC_SPHERE_BUFFER_SEGMENT_COUNTS;
	Assert(ArrayCount(sphereRingCounts) == SphereQuality_NumQualities);
	Assert(ArrayCount(sphereSegmentCounts) == SphereQuality_NumQualities);
	for (u64 sIndex = 0; sIndex < SphereQuality_NumQualities; sIndex++)
	{
		PrimitiveIndexedVerts_t primVerts = GenerateVertsForSphere(NewSphere(Vec3_Zero, 1), sphereRingCounts[sIndex], sphereSegmentCounts[sIndex], true, &pig->largeAllocHeap);
		NotNull(primVerts.vertices);
		NotNull(primVerts.indices);
		InvertPrimitiveVerts(&primVerts);
		const bool copyVertices = false; //these vertices take up a decent amount of space, we won't enable this unless needed for some reason
		if (!CreateVertBufferFromIndexedPrimitiveVerts3D(fixedHeap, &rc->sphereBuffers[sIndex], false, &primVerts, White, copyVertices))
		{
			WriteLine_E("Failed to create the sphere vertex buffer!");
			DebugAssert(false);
		}
		FreePrimitiveIndexedVerts(&primVerts);
	}
	
	r32 triangleHeight = SqrtR32(3)/2.0f;
	Vertex2D_t equilTriangleVertices[] =
	{
		//position, color, texCoord
		{  { triangleHeight, 0.0f, 0.0f}, ToVec4(White), {1.0f, 0.5f} },
		{  { 0.0f,           0.5f, 0.0f}, ToVec4(White), {0.0f, 1.0f} },
		{  { 0.0f,          -0.5f, 0.0f}, ToVec4(White), {0.0f, 0.0f} },
	};
	if (!CreateVertBuffer2D(fixedHeap, &rc->equilTriangleBuffer, false, ArrayCount(equilTriangleVertices), equilTriangleVertices, true))
	{
		WriteLine_E("Failed to create the equilateral triangle vertex buffer!");
		DebugAssert(false);
	}
	
	Vertex3D_t scratchBuffer3DVerts[RC_SCRATCH_BUFFER_SIZE];
	MyMemSet(&scratchBuffer3DVerts[0], 0x00, sizeof(Vertex3D_t) * RC_SCRATCH_BUFFER_SIZE);
	if (!CreateVertBuffer3D(fixedHeap, &rc->scratchBuffer3D, true, RC_SCRATCH_BUFFER_SIZE, &scratchBuffer3DVerts[0], true))
	{
		WriteLine_E("Failed to create the scratch buffer 3D!");
		DebugAssert(false);
	}
	
	Vertex2D_t scratchBuffer2DVerts[RC_SCRATCH_BUFFER_SIZE];
	MyMemSet(&scratchBuffer2DVerts[0], 0x00, sizeof(Vertex2D_t) * RC_SCRATCH_BUFFER_SIZE);
	if (!CreateVertBuffer2D(fixedHeap, &rc->scratchBuffer2D, true, RC_SCRATCH_BUFFER_SIZE, &scratchBuffer2DVerts[0], true))
	{
		WriteLine_E("Failed to create the scratch buffer 2D!");
		DebugAssert(false);
	}
	
	if (!LoadTexture(fixedHeap, &rc->invalidTexture, NewStr(RESOURCE_FOLDER_TEXTURES "/pig_invalid.png"), true, true))
	{
		PrintLine_E("Failed to load invalid texture! Error %s%s%s", GetTextureErrorStr(rc->invalidTexture.error), (rc->invalidTexture.error == TextureError_ApiError) ? ": " : "", (rc->invalidTexture.error == TextureError_ApiError) ? rc->invalidTexture.apiErrorStr.pntr : "");
		DestroyTexture(&rc->invalidTexture);
		DebugAssert(false);
	}
	
	Color_t gradientColors[256];
	for (u32 cIndex = 0; cIndex < ArrayCount(gradientColors); cIndex++)
	{
		gradientColors[cIndex].r = (u8)cIndex;
		gradientColors[cIndex].g = (u8)cIndex;
		gradientColors[cIndex].b = (u8)cIndex;
		gradientColors[cIndex].a = 255;
	}
	PlatImageData_t gradientImageData = {};
	gradientImageData.pixelSize = sizeof(u32);
	gradientImageData.size = NewVec2i(ArrayCount(gradientColors), 1);
	gradientImageData.rowSize = gradientImageData.pixelSize * gradientImageData.width;
	gradientImageData.dataSize = gradientImageData.rowSize * gradientImageData.height;
	gradientImageData.data32 = (u32*)&gradientColors[0];
	if (!CreateTexture(fixedHeap, &rc->gradientHorizontal, &gradientImageData, false, false))
	{
		PrintLine_E("Failed to load gradientHorizontal texture! Error %s%s%s", GetTextureErrorStr(rc->gradientHorizontal.error), (rc->gradientHorizontal.error == TextureError_ApiError) ? ": " : "", (rc->gradientHorizontal.error == TextureError_ApiError) ? rc->gradientHorizontal.apiErrorStr.pntr : "");
		DestroyTexture(&rc->gradientHorizontal);
		DebugAssert(false);
	}
	gradientImageData.size = NewVec2i(1, ArrayCount(gradientColors));
	gradientImageData.rowSize = gradientImageData.pixelSize * gradientImageData.width;
	if (!CreateTexture(fixedHeap, &rc->gradientVertical, &gradientImageData, false, false))
	{
		PrintLine_E("Failed to load gradientVertical texture! Error %s%s%s", GetTextureErrorStr(rc->gradientVertical.error), (rc->gradientVertical.error == TextureError_ApiError) ? ": " : "", (rc->gradientVertical.error == TextureError_ApiError) ? rc->gradientVertical.apiErrorStr.pntr : "");
		DestroyTexture(&rc->gradientVertical);
		DebugAssert(false);
	}
	
	Color_t dotTexturePixel = White;
	PlatImageData_t dotTextureData = {};
	dotTextureData.data32 = &dotTexturePixel.value;
	dotTextureData.size = Vec2i_One;
	dotTextureData.pixelSize = sizeof(u32);
	dotTextureData.rowSize = dotTextureData.pixelSize * dotTextureData.width;
	dotTextureData.dataSize = dotTextureData.rowSize * dotTextureData.height;
	if (!CreateTexture(fixedHeap, &rc->dotTexture, &dotTextureData, true, true))
	{
		PrintLine_E("Failed to create dotTexture! Error %s%s%s", GetTextureErrorStr(rc->dotTexture.error), (rc->dotTexture.error == TextureError_ApiError) ? ": " : "", (rc->dotTexture.error == TextureError_ApiError) ? rc->dotTexture.apiErrorStr.pntr : "");
		DestroyTexture(&rc->dotTexture);
		DebugAssert(false);
	}
}
