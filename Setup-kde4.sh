#!/bin/bash

# TODO: uninstall: http://www.cmake.org/Wiki/CMake_FAQ

set -e

PREFIX=`kde4-config --prefix`
if [ -z "$PREFIX" ]; then
	PREFIX=/usr/local
fi

BUILD_DIR="build.tmp"
rm -fR "$BUILD_DIR"
mkdir "$BUILD_DIR"

BUILD_TYPE=Release

pushd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX="$PREFIX" ..
make
"$PREFIX/lib/kde4/libexec/kdesu" -t -c "make install"

popd
