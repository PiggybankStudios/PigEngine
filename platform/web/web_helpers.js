// ========================== Start of web_helpers.js ==========================
/*
File:   web_helpers.js
Author: Taylor Robbins
Date:   10\17\2023
Description:
	** Holds a bunch of functions that help us implement stuff in web_js_funcs.js
	** but don't get exposed directly to the WASM module
*/

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