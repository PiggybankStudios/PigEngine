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
	let labelStr = StrPntrToJsStr(labelStrPntr);
	console.log(labelStr + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = StrPntrToJsStr(labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(labelStrPntr, strPntr)
{
	let labelStr = StrPntrToJsStr(labelStrPntr);
	let str = StrPntrToJsStr(strPntr);
	console.log(labelStr + ": " + str);
}

function jsPrintCallStack(labelStrPntr)
{
	let labelStr = StrPntrToJsStr(labelStrPntr);
	console.log(labelStr + " (Stack Trace):");
	console.trace();
}

function jsConsoleWriteLine(level, messagePntr)
{
	let messageStr = StrPntrToJsStr(messagePntr);
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

function jsInitRendering(canvasWidth, canvasHeight)
{
	console.assert(globals.canvas == null);
	globals.canvas = PigWasm_AcquireCanvas(canvasWidth, canvasHeight);
	console.assert(globals.canvas != null);
	globals.glContext = PigWasm_CreateGlContext(globals.canvas);
	console.assert(globals.glContext != null);
	globals.pixelRatio = window.devicePixelRatio;
}

function jsDownloadFile(urlPntr, memArenaPntr, callbackPntr)
{
	let url = StrPntrToJsStr(urlPntr);
	fetch(url, { cache: "no-cache" })
	.then(data => data.blob())
	.then(blob => blob.arrayBuffer())
	.then(resultBuffer =>
	{
		// console.log(resultBuffer);
		let bufferU8 = new Uint8Array(resultBuffer);
		let spacePntr = globals.wasmModule.exports.WasmAllocMem(memArenaPntr, resultBuffer.byteLength);
		// console.log("Allocated at " + spacePntr);
		let buf = new Uint8Array(wasmMemory.buffer);
		for (let bIndex = 0; bIndex < resultBuffer.byteLength; bIndex++)
		{
			buf[spacePntr + bIndex] = bufferU8[bIndex];
		}
		globals.wasmModule.exports.WasmFileFinishedDownload(urlPntr, resultBuffer.byteLength, spacePntr, callbackPntr);
		globals.wasmModule.exports.WasmFreeMem(memArenaPntr, spacePntr, resultBuffer.byteLength);
	});
}

function jsGetString(memArenaPntr)
{
	return JsStrToStrPntr(memArenaPntr, "Hello from Javascript!");
}

webJsFuncs =
{
	jsPrintInteger: jsPrintInteger,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
	jsPrintCallStack: jsPrintCallStack,
	jsConsoleWriteLine: jsConsoleWriteLine,
	jsGetTime: jsGetTime,
	jsInitRendering: jsInitRendering,
	jsDownloadFile: jsDownloadFile,
	jsGetString: jsGetString,
};

// ========================== End of web_js_funcs.js ===========================