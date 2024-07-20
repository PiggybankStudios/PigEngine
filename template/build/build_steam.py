# File:   package_game.py
# Author: Taylor Robbins
# Date:   03\24\2020
# Description: 
# 	** Handles copying the game files to a win_developer, win_release or win_demo folder and calling the steam command
#   ** line tool to upload through Steam Pipe and create a build with the necassary depots on Steamworks
#   ** This script also handles creating a zip file with the build contents of the demo for uploading to Itch.io

import os, sys, argparse, shutil, math, re, getpass;

# +--------------------------------------------------------------+
# |                       Helper Functions                       |
# +--------------------------------------------------------------+
def FormatSize(sizeInBytes):
#
	remainder = sizeInBytes;
	kilo = math.floor(sizeInBytes / 1024);
	mega = math.floor(kilo / 1024);
	giga = math.floor(mega / 1024);
	kilo = (kilo % 1024);
	mega = (mega % 1024);
	remainder = (remainder % 1024);
	if (kilo > 0):
	#
		if (mega > 0):
		#
			if (giga > 0): return ("%dGb %dMb %dkb %db" % (giga, mega, kilo, remainder));
			else: return ("%dMb %dkb %db" % (mega, kilo, remainder));
		#
		else: return ("%dkb %db" % (kilo, remainder));
	#
	else: return ("%db" % (remainder));
#
def DoesPathMatchPattern(filePath, pattern):
#
	if (filePath.lower() == pattern.lower()): return True;
	if (len(pattern) > 1 and pattern[0] == "."):
	#
		extLength = len(pattern);
		if (len(filePath) > extLength and filePath[-extLength:].lower() == pattern.lower()): return True;
	#
	if (len(pattern) > 1 and pattern[-1] == "/"):
	#
		fileFolder = filePath;
		lastSlash = fileFolder.rfind("/");
		if (lastSlash >= 0): fileFolder = fileFolder[0:lastSlash+1];
		if (fileFolder.lower() == pattern.lower()): return True;
	#
	return False;
#
def ShouldFileBeIncluded(filePath, whiteList, blackList):
#
	result = False;
	for whiteListEntry in whiteList:
	#
		if (DoesPathMatchPattern(filePath, whiteListEntry)):
		#
			result = True;
			break;
		#
	#
	for blackListEntry in blackList:
	#
		if (DoesPathMatchPattern(filePath, blackListEntry)):
		#
			result = False;
			break;
		#
	#
	return result;
#
totalOutputSize = 0;
numResourceFiles = 0;
totalResourcesSize = 0;
applicationSize = 0;
numLevelFiles = 0;
def CopyFilesRecursive(srcFolder, srcRelFolder, destFolder, whiteList, blackList):
#
	numFilesCopied = 0;
	for fileName in os.listdir(srcFolder):
	#
		relFilePath = srcRelFolder + fileName;
		srcFilePath = srcFolder + fileName;
		destFilePath = destFolder + relFilePath;
		if (os.path.isdir(srcFilePath)):
		#
			folderName = fileName + "/";
			if (folderName[0] == "."): continue; #Skips .git, .., and other similar hidden folders
			# print("Recursing into \"" + srcFolder + folderName + "\"");
			numFilesCopied += CopyFilesRecursive(srcFolder + folderName, srcRelFolder + folderName, destFolder, whiteList, blackList);
		#
		elif (os.path.isfile(srcFilePath)):
		#
			if (ShouldFileBeIncluded(relFilePath, whiteList, blackList)):
			#
				isResourceFile = (relFilePath.lower().find("resources/") >= 0);
				isAppFile = (relFilePath.lower() == "pigparts.exe" or relFilePath.lower() == "pigparts.dll");
				# print("Copying \"" + relFilePath + "\" to output");
				destFolderPath = os.path.dirname(destFilePath);
				if (not os.path.exists(destFolderPath)):
				#
					os.makedirs(destFolderPath);
				#
				shutil.copyfile(srcFilePath, destFilePath);
				fileSize = os.path.getsize(srcFilePath);
				global totalOutputSize;
				totalOutputSize += fileSize;
				if (isResourceFile):
				#
					global numResourceFiles;
					global totalResourcesSize;
					numResourceFiles += 1;
					totalResourcesSize += fileSize;
				#
				if (isAppFile): 
				#
					global applicationSize;
					applicationSize += fileSize;
				#
				numFilesCopied += 1;
			#
			else:
			#
				# print("Not copying \"" + relFilePath + "\" to output");
				pass;
			#
		#
	#
	return numFilesCopied;
#
def ReplaceRegexMatchedSection(fileContent, regularExpression, replaceStr):
#
	if (fileContent == False): return False;
	searchResult = re.search(regularExpression, fileContent);
	if (searchResult == None or len(searchResult.groups()) < 1):
	#
		return False;
	#
	return fileContent[0:searchResult.start(1)] + replaceStr + fileContent[searchResult.end(1):];
#

