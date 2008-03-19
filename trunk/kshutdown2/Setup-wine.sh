#!/bin/bash

set -e

wineconsole Setup-win32.bat
zip -r9 kshutdown-portable-2.0alpha5.zip kshutdown-portable
