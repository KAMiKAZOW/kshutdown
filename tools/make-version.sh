#!/bin/bash

KS_BUILD=`date +%Y%m%d`
KS_VERSION=`cat tools/VERSION`

cat > "src/version.h" <<EOF
// Generated by $0, do not modify!
#ifndef KSHUTDOWN_VERSION_H
#define KSHUTDOWN_VERSION_H

#define KS_BUILD "$KS_BUILD"
#define KS_VERSION "$KS_VERSION"

#endif // KSHUTDOWN_VERSION_H
EOF