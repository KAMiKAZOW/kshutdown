#!/bin/bash

function ks_info()
{
	echo
	echo "INFO: $1"
	echo
}

pushd "src"

set -e

ks_info "Configuring..."
qmake -config release

ks_info "Cleaning..."
make clean

ks_info "Compiling..."
make -j2

popd
