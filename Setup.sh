#!/bin/bash

default_item=""
kshutdown_full_version=`sed 2!d tools/VERSION`

function doError()
{
	echo
	echo "ERROR: $1"
	echo
	exit 1
}

function doCompile()
{
	clear
	if [ $1 == "kde4" ]; then
		if ./Setup-kde4.sh; then
			pushd build.tmp
			echo
			echo "INFO: Enter the \"root\" password to install KShutdown:"
			echo
			sudo make install
			popd
		else
			doError "Build failed. See README.html for troubleshooting information."
		fi
	elif [ $1 == "qt4" ]; then
		if ./Setup-qt4.sh; then
			dialog \
				--msgbox \
"Compiled KShutdown program (\"kshutdown\" file)\n
can be found in the \"${PWD}\" directory.
\n\nNo installation required." \
				0 0
			./src/kshutdown
		else
			doError "Build failed. See README.html for troubleshooting information."
		fi
	else
		doError "Unknown build type: $1"
	fi
}

function doQuit()
{
	clear
}

if [[ $DESKTOP_SESSION == "kde4" || $DESKTOP_SESSION == "kde" || $DESKTOP_SESSION == "kde-plasma" ]]; then
	default_item="kde4"
else
	default_item="qt4"
fi

out=`dialog \
	--backtitle "KShutdown $kshutdown_full_version Setup" \
	--default-item "$default_item" \
	--ok-label "Compile" \
	--stdout \
	--title "Select KShutdown version to build:" \
	--menu "" 0 0 0 \
	"kde4" "KDE 4 - better integration with KDE 4 + extra features" \
	"qt4"  "Qt 4  - does NOT require KDE 4 libraries"`

case $? in
	0) doCompile $out;;
	*) doQuit;;
esac
