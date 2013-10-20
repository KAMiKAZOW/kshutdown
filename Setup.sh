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
	if [ $1 == "kshutdown" ]; then
		if ! ./Setup-kde4.sh; then
			doError "Build failed. See README.html for troubleshooting information."
		fi
	elif [ $1 == "kshutdown-qt" ]; then
		if ./Setup-qt4.sh; then
			dialog \
				--msgbox \
"Compiled KShutdown program (\"kshutdown-qt\" file)\n
can be found in the \"${PWD}\" directory.\n
\n
Installation is not required.\n
However, you can run \"cd src; sudo make install\"\n
to setup menu shortcut (Utilities section),\n
and copy \"kshutdown-qt\" to the \"/usr/bin\" directory." \
				0 0
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

if [[ $DESKTOP_SESSION == *kde* ]]; then
	default_item="kshutdown"
else
	default_item="kshutdown-qt"
fi

out=`dialog \
	--backtitle "KShutdown $kshutdown_full_version Setup" \
	--default-item "$default_item" \
	--ok-label "OK, compile!" \
	--cancel-label "Maybe later" \
	--item-help \
	--no-lines \
	--no-shadow \
	--stdout \
	--title "Select a KShutdown version to build:" \
	--menu "" 0 0 0 \
	"kshutdown" "Version for KDE 4 with additional features" "Required libraries: Qt 4.8+, KDE 4 libs" \
	"kshutdown-qt"  "Version for Xfce, LXDE, MATE, KDE, etc. - lightweight" "Required libraries: Qt 4.8+ or Qt 5.x, NO KDE 4 libs"`

case $? in
	0) doCompile $out;;
	*) doQuit;;
esac