# +--------------------------------------------------------------+
# |                       Parse Arguments                        |
# +--------------------------------------------------------------+
parser = argparse.ArgumentParser(description="Helps us upload builds to Steam");
parser.add_argument("-t", "--test", action="count", help="A test option");
parser.add_argument("-u", "--username", action="store", help="The username to use when logging in to Steam");
parser.add_argument("type", help="The type of build to do. Should be \"developer\", \"release\", or \"demo\"");
args = parser.parse_args();

buildType = args.type;
if (buildType != "developer" and buildType != "release" and buildType != "demo"):
#
	print("Uknown build type \"" + buildType + "\". Should be \"developer\", \"release\", or \"demo\".");
	exit(1);
#

# print(args);

fileFolder = os.path.dirname(os.path.realpath(__file__));
os.chdir(fileFolder);
workingDir = os.getcwd();
print("Runnning from \"" + workingDir + "\"");

onlyTest = (args.test != None and args.test > 0);
uploadSharedDepot = True;
if (buildType == "demo"): uploadSharedDepot = False;
dataFolder      = "../data/";
resourcesFolder = dataFolder + "Resources/";
outputFolder    = "../release/win_" + buildType + "/";
steamCmd        = "../../../lib/steam/tools/ContentBuilder/builder/steamcmd.exe";
steamUsername   = args.username;
steamPassword   = ""
appDepotBuildScriptName    = "build_steam_app.vdf";
mainDepotBuildScriptName   = "build_steam_main.vdf";
sharedDepotBuildScriptName = "build_steam_shared.vdf";
appDepotBuildScriptPath    = workingDir + "/" + appDepotBuildScriptName;
mainDepotBuildScriptPath   = workingDir + "/" + mainDepotBuildScriptName;
sharedDepotBuildScriptPath = workingDir + "/" + sharedDepotBuildScriptName;
versionMajor = 0;
versionMinor = 0;
versionBuild = 0;
steamAppId         = 2185480;
steamDepotId       = 2185481; # "Pig Parts Content"
steamSharedDepotId = 2185482; # "Pig Parts Shared"
if (buildType == "demo"):
#
	steamAppId   = 0; #TODO: Add me if we add demo support!
	steamDepotId = 0; #TODO: Add me if we add demo support!
#

if (not os.path.exists(dataFolder)):
#
	print("ERROR: Couldn't find data folder!");
	exit(1);
#
if (not os.path.exists(resourcesFolder)):
#
	print("ERROR: Couldn't find Resources folder!");
	exit(1);
#

# +--------------------------------------------------------------+
# |                   Find the Version Number                    |
# +--------------------------------------------------------------+
versionFilePath = "../game/game_version.h";
if (not os.path.exists(versionFilePath)):
#
	print("Expected to find the app version file at \"" + versionFilePath + "\"");
	exit(1);
#
versionFile	= open(versionFilePath);
versionFileContent = versionFile.read();
versionFile.close()
majorSearchExpression = "\\#define\\s*[A-Z\\_]*VERSION\\_MAJOR\\s*([0-9]+)[^\\n]*";
minorSearchExpression = "\\#define\\s*[A-Z\\_]*VERSION\\_MINOR\\s*([0-9]+)[^\\n]*";
buildSearchExpression = "\\#define\\s*[A-Z\\_]*VERSION\\_BUILD\\s*([0-9]+)[^\\n]*";
majorSearchResult = re.search(majorSearchExpression, versionFileContent);
minorSearchResult = re.search(minorSearchExpression, versionFileContent);
buildSearchResult = re.search(buildSearchExpression, versionFileContent);
if (majorSearchResult == None or len(majorSearchResult.groups()) < 1):
#
	print("Couldn't find the major version number in version file at \"" + versionFilePath + "\"");
	exit(1);
#
if (minorSearchResult == None or len(minorSearchResult.groups()) < 1):
#
	print("Couldn't find the minor version number in version file at \"" + versionFilePath + "\"");
	exit(1);
#
if (buildSearchResult == None or len(buildSearchResult.groups()) < 1):
#
	print("Couldn't find the build version number in version file at \"" + versionFilePath + "\"");
	exit(1);
#

versionMajor = int(versionFileContent[majorSearchResult.start(1):majorSearchResult.end(1)]);
versionMinor = int(versionFileContent[minorSearchResult.start(1):minorSearchResult.end(1)]);
versionBuild = int(versionFileContent[buildSearchResult.start(1):buildSearchResult.end(1)]);

print("");
print("Packaging version " + str(versionMajor) + "." + str(versionMinor) + "(" + str(versionBuild) + ")");
print("");

# +--------------------------------------------------------------+
# |            Delete the Old Contents of the Folder             |
# +--------------------------------------------------------------+
if (os.path.exists(outputFolder)):
#
	print("Deleting old output folder");
	shutil.rmtree(outputFolder);
#
print("Outputting to \"" + outputFolder + "\"");
os.makedirs(outputFolder);

