// utils.cpp - Misc. utilities
// Copyright (C) 2008  Konrad Twardowski
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=417301
#include <cstdlib>

#include "utils.h"

// public

QString Utils::getUser() {
	QByteArray LOGNAME = QByteArray(::getenv("LOGNAME"));
	
	if (!LOGNAME.isNull())
		return QString(LOGNAME);

	QByteArray USER = QByteArray(::getenv("USER"));
	
	if (!USER.isNull())
		return QString(USER);
		
	return QString::null;
}

bool Utils::isGDM() {
	return ::getenv("GDMSESSION");
}

bool Utils::isGNOME() {
	return
		::getenv("GNOME_DESKTOP_SESSION_ID") ||
		(qstrcmp(::getenv("DESKTOP_SESSION"), "gnome") == 0);
}

bool Utils::isKDEFullSession() {
	return (qstrcmp(::getenv("KDE_FULL_SESSION"), "true") == 0);
}

bool Utils::isKDE_3() {
	return
		isKDEFullSession() &&
		(qstrcmp(::getenv("DESKTOP_SESSION"), "kde") == 0);
}

bool Utils::isKDE_4() {
	return
		isKDEFullSession() &&
		(
			(qstrcmp(::getenv("DESKTOP_SESSION"), "kde4") == 0) ||
			(qstrcmp(::getenv("KDE_SESSION_VERSION"), "4") == 0)
		);
}

bool Utils::isKDM() {
	return ::getenv("XDM_MANAGED");
}
