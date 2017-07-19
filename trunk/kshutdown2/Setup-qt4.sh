#!/bin/bash

function ks_info()
{
	echo
	echo "INFO: $1"
	echo
}

pushd "src"

if [ "$1" == "-qt5" ]; then
	QMAKE=qmake
else
	QMAKE=$(which qmake-qt4)
	if [ -z "$QMAKE" ]; then
		QMAKE=qmake
	fi
fi

set -e

ks_info "Configuring..."
if [ "$1" == "-qt5" ]; then
# HACK: workaround for error:
# qmake: could not exec '/usr/lib/x86_64-linux-gnu/qt4/bin/qmake': No such file or directory
	$QMAKE -qt5 -config release
else
	$QMAKE -config release
fi

ks_info "Cleaning..."
make clean

ks_info "Compiling..."
make -j2

popd
