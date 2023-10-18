// ========================== Start of web_gl_api.js ===========================
/*
File:   web_gl_api.js
Author: Taylor Robbins
Date:   10\15\2023
Description:
	** Contains a bunch of functions that let the WASM module interact with Web GL
*/

var glGlobals =
{
	shaders: [],
	programs: [],
	textures: [],
	buffers: [],
	vaos: [],
	attribLocations: [],
	uniformLocations: [],
};

const webglDrawingFunctions =
{
	jsGlCreateShader: (type) =>
	{
		let newShader = stdGlobals.glContext.createShader(type);
		let result = glGlobals.shaders.length;
		glGlobals.shaders.push(newShader);
		// console.log("Created shader[" + result + "]");
		return result;
	},
	jsGlCreateProgram: () =>
	{
		let newProgram = stdGlobals.glContext.createProgram();
		let result = glGlobals.programs.length;
		glGlobals.programs.push(newProgram);
		// console.log("Created program[" + result + "]");
		return result;
	},
	jsGlCreateTexture: () =>
	{
		let newTexture = stdGlobals.glContext.createTexture(); 
		let result = glGlobals.textures.length;
		glGlobals.textures.push(newTexture);
		// console.log("Created texture[" + result + "]");
		return result;
	},
	// glDeleteBuffers: (count, buffers)                 => { stdGlobals.glContext.deleteBuffers(count, buffers); },
	jsGlCreateBuffer: () =>
	{
		let newBuffer = stdGlobals.glContext.createBuffer();
		let result = glGlobals.buffers.length;
		glGlobals.buffers.push(newBuffer);
		// console.log("Created buffer[" + result + "]");
		return result;
	},
	jsGlCreateVertexArray: () =>
	{
		let newVao = stdGlobals.glContext.createVertexArray(); 
		let result = glGlobals.vaos.length;
		glGlobals.vaos.push(newVao);
		// console.log("Created vao[" + result + "]");
		return result;
	},
	jsGlGetAttribLocation: (programIndex, namePntr) =>
	{
		let newLocation = stdGlobals.glContext.getAttribLocation(glGlobals.programs[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glGlobals.attribLocations.length;
		glGlobals.attribLocations.push(newLocation);
		return result;
	},
	jsGlGetUniformLocation: (programIndex, namePntr) =>
	{
		let newLocation = stdGlobals.glContext.getUniformLocation(glGlobals.programs[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glGlobals.uniformLocations.length;
		glGlobals.uniformLocations.push(newLocation);
		return result;
	},
	
	jsGlGetError:                ()                                              => { stdGlobals.glContext.getError(); },
	jsGlClearColor:              (r, g, b, a)                                    => { stdGlobals.glContext.clearColor(r, g, b, a); },
	jsGlClear:                   (mask)                                          => { stdGlobals.glContext.clear(mask); },
	jsGlViewport:                (x, y, width, height)                           => { stdGlobals.glContext.viewport(x, y, width, height); },
	jsGlEnable:                  (cap)                                           => { stdGlobals.glContext.enable(cap); },
	jsGlDisable:                 (cap)                                           => { stdGlobals.glContext.disable(cap); },
	jsGlBlendFunc:               (sfactor, dfactor)                              => { stdGlobals.glContext.blendFunc(sfactor, dfactor); },
	jsGlBlendFuncSeparate:       (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) => { stdGlobals.glContext.blendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); },
	jsGlDepthFunc:               (func)                                          => { stdGlobals.glContext.depthFunc(func); },
	jsGlFrontFace:               (mode)                                          => { stdGlobals.glContext.frontFace(mode); },
	jsGlLineWidth:               (width)                                         => { stdGlobals.glContext.lineWidth(width); },
	jsGlShaderSource:            (shaderIndex, count, stringPntr)                => { stdGlobals.glContext.shaderSource(glGlobals.shaders[shaderIndex], wasmPntrToJsString(wasmMemory, stringPntr)); },
	jsGlCompileShader:           (shaderIndex)                                   => { stdGlobals.glContext.compileShader(glGlobals.shaders[shaderIndex]); },
	jsGlAttachShader:            (programIndex, shaderIndex)                     => { stdGlobals.glContext.attachShader(glGlobals.programs[programIndex], glGlobals.shaders[shaderIndex]); },
	jsGlLinkProgram:             (programIndex)                                  => { stdGlobals.glContext.linkProgram(glGlobals.programs[programIndex]); },
	jsGlUniform1i:               (locationIndex, v0)                             => { stdGlobals.glContext.uniform1i(glGlobals.uniformLocations[locationIndex], v0); },
	jsGlUniform1f:               (locationIndex, v0)                             => { stdGlobals.glContext.uniform1f(glGlobals.uniformLocations[locationIndex], v0); },
	jsGlUniform2f:               (locationIndex, v0, v1)                         => { stdGlobals.glContext.uniform2f(glGlobals.uniformLocations[locationIndex], v0, v1); },
	jsGlUniform3f:               (locationIndex, v0, v1, v2)                     => { stdGlobals.glContext.uniform3f(glGlobals.uniformLocations[locationIndex], v0, v1, v2); },
	jsGlUniform4f:               (locationIndex, v0, v1, v2, v3)                 => { stdGlobals.glContext.uniform4f(glGlobals.uniformLocations[locationIndex], v0, v1, v2, v3); },
	jsGlUniformMatrix4fv:        (locationIndex, count, transpose, valuePntr)    => { stdGlobals.glContext.uniformMatrix4fv(glGlobals.uniformLocations[locationIndex], transpose, new Float32Array(wasmMemory.buffer, valuePntr, count * (4*4))); },
	jsGlUseProgram:              (programIndex)                                  => { stdGlobals.glContext.useProgram(glGlobals.programs[programIndex]); },
	jsGlBindTexture:             (target, textureIndex)                          => { stdGlobals.glContext.bindTexture(target, glGlobals.textures[textureIndex]); },
	jsGlTexImage2D:              (target, level, internalformat, width, height, border, format, type, pixelsPntr) => { stdGlobals.glContext.texImage2D(target, level, internalformat, width, height, border, format, type, new Uint8Array(wasmMemory.buffer, pixelsPntr)); },
	jsGlTexParameteri:           (target, pname, param)                          => { stdGlobals.glContext.texParameteri(target, pname, param); },
	jsGlGenerateMipmap:          (target)                                        => { stdGlobals.glContext.generateMipmap(target); },
	jsGlActiveTexture:           (texture)                                       => { stdGlobals.glContext.activeTexture(texture); },
	jsGlBindBuffer:              (target, bufferIndex)                           => { stdGlobals.glContext.bindBuffer(target, glGlobals.buffers[bufferIndex]); },
	jsGlBufferData:              (target, size, data, usage)                     => { stdGlobals.glContext.bufferData(target, new Uint8Array(wasmMemory.buffer, data), usage, 0, size); },
	jsGlBindVertexArray:         (vaoIndex)                                      => { stdGlobals.glContext.bindVertexArray(glGlobals.vaos[vaoIndex]); },
	jsGlEnableVertexAttribArray: (index)                                         => { stdGlobals.glContext.enableVertexAttribArray(glGlobals.attribLocations[index]); },
	jsGlVertexAttribPointer:     (index, size, type, normalized, stride, offset) => { stdGlobals.glContext.vertexAttribPointer(glGlobals.attribLocations[index], size, type, normalized, stride, offset); },
	jsGlDrawArrays:              (mode, first, count)                            => { stdGlobals.glContext.drawArrays(mode, first, count); },
};

// =========================== End of web_gl_api.js ============================