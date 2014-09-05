#!/bin/bash

# DOC: http://clang-analyzer.llvm.org/

if [ ! -d "src" ]; then
	echo "Usage: ./tools/scan-build.sh"
	exit 1
fi

pushd src
make clean
scan-build --view make
popd
