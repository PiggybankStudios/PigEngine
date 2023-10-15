// ============================= Start of web_main.js =============================
/*
File:   web_main.js
Author: Taylor Robbins
Date:   10\15\2023
Description:
	** Contains the main client-side code for the web platform layer of Pig Engine
*/

function jsPrintInteger(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(stdGlobals.wasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(stdGlobals.wasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(labelStrPntr, strPntr)
{
	let labelStr = wasmPntrToJsString(stdGlobals.wasmMemory, labelStrPntr);
	let str = wasmPntrToJsString(stdGlobals.wasmMemory, strPntr);
	console.log(labelStr + ": " + str);
}

appApiFuncs = {
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
};

async function MainLoop()
{
	// console.log("MainLoop start...");
	canvas = PigWasm_AcquireCanvas(800, 600);
	glContext = PigWasm_CreateGlContext(canvas);
	
	// console.log("Calling init...");
	initialWasmPageCount = 64;
	wasmMemory = PigWasm_InitMemory(initialWasmPageCount);
	wasmModule = await PigWasm_Init(
		wasmMemory,
		initialWasmPageCount,
		"PigEngineTest.wasm",
		appApiFuncs
	);
	
	// console.log("Getting time...");
	let initializeTimestamp = Math.floor(Date.now() / 1000); //TODO: Should we be worried about this being a 32-bit float?
	// console.log("Calling Initialize...");
	wasmModule.exports.Initialize(initializeTimestamp);
	
	// window.addEventListener("mousemove", function(event)
	// {
	// 	let clientBounds = canvas.getBoundingClientRect();
	// 	let pixelRatio = window.devicePixelRatio;
	// 	mousePositionX = Math.round(event.clientX - clientBounds.left) * pixelRatio;
	// 	mousePositionY = Math.round(event.clientY - clientBounds.top) * pixelRatio;
	// });
	// window.addEventListener("keydown", function(event)
	// {
	// 	let key = KeyDownEventStrToKeyEnum(event.code);
	// 	wasmModule.exports.HandleKeyPressOrRelease(key, true);
	// });
	// window.addEventListener("keyup", function(event)
	// {
	// 	let key = KeyDownEventStrToKeyEnum(event.code);
	// 	wasmModule.exports.HandleKeyPressOrRelease(key, false);
	// });
	// window.addEventListener("mousedown", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	wasmModule.exports.HandleMousePressOrRelease(mouseBtn, true);
	// });
	// window.addEventListener("mouseup", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	wasmModule.exports.HandleMousePressOrRelease(mouseBtn, false);
	// });
	
	// wasmModule.exports.UpdateAndRender(0);
	
	// function renderFrame()
	// {
	// 	wasmModule.exports.UpdateAndRender(16.6666);
	// 	window.requestAnimationFrame(renderFrame);
	// }
	// window.requestAnimationFrame(renderFrame);
	// console.log("MainLoop Done!");
}

MainLoop();

// ============================== End of main.js ==============================