/*
File:   main.js
Author: Taylor Robbins
Date:   03\20\2022
Description:
	** This file contains all javascript code that wraps the Pig Engine web assembly platform layer
*/

// +--------------------------------------------------------------+
// |                          Constants                           |
// +--------------------------------------------------------------+
const UseSimpleRenderContext = false;
const WASM_PAGE_SIZE = (64 * 1024); //64kB
const WASM_PROTECTED_SIZE = 1024 //1kB at start of wasm memory should be unused and should never be written to

const INITIAL_PLAT_PAGE_COUNT = 7;
var engine = null;
var wasmMemory = null;
var stackBase = 0; //TODO: This is filled with a crude calculation. Can we get the exact address somehow?
var heapBase = 0;
var canvas = null;
var canvasContainer = null;
var canvasContext2d = null;
var canvasContextGl = null;
var glShaders = [];
var glPrograms = [];
var glTextures = [];
var glBuffers = [];
var glVaos = [];
var glAttribLocations = [];
var glUniformLocations = [];
var pixelRatio = 0;

const assembFilePath = "assembFuncs.wasm";
const engineFilePath = "PigEngine.wasm";

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
function sleep(delay)
{
	var start = new Date().getTime();
	while (new Date().getTime() < start + delay);
}

//TODO: For some reason this function isn't working properly. We should fix it at some point and use it
function checkPlatProtectedMemory(memory)
{
	let memArray = new Uint8Array(memory);
	var foundCorruption = false;
	for (var bIndex = 0; bIndex < WASM_PROTECTED_SIZE; bIndex++)
	{
		if (memArray[bIndex] != 0x55)
		{
			foundCorruption = true;
			memArray[bIndex] = 0x55;
		}
	}
	return (foundCorruption == false);
}

//TODO: Add support for utf-8 encoding
function wasmPntrToJsString(memory, ptr)
{
	const codes = [];
	const buf = new Uint8Array(memory.buffer);
	
	let cIndex = 0;
	while (true)
	{
		const char = buf[ptr + cIndex];
		if (!char) { break; }
		codes.push(char);
		cIndex++;
	}
	
	//TODO: Can we do something else? If we do our own UTF-8 parsing maybe?
	return String.fromCharCode(...codes);
}

function jsStringToWasmPntr(engine, memory, jsString)
{
	let allocSize = jsString.length+1;
	let result = engine.exports.AllocWasmMemory(allocSize);
	if (result != 0)
	{
		let buf = new Uint8Array(memory.buffer);
		for (var cIndex = 0; cIndex < jsString.length; cIndex++)
		{
			buf[result + cIndex] = jsString[cIndex];
		}
		buf[result + jsString.length] = '\0';
	}
	return result;
}

async function loadWasmModule(filePath, environment)
{
	console.log("Loading " + filePath + "...");
	let result = null;
	try
	{
		const fetchPromise = fetch(filePath);
		const wasmModule = await WebAssembly.instantiateStreaming(
			fetchPromise,
			{ env: environment }
		);
		result = wasmModule.instance;
		console.log("Loaded module exports:", result.exports);
	}
	catch (exception)
	{
		console.error("Failed to load WASM module from \"" + filePath + "\":", exception);
	}
	return result;
}

