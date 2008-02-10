#!/bin/bash

if [ ! -f "./tools/make-version.sh" ]; then
	echo "Usage: $0"
	exit 1
fi

# make version number

./tools/make-version.sh
KS_VERSION=`cat tools/VERSION`

# init variables

KS_DIR="kshutdown-$KS_VERSION"
KS_DIST_DIR="dist.tmp"
KS_ZIP="kshutdown-src-$KS_VERSION.zip"

# clean before copy

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
cp -r "patches" "$KS_DIR"
cp -r "src" "$KS_DIR"
cp -r "tools" "$KS_DIR"

# zip and checksum

mkdir "$KS_DIST_DIR"
zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR" -x "*~" -x "*/.svn/*"
pushd "$KS_DIST_DIR"
sha1sum "$KS_ZIP">SHA1SUM
popd
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo
