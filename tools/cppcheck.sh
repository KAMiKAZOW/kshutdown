#!/bin/bash

if [ ! -d "src" ]; then
	echo "Usage: ./tools/cppcheck.sh"
	exit 1
fi

# HACK: -i and --suppress option does not work as expected
rm -f src/moc_*.cpp src/moc_*.h src/qrc_kshutdown.cpp

cppcheck \
	--enable=all \
	--force \
	--language=c++ \
	src/*.cpp src/*.h \
	src/actions/*.cpp src/actions/*.h \
	src/triggers/*.cpp src/triggers/*.h
