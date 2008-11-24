#!/bin/bash

# validate "Desktop Entry" files

if [ ! -d "src" ]; then
	echo "Usage: ./tools/check-desktop-files.sh"
	exit 1
fi

find \( -name "*.desktop" -or -name ".directory" \) -exec desktop-file-validate {} \;
