#!/bin/bash

# DOC: https://clang-analyzer.llvm.org/scan-build.html

if [ ! -d "src" ]; then
	echo "Usage: ./tools/scan-build.sh"
	exit 1
fi

pushd src
make clean
scan-build make -j2
popd
