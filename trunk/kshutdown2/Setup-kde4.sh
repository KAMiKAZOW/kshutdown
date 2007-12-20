#!/bin/sh

# TODO: uninstall: http://www.cmake.org/Wiki/CMake_FAQ

set -e

PREFIX=`kde4-config --prefix`
if [ -z "$PREFIX" ]; then
	PREFIX=/usr/local
fi

BUILD_DIR="build.tmp"
rm -fR "$BUILD_DIR"
mkdir "$BUILD_DIR"

cd "$BUILD_DIR"

cmake -DCMAKE_INSTALL_PREFIX="$PREFIX" ..
make
	#!!! kde4
kdesu -c "make install"

cd ..
