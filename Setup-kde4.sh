#!/bin/bash

echo
echo "TIP: Run \"$0 /your/prefix/dir\" to specify custom installation directory"
echo

KDE4_CONFIG=$(which kde4-config)
PREFIX="$1"
BUILD_TYPE="$2"

if [ -z "$PREFIX" ]; then
	if [ -z "$KDE4_CONFIG" ]; then
		PREFIX=/usr/local
		echo "WARNING: \"kde4-config\" not found; using default installation prefix: $PREFIX"
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
if [ -n "$KDE4_CONFIG" ]; then
	KDESU="`$KDE4_CONFIG --path libexec`kdesu"
	if [ -x "$KDESU" ]; then
		echo "INFO: Installing KShutdown..."
		if ! "$KDESU" -n -t -c "make install"; then
			echo "INFO: Skipping installation"
		fi
	else
		echo "INFO: Skipping installation"
	fi
else
	echo
	echo "TIP: Run \"make install\" to install KShutdown"
	echo "     (may require administrator privileges). Examples:"
	echo "     cd build.tmp; sudo make install  (Ubuntu, etc.)"
	echo "     cd build.tmp; su -c \"make install\"  (Fedora, etc.)"
fi

popd
