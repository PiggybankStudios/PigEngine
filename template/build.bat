@echo off

set ProjectName=PigEngineTest

set CompilePlatform=1
set CompileEngine=1
set CompilePlugins=0

set DebugBuild=1
set DeveloperBuild=1
set DemoBuild=0
set SteamBuild=0
set EarlyAccessBuild=1
set OpenGlSupport=1
set VulkanSupport=0
set DirectXSupport=0

set CopyToDataDirectory=1

set LibDirectory=..\lib
set EngineSourceDirectory=..\engine
set GameSourceDirectory=..\game
set DataDirectory=..\data
set PlatformCodePath=%EngineSourceDirectory%\platform\win32\win32_main.cpp
set EngineCodePath=%EngineSourceDirectory%\pig\pig_main.cpp
set IncVersNumScriptPath=..\IncrementVersionNumber.py
set PlatformVersionFilePath=%EngineSourceDirectory%\platform\win32\win32_version.h
set EngineVersionFilePath=%EngineSourceDirectory%\pig\pig_version.h
set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%

echo Running on %ComputerName%

if "%DemoBuild%"=="1" (
	set ProjectName=%ProjectName%_Demo
)

python --version 2>NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. The build number will not be incremented
	set PythonInstalled=0
) else (
	set PythonInstalled=1
)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CompilerFlags=-DWINDOWS_COMPILATION -DDEBUG_BUILD=%DebugBuild% -DDEVELOPER_BUILD=%DeveloperBuild% -DDEMO_BUILD=%DemoBuild% -DSTEAM_BUILD=%SteamBuild% -DEARLY_ACCESS_BUILD=%EarlyAccessBuild% -DOPENGL_SUPPORTED=%OpenGlSupport% -DVULKAN_SUPPORTED=%VulkanSupport% -DDIRECTX_SUPPORTED=%DirectXSupport%
rem /FC = Full path for error messages
rem /EHsc = Exception Handling Model: Standard C++ Stack Unwinding. Functions declared as extern "C" can't throw exceptions
rem /EHa- = TODO: Do we really need this?? It seems like this option should be off if we specify s and c earlier
rem /nologo = Suppress the startup banner
rem /GS- = Buffer overrun protection is turned off
rem /Gm- = Minimal rebuild is enabled [deprecated]
rem /GR- = Run-Time Type Information is Disabled [_CPPRTTI macro doesn't work]
rem /Fm = Enables map file output [with default name, same as /MAP linker option]
rem /Oi = Enable intrinsic generation
rem /WX = Treat warnings as errors
rem /W4 = Warning level 4 [just below /Wall]
set CompilerFlags=%CompilerFlags% /FC /EHsc /EHa- /nologo /GS- /Gm- /GR- /Fm /Oi /WX /W4
rem /wd4130 = Logical operation on address of string constant [W4] TODO: Should we re-enable this one? Don't know any scenarios where I want to do this
rem /wd4201 = Nonstandard extension used: nameless struct/union [W4] TODO: Should we re-enable this restriction for ANSI compatibility?
rem /wd4324 = Structure was padded due to __declspec[align[]] [W4]
rem /wd4458 = Declaration of 'identifier' hides class member [W4]
rem /wd4505 = Unreferenced local function has been removed [W4]
rem /wd4996 = Usage of deprecated function, class member, variable, or typedef [W3]
rem /wd4127 = Conditional expression is constant [W4]
set CompilerFlags=%CompilerFlags% /wd4130 /wd4201 /wd4324 /wd4458 /wd4505 /wd4996 /wd4127
rem TODO: Add OpenAL replacement library for audio?
set Libraries=opengl32.lib
set EngineLibraries=qu3e.lib
set LinkerFlags=-incremental:no
rem TODO: Do we really need all of these? Maybe go through and document what functions we use from them?
rem glfw3.lib    = GLFW, platform independent window creation + device context + input
rem gdi32.lib    = ?
rem User32.lib   = ?
rem Shell32.lib  = ?
rem kernel32.lib = ?
rem winmm.lib    = ?
rem Winhttp.lib  = ?
rem Shlwapi.lib  = ?
rem Ole32.lib    = Combaseapi.h, CoCreateInstance
set PlatformLibraries=glfw3.lib gdi32.lib User32.lib Shell32.lib kernel32.lib winmm.lib Winhttp.lib Shlwapi.lib Ole32.lib
set IncludeDirectories=/I"%EngineSourceDirectory%" /I"%EngineSourceDirectory%"\platform /I"%GameSourceDirectory%" /I"%LibDirectory%\include" /I"%LibDirectory%\include\my_glfw\include" /I"%LibDirectory%\include\bullet3\src"
set EngineDllExports=/EXPORT:Pig_GetVersion /EXPORT:Pig_GetStartupOptions /EXPORT:Pig_Initialize /EXPORT:Pig_Update /EXPORT:Pig_AudioService /EXPORT:Pig_ShouldWindowClose /EXPORT:Pig_Closing /EXPORT:Pig_PreReload /EXPORT:Pig_PostReload /EXPORT:Pig_PerformTask
set EngineDllDirective=/DLL /PDB:"%ProjectName%_%TimeString%.pdb"

if "%DebugBuild%"=="1" (
	rem /Od = Optimization level: Debug
	rem /Zi = Generate complete debugging information
	rem /MTd = Statically link the standard library [not as a DLL, Debug version]
	rem /wd4065 = Switch statement contains 'default' but no 'case' labels
	rem /wd4100 = Unreferenced formal parameter [W4]
	rem /wd4101 = Unreferenced local variable [W3]
	rem /wd4127 = Conditional expression is constant [W4]
	rem /wd4189 = Local variable is initialized but not referenced [W4]
	rem /wd4702 = Unreachable code [W4]
	set CompilerFlags=%CompilerFlags% /Od /Zi /MTd /wd4065 /wd4100 /wd4101 /wd4127 /wd4189 /wd4702
	set LibraryDirectories=/LIBPATH:"%LibDirectory%\debug" /LIBPATH:"%LibDirectory%\include\my_glfw\debug"
	rem glew32d.lib
	set Libraries=%Libraries% Dbghelp.lib
	set EngineLibraries=%EngineLibraries%
) else (
	rem /Ot = Favors fast code over small code
	rem /Oy = Omit frame pointer [x86 only]
	rem /O2 = Optimization level 2: Creates fast code
	rem /MT = Statically link the standard library [not as a DLL]
	set CompilerFlags=%CompilerFlags% /Ot /Oy /O2 /MT /Zi
	set LibraryDirectories=/LIBPATH:"%LibDirectory%\release" /LIBPATH:"%LibDirectory%\include\my_glfw\release"
	rem glew32.lib
	rem set Libraries=%Libraries%
	set EngineLibraries=%EngineLibraries%
)

if "%SteamBuild%"=="1" (
	set Libraries=%Libraries% steam_api64.lib
)

rem Compile the resources file to generate resources.res which defines our program icon
rem TODO: Make the .res file show up in the build folder. Also maybe make this file path another define above
rc /nologo resources.rc

del *.pdb > NUL 2> NUL

if "%CompilePlatform%"=="1" (
	echo[
	
	if "%PythonInstalled%"=="1" (
		python %IncVersNumScriptPath% %PlatformVersionFilePath%
	)
	
	cl /Fe%ProjectName%.exe %CompilerFlags% %IncludeDirectories% "%PlatformCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %PlatformLibraries% resources.res
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying %ProjectName%.exe to data directory]
		XCOPY ".\%ProjectName%.exe" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Platform Build Finished!]
	)
)

if "%CompileEngine%"=="1" (
	echo[
	
	if "%PythonInstalled%"=="1" (
		python %IncVersNumScriptPath% %EngineVersionFilePath%
	)
	
	cl /Fe%ProjectName%.dll %CompilerFlags% %IncludeDirectories% "%EngineCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %EngineLibraries% %EngineDllExports% %EngineDllDirective%
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying %ProjectName%.dll to data directory]
		XCOPY ".\%ProjectName%.dll" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Engine Build Finished!]
	)
)

if "%CompilePlugins%"=="1" (
	rem echo[
	
	if "%PythonInstalled%"=="1" (
		rem python %IncVersNumScriptPath% %GameSourceDirectory%\worldgen\worldgen_version.h
	)
	
	rem cl /FePlugin_WorldGen.dll %CompilerFlags% %IncludeDirectories% "%GameSourceDirectory%\worldgen\worldgen_main.cpp" /link %LibraryDirectories% %LinkerFlags% %Libraries% %PluginDllExports% %PluginDllDirective%
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying plugin DLLs to data directory]
		rem XCOPY ".\Plugin_WorldGen.dll" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Plugin Builds Finished!]
	)
)