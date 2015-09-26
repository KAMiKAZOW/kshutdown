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

ks_info "Compiled program can be found in \"$(pwd)/src\" directory (no installation required)"
echo "Application file:"
ls -lh src/kshutdown-qt

echo
echo "TIP: Run \"make install\" to install KShutdown"
echo "     (may require administrator privileges). Examples:"
echo "     cd src; sudo make install  (Ubuntu, etc.)"
echo "     cd src; su -c \"make install\"  (Fedora, etc.)"
