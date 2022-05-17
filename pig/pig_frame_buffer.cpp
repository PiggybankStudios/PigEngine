/*
File:   pig_frame_buffer.cpp
Author: Taylor Robbins
Date:   02\20\2022
Description: 
	** Holds functions that help us create frame buffers, targets that we can bind and render to
*/

void DestroyFrameBuffer(FrameBuffer_t* buffer)
{
	NotNull(buffer);
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			DestroyTexture(&buffer->texture);
			if (buffer->antialiasingNumSamples > 0) { DestroyTexture(&buffer->outTexture); }
			if (buffer->glId != 0)
			{
				glDeleteFramebuffers(1, &buffer->glId);
				AssertNoOpenGlError();
			}
			if (buffer->glOutId != 0 && buffer->antialiasingNumSamples > 0)
			{
				glDeleteFramebuffers(1, &buffer->glOutId);
				AssertNoOpenGlError();
			}
			if (buffer->glRenderBuffId != 0)
			{
				glDeleteRenderbuffers(1, &buffer->glRenderBuffId);
				AssertNoOpenGlError();
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in DestroyFrameBuffer"); break;
	}
	if (buffer->apiErrorStr.pntr != nullptr)
	{
		NotNull(buffer->allocArena);
		FreeString(buffer->allocArena, &buffer->apiErrorStr);
	}
	ClearPointer(buffer);
}

bool CreateFrameBuffer(MemArena_t* memArena, FrameBuffer_t* bufferOut, v2i size, u64 antialiasingNumSamples, u8 channelFlags = FrameBufferChannel_Default)
{
	AssertSingleThreaded();
	NotNull(memArena);
	Assert(memArena != TempArena);
	NotNull(bufferOut);
	Assert(size.width > 0 && size.height > 0);
	ClearPointer(bufferOut);
	bufferOut->allocArena = memArena;
	bufferOut->id = pig->nextFrameBufferId;
	pig->nextFrameBufferId++;
	bufferOut->isValid = false;
	bufferOut->size = size;
	bufferOut->antialiasingNumSamples = antialiasingNumSamples;
	bufferOut->channelFlags = channelFlags;
	bufferOut->error = FrameBufferError_None;
	bool hasMsaa = (antialiasingNumSamples > 0);
	
	#if OPENGL_SUPPORTED
	#define CreateFrameBuffer_CheckOpenGlError(apiCallStr)                                         \
	{                                                                                              \
		errorStr = CheckOpenGlError(true);                                                         \
		if (errorStr != nullptr)                                                                   \
		{                                                                                          \
			bufferOut->apiErrorStr = PrintInArenaStr(memArena, apiCallStr " error: %s", errorStr); \
			bufferOut->error = FrameBufferError_ApiError;                                          \
		}                                                                                          \
	} if (errorStr != nullptr)
	#endif
	
	const char* errorStr = nullptr;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			glGenFramebuffers((GLsizei)1, &bufferOut->glId);
			CreateFrameBuffer_CheckOpenGlError("glGenFramebuffers(1)") { break; }
			
			glBindFramebuffer(GL_FRAMEBUFFER, bufferOut->glId);
			CreateFrameBuffer_CheckOpenGlError("glBindFramebuffer(GL_FRAMEBUFFER)") { break; }
			
			//this is reused below for the outTexture
			PlatImageData_t emptyImageData = {};
			emptyImageData.size = size;
			emptyImageData.pixelSize = (IsFlagSet(channelFlags, FrameBufferChannel_Opacity) ? 4 : 3); //either 32 pixel depth or 24
			emptyImageData.rowSize = (u64)size.width * emptyImageData.pixelSize; //TODO: This is technically unneeded by CreateTexture right now. Can we enforce that assumption?
			emptyImageData.dataSize = (u64)size.height * emptyImageData.rowSize; //TODO: This is technically unneeded by CreateTexture right now. Can we enforce that assumption?
			emptyImageData.data8 = nullptr;
			
			const bool pixelated = false, repeating = false, reverseByteOrder = false, generateMipmap = !hasMsaa;
			if (!CreateTexture(memArena, &bufferOut->texture, &emptyImageData, pixelated, repeating, reverseByteOrder, generateMipmap, antialiasingNumSamples))
			{
				bufferOut->error = FrameBufferError_TextureError;
				break;
			}
			// Framebuffers render with (0, 0) in the bottom left of the texture, so when we go to use this texture we should flip it on the y-axis to act like other textures we use
			bufferOut->texture.isFlippedY = true;
			
			//If we want depth or stencil components to the buffer then we need to create a renderbuffer and attach it
			if (IsFlagSet(channelFlags, FrameBufferChannel_Depth) || IsFlagSet(channelFlags, FrameBufferChannel_Stencil))
			{
				glGenRenderbuffers(1, &bufferOut->glRenderBuffId);
				CreateFrameBuffer_CheckOpenGlError("glGenRenderbuffers(1)") { break; }
				glBindRenderbuffer(GL_RENDERBUFFER, bufferOut->glRenderBuffId);
				CreateFrameBuffer_CheckOpenGlError("glBindRenderbuffer(GL_RENDERBUFFER)") { break; }
				
				GLenum renderbufferFormat = GL_DEPTH_COMPONENT24;
				if (IsFlagSet(channelFlags, FrameBufferChannel_Stencil))
				{
					renderbufferFormat = (IsFlagSet(channelFlags, FrameBufferChannel_Depth) ? GL_DEPTH24_STENCIL8 : GL_STENCIL_INDEX8);
				}
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, renderbufferFormat, size.width, size.height);
				CreateFrameBuffer_CheckOpenGlError("glRenderbufferStorage(GL_RENDERBUFFER)") { break; }
				
				if (IsFlagSet(channelFlags, FrameBufferChannel_Stencil))
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufferOut->glRenderBuffId);
					CreateFrameBuffer_CheckOpenGlError("glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER)") { break; }
				}
				if (IsFlagSet(channelFlags, FrameBufferChannel_Depth))
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferOut->glRenderBuffId);
					CreateFrameBuffer_CheckOpenGlError("glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER)") { break; }
				}
			}
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (hasMsaa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), bufferOut->texture.glId, 0);
			CreateFrameBuffer_CheckOpenGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0)") { break; }
			
			GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, &drawBuffers[0]);
			CreateFrameBuffer_CheckOpenGlError("glDrawBuffers(1)") { break; }
			
			GLenum bufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (bufferStatus != GL_FRAMEBUFFER_COMPLETE)
			{
				const char* statusStr = "";
				switch (bufferStatus)
				{
					case GL_FRAMEBUFFER_UNDEFINED:                     statusStr = "GL_FRAMEBUFFER_UNDEFINED"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         statusStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
					case GL_FRAMEBUFFER_UNSUPPORTED:                   statusStr = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
					case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      statusStr = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
					default: statusStr = TempPrint("%d", bufferStatus); break;
				}
				bufferOut->apiErrorStr = PrintInArenaStr(memArena, "FramebufferStatus came back as \"%s\"", statusStr);
				bufferOut->error = FrameBufferError_ApiError;
				break;
			}
			
			if (hasMsaa)
			{
				glGenFramebuffers(1, &bufferOut->glOutId);
				CreateFrameBuffer_CheckOpenGlError("glGenFramebuffers(1, glOutId)") { break; }
				
				glBindFramebuffer(GL_FRAMEBUFFER, bufferOut->glOutId);
				CreateFrameBuffer_CheckOpenGlError("glBindFramebuffer(GL_FRAMEBUFFER, glOutId)") { break; }
				
				const bool outPixelated = false, outRepeating = false;
				if (!CreateTexture(memArena, &bufferOut->outTexture, &emptyImageData, outPixelated, outRepeating))
				{
					bufferOut->error = FrameBufferError_OutTextureError;
					break;
				}
				bufferOut->outTexture.isFlippedY = true;
				
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferOut->outTexture.glId, 0);
				CreateFrameBuffer_CheckOpenGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D)") { break; }
				
				GLenum intDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, &intDrawBuffers[0]);
				CreateFrameBuffer_CheckOpenGlError("glDrawBuffers(1)") { break; }
				
				GLenum intBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (intBufferStatus != GL_FRAMEBUFFER_COMPLETE)
				{
					const char* statusStr = "";
					switch (intBufferStatus)
					{
						case GL_FRAMEBUFFER_UNDEFINED:                     statusStr = "GL_FRAMEBUFFER_UNDEFINED"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         statusStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
						case GL_FRAMEBUFFER_UNSUPPORTED:                   statusStr = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
						case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      statusStr = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
						default: statusStr = TempPrint("%d", intBufferStatus); break;
					}
					bufferOut->apiErrorStr = PrintInArenaStr(memArena, "Out FramebufferStatus came back as \"%s\"", statusStr);
					bufferOut->error = FrameBufferError_ApiError;
					break;
				}
			}
			
			bufferOut->isValid = true;
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			bufferOut->error = FrameBufferError_UnsupportedApi;
		} break;
	}
	
	AssertIf(!bufferOut->isValid, bufferOut->error != FrameBufferError_None);
	return bufferOut->isValid;
}

