#!/bin/bash

#

# !!!

echo "SORRY, UNDER CONSTRUCTION!"
exit 1

pushd "src"
QMAKE=`which qmake-qt4`
if [ -z "$QMAKE" ]; then
	QMAKE=qmake
fi

set -e
$QMAKE -config release
make clean
make
popd
