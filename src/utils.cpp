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

#include "pureqt.h"

// http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=417301
#include <cstdlib>

#include <QWidget>

#ifdef KS_NATIVE_KDE
	#include <KCmdLineArgs>
#endif // KS_NATIVE_KDE

#include "utils.h"

// private

#ifdef KS_NATIVE_KDE
	KCmdLineArgs *Utils::m_args = 0;
#else
	QStringList Utils::m_args;
#endif // KS_NATIVE_KDE

// public

QString Utils::getOption(const QString &name) {
#ifdef KS_NATIVE_KDE
	return m_args->getOption(name.toAscii());
#else
	int i = m_args.indexOf("-" + name, 1);
	if (i == -1) {
		i = m_args.indexOf("--" + name, 1);
		if (i == -1) {
			U_DEBUG << "Argument not found: " << name U_END;

			return QString::null;
		}
	}

	int argIndex = (i + 1);

	if (argIndex < m_args.size()) {
		U_DEBUG << "Value of " << name << " is " << m_args[argIndex] U_END;

		return m_args[argIndex];
	}

	U_DEBUG << "Argument value is not set: " << name U_END;

	return QString::null;
#endif // KS_NATIVE_KDE
}

QString Utils::getUser() {
	QByteArray LOGNAME = QByteArray(::getenv("LOGNAME"));
	
	if (!LOGNAME.isNull())
		return QString(LOGNAME);

	QByteArray USER = QByteArray(::getenv("USER"));
	
	if (!USER.isNull())
		return QString(USER);
		
	return QString::null;
}

void Utils::init() {
#ifdef KS_NATIVE_KDE
	m_args = KCmdLineArgs::parsedArgs();
#else
	m_args = U_APP->arguments();
#endif // KS_NATIVE_KDE
}

bool Utils::isArg(const QString &name) {
#ifdef KS_NATIVE_KDE
	return m_args->isSet(name.toAscii());
#else
	return (m_args.contains("-" + name) || m_args.contains("--" + name));
#endif // KS_NATIVE_KDE
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
		(qstrcmp(::getenv("DESKTOP_SESSION"), "kde") == 0) &&
		(qstrcmp(::getenv("KDE_SESSION_VERSION"), "4") < 0);
}

bool Utils::isKDE_4() {
	return
		isKDEFullSession() &&
		(
			(qstrcmp(::getenv("DESKTOP_SESSION"), "kde4") == 0) ||
			(qstrcmp(::getenv("KDE_SESSION_VERSION"), "4") >= 0)
		);
}

bool Utils::isKDM() {
	return ::getenv("XDM_MANAGED");
}

void Utils::setFont(QWidget *widget, const int relativeSize, const bool bold) {
	QFont newFont(widget->font());
	if (bold)
		newFont.setBold(bold);
	int size = newFont.pointSize();
	if (size != -1) {
		newFont.setPointSize(qMax(8, size + relativeSize));
	}
	else {
		size = newFont.pixelSize();
		newFont.setPixelSize(qMax(8, size + relativeSize));
	}
	widget->setFont(newFont);
}

void Utils::shutDown() {
#ifdef KS_NATIVE_KDE
	if (m_args)
		m_args->clear();
#endif // KS_NATIVE_KDE
}
