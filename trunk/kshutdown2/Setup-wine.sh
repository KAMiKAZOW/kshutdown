#!/bin/bash

set -e

wineconsole Setup-qt4.bat

rm -f kshutdown-portable/kshutdown.ini
zip -r9 kshutdown-portable-3.0beta5-win32.zip kshutdown-portable
7z a kshutdown-portable-3.0beta5-win32.7z kshutdown-portable
