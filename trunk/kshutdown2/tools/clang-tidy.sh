#!/bin/bash

# DOC: https://clang.llvm.org/extra/clang-tidy/

if [ ! -d "src" ]; then
	echo "Usage: ./tools/clang-tidy.sh"
	exit 1
fi

CHECKS="*"\
",-cppcoreguidelines-owning-memory,-cppcoreguidelines-pro-type-static-cast-downcast,-cppcoreguidelines-pro-type-vararg"\
",-fuchsia-default-arguments"\
",-google-readability-braces-around-statements,-google-readability-todo"\
",-hicpp-braces-around-statements,-hicpp-vararg"\
",-readability-braces-around-statements"\

run-clang-tidy \
	-checks "$CHECKS" \
	-j2 \
	-p build.tmp
