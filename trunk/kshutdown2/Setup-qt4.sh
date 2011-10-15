#!/bin/bash

function ks_info()
{
	echo
	echo "INFO: $1"
	echo
}

pushd "src"

QMAKE=`which qmake-qt4`
if [ -z "$QMAKE" ]; then
	QMAKE=qmake
fi

set -e

ks_info "Configuring project..."
$QMAKE -config release

ks_info "Cleaning project..."
make clean

ks_info "Compiling... This may take a few minutes ;)"
make

popd

cp src/kshutdown-qt .
ks_info "See the \"$(pwd)\" directory; no installation required:"
ls -lh kshutdown-qt
echo