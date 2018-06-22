#!/bin/bash

# SEE: https://github.com/Krazy-collection/krazy
# DOC: https://community.kde.org/Guidelines_and_HOWTOs/Code_Checking

topdir="$1"

if [ ! -d "$topdir" ]; then
	echo "Usage: $0 <directory to test>"
	echo "Example: ./tools/krazy2.sh src"
	exit 1
fi

export PATH=$PATH:/usr/local/Krazy2/bin

krazy2all \
	--check-sets c++,foss,kde5,qt5 \
	--brief \
	--topdir "$topdir" \
	--verbose
