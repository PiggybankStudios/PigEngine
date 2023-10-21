// =========================== Start of web_main.js ============================
/*
File:   web_main.js
Author: Taylor Robbins
Date:   10\15\2023
Description:
	** Contains the asynchronous StartMainLoop function (and a call to it) that
	** orchestrates loading the WASM module and calling WasmInitialize.
	** Then, assuming the module called jsInitRendering, we kick off a series of
	** window.requestAnimationFrame calls which ultimately call WasmUpdateAndRender.
	** Everything else is handled by js* functions in web_js_funcs.js and jsGl*
	** functions in web_gl_api.js.
*/

appApiFuncs =
{
	...webJsFuncs,
	...webglDrawingFunctions,
};

async function StartMainLoop()
{
	// console.log("Calling PigWasm_Init...");
	initialWasmPageCount = 2;
	globals.wasmMemory = PigWasm_InitMemory(initialWasmPageCount);
	globals.wasmModule = await PigWasm_Init(
		globals.wasmMemory,
		initialWasmPageCount,
		"PigEngineTest.wasm",
		appApiFuncs
	);
	
	// console.log("Calling WasmInitialize...");
	let initializeTimestamp = Math.floor(Date.now() / 1000); //TODO: Should we be worried about this being a 32-bit float?
	globals.wasmModule.exports.WasmInitialize(initializeTimestamp);
	// console.log("wasmModule:", globals.wasmModule);
	
	if (globals.canvas != null)
	{
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
		console.log("Render loop has begun!");
	}
	else
	{
		console.error("The WasmModule failed to call jsInitRendering during WasmInitialize!");
	}
}

StartMainLoop();

// ============================ End of web_main.js =============================