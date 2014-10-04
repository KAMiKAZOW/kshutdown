#!/bin/bash

if [ ! -f "./tools/make-version.sh" ]; then
	echo "Usage: ./tools/`basename $0`"
	exit 1
fi

# make version number

./tools/make-version.sh
KS_FILE_VERSION=`sed 1!d tools/VERSION`

# init variables

KS_DIR="kshutdown-$KS_FILE_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-source-$KS_FILE_VERSION.zip"

# clean before copy

rm -f ./kshutdown
rm -f ./kshutdown-qt
rm -fR "$KS_DIR"
rm -fR "$KS_DIST_DIR"

pushd "src"
make clean
rm ./kshutdown
rm ./kshutdown-qt
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

cp -r ".kdev4" "$KS_DIR"
cp -r "src" "$KS_DIR"
cp -r "tools" "$KS_DIR"

# zip and checksum

mkdir "$KS_DIST_DIR"
zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR" -x "*~" -x "*/.svn/*" -x "*/src/extras/*" -x "*/src/kshutdown.ini"
pushd "$KS_DIST_DIR"
sha1sum "$KS_ZIP">SHA1SUM
popd
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo
