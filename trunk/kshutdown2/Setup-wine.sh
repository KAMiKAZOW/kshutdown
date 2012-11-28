#!/bin/bash

set -e

wineconsole Setup-qt4.bat

KS_FILE_VERSION=`sed 1!d tools/VERSION`
rm -f kshutdown-portable/kshutdown.ini
zip -r9 "kshutdown-portable-${KS_FILE_VERSION}-win32.zip" kshutdown-portable
7z a "kshutdown-portable-${KS_FILE_VERSION}-win32.7z" kshutdown-portable
