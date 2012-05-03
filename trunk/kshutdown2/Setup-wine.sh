#!/bin/bash

set -e

wineconsole Setup-qt4.bat
zip -r9 kshutdown-portable-3.0beta5-win32.zip kshutdown-portable
