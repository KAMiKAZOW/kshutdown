#!/bin/bash

if [ ! -f "./tools/api.sh" ]; then
	echo "Usage: ./tools/`basename $0`"
	exit 1
fi

rm -rf API.tmp

doxygen
