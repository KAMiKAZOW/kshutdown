#!/bin/bash

function ks_copy()
{
	mkdir "$KS_DIR$1"
	cp .$1/* "$KS_DIR$1"
}

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

pushd "src"
make clean
rm ./kshutdown
rm ./Makefile
popd

ks_copy ""
ks_copy "/src"
ks_copy "/src/images"
ks_copy "/tools"

# zip and checksum
mkdir "$KS_DIST_DIR"
zip -r9 "$KS_DIST_DIR/$KS_ZIP" "$KS_DIR" -x "*~"
pushd "$KS_DIST_DIR"
sha1sum "$KS_ZIP">SHA1SUM
popd
rm -fR "$KS_DIR"

echo
echo "HINT: See \"dist.tmp\" directory..."
echo
