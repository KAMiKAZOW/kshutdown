!include "MUI.nsh"
Name "KShutdown for Windows"
OutFile "kshutdown-2.0alpha1-win32.exe"
InstallDir "$PROGRAMFILES\KShutdown"
InstallDirRegKey HKCU "Software\KShutdown" ""

!define APP_UNINSTALL_REG "Software\Microsoft\Windows\CurrentVersion\Uninstall\KShutdown"

# FIXME: what is this? ;)
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

Section "Install"
	SetOutPath "$INSTDIR"
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKCU "Software\KShutdown" "" $INSTDIR	
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayIcon" "$INSTDIR\kshutdown.ico"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayName" "KShutdown"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=93707"
	
	File src\release\kshutdown.exe
	File LICENSE
	File C:\Qt\4.3.0\bin\mingwm10.dll
	File C:\Qt\4.3.0\bin\QtCore4.dll
	File C:\Qt\4.3.0\bin\QtGui4.dll
	
	SetShellVarContext all
	#!!!kshutdown.ico
	CreateShortCut "$DESKTOP\KShutdown.lnk" "$INSTDIR\kshutdown.exe"
	CreateShortCut "$SMPROGRAMS\KShutdown.lnk" "$INSTDIR\kshutdown.exe"
# TODO: support for silent mode
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
	DeleteRegKey HKLM "${APP_UNINSTALL_REG}"

	SetShellVarContext all
	Delete "$DESKTOP\KShutdown.lnk"
	Delete "$SMPROGRAMS\KShutdown.lnk"
SectionEnd