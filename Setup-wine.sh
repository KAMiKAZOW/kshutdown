#!/bin/bash

set -e

wineconsole Setup-qt4.bat
zip -r9 kshutdown-portable-3.0beta3-win32.zip kshutdown-portable
