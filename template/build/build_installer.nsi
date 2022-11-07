
!include "MUI2.nsh"

!define MUI_ICON "..\data\Resources\icon.ico"
!define MUI_UNICON "..\data\Resources\icon.ico"

OutFile "PigPartsInstaller.exe"
Name "Pig Parts"
Unicode True
InstallDir "$PROGRAMFILES64\Pig Parts"
; RequestExecutionLevel user ; For removing Start Menu shortcut in Windows 7

!define MUI_HEADERIMAGE
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\data\Resources\installer_welcome_image.bmp"
!define MUI_HEADERIMAGE_BITMAP "..\data\Resources\installer_header_image.bmp"
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\PigParts.exe"

; !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
; !insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Pig Parts" PigParts
SetOutPath $INSTDIR
File ..\data\PigParts.exe
File ..\data\PigParts.dll
File ..\data\debug_bindings.txt
File ..\data\DEVELOPER_README.txt
SetOutPath $INSTDIR\Resources
File ..\data\Resources\*.ico
File ..\data\Resources\*.png
File /r ..\data\Resources\Fonts
File /r ..\data\Resources\Models
File /r ..\data\Resources\Music
File /r ..\data\Resources\Shaders
File /r ..\data\Resources\Sheets
File /r ..\data\Resources\Sounds
File /r ..\data\Resources\Sprites
File /r ..\data\Resources\Text
File /r ..\data\Resources\Textures
File /r ..\data\Resources\Vector
SetOutPath $INSTDIR
WriteUninstaller $INSTDIR\uninstall.exe
CreateShortcut "$SMPROGRAMS\Pig Parts.lnk" "$INSTDIR\PigParts.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PigParts" "DisplayName" "Pig Parts"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PigParts" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PigParts" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
SectionEnd

;Language strings
LangString DESC_PigParts ${LANG_ENGLISH} "A voxel game on asteroids"

;Assign language strings to sections
; !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
; !insertmacro MUI_DESCRIPTION_TEXT ${PigParts} $(DESC_PigParts)
; !insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
Delete "$SMPROGRAMS\Pig Parts.lnk"
Delete $INSTDIR\PigParts.exe
Delete $INSTDIR\PigParts.dll
Delete $INSTDIR\PigParts_TEMP.dll
Delete $INSTDIR\Resources\*.ico
Delete $INSTDIR\Resources\*.png
RMDir /r $INSTDIR\Resources\Fonts
RMDir /r $INSTDIR\Resources\Models
RMDir /r $INSTDIR\Resources\Music
RMDir /r $INSTDIR\Resources\Shaders
RMDir /r $INSTDIR\Resources\Sheets
RMDir /r $INSTDIR\Resources\Slug
RMDir /r $INSTDIR\Resources\Sounds
RMDir /r $INSTDIR\Resources\Sprites
RMDir /r $INSTDIR\Resources\Text
RMDir /r $INSTDIR\Resources\Textures
RMDir /r $INSTDIR\Resources\Vector
RMDir $INSTDIR\Resources
Delete $INSTDIR\uninstall.exe
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PigParts"
RMDir $INSTDIR
SectionEnd

; +==============================+
; |  Simple Create File Example  |
; +==============================+
; Section
; MessageBox MB_OK "Hello from the installer!"
; FileOpen $0 "$DESKTOP\hello_world.txt" w
; FileWrite $0 "Hello from the installer!"
; FileClose $0
; MessageBox MB_OK "hello_world.txt has been created on your Desktop. Go check it out!"
; SectionEnd

; +==============================+
; | Simple Install File Example  |
; +==============================+
; InstallDir $DESKTOP
; Section
; SetOutPath $INSTDIR
; File test.txt
; SectionEnd

; +==============================+
; |   Start Menu Entry Example   |
; +==============================+
; InstallDir $DESKTOP
; RequestExecutionLevel user ; For removing Start Menu shortcut in Windows 7
; Section
; SetOutPath $INSTDIR
; WriteUninstaller "$INSTDIR\UninstallPigParts.exe"
; CreateShortcut "$SMPROGRAMS\PigParts.lnk" "$INSTDIR\UninstallPigParts.exe"
; SectionEnd
; Section "Uninstall"
; Delete "$SMPROGRAMS\PigParts.lnk"
; Delete $INSTDIR\UninstallPigParts.exe
; RMDir $INSTDIR
; SectionEnd

; +========================================+
; | Check if user is Administrator Example |
; +========================================+
; Section
; UserInfo::GetAccountType
; Pop $0
; StrCmp $0 "Admin" +3
; MessageBox MB_OK "not admin: $0"
; Return
; MessageBox MB_OK "is admin"
; SectionEnd