// +--------------------------------------------------------------+
// |               Environment Function Collections               |
// +--------------------------------------------------------------+
const intrinsicFunctions =
{
	fminf:     (value1, value2)  => { return Math.min(value1, value2);       },
	fmin:      (value1, value2)  => { return Math.min(value1, value2);       },
	fmaxf:     (value1, value2)  => { return Math.max(value1, value2);       },
	fmax:      (value1, value2)  => { return Math.max(value1, value2);       },
	fmodf:     (numer, denom)    => { return (numer % denom);                },
	fmod:      (numer, denom)    => { return (numer % denom);                },
	fmodl:     (numer, denom)    => { return (numer % denom);                },
	roundf:    (value)           => { return Math.round(value);              },
	round:     (value)           => { return Math.round(value);              },
	sinf:      (value)           => { return Math.sin(value);                },
	sin:       (value)           => { return Math.sin(value);                },
	asinf:     (value)           => { return Math.asin(value);               },
	asin:      (value)           => { return Math.asin(value);               },
	cosf:      (value)           => { return Math.cos(value);                },
	cos:       (value)           => { return Math.cos(value);                },
	acosf:     (value)           => { return Math.acos(value);               },
	acos:      (value)           => { return Math.acos(value);               },
	tanf:      (value)           => { return Math.tan(value);                },
	tan:       (value)           => { return Math.tan(value);                },
	atanf:     (value)           => { return Math.atan(value);               },
	atan:      (value)           => { return Math.atan(value);               },
	atan2f:    (numer, denom)    => { return Math.atan2(numer, denom);       },
	atan2:     (numer, denom)    => { return Math.atan2(numer, denom);       },
	powf:      (value, power)    => { return Math.pow(value, power);         },
	pow:       (value, power)    => { return Math.pow(value, power);         },
	cbrtf:     (value)           => { return Math.cbrt(value);               },
	cbrt:      (value)           => { return Math.cbrt(value);               },
	scalbnf:   (value, power)    => { return Math.scalbn(value, power);      },
	scalbn:    (value, power)    => { return Math.scalbn(value, power);      },
	scalbnl:   (value, power)    => { return Math.scalbn(value, power);      },
	copysignf: (magnitude, sign) => { return Math.copysign(magnitude, sign); },
	copysign:  (magnitude, sign) => { return Math.copysign(magnitude, sign); },
	copysignl: (magnitude, sign) => { return Math.copysign(magnitude, sign); },
	__extenddftf2: () => { }, //TODO: Implement me!
	__floatsitf:   () => { }, //TODO: Implement me!
	__floatunsitf: () => { }, //TODO: Implement me!
	__multf3:      () => { }, //TODO: Implement me!
	__divtf3:      () => { }, //TODO: Implement me!
	__addtf3:      () => { }, //TODO: Implement me!
	__subtf3:      () => { }, //TODO: Implement me!
	__netf2:       () => { }, //TODO: Implement me!
	__getf2:       () => { }, //TODO: Implement me!
	__extendsftf2: () => { }, //TODO: Implement me!
	__trunctfdf2:  () => { }, //TODO: Implement me!
};
const informationFunctions =
{
	js_GetTime: () => new Date().getTime(),
};
const memoryFunctions =
{
	js_GetHeapSize: () => //returns bytes
	{
		return wasmMemory.buffer.byteLength - heapBase;
	},
	js_GrowHeap: (numPages) => //takes pages, not bytes
	{
		wasmMemory.grow(numPages);
		console.log("Growing wasm memory by " + numPages + " page" + (numPages == 1 ? "" : "s") + " (total size: " + wasmMemory.buffer.byteLength.toString(16) + " " + wasmMemory.buffer.byteLength / WASM_PAGE_SIZE + " pages)");
	},
};
const consoleFunctions =
{
	js_PrintNumber:   (number) => console.log("Number: 0x" + number.toString(16) + " (" + number.toString(10) + ")"),
	js_ConsoleDebug:  ptr => console.debug("%c" + wasmPntrToJsString(wasmMemory, ptr), "color: #AFAFA2;"), //MonokaiGray1
	js_ConsoleLog:    ptr => console.log(wasmPntrToJsString(wasmMemory, ptr)),
	js_ConsoleInfo:   ptr => console.info("%c" + wasmPntrToJsString(wasmMemory, ptr), "color: #A6E22E;"), //MonokaiGreen
	js_ConsoleNotify: ptr => console.info("%c" + wasmPntrToJsString(wasmMemory, ptr), "color: #AE81FF;"), //MonokaiPurple
	js_ConsoleOther:  ptr => console.info("%c" + wasmPntrToJsString(wasmMemory, ptr), "color: #66D9EF;"), //MonokaiBlue
	js_ConsoleWarn:   ptr => console.warn(wasmPntrToJsString(wasmMemory, ptr)),
	js_ConsoleError:  ptr => console.error(wasmPntrToJsString(wasmMemory, ptr)),
};
const fileFunctions =
{
	js_LoadFile: (pathPtr) =>
	{
		let jsPath = wasmPntrToJsString(pathPtr);
		let wasmPath = jsStringToWasmPntr(engine, wasmMemory, jsPath);
		console.assert(wasmPath != 0, "Couldn't allocate path string copy!");
		
		var client = new XMLHttpRequest();
		client.open("GET", "/" + jsPath);
		client.onreadystatechange = function()
		{
			console.log("onreadystatechange:", client);
			if (engine !== null)
			{
				let wasmResponse = jsStringToWasmPntr(engine, wasmMemory, client.responseText);
				if (wasmResponse != 0)
				{
					console.log("wasmPath: " + wasmPath + " wasmResponse: " + wasmResponse);
					engine.exports.FileLoadedCallback(wasmPath, wasmResponse);
					engine.exports.FreeWasmMemory(wasmResponse, client.responseText.length+1);
				}
				engine.exports.FreeWasmMemory(wasmPath, jsPath.length+1);
			}
		}
		client.send();
	},
};
const basic2dDrawingFunctions =
{
	js_ClearCanvas: () => canvasContext2d.clearRect(0, 0, canvas.width, canvas.height),
	
	js_DrawRectangle: (x, y, width, height, red, green, blue, alpha) =>
	{
		const dx = x * pixelRatio;
		const dy = y * pixelRatio;
		const dwidth = width * pixelRatio; // device inner width
		const dheight = height * pixelRatio; // device inner height
		canvasContext2d.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
		canvasContext2d.fillRect(dx, dy, dwidth, dheight);
	},
	
	js_DrawRoundedRectangle: (x, y, width, height, radius, red, green, blue, alpha) =>
	{
		radius = Math.min(radius, width / 2, height / 2);
		
		const dx = x * pixelRatio;
		const dy = y * pixelRatio;
		const dradius = radius * pixelRatio;
		
		const diwidth = (width - (2 * radius)) * pixelRatio; // device inner width
		const diheight = (height - (2 * radius)) * pixelRatio; // device inner height
		
		canvasContext2d.beginPath();
		canvasContext2d.moveTo(dx + dradius, dy);
		canvasContext2d.lineTo(dx + dradius + diwidth, dy);
		canvasContext2d.arc(dx + dradius + diwidth, dy + dradius, dradius, -Math.PI/2, 0);
		canvasContext2d.lineTo(dx + dradius + diwidth + dradius, dy + dradius + diheight);
		canvasContext2d.arc(dx + dradius + diwidth, dy + dradius + diheight, dradius, 0, Math.PI/2);
		canvasContext2d.lineTo(dx + dradius, dy + dradius + diheight + dradius);
		canvasContext2d.arc(dx + dradius, dy + dradius + diheight, dradius, Math.PI/2, Math.PI);
		canvasContext2d.lineTo(dx, dy + dradius);
		canvasContext2d.arc(dx + dradius, dy + dradius, dradius, Math.PI, (3*Math.PI)/2);
		
		canvasContext2d.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
		canvasContext2d.fill();
	},
	
	js_DrawLine: (x1, y1, x2, y2, thickness, red, green, blue, alpha) =>
	{
		const dx1 = x1 * pixelRatio;
		const dy1 = y1 * pixelRatio;
		const dx2 = x2 * pixelRatio;
		const dy2 = y2 * pixelRatio;
		const dthickness = thickness * pixelRatio;
		const lineAngle = Math.atan2(dy2 - dy1, dx2 - dx1);
		const lineLength = Math.sqrt((dx2 - dx1)*(dx2 - dx1) + (dy2 - dy1)*(dy2 - dy1));
		if (lineLength <= 0) { return; }
		const forwardX = (dx2 - dx1) / lineLength;
		const forwardY = (dy2 - dy1) / lineLength;
		const normalX = -forwardY;
		const normalY = forwardX;
		// console.log("start: (" + x1 + ", " + y1 + ") end: (" + x2 + ", " + y2 + ") angle: " + lineAngle);
		
		canvasContext2d.beginPath();
		canvasContext2d.moveTo(dx1, dy1);
		canvasContext2d.lineTo(dx2, dy2);
		canvasContext2d.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
		canvasContext2d.lineWidth = thickness;
		canvasContext2d.stroke();
	},
};
const webglDrawingFunctions =
{
	glCreateShader:  (type) =>
	{
		let newShader = canvasContextGl.createShader(type);
		let result = glShaders.length;
		glShaders.push(newShader);
		// console.log("Created shader[" + result + "]");
		return result;
	},
	glCreateProgram: () =>
	{
		let newProgram = canvasContextGl.createProgram();
		let result = glPrograms.length;
		glPrograms.push(newProgram);
		// console.log("Created program[" + result + "]");
		return result;
	},
	glCreateTexture: () =>
	{
		let newTexture = canvasContextGl.createTexture(); 
		let result = glTextures.length;
		glTextures.push(newTexture);
		// console.log("Created texture[" + result + "]");
		return result;
	},
	// glDeleteBuffers: (count, buffers)                 => { canvasContextGl.deleteBuffers(count, buffers); },
	glCreateBuffer: () =>
	{
		let newBuffer = canvasContextGl.createBuffer();
		let result = glBuffers.length;
		glBuffers.push(newBuffer);
		// console.log("Created buffer[" + result + "]");
		return result;
	},
	glCreateVertexArray: () =>
	{
		let newVao = canvasContextGl.createVertexArray(); 
		let result = glVaos.length;
		glVaos.push(newVao);
		// console.log("Created vao[" + result + "]");
		return result;
	},
	glGetAttribLocation: (programIndex, namePntr) =>
	{
		let newLocation = canvasContextGl.getAttribLocation(glPrograms[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glAttribLocations.length;
		glAttribLocations.push(newLocation);
		return result;
	},
	glGetUniformLocation: (programIndex, namePntr) =>
	{
		let newLocation = canvasContextGl.getUniformLocation(glPrograms[programIndex], wasmPntrToJsString(wasmMemory, namePntr));
		if (newLocation === -1) { return -1; }
		let result = glUniformLocations.length;
		glUniformLocations.push(newLocation);
		return result;
	},
	
	glGetError:                ()                                              => { canvasContextGl.getError(); },
	glClearColor:              (r, g, b, a)                                    => { canvasContextGl.clearColor(r, g, b, a); },
	glClear:                   (mask)                                          => { canvasContextGl.clear(mask); },
	glViewport:                (x, y, width, height)                           => { canvasContextGl.viewport(x, y, width, height); },
	glEnable:                  (cap)                                           => { canvasContextGl.enable(cap); },
	glDisable:                 (cap)                                           => { canvasContextGl.disable(cap); },
	glBlendFunc:               (sfactor, dfactor)                              => { canvasContextGl.blendFunc(sfactor, dfactor); },
	glBlendFuncSeparate:       (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) => { canvasContextGl.blendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); },
	glDepthFunc:               (func)                                          => { canvasContextGl.depthFunc(func); },
	glFrontFace:               (mode)                                          => { canvasContextGl.frontFace(mode); },
	glLineWidth:               (width)                                         => { canvasContextGl.lineWidth(width); },
	glShaderSource:            (shaderIndex, count, stringPntr)                => { canvasContextGl.shaderSource(glShaders[shaderIndex], wasmPntrToJsString(wasmMemory, stringPntr)); },
	glCompileShader:           (shaderIndex)                                   => { canvasContextGl.compileShader(glShaders[shaderIndex]); },
	glAttachShader:            (programIndex, shaderIndex)                     => { canvasContextGl.attachShader(glPrograms[programIndex], glShaders[shaderIndex]); },
	glLinkProgram:             (programIndex)                                  => { canvasContextGl.linkProgram(glPrograms[programIndex]); },
	glUniform1i:               (locationIndex, v0)                             => { canvasContextGl.uniform1i(glUniformLocations[locationIndex], v0); },
	glUniform1f:               (locationIndex, v0)                             => { canvasContextGl.uniform1f(glUniformLocations[locationIndex], v0); },
	glUniform2f:               (locationIndex, v0, v1)                         => { canvasContextGl.uniform2f(glUniformLocations[locationIndex], v0, v1); },
	glUniform3f:               (locationIndex, v0, v1, v2)                     => { canvasContextGl.uniform3f(glUniformLocations[locationIndex], v0, v1, v2); },
	glUniform4f:               (locationIndex, v0, v1, v2, v3)                 => { canvasContextGl.uniform4f(glUniformLocations[locationIndex], v0, v1, v2, v3); },
	glUniformMatrix4fv:        (locationIndex, count, transpose, valuePntr)    => { canvasContextGl.uniformMatrix4fv(glUniformLocations[locationIndex], transpose, new Float32Array(wasmMemory.buffer, valuePntr, count * (4*4))); },
	glUseProgram:              (programIndex)                                  => { canvasContextGl.useProgram(glPrograms[programIndex]); },
	glBindTexture:             (target, textureIndex)                          => { canvasContextGl.bindTexture(target, glTextures[textureIndex]); },
	glTexImage2D:              (target, level, internalformat, width, height, border, format, type, pixelsPntr) => { canvasContextGl.texImage2D(target, level, internalformat, width, height, border, format, type, new Uint8Array(wasmMemory.buffer, pixelsPntr)); },
	glTexParameteri:           (target, pname, param)                          => { canvasContextGl.texParameteri(target, pname, param); },
	glGenerateMipmap:          (target)                                        => { canvasContextGl.generateMipmap(target); },
	glActiveTexture:           (texture)                                       => { canvasContextGl.activeTexture(texture); },
	glBindBuffer:              (target, bufferIndex)                           => { canvasContextGl.bindBuffer(target, glBuffers[bufferIndex]); },
	glBufferData:              (target, size, data, usage)                     => { canvasContextGl.bufferData(target, new Uint8Array(wasmMemory.buffer, data), usage, 0, size); },
	glBindVertexArray:         (vaoIndex)                                      => { canvasContextGl.bindVertexArray(glVaos[vaoIndex]); },
	glEnableVertexAttribArray: (index)                                         => { canvasContextGl.enableVertexAttribArray(glAttribLocations[index]); },
	glVertexAttribPointer:     (index, size, type, normalized, stride, offset) => { canvasContextGl.vertexAttribPointer(glAttribLocations[index], size, type, normalized, stride, offset); },
	glDrawArrays:              (mode, first, count)                            => { canvasContextGl.drawArrays(mode, first, count); },
};

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
async function initialize()
{
	console.log("Pig Engine javascript layer is booting up...\n");
	
	console.log("Allocating memory for WASM modules...");
	wasmMemory = new WebAssembly.Memory({ initial: INITIAL_PLAT_PAGE_COUNT });
	
	// +==============================+
	// |         Find Canvas          |
	// +==============================+
	canvas = document.getElementsByTagName("canvas")[0];
	canvasContainer = document.getElementById("canvas_container");
	console.assert(canvas != null, "Couldn't find canvas DOM element!");
	console.assert(canvasContainer != null, "Couldn't find canvas container DOM element!");
	if (UseSimpleRenderContext)
	{
		canvasContext2d = canvas.getContext("2d");
		if (canvasContext2d === null) { console.error("Unable to initialize 2D render context. Your browser or machine may not support it :("); return; }
	}
	else
	{
		canvasContextGl = canvas.getContext("webgl2");
		if (canvasContextGl === null) { console.error("Unable to initialize WebGL render context. Your browser or machine may not support it :("); return; }
		console.log(canvasContextGl);
	}
	pixelRatio = window.devicePixelRatio;
	
	// +==============================+
	// |    Load assembFuncs.wasm     |
	// +==============================+
	let assembEnvironment =
	{
		memory: wasmMemory,
	};
	let assembWasm = await loadWasmModule(assembFilePath, assembEnvironment);
	
	// +==============================+
	// |     Load PigEngine.wasm      |
	// +==============================+
	let engineEnvironment =
	{
		memory: wasmMemory,
		
		...assembWasm.exports,
		...intrinsicFunctions,
		...informationFunctions,
		...memoryFunctions,
		...consoleFunctions,
		...fileFunctions,
		...basic2dDrawingFunctions,
		...webglDrawingFunctions,
		
		js_TestFunc: () =>
		{
			//TODO: Put tests here
		},
	};
	engine = await loadWasmModule(engineFilePath, engineEnvironment);
	
	// assembWasm.exports.stack_init();
	// let intrinResult0 = assembWasm.exports.get_stack_base();
	// console.log("intrinResult0: 0x" + intrinResult0.toString(16) + " (" + intrinResult0.toString(10) + ")");
	// let intrinResult1 = assembWasm.exports.get_stack_end();
	// console.log("intrinResult1: 0x" + intrinResult1.toString(16) + " (" + intrinResult1.toString(10) + ")");
	
	//Run this once to fill the protected region with known values.
	//It will report corruption here but we throw away the return value since this is expected on the first call.
	// TODO: Re-enable this once it's fixed
	// checkPlatProtectedMemory(wasmMemory);
	
	stackBase = engine.exports.GetStackBase();
	// TODO: Remove 1024 once we have a more reliable way to find where the stack starts
	heapBase = stackBase + 1024; //1kB should account for any innacuracy in our crude method of calculating the stack base
	if ((heapBase % WASM_PAGE_SIZE) != 0)
	{
		heapBase += WASM_PAGE_SIZE - (heapBase % WASM_PAGE_SIZE); //align to a page barrier
	}
	console.log("memory", stackBase.toString(16), heapBase.toString(16), wasmMemory.buffer.byteLength.toString(16));
	
	engine.exports.Initialize(heapBase);
	
	// engine.exports.TestFunc(10); //TODO: Remove me!
	
	window.addEventListener('mousemove', evt => {
		// console.log("mousemove", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		engine.exports.MouseMoved(evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	window.addEventListener('mousedown', evt => {
		// console.log("mousedown", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		engine.exports.MouseBtnChanged(evt.button, true, evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	window.addEventListener('mouseup', evt => {
		// console.log("mouseup", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		engine.exports.MouseBtnChanged(evt.button, false, evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	
	function renderFrame()
	{
		engine.exports.RenderFrame(canvas.width / pixelRatio, canvas.height / pixelRatio);
		window.requestAnimationFrame(renderFrame);
		//TODO: Re-enable this once it's fixed
		// console.assert(checkPlatProtectedMemory(wasmMemory), "WASM memory corruption detected! The first 1kB section was written to! Check that the stack isn't overflowing and your pointers aren't corrupted!");
	}
	
	window.requestAnimationFrame(renderFrame);
}

initialize();
