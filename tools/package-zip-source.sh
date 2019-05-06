#!/bin/bash

set -u

if [ ! -d "src" ]; then
	echo "Usage: ./tools/$(basename "$0")"
	exit 1
fi

# merge and compile translations

./tools/i18n.sh

# make version number

./tools/make-version.sh
KS_APP_VERSION=$(sed 1!d VERSION)

# init variables

KS_DIR="kshutdown-$KS_APP_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-source-$KS_APP_VERSION.zip"

# clean before copy

rm -f ./kshutdown-*-win32.exe ./kshutdown-portable-*-win32.7z
rm -fR ./kshutdown-portable
rm -fR ./src/debug ./src/release
rm -f \
	./src/portable.pri \
	./src/Makefile.Debug ./src/Makefile.Release \
	./src/object_script.kshutdown.Debug ./src/object_script.kshutdown.Release
rm -fR "$KS_DIR"
rm -fR "$KS_DIST_DIR"

pushd "src"
make clean
rm -f kshutdown Makefile .qmake.stash
popd

# copy source files

mkdir "$KS_DIR"
cp -- * .editorconfig "$KS_DIR"
cp -r "po" "$KS_DIR"
cp -r "src" "$KS_DIR"
cp -r "tools" "$KS_DIR"

# zip and checksum

mkdir "$KS_DIST_DIR"

zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR" \
	-x "*~" -x "*/src/extras/*" -x "*/src/kshutdown.ini" \
	-x "*/src/*.o" -x "*/src/moc_*.*" -x "*/src/qrc_*.*"

pushd "$KS_DIST_DIR"
sha256sum "$KS_ZIP">SHA256SUM
popd
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo
