// =========================== Start of web_main.js ============================
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

function jsPrintCallStack(labelStrPntr)
{
	let labelStr = wasmPntrToJsString(stdGlobals.wasmMemory, labelStrPntr);
	console.log(labelStr + " (Stack Trace):");
	console.trace();
}

function jsConsoleWriteLine(level, messagePntr)
{
	let messageStr = wasmPntrToJsString(stdGlobals.wasmMemory, messagePntr);
	if      (level == 0) { console.debug("%c" + messageStr, "color: #AFAFA2;"); } //debug (MonokaiGray1)
	else if (level == 1) { console.log(messageStr);                             } //regular/none
	else if (level == 2) { console.info("%c" + messageStr, "color: #A6E22E;");  } //info (MonokaiGreen)
	else if (level == 3) { console.info("%c" + messageStr, "color: #AE81FF;");  } //notify (MonokaiPurple)
	else if (level == 4) { console.info("%c" + messageStr, "color: #66D9EF;");  } //other (MonokaiBlue)
	else if (level == 5) { console.warn(messageStr);                            } //warning
	else if (level == 6) { console.error(messageStr);                           } //error
	else { console.log(messageStr); }
}

function jsGetTime()
{
	return new Date().getTime();
}

appApiFuncs = {
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
	jsPrintCallStack: jsPrintCallStack,
	jsConsoleWriteLine: jsConsoleWriteLine,
	jsGetTime: jsGetTime,
	...webglDrawingFunctions,
};

async function StartMainLoop()
{
	// console.log("StartMainLoop...");
	canvas = PigWasm_AcquireCanvas(800, 600);
	glContext = PigWasm_CreateGlContext(canvas);
	
	// console.log("Calling init...");
	initialWasmPageCount = 2;
	wasmMemory = PigWasm_InitMemory(initialWasmPageCount);
	wasmModule = await PigWasm_Init(
		wasmMemory,
		initialWasmPageCount,
		"PigEngineTest.wasm",
		appApiFuncs
	);
	
	// console.log("Getting time...");
	let initializeTimestamp = Math.floor(Date.now() / 1000); //TODO: Should we be worried about this being a 32-bit float?
	// console.log("Calling WasmInitialize...");
	// console.log("wasmMemory.buffer.byteLength after WasmInitialize:", wasmMemory.buffer.byteLength.toString(16));
	wasmModule.exports.WasmInitialize(initializeTimestamp);
	// console.log("wasmMemory.buffer.byteLength after WasmInitialize:", wasmMemory.buffer.byteLength.toString(16));
	
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
	
	// function UpdateAndRenderCallback()
	// {
	// 	wasmModule.exports.WasmUpdateAndRender(); //TODO: Measure elapsed time!
	// 	window.requestAnimationFrame(UpdateAndRenderCallback);
	// }
	// wasmModule.exports.WasmUpdateAndRender();
	// window.requestAnimationFrame(UpdateAndRenderCallback);
	// console.log("StartMainLoop Done!");
}

StartMainLoop();

// ============================ End of web_main.js =============================