#!/bin/bash

if [ ! -d "src" ]; then
	echo "Usage: ./tools/$(basename $0)"
	exit 1
fi

# make version number

./tools/make-version.sh
KS_FILE_VERSION=$(sed 1!d VERSION)

# init variables

KS_DIR="kshutdown-$KS_FILE_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-source-$KS_FILE_VERSION.zip"

# clean before copy

rm -f ./kshutdown-*-win32.exe ./kshutdown-portable-*-win32.7z
rm -fR ./kshutdown-portable
rm -fR ./src/debug ./src/release
rm -f ./src/.qmake.stash \
	./src/Makefile.Debug ./src/Makefile.Release \
	./src/object_script.kshutdown.Debug ./src/object_script.kshutdown.Release
rm -fR "$KS_DIR"
rm -fR "$KS_DIST_DIR"

pushd "src"
make clean
rm ./kshutdown
rm ./Makefile
popd

# copy source files

mkdir "$KS_DIR"
cp * "$KS_DIR"

#mkdir "$KS_DIR/api"
#cp API.tmp/html/* "$KS_DIR/api"

cp -r "patches" "$KS_DIR"

rm -f po/*.po~
cp -r "po" "$KS_DIR"

cp -r "src" "$KS_DIR"
cp -r "tools" "$KS_DIR"

# zip and checksum

mkdir "$KS_DIST_DIR"
zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR" -x "*~" -x "*/.svn/*" -x "*/src/extras/*" -x "*/src/kshutdown.ini"
pushd "$KS_DIST_DIR"
sha256sum "$KS_ZIP">SHA256SUM
popd
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo
