#!/bin/bash

BIN=./src/kshutdown
echo "Testing ${BIN}..."

valgrind \
	--leak-check=full \
	--show-reachable=yes \
	--tool=memcheck \
	"${BIN}"
