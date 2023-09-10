@echo off

rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set ProjectName=Template (Pig Engine)
set ProjectNameSafe=Template

set DebugBuild=1
set DeveloperBuild=1
set DemoBuild=0
set AssertionsEnabled=1

set LibDirectory=..\lib
set EngineSourceDirectory=..\engine\platform\playdate
set GameSourceDirectory=..\game
set DataDirectory=..\data
set PlaydateSdkDirectory=C:\Users\robbitay\Documents\MyStuff\Programs\PlaydateSDK
set SimCompilerExeName=gcc
set ArmCompilerExeName=arm-none-eabi-gcc
set PdcExeName=%PlaydateSdkDirectory%\bin\pdc
set MainSourcePath=%EngineSourceDirectory%\pig_main.cpp
set OutputObjName=%ProjectNameSafe%.obj
set OutputLibName=pdex.lib
set OutputDllName=pdex.dll
set OutputPdbName=pdex.pdb
set PdcOutputFolder=%ProjectNameSafe%.pdx
set IncVersNumScriptPath=..\IncrementVersionNumber.py
set EngineVersionFilePath=%EngineSourceDirectory%\platform\playdate\pig_playdate_version.h
set GameVersionFilePath=%GameSourceDirectory%\game_version.h
set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%
set OutputObjPdbName=%ProjectNameSafe%_obj_%TimeString%.pdb

echo Running on %ComputerName%

if "%DemoBuild%"=="1" (
	set ProjectName=%ProjectName% Demo
)

python --version 2>NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. The build number will not be incremented
	set PythonInstalled=0
) else (
	set PythonInstalled=1
)

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

set CompilerFlags=/D "PLAYDATE_COMPILATION" /D "PROJECT_NAME=\"%ProjectName%\"" /D "PROJECT_NAME_SAFE=\"%ProjectNameSafe%\"" /D "DEBUG_BUILD=%DebugBuild%" /D "DEVELOPER_BUILD=%DeveloperBuild%" /D "DEMO_BUILD=%DemoBuild%" /D "STEAM_BUILD=0" /D "PROCMON_SUPPORTED=0" /D "SOCKETS_SUPPORTED=0" /D "BOX2D_SUPPORTED=0" /D "OPENGL_SUPPORTED=0" /D "VULKAN_SUPPORTED=0" /D "DIRECTX_SUPPORTED=0" /D "SLUG_SUPPORTED=0" /D "JSON_SUPPORTED=0" /D "ASSERTIONS_ENABLED=%AssertionsEnabled%"
set CompilerFlags=%CompilerFlags% /D "_WINDLL" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "TARGET_SIMULATOR=1" /D "_WINDLL=1" /D "TARGET_EXTENSION=1"
rem /GS = Buffer overrun protection is turned on
rem /Zi = Generates complete debugging information
rem /Gm- = Deprecated. Enables minimal rebuild
rem /Od = Disables optimization TODO: Debug only?
rem /RTC1 = Enable fast runtime checks (equivalent to /RTCsu)
rem /std:c++20 = C++20 standard ISO/IEC 14882:2020
rem /Gd = Uses the __cdecl calling convention (x86 only)
rem /MDd = Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib
rem /Ob0 = Controls inline expansion (0 level = no expansion?)
rem /W3 = Set output warning level
rem /WX- = (Don't?) Treat warnings as errors TODO: Do we need this?
rem /nologo = Suppress the startup banner
set CompilerFlags=%CompilerFlags% /GS /Zi /Gm- /Od /RTC1 /std:c++20 /Gd /MDd /Ob0 /W3 /WX- /nologo
rem /Fd = Sets the obj .pdb file name
rem /Fp = Specifies a precompiled header file name TODO: Do we need this?
set CompilerFlags=%CompilerFlags% /Fd"%OutputObjPdbName%" /Fp"%ProjectNameSafe%.pch"
rem /errorReport:prompt = Deprecated. Windows Error Reporting (WER) settings control error reporting TODO: Do we need this?
rem /diagnostics:column =  Diagnostics format: prints column information. TODO: Do we need this? (Optional)
rem /Zc:forScope = Enforce Standard C++ for scoping rules (on by default)
rem /Zc:inline = Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
rem /Zc:wchar_t = wchar_t is a native type, not a typedef (on by default)
rem /fp:precise = "precise" floating-point model; results are predictable
set CompilerFlags=%CompilerFlags% /errorReport:prompt /Zc:forScope /Zc:inline /Zc:wchar_t /fp:precise
set IncludeDirectories=/I"%EngineSourceDirectory%" /I"%GameSourceDirectory%" /I"%LibDirectory%\include" /I"%PlaydateSdkDirectory%\C_API"
set LinkerFlags=/MANIFEST /NXCOMPAT /DYNAMICBASE /DEBUG /DLL /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:CONSOLE /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
set LinkerFlags=%LinkerFlags% /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"%OutputDllName%.intermediate.manifest" /LTCGOUT:"%ProjectNameSafe%.iobj" /ILK:"%ProjectNameSafe%.ilk"
set Libraries="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "comdlg32.lib" "advapi32.lib"
set PdcFlags=-k -sdkpath "%PlaydateSdkDirectory%"

if "%PythonInstalled%"=="1" (
	python %IncVersNumScriptPath% %EngineVersionFilePath%
	python %IncVersNumScriptPath% %GameVersionFilePath%
)

del *.pdb > NUL 2> NUL
del *.obj > NUL 2> NUL

cl /Fo"%OutputObjName%" %CompilerFlags% %IncludeDirectories% /c "%MainSourcePath%"
LINK %LinkerFlags% %Libraries% "%OutputObjName%" /OUT:"%OutputDllName%" /IMPLIB:"%OutputLibName%" /PDB:"%OutputPdbName%"
XCOPY ".\%OutputDllName%" "%DataDirectory%\" /Y > NUL

%PdcExeName% %PdcFlags% "%DataDirectory%" "%PdcOutputFolder%"
