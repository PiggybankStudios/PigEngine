// =========================== Start of web_main.js ============================
/*
File:   web_main.js
Author: Taylor Robbins
Date:   10\15\2023
Description:
	** Contains the main client-side code for the web platform layer of Pig Engine
*/

appApiFuncs =
{
	...webJsFuncs,
	...webglDrawingFunctions,
};

async function StartMainLoop()
{
	// console.log("StartMainLoop...");
	
	// console.log("Calling PigWasm_Init...");
	initialWasmPageCount = 2;
	globals.wasmMemory = PigWasm_InitMemory(initialWasmPageCount);
	globals.wasmModule = await PigWasm_Init(
		globals.wasmMemory,
		initialWasmPageCount,
		"PigEngineTest.wasm",
		appApiFuncs
	);
	
	// console.log("Getting time...");
	let initializeTimestamp = Math.floor(Date.now() / 1000); //TODO: Should we be worried about this being a 32-bit float?
	// console.log("Calling WasmInitialize...");
	// console.log("wasmMemory.buffer.byteLength after WasmInitialize:", globals.wasmMemory.buffer.byteLength.toString(16));
	globals.wasmModule.exports.WasmInitialize(initializeTimestamp);
	// console.log("wasmMemory.buffer.byteLength after WasmInitialize:", globals.wasmMemory.buffer.byteLength.toString(16));
	
	if (globals.canvas == null)
	{
		console.error("The WasmModule failed to call jsStartRendering during WasmInitialize!");
	}
	
	console.log("StartMainLoop Done!");
}

StartMainLoop();

// ============================ End of web_main.js =============================