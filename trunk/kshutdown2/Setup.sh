#!/bin/bash

if [ ! -x "$(which dialog)" ]; then
	echo "ERROR: This script requires 'dialog' package"
	exit 1
fi

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

function doBuildError()
{
	doError "Build failed. See README.html for troubleshooting information."
}

function doCompile()
{
	clear

	if [ "$1" == "kshutdown-kde4" ]; then
		if ./Setup-kde4.sh; then
			doSuccess "build.tmp" "./build.tmp/src/kshutdown"
		else
			doBuildError
		fi
	elif [ "$1" == "kshutdown-kf5" ]; then
		if ./Setup-kf5.sh; then
			doSuccess "build.tmp" "./build.tmp/src/kshutdown"
		else
			doBuildError
		fi
	elif [ "$1" == "kshutdown-qt4" ]; then
		if ./Setup-qt4.sh; then
			doSuccess "src" "./src/kshutdown-qt"
		else
			doBuildError
		fi
	elif [ "$1" == "kshutdown-qt5" ]; then
		if ./Setup-qt5.sh; then
			doSuccess "src" "./src/kshutdown-qt"
		else
			doBuildError
		fi
	elif [ "$1" == "kshutdown-qt4-win32" ]; then
		if ./Setup-wine.sh; then
			doSuccess "src" "./src/release/kshutdown-qt.exe"
		else
			doBuildError
		fi
	else
		doError "Unknown build type: $1"
	fi
}

function doSuccess()
{
	local text="1. Run \"$2\" to launch KShutdown without installation.\n\n"

	text+="2. Run \"make install\" to install KShutdown.\n"
	text+="   This will install program, menu shortcut (Utilities section), and icons.\n\n"
	text+="Examples:\n"
	text+="cd $1; sudo make install  (Ubuntu)\n"
	text+="cd $1; su -c \"make install\"  (Fedora)\n\n"
	text+="3. Run \"make uninstall\" to uninstall KShutdown.\n"

	dialog --msgbox "$text" 0 0
}

function doQuit()
{
	clear
}

# TEST:
#doSuccess "src" "./src/kshutdown-qt"
#doSuccess "build.tmp" "./build.tmp/src/kshutdown"
#exit

if [[ $DESKTOP_SESSION == "plasma" || $DESKTOP_SESSION == *kde* || $XDG_CURRENT_DESKTOP == *KDE* ]]; then
	default_item="kshutdown-kf5"
else
	default_item="kshutdown-qt5"
fi

out=$(dialog \
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
	"kshutdown-kf5" "An universal version for KDE Plasma and other desktop environments" "Required libraries: KDE Frameworks 5.x (KF5)" \
	"kshutdown-qt5" "A lightweight version for non-KDE desktop environments" "Required libraries: Qt 5.x only" \
	"kshutdown-kde4" "A classic version for KDE 4 with additional features (obsolete)" "Required libraries: Qt 4.8+, KDE 4 libs" \
	"kshutdown-qt4" "A lightweight version for non-KDE desktop environments (obsolete)" "Required libraries: Qt 4.8+ only" \
	"kshutdown-qt4-win32" "A lightweight version for Windows" "Required libraries: Qt 4.8+ only; compiled in Wine")
case $? in
	0) doCompile "$out";;
	*) doQuit;;
esac
