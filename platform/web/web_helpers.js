// ========================== Start of web_helpers.js ==========================
/*
File:   web_helpers.js
Author: Taylor Robbins
Date:   10\17\2023
Description:
	** Holds a bunch of functions that help us implement stuff in web_js_funcs.js
	** but don't get exposed directly to the WASM module
*/

// +--------------------------------------------------------------+
// |                 String Marshalling Functions                 |
// +--------------------------------------------------------------+
function StrPntrToJsStr(strPntr)
{
	const codes = [];
	const wasmMemBuffer = new Uint8Array(globals.wasmMemory.buffer).subarray(strPntr);
	
	let cIndex = 0;
	while (true)
	{
		const char = wasmMemBuffer[cIndex];
		if (char == 0) { break; }
		codes.push(char);
		cIndex++;
	}
	
	//TODO: Can we do something else? If we do our own UTF-8 parsing maybe?
	return String.fromCharCode(...codes);
}
const encoder = new TextEncoder("utf8");
function JsStrToStrPntr(memArenaPntr, jsString)
{
	let encodedStr = encoder.encode(jsString + "\0");
	let allocSize = encodedStr.length;
	let resultPntr = globals.wasmModule.exports.WasmAllocMem(memArenaPntr, allocSize);
	if (resultPntr != 0)
	{
		var wasmMemBuffer = new Uint8Array(globals.wasmMemory.buffer).subarray(resultPntr);
		wasmMemBuffer.set(encodedStr);
	}
	return resultPntr;
}
function FreeStrPntr(memArenaPntr, strPntr)
{
	//TODO: We should probably calculate the size of the string by looping until we find a null-term char in the wasm memory
	// Freeing the string doesn't REQUIRE we know the length, but it can help with bookkeeping in some kinds of MemArenas
	globals.wasmModule.exports.WasmFreeMem(memArenaPntr, strPntr, 0);
}

// "https://stackoverflow.com/questions/22733685/how-to-download-files-using-javascript-asynchronously"
// Download a file form a url.
function saveFile(url)
{
	return new Promise(function(resolve, reject) {
		// Get file name from url.
		var xhr = new XMLHttpRequest();
		xhr.responseType = 'blob';
		xhr.onload = function() {
			resolve(xhr);
		};
		xhr.onerror = reject;
		xhr.open('GET', url);
		xhr.send();
	}).then(function(xhr) {
		var filename = url.substring(url.lastIndexOf("/") + 1).split("?")[0];
		var a = document.createElement('a');
		a.href = window.URL.createObjectURL(xhr.response); // xhr.response is a blob
		a.download = filename; // Set the file name.
		a.style.display = 'none';
		document.body.appendChild(a);
		a.click();
		return xhr;
	});
}

function download(urls)
{
	return Promise.all(urls.map(saveFile));
}

// =========================== End of web_helpers.js ===========================