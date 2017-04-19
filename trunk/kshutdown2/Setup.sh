#!/bin/bash

# TODO: autorun shellcheck on all scripts

default_item=""
kshutdown_full_version=$(sed 2!d VERSION)

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
# TODO: common code
			dialog --msgbox \
"Compiled KShutdown program (\"kshutdown-qt\" file)\n
can be found in the \"${PWD}/src\" directory.\n
\n
Installation is not required.\n
However, you can run\n
\n
cd src; sudo make install  (Ubuntu, etc.)\n
or\n
cd src; su -c \"make install\"  (Fedora, etc.)\n
\n
to setup menu shortcut (Utilities section),\n
and copy \"kshutdown-qt\" to the \"/usr/bin\" directory." \
0 0
		else
			doError "Build failed. See README.html for troubleshooting information."
		fi
	elif [ $1 == "kshutdown-kf5" ]; then
		if ! ./Setup-kf5.sh; then
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

# TODO: $DESKTOP_SESSION == *plasma* (KF5)
if [[ $DESKTOP_SESSION == *kde* || $XDG_CURRENT_DESKTOP == *KDE* ]]; then
	default_item="kshutdown"
else
	default_item="kshutdown-qt"
fi

# TODO: update required libs info
out=`dialog \
	--backtitle "KShutdown $kshutdown_full_version Setup" \
	--default-item "$default_item" \
	--ok-label "OK, compile!" \
	--cancel-label "Maybe later" \
	--item-help \
	--no-lines \
	--no-shadow \
	--stdout \
	--title "Select a KShutdown Build (see README.html for more details)" \
	--menu "" 0 0 0 \
	"kshutdown" "A version for KDE 4 with additional features" "Required libraries: Qt 4.8+, KDE 4 libs" \
	"kshutdown-qt" "A lightweight version for non-KDE desktop environments" "Required libraries: Qt 4.8+ or Qt 5.x, no KDE 4 libs" \
	"kshutdown-kf5" "An universal version compiled using KDE Frameworks" "Required libraries: KDE Frameworks 5.x"`

case $? in
	0) doCompile $out;;
	*) doQuit;;
esac
