#!/bin/bash

KS_BAT_COMMAND="$1"

if [ -z "$KS_BAT_COMMAND" ]; then
	echo "Usage: ./Setup-wine.sh <command>"
	echo "Commands:"
	echo "all   - build everything"
	echo "setup - build installer for Windows"
	echo "test  - compile and run"
	exit 1
fi

export WINEARCH=win32
export WINEPREFIX=~/.wine-kshutdown

KS_APP_VERSION=$(sed 1!d VERSION)
KS_QT_VERSION=$(sed 5!d VERSION)

pushd ~/.wine-kshutdown/drive_c/kshutdown2
wineconsole Setup-qt5.bat "$KS_APP_VERSION" "$KS_QT_VERSION" "$KS_BAT_COMMAND"
popd

rm -f kshutdown-portable/kshutdown.ini
7z a "kshutdown-portable-${KS_APP_VERSION}-win32.7z" kshutdown-portable
