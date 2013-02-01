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
QString Utils::m_desktopSession;
QString Utils::m_xdgCurrentDesktop;

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
	m_desktopSession = m_env.value("DESKTOP_SESSION");
	m_xdgCurrentDesktop = m_env.value("XDG_CURRENT_DESKTOP");
}

void Utils::initArgs() {
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

bool Utils::isEnlightenment() {
	return m_desktopSession.contains("enlightenment", Qt::CaseInsensitive);
}

bool Utils::isHelpArg() {
#ifdef KS_NATIVE_KDE
	return false; // "--help" argument handled by KDE
#else
	return (m_args.contains("/?") || isArg("help"));
#endif // KS_NATIVE_KDE
}

// TODO: test me
bool Utils::isGNOME() {
	return
		m_desktopSession.contains("gnome", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("gnome", Qt::CaseInsensitive);
}

// FIXME: test GNOME 2
bool Utils::isGNOME_3() {
	bool g3 =
		m_desktopSession.contains("gnome-shell", Qt::CaseInsensitive) ||
		m_desktopSession.contains("gnome-classic", Qt::CaseInsensitive) ||
		m_desktopSession.contains("gnome-fallback", Qt::CaseInsensitive);
	
	return g3 ? true : isUnity();
}

bool Utils::isGTKStyle() {
	#ifdef Q_OS_HAIKU
	return true;
	#else
	return isGNOME() || isGNOME_3() || isLXDE() || isXfce() || isUnity();
	#endif // Q_OS_HAIKU
}

bool Utils::isHaiku() {
	#ifdef Q_OS_HAIKU
	return true;
	#else
	return false;
	#endif // Q_OS_HAIKU
}

bool Utils::isKDEFullSession() {
	return m_env.value("KDE_FULL_SESSION") == "true";
}

bool Utils::isKDE_4() {
	return
		isKDEFullSession() &&
		(
			m_desktopSession.contains("kde", Qt::CaseInsensitive) ||
			m_xdgCurrentDesktop.contains("kde", Qt::CaseInsensitive) ||
			(m_env.value("KDE_SESSION_VERSION").toInt() >= 4)
		);
}

bool Utils::isLXDE() {
	return
		m_desktopSession.contains("LXDE", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("LXDE", Qt::CaseInsensitive);
}

bool Utils::isRestricted(const QString &action) {
#ifdef KS_NATIVE_KDE
	return !KAuthorized::authorize(action);
#else
	Q_UNUSED(action)

	return false;
#endif // KS_NATIVE_KDE
}

bool Utils::isUnity() {
	return
		m_desktopSession.contains("UBUNTU", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("UNITY", Qt::CaseInsensitive);
}

bool Utils::isXfce() {
	return
		m_desktopSession.contains("xfce", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("xfce", Qt::CaseInsensitive);
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

QString Utils::trim(QString &text, const int maxLength) {
	if (text.length() > maxLength) {
		text.truncate(maxLength);
		text = text.trimmed();
		text.append("...");
	}
	
	return text;
}
