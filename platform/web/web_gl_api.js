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
		let newShader = globals.glContext.createShader(type);
		let result = glGlobals.shaders.length;
		glGlobals.shaders.push(newShader);
		// console.log("Created shader[" + result + "]");
		return result;
	},
	jsGlCreateProgram: () =>
	{
		let newProgram = globals.glContext.createProgram();
		let result = glGlobals.programs.length;
		glGlobals.programs.push(newProgram);
		// console.log("Created program[" + result + "]");
		return result;
	},
	jsGlCreateTexture: () =>
	{
		let newTexture = globals.glContext.createTexture(); 
		let result = glGlobals.textures.length;
		glGlobals.textures.push(newTexture);
		// console.log("Created texture[" + result + "]");
		return result;
	},
	// glDeleteBuffers: (count, buffers)                 => { globals.glContext.deleteBuffers(count, buffers); },
	jsGlCreateBuffer: () =>
	{
		let newBuffer = globals.glContext.createBuffer();
		let result = glGlobals.buffers.length;
		glGlobals.buffers.push(newBuffer);
		// console.log("Created buffer[" + result + "]");
		return result;
	},
	jsGlCreateVertexArray: () =>
	{
		let newVao = globals.glContext.createVertexArray(); 
		let result = glGlobals.vaos.length;
		glGlobals.vaos.push(newVao);
		// console.log("Created vao[" + result + "]");
		return result;
	},
	jsGlGetAttribLocation: (programIndex, namePntr) =>
	{
		let newLocation = globals.glContext.getAttribLocation(glGlobals.programs[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glGlobals.attribLocations.length;
		glGlobals.attribLocations.push(newLocation);
		return result;
	},
	jsGlGetUniformLocation: (programIndex, namePntr) =>
	{
		let newLocation = globals.glContext.getUniformLocation(glGlobals.programs[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glGlobals.uniformLocations.length;
		glGlobals.uniformLocations.push(newLocation);
		return result;
	},
	
	jsGlGetError:                ()                                              => { globals.glContext.getError(); },
	jsGlClearColor:              (r, g, b, a)                                    => { globals.glContext.clearColor(r, g, b, a); },
	jsGlClear:                   (mask)                                          => { globals.glContext.clear(mask); },
	jsGlViewport:                (x, y, width, height)                           => { globals.glContext.viewport(x, y, width, height); },
	jsGlEnable:                  (cap)                                           => { globals.glContext.enable(cap); },
	jsGlDisable:                 (cap)                                           => { globals.glContext.disable(cap); },
	jsGlBlendFunc:               (sfactor, dfactor)                              => { globals.glContext.blendFunc(sfactor, dfactor); },
	jsGlBlendFuncSeparate:       (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) => { globals.glContext.blendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); },
	jsGlDepthFunc:               (func)                                          => { globals.glContext.depthFunc(func); },
	jsGlFrontFace:               (mode)                                          => { globals.glContext.frontFace(mode); },
	jsGlLineWidth:               (width)                                         => { globals.glContext.lineWidth(width); },
	jsGlShaderSource:            (shaderIndex, count, stringPntr)                => { globals.glContext.shaderSource(glGlobals.shaders[shaderIndex], wasmPntrToJsString(wasmMemory, stringPntr)); },
	jsGlCompileShader:           (shaderIndex)                                   => { globals.glContext.compileShader(glGlobals.shaders[shaderIndex]); },
	jsGlAttachShader:            (programIndex, shaderIndex)                     => { globals.glContext.attachShader(glGlobals.programs[programIndex], glGlobals.shaders[shaderIndex]); },
	jsGlLinkProgram:             (programIndex)                                  => { globals.glContext.linkProgram(glGlobals.programs[programIndex]); },
	jsGlUniform1i:               (locationIndex, v0)                             => { globals.glContext.uniform1i(glGlobals.uniformLocations[locationIndex], v0); },
	jsGlUniform1f:               (locationIndex, v0)                             => { globals.glContext.uniform1f(glGlobals.uniformLocations[locationIndex], v0); },
	jsGlUniform2f:               (locationIndex, v0, v1)                         => { globals.glContext.uniform2f(glGlobals.uniformLocations[locationIndex], v0, v1); },
	jsGlUniform3f:               (locationIndex, v0, v1, v2)                     => { globals.glContext.uniform3f(glGlobals.uniformLocations[locationIndex], v0, v1, v2); },
	jsGlUniform4f:               (locationIndex, v0, v1, v2, v3)                 => { globals.glContext.uniform4f(glGlobals.uniformLocations[locationIndex], v0, v1, v2, v3); },
	jsGlUniformMatrix4fv:        (locationIndex, count, transpose, valuePntr)    => { globals.glContext.uniformMatrix4fv(glGlobals.uniformLocations[locationIndex], transpose, new Float32Array(wasmMemory.buffer, valuePntr, count * (4*4))); },
	jsGlUseProgram:              (programIndex)                                  => { globals.glContext.useProgram(glGlobals.programs[programIndex]); },
	jsGlBindTexture:             (target, textureIndex)                          => { globals.glContext.bindTexture(target, glGlobals.textures[textureIndex]); },
	jsGlTexImage2D:              (target, level, internalformat, width, height, border, format, type, pixelsPntr) => { globals.glContext.texImage2D(target, level, internalformat, width, height, border, format, type, new Uint8Array(wasmMemory.buffer, pixelsPntr)); },
	jsGlTexParameteri:           (target, pname, param)                          => { globals.glContext.texParameteri(target, pname, param); },
	jsGlGenerateMipmap:          (target)                                        => { globals.glContext.generateMipmap(target); },
	jsGlActiveTexture:           (texture)                                       => { globals.glContext.activeTexture(texture); },
	jsGlBindBuffer:              (target, bufferIndex)                           => { globals.glContext.bindBuffer(target, glGlobals.buffers[bufferIndex]); },
	jsGlBufferData:              (target, size, data, usage)                     => { globals.glContext.bufferData(target, new Uint8Array(wasmMemory.buffer, data), usage, 0, size); },
	jsGlBindVertexArray:         (vaoIndex)                                      => { globals.glContext.bindVertexArray(glGlobals.vaos[vaoIndex]); },
	jsGlEnableVertexAttribArray: (index)                                         => { globals.glContext.enableVertexAttribArray(glGlobals.attribLocations[index]); },
	jsGlVertexAttribPointer:     (index, size, type, normalized, stride, offset) => { globals.glContext.vertexAttribPointer(glGlobals.attribLocations[index], size, type, normalized, stride, offset); },
	jsGlDrawArrays:              (mode, first, count)                            => { globals.glContext.drawArrays(mode, first, count); },
};

// =========================== End of web_gl_api.js ============================