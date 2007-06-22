#!/bin/sh

PREFIX=`kde4-config --prefix`
if [ -z "$PREFIX" ]; then
	PREFIX=/usr/local
fi

BUILD_DIR="build.tmp"
rm -fR "$BUILD_DIR"
mkdir "$BUILD_DIR"

cd "$BUILD_DIR"

if ! cmake -DCMAKE_INSTALL_PREFIX="$PREFIX" ..; then
	exit 1
fi

if make; then
	kdesu -c "make install"
fi

cd ..
