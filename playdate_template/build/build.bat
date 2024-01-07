@echo off

rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set ProjectName=Playdate Template
set ProjectNameSafe=PlaydateTemplate

set DebugBuild=1
set DeveloperBuild=1
set SimulatorBuild=1
set BuildBoth=0
set DemoBuild=0
set AssertionsEnabled=1

set LibDirectory=..\lib
set EngineSourceDirectory=..\engine\platform\playdate
set GameSourceDirectory=..\game
set DataDirectory=..\data
set PlaydateSdkDirectory=%PLAYDATE_SDK_PATH%
set PdcExeName=%PlaydateSdkDirectory%\bin\pdc
set MainSourcePath=%EngineSourceDirectory%\pig_main.cpp
set OutputObjName=%ProjectNameSafe%.obj
set OutputElfName=pdex.elf
set OutputLibName=pdex.lib
set OutputDllName=pdex.dll
set OutputPdbName=pdex.pdb
set OutputMapName=pdex.map
set PdcOutputFolder=%ProjectNameSafe%.pdx
set IncVersNumScriptPath=%LibDirectory%\include\gylib\IncrementVersionNumber.py
set EngineVersionFilePath=%EngineSourceDirectory%\pig_playdate_version.h
set GameVersionFilePath=%GameSourceDirectory%\game_version.h
set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%
set OutputObjPdbName=%ProjectNameSafe%_obj_%TimeString%.pdb
set PdcFlags=-q -sdkpath "%PlaydateSdkDirectory%"
rem TODO: We probably want to add the -s flag when in release mode (strips debug symbols)

set ArmCompilerExeName=arm-none-eabi-gcc
set ArmObjCopyExeName=arm-none-eabi-objcopy
set ArmStripExeName=arm-none-eabi-strip
set PlaydateChip=cortex-m7

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

del *.pdb > NUL 2> NUL
del *.obj > NUL 2> NUL

if "%BuildBoth%"=="1" (
	goto CompileForSimulator
) else if "%SimulatorBuild%"=="1" (
	goto CompileForSimulator
) else (
	goto CompileForDevice
)

rem +--------------------------------------------------------------+
rem |                     CompileForSimulator                      |
rem +--------------------------------------------------------------+
:CompileForSimulator

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
if "%ComputerName%"=="BUTTER_COMP" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
) else (
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
)

set CompilerFlags=/D "PLAYDATE_COMPILATION" /D "PLAYDATE_SIMULATOR" /D "PROJECT_NAME=\"%ProjectName%\"" /D "PROJECT_NAME_SAFE=\"%ProjectNameSafe%\"" /D "DEBUG_BUILD=%DebugBuild%" /D "DEVELOPER_BUILD=%DeveloperBuild%" /D "DEMO_BUILD=%DemoBuild%" /D "STEAM_BUILD=0" /D "PROCMON_SUPPORTED=0" /D "SOCKETS_SUPPORTED=0" /D "BOX2D_SUPPORTED=0" /D "OPENGL_SUPPORTED=0" /D "VULKAN_SUPPORTED=0" /D "DIRECTX_SUPPORTED=0" /D "SLUG_SUPPORTED=0" /D "JSON_SUPPORTED=0" /D "ASSERTIONS_ENABLED=%AssertionsEnabled%"
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
rem -k = Skip unknown file types (Removed because we have .txt files)
rem -q = Suppress non-error output (Optional)

if "%PythonInstalled%"=="1" (
	python %IncVersNumScriptPath% %EngineVersionFilePath%
	python %IncVersNumScriptPath% %GameVersionFilePath%
)

cl /Fo"%OutputObjName%" %CompilerFlags% %IncludeDirectories% /c "%MainSourcePath%"
LINK %LinkerFlags% %Libraries% "%OutputObjName%" /OUT:"%OutputDllName%" /IMPLIB:"%OutputLibName%" /PDB:"%OutputPdbName%"
XCOPY ".\%OutputDllName%" "%DataDirectory%\" /Y > NUL

if "%BuildBoth%"=="1" (
	goto CompileForDevice
) else (
	goto PackageGame
)

rem +--------------------------------------------------------------+
rem |                       CompileForDevice                       |
rem +--------------------------------------------------------------+
:CompileForDevice

