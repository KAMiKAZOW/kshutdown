﻿# NOTE: UTF-8 BOM is required to Unicode work correctly
Unicode true

!include "MUI.nsh"
!include "kshutdown.nsh"

Name "KShutdown"
OutFile "kshutdown-${APP_FILE_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\KShutdown"
InstallDirRegKey HKCU "Software\kshutdown.sf.net" ""

SetCompressor /SOLID lzma
# TODO: ManifestDPIAware true

!define APP_UNINSTALL_REG "Software\Microsoft\Windows\CurrentVersion\Uninstall\KShutdown"

!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_NODESC

# GNU GPL is not an EULA
#!define MUI_LICENSEPAGE_TEXT_TOP "tl;dr http://tldrlegal.com/l/gpl2"
#!insertmacro MUI_PAGE_LICENSE "LICENSE"

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "-"
	SetOutPath "$INSTDIR"
	WriteUninstaller "$INSTDIR\uninstall.exe"

	WriteRegStr HKCU "Software\kshutdown.sf.net" "" $INSTDIR
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\kshutdown.exe" "" "$INSTDIR\kshutdown.exe"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayIcon" "$INSTDIR\kshutdown.ico"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayName" "KShutdown"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "DisplayVersion" "${APP_FULL_VERSION}"
	WriteRegDWORD HKLM "${APP_UNINSTALL_REG}" "NoModify" 1
	WriteRegDWORD HKLM "${APP_UNINSTALL_REG}" "NoRepair" 1
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "Publisher" "Konrad Twardowski"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "URLInfoAbout" "https://kshutdown.sourceforge.io/"
	WriteRegStr HKLM "${APP_UNINSTALL_REG}" "URLUpdateInfo" "https://kshutdown.sourceforge.io/download.html"
	
	File src\images\kshutdown.ico
	File /oname=kshutdown.exe src\release\kshutdown-qt.exe
	File LICENSE

	!define KS_QT_VERSION "5.11.0"
	!define KS_QT_BIN "C:\Qt\Qt${KS_QT_VERSION}\${KS_QT_VERSION}\mingw53_32\bin"

	File "${KS_QT_BIN}\libgcc_s_dw2-1.dll"
	File "${KS_QT_BIN}\libwinpthread-1.dll"
	File "${KS_QT_BIN}\libstdc++-6.dll"

	File "${KS_QT_BIN}\Qt5Core.dll"
	File "${KS_QT_BIN}\Qt5Gui.dll"
	File "${KS_QT_BIN}\Qt5Widgets.dll"
	File "${KS_QT_BIN}\Qt5WinExtras.dll"
	File "${KS_QT_BIN}\..\plugins\platforms\qwindows.dll"
	
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\KShutdown.lnk" "$INSTDIR\kshutdown.exe" "" "$INSTDIR\kshutdown.ico"
SectionEnd

# TODO: remember if option was checked in case of reinstallation
Section /o "Autostart with Windows" SectionAutostart
	SetShellVarContext all
	IfSilent NoAutostart
	CreateDirectory "$SMSTARTUP"
# TODO: rename Autostart link
	CreateShortCut "$SMSTARTUP\KShutdown.lnk" "$INSTDIR\kshutdown.exe" "--init" "$INSTDIR\kshutdown.ico"
NoAutostart:
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\kshutdown.exe"
	Delete "$INSTDIR\kshutdown.ico"
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\libgcc_s_dw2-1.dll"
	Delete "$INSTDIR\libstdc++-6.dll"
	Delete "$INSTDIR\libwinpthread-1.dll"

	Delete "$INSTDIR\Qt5Core.dll"
	Delete "$INSTDIR\Qt5Gui.dll"
	Delete "$INSTDIR\Qt5Widgets.dll"
	Delete "$INSTDIR\Qt5WinExtras.dll"
	Delete "$INSTDIR\qwindows.dll"

	# Remove DLLs from olders versions, too
	Delete "$INSTDIR\mingwm10.dll"
	Delete "$INSTDIR\QtCore4.dll"
	Delete "$INSTDIR\QtGui4.dll"

	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"

# FIXME: should ask user
	DeleteRegKey HKCU "Software\kshutdown.sf.net"

	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\kshutdown.exe"
	DeleteRegKey HKLM "${APP_UNINSTALL_REG}"

	SetShellVarContext all
	Delete "$DESKTOP\KShutdown.lnk"
	Delete "$SMPROGRAMS\KShutdown.lnk"
	Delete "$SMSTARTUP\KShutdown.lnk"
SectionEnd