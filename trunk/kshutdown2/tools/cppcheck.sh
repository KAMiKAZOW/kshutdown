#!/bin/bash

cppcheck \
	--enable=all \
	--force \
	src/*.cpp src/*.h \
	src/actions/*.cpp src/actions/*.h \
	src/triggers/*.cpp src/triggers/*.h
