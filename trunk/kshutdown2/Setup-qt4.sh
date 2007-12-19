#!/bin/sh

set -e

# !!!

echo "SORRY, UNDER CONSTRUCTION!"
#exit 1

cd "src"
qmake-qt4
make clean
make
cd ..
