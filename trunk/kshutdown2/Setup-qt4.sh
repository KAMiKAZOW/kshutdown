#!/bin/sh

cd "src"
qmake-qt4
make clean
make
cd ..
