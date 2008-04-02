#!/bin/bash

set -e

wineconsole Setup-qt4.bat
zip -r9 kshutdown-portable-2.0beta1.zip kshutdown-portable
