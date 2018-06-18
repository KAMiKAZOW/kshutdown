#!/bin/bash

export WINEARCH=win32
export WINEPREFIX=~/.wine-kshutdown

wineconsole Setup-qt5.bat

KS_FILE_VERSION=$(sed 1!d VERSION)
rm -f kshutdown-portable/kshutdown.ini
7z a "kshutdown-portable-${KS_FILE_VERSION}-win32.7z" kshutdown-portable