# +--------------------------------------------------------------+
# |          Copy Files from the Top Level data Folder           |
# +--------------------------------------------------------------+
whiteList = [
	"steam_api64.dll",
	"Resources/icon.ico",
	"Resources/icon16.png",
	"Resources/icon24.png",
	"Resources/icon32.png",
	"Resources/icon64.png",
	"Resources/icon128.png",
	"Resources/Fonts/",
	"Resources/Models/",
	"Resources/Models/Textures/",
	"Resources/Music/",
	"Resources/Shaders/",
	"Resources/Sheets/",
	"Resources/Sounds/",
	"Resources/Sprites/",
	"Resources/Text/",
	"Resources/Textures/",
	"Resources/Vector/",
];
blackList = [
	"steam_appid.txt",
	"Resources/Raw/",
];
if (buildType != "developer"):
#
	blackList.append("DEVELOPER_README.txt")
	blackList.append("debug_bindings.txt")
#
if (buildType == "demo"):
#
	whiteList.append("PigPartsDemo.exe");
	whiteList.append("PigPartsDemo.dll");
#
else:
#
	whiteList.append("PigParts.exe");
	whiteList.append("PigParts.dll");
#

numFilesCopied = CopyFilesRecursive(dataFolder, "", outputFolder, whiteList, blackList);

# if (buildType == "demo"):
# #
# 	os.rename(outputFolder + "Resources/Levels/DemoPacks.lst", outputFolder + "Resources/Levels/Packs.lst");
# #

print("Copied " + str(numFilesCopied) + " files to output (" + FormatSize(totalOutputSize) + " total)");
print("Application is " + FormatSize(applicationSize) + " bytes");
print("Resources contains " + str(numResourceFiles) + " files and is " + FormatSize(totalResourcesSize) + " bytes total");

# +--------------------------------------------------------------+
# |                   Modify the Build Scripts                   |
# +--------------------------------------------------------------+
if (not os.path.exists(appDepotBuildScriptPath)):
#
	print("Expected to find app build script at \"" + appDepotBuildScriptPath + "\"");
	exit(1);
#
buildScriptFile = open(appDepotBuildScriptPath);
buildScript = buildScriptFile.read();
buildScriptFile.close();

appidSearchExpression         = """\"appid\"\s*\"([^\n\"]+)\"""";
descSearchExpression          = """\"desc\"\s*\"([^\n\"]+)\"""";
contentRootSearchExpression   = """\"contentroot\"\s*\"([^\n\"]+)\"""";
mainDepotIdSearchExpression   = """\"([0-9]+)\"\s*\"""" + mainDepotBuildScriptName + "\"";
sharedDepotIdSearchExpression = """\"([0-9]+)\"\s*\"""" + sharedDepotBuildScriptName + "\"";
commentSharedSearchExpression = """(\/*\")""" + str(steamSharedDepotId);

print(commentSharedSearchExpression);

buildDescriptionStr = ("%s Build v%d.%02d(%d)" % (buildType.capitalize(), versionMajor, versionMinor, versionBuild));
shareDepotCommentStr = "\"";
if (uploadSharedDepot == False): shareDepotCommentStr = "//\"";

buildScript = ReplaceRegexMatchedSection(buildScript, appidSearchExpression,         str(steamAppId));
buildScript = ReplaceRegexMatchedSection(buildScript, descSearchExpression,          buildDescriptionStr);
buildScript = ReplaceRegexMatchedSection(buildScript, contentRootSearchExpression,   outputFolder);
buildScript = ReplaceRegexMatchedSection(buildScript, mainDepotIdSearchExpression,   str(steamDepotId));
buildScript = ReplaceRegexMatchedSection(buildScript, sharedDepotIdSearchExpression, str(steamSharedDepotId));
buildScript = ReplaceRegexMatchedSection(buildScript, commentSharedSearchExpression, shareDepotCommentStr);
if (buildScript == False):
#
	print("Failed to update the app build script using regular expression matching at \"" + appDepotBuildScriptPath + "\"");
	exit(1);
#

buildScriptFile = open(appDepotBuildScriptPath, "w");
buildScriptFile.write(buildScript);
buildScriptFile.close();
print("Updated the app build script at \"" + appDepotBuildScriptPath + "\"");

# +--------------------------------------------------------------+
# |                   Start the Upload Script                    |
# +--------------------------------------------------------------+

if (not onlyTest):
#
	if (steamUsername == "" or steamUsername == None):
	#
		steamUsername = getpass.getpass("Steam Username: ");
	#
	if (steamPassword == "" or steamPassword == None):
	#
		steamPassword = getpass.getpass("Steam Password: ");
	#
	
	uploadCommand = steamCmd.replace("/", "\\");
	uploadCommand += " +login " + steamUsername + " \"" + steamPassword + "\"";
	uploadCommand += " +run_app_build \"" + appDepotBuildScriptPath.replace("/", "\\\\") + "\"";
	uploadCommand += " +quit";
	
	print(" +================================+");
	print(" | Running steam upload script... |");
	print(" +================================+");
	os.system(uploadCommand);
	print(" +================================+");
	print(" |     Upload Script Finished     |");
	print(" +================================+");
#
else:
#
	print("This was just a test");
#
