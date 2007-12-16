!include "MUI.nsh"
Name "KShutdown for Windows"
OutFile "kshutdown-2.0alpha4-win32.exe"
InstallDir "$PROGRAMFILES\KShutdown"
InstallDirRegKey HKCU "Software\kshutdown.sf.net" ""

!define APP_UNINSTALL_REG "Software\Microsoft\Windows\CurrentVersion\Uninstall\KShutdown"

# FIXME: what is this? ;)
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
	SetOutPath "$INSTDIR"
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKCU "Software\kshutdown.sf.net" "" $INSTDIR	
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayIcon" "$INSTDIR\kshutdown.ico"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayName" "KShutdown"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=93707"
	
	File src\images\kshutdown.ico
	File src\release\kshutdown.exe
	File LICENSE
	File C:\Qt\4.3.2\bin\mingwm10.dll
	#File C:\Qt\4.3.1\bin\QtCore4.dll
	#File C:\Qt\4.3.1\bin\QtGui4.dll
	#File C:\Qt\4.3.1\bin\QtXml4.dll
	
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\KShutdown.lnk" "$INSTDIR\kshutdown.exe" "" "$INSTDIR\kshutdown.ico"
	
	CreateDirectory "$SMSTARTUP"
	CreateShortCut "$SMSTARTUP\KShutdown.lnk" "$INSTDIR\kshutdown.exe" "--init" "$INSTDIR\kshutdown.ico"
# TODO: support for silent mode
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\kshutdown.exe"
	Delete "$INSTDIR\kshutdown.ico"
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\mingwm10.dll"
	#Delete "$INSTDIR\QtCore4.dll"
	#Delete "$INSTDIR\QtGui4.dll"
	#Delete "$INSTDIR\QtXml4.dll"
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"

	DeleteRegKey HKCU "Software\kshutdown.sf.net"
	DeleteRegKey HKLM "${APP_UNINSTALL_REG}"

	SetShellVarContext all
	Delete "$DESKTOP\KShutdown.lnk"
	Delete "$SMPROGRAMS\KShutdown.lnk"
	Delete "$SMSTARTUP\KShutdown.lnk"
SectionEnd