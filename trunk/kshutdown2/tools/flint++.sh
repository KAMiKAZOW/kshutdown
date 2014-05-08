#!/bin/bash

if [ ! -d "src" ]; then
	echo "Usage: ./tools/flint++.sh"
	exit 1
fi

echo "Hint: https://github.com/L2Program/FlintPlusPlus"

# TODO: exclude moc_* and qrc_* files

flint++ \
	--level 2 \
	--recursive \
	src
