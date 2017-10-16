#!/bin/bash

# DOC: http://clang.llvm.org/extra/clang-tidy/index.html

if [ ! -d "src" ]; then
	echo "Usage: ./tools/clang-tidy.sh"
	exit 1
fi

echo "Hint: 1. Set Clang as C++ compiler: export CXX=/usr/bin/clang++"
echo "      2. Reconfigure project: ./Setup-kf5.sh"

CHECKS="*"\
",-cert-err58-cpp"\
",-google-readability-braces-around-statements,-google-readability-todo"\
",-modernize-raw-string-literal"\
",-readability-braces-around-statements,-readability-implicit-bool-cast,-readability-redundant-declaration"
# NOTE: readability-redundant-declaration is false positive (?)

run-clang-tidy-4.0.py \
	-checks "$CHECKS" -j2 -p build.tmp \
	src/*.cpp src/actions/*.cpp src/triggers/*.cpp \
