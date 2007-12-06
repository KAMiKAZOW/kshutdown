#!/bin/bash

if [ ! -f "./tools/make-version.sh" ]; then
	echo "Usage: $0"
	exit 1
fi

# version
./tools/make-version.sh

KS_VERSION=`cat tools/VERSION`
KS_DIR="kshutdown-$KS_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-src-$KS_VERSION.zip"

# clean
rm -fR "$KS_DIR"
rm -fR "$KS_DIST_DIR"

cd src
make clean
cd ..

# /
mkdir "$KS_DIR"
cp ChangeLog CMakeLists.txt kshutdown.nsi LICENSE README.html TODO *.bat *.sh "$KS_DIR"

# /nbproject
#mkdir "$KS_DIR/nbproject"
#cp nbproject/* "$KS_DIR/nbproject"

# /src
mkdir "$KS_DIR/src"
cp src/CMakeLists.txt src/kshutdown.qrc src/src.pro src/*.cpp src/*.h "$KS_DIR/src"

# /src/images
mkdir "$KS_DIR/src/images"
cp src/images/*.ico src/images/*.png src/images/*.svg "$KS_DIR/src/images"

# /tools
mkdir "$KS_DIR/tools"
cp tools/VERSION tools/*.sh "$KS_DIR/tools"

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
