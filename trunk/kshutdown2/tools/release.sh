#!/bin/bash

# version
./tools/make-version.sh

KS_VERSION=`cat tools/VERSION`
KS_DIR="kshutdown-$KS_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-src-$KS_VERSION.zip"

# clean
rm -fR "$KS_DIR"
rm -fR "$KS_DIST_DIR"

if [ "$1" == "clean" ]; then
	exit
fi

# /
mkdir "$KS_DIR"
cp ChangeLog CMakeLists.txt kshutdown.nsi LICENSE README.html TODO *.bat *.sh "$KS_DIR"

# /src
mkdir "$KS_DIR/src"
cp src/CMakeLists.txt src/kshutdown.qrc src/src.pro src/*.cpp src/*.h "$KS_DIR/src"

# /src/images
mkdir "$KS_DIR/src/images"
cp src/images/*.png "$KS_DIR/src/images"

# /tools
mkdir "$KS_DIR/tools"
cp tools/*.sh tools/VERSION "$KS_DIR/tools"

# zip and checksum
mkdir "$KS_DIST_DIR"
zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR"
cd "$KS_DIST_DIR"
sha1sum "$KS_ZIP">SHA1SUM
cd ..
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo