// ========================== Start of web_globals.js ==========================
/*
File:   web_js_funcs.js
Author: Taylor Robbins
Date:   10\17\2023
Description:
	** Since calling from wasm to javascript doesn't implicitly pass any context,
	** we need a bunch of things to be "global" in the javascript code so we can
	** access things that the javascript side is managing.
*/

var globals =
{
	wasmMemory: null,
	wasmModule: null,
	
	canvas: null,
	glContext: null,
	pixelRatio: null,
};

// =========================== End of web_globals.js ===========================