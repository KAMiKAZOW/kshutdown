#!/bin/bash

set -e

# !!!

echo "SORRY, UNDER CONSTRUCTION!"
exit 1

pushd "src"
qmake-qt4
make clean
make
popd