const char* PrintFrameBufferError(const FrameBuffer_t* buffer)
{
	NotNull(buffer);
	if (buffer->error == FrameBufferError_TextureError)
	{
		return TempPrint("Texture: %s", PrintTextureError(&buffer->texture));
	}
	else if (buffer->error == FrameBufferError_OutTextureError)
	{
		return TempPrint("OutTexture: %s", PrintTextureError(&buffer->outTexture));
	}
	else if (buffer->error == FrameBufferError_ApiError)
	{
		return TempPrint("Api: %.*s", buffer->apiErrorStr.length, buffer->apiErrorStr.pntr);
	}
	else
	{
		return GetFrameBufferErrorStr(buffer->error);
	}
}

void PrepareFrameBufferTexture(FrameBuffer_t* buffer)
{
	NotNull(buffer);
	Assert(buffer->isValid);
	
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			if (buffer->antialiasingNumSamples > 0)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->glId);
				AssertNoOpenGlError();
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->glOutId);
				AssertNoOpenGlError();
				glBlitFramebuffer(0, 0, buffer->width, buffer->height, 0, 0, buffer->width, buffer->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				AssertNoOpenGlError();
			}
			else
			{
				//For non-msaa buffers there is only one texture, but usage code is going to look at
				//outTexture so we're just gonna copy texture over there
				buffer->outTexture = buffer->texture;
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in PrepareFrameBufferTexture"); break;
	}
}