set SharedFlags=%SharedFlags% -D "TARGET_PLAYDATE=1" -D "TARGET_EXTENSION=1"
rem -mfloat-abi=hard = Use hardware instructions for floating-point operations (from FPU)
rem -mfpu=fpv5-sp-d16 = Specifies the target FPU architecture, that is the floating-point hardware available on the target. (Armv7 FPv5-SP-D16 floating-point extension) (from FPU)
rem __FPU_USED = ? (from FPU)
rem -mthumb = Requests that the compiler targets the T32 (Thumb) instruction set instead of A32 (Arm) (from MCFLAGS)
rem -mcpu=cortex-m7 = Enables code generation for a specific Arm processor. (from MCFLAGS)
rem -specs=nano.specs = Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library (https://stackoverflow.com/questions/5764414/undefined-reference-to-sbrk) and (https://devforum.play.date/t/c-api-converting-string-to-float/10097)
rem -specs=nosys.specs = ?
set SharedFlags=%SharedFlags% -mthumb -mcpu=%PlaydateChip% -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1 -specs=nano.specs -specs=nosys.specs
set CompilerFlags=-D "PLAYDATE_COMPILATION" -D "PLAYDATE_DEVICE" -D "PROJECT_NAME=\"%ProjectName%\"" -D "PROJECT_NAME_SAFE=\"%ProjectNameSafe%\"" -D "DEBUG_BUILD=%DebugBuild%" -D "DEVELOPER_BUILD=%DeveloperBuild%" -D "DEMO_BUILD=%DemoBuild%" -D "STEAM_BUILD=0" -D "PROCMON_SUPPORTED=0" -D "SOCKETS_SUPPORTED=0" -D "BOX2D_SUPPORTED=0" -D "OPENGL_SUPPORTED=0" -D "VULKAN_SUPPORTED=0" -D "DIRECTX_SUPPORTED=0" -D "SLUG_SUPPORTED=0" -D "JSON_SUPPORTED=0" -D "ASSERTIONS_ENABLED=%AssertionsEnabled%"
rem -g3 = Produce debugging information in the operating system's native format (3 = ?)
rem -std=gnu11 = ? (Removed, because we are compiling C++, not C)
rem -MD = (MSVC Option) Use the multithread-specific and DLL-specific version of the run-time library
rem -MT = (MSVC Option) Use the multithread, static version of the run-time library
rem -MP = This option instructs CPP to add a phony target for each dependency other than the main file, causing each to depend on nothing. These dummy rules work around errors make gives if you remove header files without updating the Makefile to match
rem       ==OR== (MSVC Option) Build multiple source files concurrently (removed)
rem -MF = When used with the driver options -MD or -MMD, -MF overrides the default dependency output file
rem -gdwarf-2 = Produce debugging information in DWARF version 2 format (if that is supported). This is the format used by DBX on IRIX 6. With this option, GCC uses features of DWARF version 3 when they are useful; version 3 is upward compatible with version 2, but may still cause problems for older debuggers.
set CompilerFlags=%CompilerFlags% -g3 -MD -MF %ProjectNameSafe%.d -gdwarf-2
rem -fverbose-asm = ?
rem -fno-common = ?
rem -falign-functions = ? (from OPT)
rem -fomit-frame-pointer = ? (from OPT)
rem -ffunction-sections = ?
rem -fdata-sections = ?
rem -fno-exceptions = ? (Removed)
rem -mword-relocations = ?
set CompilerFlags=%CompilerFlags% -fverbose-asm -fno-common -falign-functions=16 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-exceptions -mword-relocations
rem -Wall = ?
rem -Wno-unknown-pragmas = ?
rem -Wdouble-promotion = ?
rem -Wno-comment = Don't warn about multi-line comments using // syntax
rem -Wno-switch = Don't warn about switch statements on enums with unhandled values
rem -Wno-nonnull = Don't warn about potentially null arguments passed to functions like memset
rem -Wno-unused = ?
rem -Wstrict-prototypes = (Removed because it isn't compatible with C++ builds?)
rem -Wa,-ahlms=%ProjectNameSafe%.lst = ? (Removed)
set CompilerFlags=%CompilerFlags% -Wall -Wno-unknown-pragmas -Wdouble-promotion -Wno-comment -Wno-switch -Wno-nonnull -Wno-unused
rem -g3 = ?
rem -gdwarf-2 = ?
rem -Wa,-amhls = ?
set AssemblerFlags=-g3 -gdwarf-2 -Wa,-amhls=%ProjectNameSafe%.lst
set IncludeDirectories=-I "%EngineSourceDirectory%" -I "%GameSourceDirectory%" -I "%LibDirectory%\include" -I "%PlaydateSdkDirectory%\C_API"
rem -nostartfiles = ?
rem --entry eventHandlerShim
rem -Map = ?
rem --cref = ?
rem --gc-sections = ?
rem --no-warn-mismatch = ?
rem --emit-relocs = ?
set LinkerFlags=-nostartfiles --entry eventHandlerShim -Wl,-Map=%OutputMapName%,--cref,--gc-sections,--no-warn-mismatch,--emit-relocs
set LinkerFlags=%LinkerFlags% -T%PlaydateSdkDirectory%\C_API\buildsupport\link_map.ld
rem -lsupc++ ?
set Libraries=

if "%DebugBuild%"=="1" (
	rem -O2 = No optimizations?
	set CompilerFlags=%CompilerFlags% -Og
) else if "%DebugBuild%"=="1" (
	rem TODO: Change this back to -O2 once we figure out why it's causing problems!
	rem -O2 = Optimize even more. GCC performs nearly all supported optimizations that do not involve a space-speed tradeoff. (from OPT)
	set CompilerFlags=%CompilerFlags% -O1
)

echo [Compiling...]
%ArmCompilerExeName% -c "%MainSourcePath%" -o "%OutputObjName%" %SharedFlags% %CompilerFlags% %IncludeDirectories%

echo [Linking...]
%ArmCompilerExeName% %OutputObjName% %Libraries% %SharedFlags% %LinkerFlags% -o %OutputElfName%

rem arm-none-eabi-strip.exe --strip-unneeded -R .comment -g %OutputElfName% -o "%DataDirectory%/pdex.elf"
rem arm-none-eabi-strip.exe -g %OutputElfName% -o "%DataDirectory%/pdex.elf"
copy "%OutputElfName%" "%DataDirectory%/pdex.elf"

goto PackageGame

rem +--------------------------------------------------------------+
rem |                         PackageGame                          |
rem +--------------------------------------------------------------+
:PackageGame
echo [Packaging Into .pdx]
"%PdcExeName%" %PdcFlags% "%DataDirectory%" "%PdcOutputFolder%"
