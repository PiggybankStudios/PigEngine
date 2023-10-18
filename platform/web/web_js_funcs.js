// ========================= Start of web_js_funcs.js ==========================
/*
File:   web_js_funcs.js
Author: Taylor Robbins
Date:   10\17\2023
Description:
	** Holds all the other functions that are provided for the WASM module from the web platform layer of Pig Engine
*/

function jsPrintInteger(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(globals.wasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(globals.wasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(labelStrPntr, strPntr)
{
	let labelStr = wasmPntrToJsString(globals.wasmMemory, labelStrPntr);
	let str = wasmPntrToJsString(globals.wasmMemory, strPntr);
	console.log(labelStr + ": " + str);
}

function jsPrintCallStack(labelStrPntr)
{
	let labelStr = wasmPntrToJsString(globals.wasmMemory, labelStrPntr);
	console.log(labelStr + " (Stack Trace):");
	console.trace();
}

function jsConsoleWriteLine(level, messagePntr)
{
	let messageStr = wasmPntrToJsString(globals.wasmMemory, messagePntr);
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

function jsStartRendering(canvasWidth, canvasHeight)
{
	console.assert(globals.canvas == null);
	globals.canvas = PigWasm_AcquireCanvas(canvasWidth, canvasHeight);
	console.assert(globals.canvas != null);
	globals.glContext = PigWasm_CreateGlContext(globals.canvas);
	console.assert(globals.glContext != null);
	globals.pixelRatio = window.devicePixelRatio;
	
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
	// 	globals.wasmModule.exports.HandleKeyPressOrRelease(key, true);
	// });
	// window.addEventListener("keyup", function(event)
	// {
	// 	let key = KeyDownEventStrToKeyEnum(event.code);
	// 	globals.wasmModule.exports.HandleKeyPressOrRelease(key, false);
	// });
	// window.addEventListener("mousedown", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	globals.wasmModule.exports.HandleMousePressOrRelease(mouseBtn, true);
	// });
	// window.addEventListener("mouseup", function(event)
	// {
	// 	let mouseBtn = MouseDownEventNumToBtnEnum(event.button);
	// 	globals.wasmModule.exports.HandleMousePressOrRelease(mouseBtn, false);
	// });
	
	function UpdateAndRenderCallback()
	{
		globals.wasmModule.exports.WasmUpdateAndRender(); //TODO: Measure elapsed time!
		window.requestAnimationFrame(UpdateAndRenderCallback);
	}
	globals.wasmModule.exports.WasmUpdateAndRender();
	window.requestAnimationFrame(UpdateAndRenderCallback);
}

webJsFuncs =
{
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
	jsPrintCallStack: jsPrintCallStack,
	jsConsoleWriteLine: jsConsoleWriteLine,
	jsGetTime: jsGetTime,
	jsStartRendering: jsStartRendering,
};

// ========================== End of web_js_funcs.js ===========================