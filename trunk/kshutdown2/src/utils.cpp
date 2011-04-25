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

#include <QWidget>

#ifdef KS_NATIVE_KDE
	#include <KAuthorized>
	#include <KCmdLineArgs>
#endif // KS_NATIVE_KDE

#include "utils.h"

// private

#ifdef KS_NATIVE_KDE
	KCmdLineArgs *Utils::m_args = 0;
#else
	QStringList Utils::m_args;
#endif // KS_NATIVE_KDE
QProcessEnvironment Utils::m_env = QProcessEnvironment::systemEnvironment();

// public

QString Utils::getOption(const QString &name) {
#ifdef KS_NATIVE_KDE
	return m_args->getOption(name.toAscii());
#else
	int i = m_args.indexOf('-' + name, 1);
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

QString Utils::getTimeOption() {
#ifdef KS_NATIVE_KDE
	if (m_args->count())
		return m_args->arg(0);
	
	return QString::null;
#else
	if (m_args.size() > 2) {
		QString timeOption = m_args.last();
		
		if (!timeOption.isEmpty() && (timeOption.at(0) != '-')) {
			//U_DEBUG << timeOption U_END;
			
			return timeOption;
		}
	}
	
	return QString::null;
#endif // KS_NATIVE_KDE
}

QString Utils::getUser() {
	QString LOGNAME = m_env.value("LOGNAME");
	
	if (!LOGNAME.isEmpty())
		return LOGNAME;

	QString USER = m_env.value("USER");
	
	if (!USER.isEmpty())
		return USER;
		
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
	return (m_args.contains('-' + name) || m_args.contains("--" + name));
#endif // KS_NATIVE_KDE
}

bool Utils::isHelpArg() {
#ifdef KS_NATIVE_KDE
	return false; // "--help" argument handled by KDE
#else
	return (m_args.contains("/?") || isArg("help"));
#endif // KS_NATIVE_KDE
}

bool Utils::isGDM() {
	return m_env.contains("GDMSESSION");
}

bool Utils::isGNOME() {
	return
		m_env.contains("GNOME_DESKTOP_SESSION_ID") ||
		(m_env.value("DESKTOP_SESSION") == "gnome");
}

bool Utils::isKDEFullSession() {
	return m_env.value("KDE_FULL_SESSION") == "true";
}

bool Utils::isKDE_4() {
	return
		isKDEFullSession() &&
		(
			(m_env.value("DESKTOP_SESSION") == "kde4") ||
			(m_env.value("KDE_SESSION_VERSION").toInt() >= 4)
		);
}

bool Utils::isKDM() {
	return m_env.contains("XDM_MANAGED");
}

bool Utils::isRestricted(const QString &action) {
#ifdef KS_NATIVE_KDE
	return !KAuthorized::authorize(action);
#else
	return false;
#endif // KS_NATIVE_KDE
}

bool Utils::isXfce() {
	return m_env.value("DESKTOP_SESSION") == "xfce";
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
