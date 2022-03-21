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
const WASM_PAGE_SIZE = (64 * 1024); //64kB
const WASM_PROTECTED_SIZE = 1024 //1kB at start of wasm memory should be unused and should never be written to

const assembFilePath = "assembFuncs.wasm";
const wasmFilePath = "PigParts.wasm";

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

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
async function initialize()
{
	console.log("Pig Engine javascript layer is booting up...\n");
	
	console.log("Allocating memory for WASM modules...");
	const platWasmInitialPageCount = 7; //Each page is 64kb
	const platWasmMemory = new WebAssembly.Memory({ initial: platWasmInitialPageCount });
	var platStackBase = 0; //TODO: This is filled with a crude calculation. Can we get the exact address somehow?
	var platHeapBase = 0;
	
	// +==============================+
	// |         Find Canvas          |
	// +==============================+
	let canvas = document.getElementsByTagName("canvas")[0];
	let canvasContainer = document.getElementById("canvas_container");
	console.assert(canvas != null, "Couldn't find canvas DOM element!");
	console.assert(canvasContainer != null, "Couldn't find canvas container DOM element!");
	let canvasContext = canvas.getContext("2d");
	const pixelRatio = window.devicePixelRatio;
	
	// +==============================+
	// |    Load assembFuncs.wasm     |
	// +==============================+
	let assembEnvironment =
	{
		memory: platWasmMemory,
	};
	
	console.log("Loading " + assembFilePath + "...");
	let assembWasm = null;
	try
	{
		const assembWasmPromise = fetch(assembFilePath);
		const assembWasmModule = await WebAssembly.instantiateStreaming(
			assembWasmPromise,
			{ env: assembEnvironment }
		);
		assembWasm = assembWasmModule.instance;
	}
	catch (exception)
	{
		console.log("Failed to load assembly functions WASM file!\n", exception);
		return;
	}
	console.log("Done loading assembly functions " + assembFilePath + "!");
	console.log("Loaded assembly functions:", assembWasm.exports);
	
	// +==============================+
	// |         Environment          |
	// +==============================+
	let platEnvironment =
	{
		memory: platWasmMemory,
		
		...assembWasm.exports,
		
		fminf:  (value1, value2) => { return Math.min(value1, value2); },
		fmin:   (value1, value2) => { return Math.min(value1, value2); },
		fmaxf:  (value1, value2) => { return Math.max(value1, value2); },
		fmax:   (value1, value2) => { return Math.max(value1, value2); },
		fmodf:  (numer, denom)   => { return (numer % denom);          },
		fmod:   (numer, denom)   => { return (numer % denom);          },
		roundf: (value)          => { return Math.round(value);        },
		round:  (value)          => { return Math.round(value);        },
		sinf:   (value)          => { return Math.sin(value);          },
		sin:    (value)          => { return Math.sin(value);          },
		asinf:  (value)          => { return Math.asin(value);         },
		asin:   (value)          => { return Math.asin(value);         },
		cosf:   (value)          => { return Math.cos(value);          },
		cos:    (value)          => { return Math.cos(value);          },
		acosf:  (value)          => { return Math.acos(value);         },
		acos:   (value)          => { return Math.acos(value);         },
		tanf:   (value)          => { return Math.tan(value);          },
		tan:    (value)          => { return Math.tan(value);          },
		atanf:  (value)          => { return Math.atan(value);         },
		atan:   (value)          => { return Math.atan(value);         },
		atan2f: (numer, denom)   => { return Math.atan2(numer, denom); },
		atan2:  (numer, denom)   => { return Math.atan2(numer, denom); },
		powf:   (value, power)   => { return Math.pow(value, power);   },
		pow:    (value, power)   => { return Math.pow(value, power);   },
		cbrtf:  (value)          => { return Math.cbrt(value);         },
		cbrt:   (value)          => { return Math.cbrt(value);         },
		
		js_GetHeapSize: () => //returns bytes
		{
			return platWasmMemory.buffer.byteLength - platHeapBase;
		},
		js_GrowHeap: (numPages) => //takes pages, not bytes
		{
			platWasmMemory.grow(numPages);
			console.log("Growing wasm memory by " + numPages + " page" + (numPages == 1 ? "" : "s") + " (total size: " + platWasmMemory.buffer.byteLength.toString(16) + ")");
		},
		
		js_TestFunc: () =>
		{
			//TODO: Put tests here
			const currentTime = new Date().getTime();
			console.log(typeof(currentTime), currentTime);
		},
		
		js_PrintNumber:   (number) => console.log("Number: 0x" + number.toString(16) + " (" + number.toString(10) + ")"),
		js_ConsoleDebug:  ptr => console.debug("%c" + wasmPntrToJsString(platWasmMemory, ptr), "color: #AFAFA2;"), //MonokaiGray1
		js_ConsoleLog:    ptr => console.log(wasmPntrToJsString(platWasmMemory, ptr)),
		js_ConsoleInfo:   ptr => console.info("%c" + wasmPntrToJsString(platWasmMemory, ptr), "color: #A6E22E;"), //MonokaiGreen
		js_ConsoleNotify: ptr => console.info("%c" + wasmPntrToJsString(platWasmMemory, ptr), "color: #AE81FF;"), //MonokaiPurple
		js_ConsoleOther:  ptr => console.info("%c" + wasmPntrToJsString(platWasmMemory, ptr), "color: #66D9EF;"), //MonokaiBlue
		js_ConsoleWarn:   ptr => console.warn(wasmPntrToJsString(platWasmMemory, ptr)),
		js_ConsoleError:  ptr => console.error(wasmPntrToJsString(platWasmMemory, ptr)),
		
		js_ClearCanvas:   () => canvasContext.clearRect(0, 0, canvas.width, canvas.height),
		
		js_DrawRectangle: (x, y, width, height, red, green, blue, alpha) =>
		{
			const dx = x * pixelRatio;
			const dy = y * pixelRatio;
			const dwidth = width * pixelRatio; // device inner width
			const dheight = height * pixelRatio; // device inner height
			canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
			canvasContext.fillRect(dx, dy, dwidth, dheight);
		},
		
		js_DrawRoundedRectangle: (x, y, width, height, radius, red, green, blue, alpha) =>
		{
			radius = Math.min(radius, width / 2, height / 2);
			
			const dx = x * pixelRatio;
			const dy = y * pixelRatio;
			const dradius = radius * pixelRatio;
			
			const diwidth = (width - (2 * radius)) * pixelRatio; // device inner width
			const diheight = (height - (2 * radius)) * pixelRatio; // device inner height
			
			canvasContext.beginPath();
			canvasContext.moveTo(dx + dradius, dy);
			canvasContext.lineTo(dx + dradius + diwidth, dy);
			canvasContext.arc(dx + dradius + diwidth, dy + dradius, dradius, -Math.PI/2, 0);
			canvasContext.lineTo(dx + dradius + diwidth + dradius, dy + dradius + diheight);
			canvasContext.arc(dx + dradius + diwidth, dy + dradius + diheight, dradius, 0, Math.PI/2);
			canvasContext.lineTo(dx + dradius, dy + dradius + diheight + dradius);
			canvasContext.arc(dx + dradius, dy + dradius + diheight, dradius, Math.PI/2, Math.PI);
			canvasContext.lineTo(dx, dy + dradius);
			canvasContext.arc(dx + dradius, dy + dradius, dradius, Math.PI, (3*Math.PI)/2);
			
			canvasContext.fillStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
			canvasContext.fill();
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
			
			canvasContext.beginPath();
			canvasContext.moveTo(dx1, dy1);
			canvasContext.lineTo(dx2, dy2);
			canvasContext.strokeStyle = `rgba(${red}, ${green}, ${blue}, ${alpha/255})`;
			canvasContext.lineWidth = thickness;
			canvasContext.stroke();
		},
	};
	
	// +==============================+
	// |      Load PigParts.wasm      |
	// +==============================+
	console.log("Loading " + wasmFilePath + "...");
	let platWasm = null;
	try
	{
		const platWasmPromise = fetch(wasmFilePath);
		const platWasmModule = await WebAssembly.instantiateStreaming(
			platWasmPromise,
			{ env: platEnvironment }
		);
		platWasm = platWasmModule.instance;
	}
	catch (exception)
	{
		console.log("Failed to load WASM file!\n", exception);
		return;
	}
	console.log("Done loading " + wasmFilePath + "!");
	console.log("Loaded exports:", platWasm.exports);
	
	// assembWasm.exports.stack_init();
	// let intrinResult0 = assembWasm.exports.get_stack_base();
	// console.log("intrinResult0: 0x" + intrinResult0.toString(16) + " (" + intrinResult0.toString(10) + ")");
	// let intrinResult1 = assembWasm.exports.get_stack_end();
	// console.log("intrinResult1: 0x" + intrinResult1.toString(16) + " (" + intrinResult1.toString(10) + ")");
	
	//Run this once to fill the protected region with known values.
	//It will report corruption here but we throw away the return value since this is expected on the first call.
	// TODO: Re-enable this once it's fixed
	// checkPlatProtectedMemory(platWasmMemory);
	
	platStackBase = platWasm.exports.GetStackBase();
	// TODO: Remove 1024 once we have a more reliable way to find where the stack starts
	platHeapBase = platStackBase + 1024; //1kB should account for any innacuracy in our crude method of calculating the stack base
	if ((platHeapBase % WASM_PAGE_SIZE) != 0)
	{
		platHeapBase += WASM_PAGE_SIZE - (platHeapBase % WASM_PAGE_SIZE); //align to a page barrier
	}
	console.log("memory", platStackBase.toString(16), platHeapBase.toString(16), platWasmMemory.buffer.byteLength.toString(16));
	
	platWasm.exports.Initialize(platHeapBase);
	platWasm.exports.TestFunc(0.1);
	platWasm.exports.TestFunc(1.0);
	platWasm.exports.TestFunc(new Date().getTime());
	platWasm.exports.TestFunc(9007199254740991);
	platWasm.exports.TestFunc(9007199254740992);
	platWasm.exports.TestFunc(9007199254740993);
	platWasm.exports.TestFunc(9007199254740994);
	
	window.addEventListener('mousemove', evt => {
		// console.log("mousemove", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		platWasm.exports.MouseMoved(evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	window.addEventListener('mousedown', evt => {
		// console.log("mousedown", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		platWasm.exports.MouseBtnChanged(evt.button, true, evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	window.addEventListener('mouseup', evt => {
		// console.log("mouseup", evt);
		const containerRect = canvasContainer.getBoundingClientRect();
		containerRect.x += 1; containerRect.y += 1; //this is necassary because of the border around the canvas right now
		platWasm.exports.MouseBtnChanged(evt.button, false, evt.clientX - containerRect.x, evt.clientY - containerRect.y);
	});
	
	function renderFrame()
	{
		platWasm.exports.RenderFrame(canvas.width / pixelRatio, canvas.height / pixelRatio);
		window.requestAnimationFrame(renderFrame);
		//TODO: Re-enable this once it's fixed
		// console.assert(checkPlatProtectedMemory(platWasmMemory), "WASM memory corruption detected! The first 1kB section was written to! Check that the stack isn't overflowing and your pointers aren't corrupted!");
	}
	
	window.requestAnimationFrame(renderFrame);
}

initialize();
