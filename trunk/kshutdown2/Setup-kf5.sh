#!/bin/bash

echo
echo "TIP: Run \"$0 /your/prefix/dir\" to specify custom installation directory"
echo

KF5_CONFIG=$(which kf5-config)
PREFIX="$1"
BUILD_TYPE="$2"

if [ -z "$PREFIX" ]; then
	if [ -z "$KF5_CONFIG" ]; then
		PREFIX=/usr/local
		echo "WARNING: \"kf5-config\" not found; using default installation prefix: $PREFIX"
	else
		PREFIX=$($KF5_CONFIG --prefix)
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
mkdir -p "$BUILD_DIR"

# HACK: use realpath, because automoc cannot find bootentry.h if KShutdown is compiled from a symlinked directory (?)
pushd "$(realpath $BUILD_DIR)"

echo "INFO: Installation prefix: $PREFIX"
echo "INFO: Build type         : $BUILD_TYPE"

cmake -DKS_KF5=true -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX="$PREFIX" ..
make -j2

popd
