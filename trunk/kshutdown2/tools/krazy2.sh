#!/bin/bash

# SEE: https://github.com/Krazy-collection/krazy
# DOC: https://community.kde.org/Guidelines_and_HOWTOs/Code_Checking

if [ ! -f "ChangeLog" ]; then
	echo "Usage: ./tools/krazy2.sh [directory to test]"
	echo "Example: ./tools/krazy2.sh src"
	exit 1
fi

export PATH=$PATH:/usr/local/Krazy2/bin

dir="$1"
if [ -z "$dir" ]; then
	dir="."
fi

pushd "$dir"
krazy2all --check-sets c++,foss,kde5,qt5
popd
