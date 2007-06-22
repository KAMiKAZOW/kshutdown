!include "MUI.nsh"
Name "KShutdown for Windows"
OutFile "kshutdown-2.0alpha1-win32.exe"
InstallDir "$PROGRAMFILES\KShutdown"
InstallDirRegKey HKCU "Software\KShutdown" ""

# FIXME: what is this? ;)
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

Section "Install"
	SetOutPath "$INSTDIR"
	WriteRegStr HKCU "Software\KShutdown" "" $INSTDIR
	WriteUninstaller "$INSTDIR\uninstall.exe"
	
	File src\release\kshutdown.exe
	File LICENSE
	File C:\Qt\4.3.0\bin\mingwm10.dll
	File C:\Qt\4.3.0\bin\QtCore4.dll
	File C:\Qt\4.3.0\bin\QtGui4.dll
	
	#!!! shortcuts
	#!!! uninstall entry
	#!!! silent mode
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\kshutdown.exe"
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\mingwm10.dll"
	Delete "$INSTDIR\QtCore4.dll"
	Delete "$INSTDIR\QtGui4.dll"
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"

	DeleteRegKey HKCU "Software\KShutdown"
SectionEnd