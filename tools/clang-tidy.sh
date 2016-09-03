#!/bin/bash

# DOC: http://clang.llvm.org/extra/clang-tidy/index.html

if [ ! -d "src" ]; then
	echo "Usage: ./tools/clang-tidy.sh"
	exit 1
fi

# TODO: fix and/or disable irrelevant warnings

CHECKS="*"\
",-google-readability-braces-around-statements,-google-readability-todo"\
",-llvm-header-guard"\
",-misc-definitions-in-headers,-misc-unused-parameters"\
",-readability-braces-around-statements"

clang-tidy-3.8 src/*.cpp src/actions/*.cpp src/triggers/*.cpp -checks $CHECKS -p build.tmp -- \
	-DKS_KF5 -DKS_NATIVE_KDE \
	-I/usr/include/KDE \
	-I/usr/include/qt4 \
	-I/usr/include/qt4/QtGui \
\;

clang-tidy-3.8 src/*.cpp src/actions/*.cpp src/triggers/*.cpp -checks $CHECKS -p build.tmp -- \
	-DKS_PURE_QT \
	-I/usr/include/KDE \
	-I/usr/include/qt4 \
	-I/usr/include/qt4/QtGui \
\;
