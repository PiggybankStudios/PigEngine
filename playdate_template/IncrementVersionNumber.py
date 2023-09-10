#!/usr/bin/python

import os.path
import sys
import re

filePath = sys.argv[1];
fileBase, fileName = os.path.split(filePath)
# print("Editing file \"" + filePath + "\"");

versionFile = open(filePath, "r");

fileContents = versionFile.read();
# print("File Contents: \"" + str(fileContents) + "\"");
versionFile.close();

searchStr = "\\#define\\s*[A-Z0-9\\_]*VERSION\\_BUILD\\s*([0-9]+)[^\\n]*";
foundVersionLine = -1;
newVersionLine = "";

searchResult = re.search(searchStr, fileContents);

if (searchResult == None or len(searchResult.groups()) < 1):
	print("Could not find version line in version file!");
else:
	intValue = int(fileContents[searchResult.start(1):searchResult.end(1)]);
	# print("intValue: " + str(intValue));
	
	newVersion = intValue + 1;
	
	fileContents = fileContents[:searchResult.start(1)] + str(newVersion) + fileContents[searchResult.end(1):]
	
	# print("New File Contents: \"" + str(fileContents) + "\"");
	
	print("[" + fileName + ": Build " + str(newVersion) + "]")
	
	versionFile = open(filePath, "w");

	versionFile.write(fileContents);

	versionFile.close();