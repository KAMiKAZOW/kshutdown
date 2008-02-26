#!/bin/bash

# TODO: uninstall: http://www.cmake.org/Wiki/CMake_FAQ

echo "TIP: Run \"$0 /your/prefix/dir\" to specify custom installation directory"

PREFIX="$1"
BUILD_TYPE="$2"

if [ -z "$PREFIX" ]; then
	KDE4_CONFIG=$(which kde4-config)
	if [ -z "$KDE4_CONFIG" ]; then
		echo "WARNING: \"kde4-config\" not found; using default installation prefix"
		PREFIX=/usr/local
	else
		PREFIX=`$KDE4_CONFIG --prefix`
		if [ -z "$PREFIX" ]; then
			PREFIX=/usr/local
		fi
	fi
fi

if [ -z "$BUILD_TYPE" ]; then
	BUILD_TYPE=Release
fi

set -e

BUILD_DIR="build.tmp"
rm -fR "$BUILD_DIR"
mkdir "$BUILD_DIR"

pushd "$BUILD_DIR"

echo "INFO: Installation prefix: $PREFIX"
echo "INFO: Build type         : $BUILD_TYPE"

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX="$PREFIX" ..
make
"$PREFIX/lib/kde4/libexec/kdesu" -t -c "make install"

popd
