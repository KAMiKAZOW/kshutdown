#!/bin/bash

BIN=./src/kshutdown
LOG=REPORT-VALGRIND.txt

echo "Testing ${BIN} -> ${LOG}"

valgrind \
	--leak-check=full \
	--log-file="${LOG}" \
	--show-reachable=yes \
	--tool=memcheck \
	"${BIN}" -style fusion
