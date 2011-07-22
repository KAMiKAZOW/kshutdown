// lock.cpp - Lock
// Copyright (C) 2009  Konrad Twardowski
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

#include "lock.h"
#include "../utils.h"

#ifdef Q_WS_WIN
	#define _WIN32_WINNT 0x0500 // for LockWorkStation, etc
	#include <windows.h>
#else
	#include <QDBusInterface>
	#include <QDesktopWidget>
#endif // Q_WS_WIN

// LockAction

// private

LockAction *LockAction::m_instance = 0;
#ifdef Q_WS_X11
QDBusInterface *LockAction::m_qdbusInterface = 0;
#endif // Q_WS_X11

// public

bool LockAction::onAction() {
#ifdef Q_WS_WIN
	BOOL result = ::LockWorkStation();
	if (result == 0) {
		setLastError();

		return false;
	}

	return true;
#else
	// HACK: This is a workaround for "lazy" initial kscreensaver repaint.
	// Now the screen content is hidden immediately.
	QWidget blackScreen(
		0,
		Qt::FramelessWindowHint |
		Qt::WindowStaysOnTopHint |
		Qt::X11BypassWindowManagerHint |
		Qt::Tool
	);
	// set black background color
	blackScreen.setAutoFillBackground(true);
	QPalette p;
	p.setColor(QPalette::Window, Qt::black);
	blackScreen.setPalette(p);
	// set full screen size
	QRect screenGeometry = QApplication::desktop()->screenGeometry();
	blackScreen.resize(screenGeometry.size());
	// show and force repaint
	blackScreen.show();
	QApplication::processEvents();

	// try DBus
	QDBusInterface *dbus = getQDBusInterface();
	dbus->call("Lock");
	QDBusError error = dbus->lastError();
	
	if (error.type() == QDBusError::NoError)
		return true;

	// try "xdg-screensaver" command
	QStringList args;
	args << "lock";
	if (launch("xdg-screensaver", args))
		return true;
		
	// try "gnome-screensaver-command" command
	if (Utils::isGNOME()) {
		args.clear();
		args << "--lock";
		if (launch("gnome-screensaver-command", args))
			return true;
	}
	
	// try "xscreensaver-command" command
	args.clear();
	args << "-lock";
	if (launch("xscreensaver-command", args))
		return true;

	// do not set "m_error" because it may block auto shutdown
	U_ERROR << "Could not lock the screen" U_END;
	
	return false;
#endif // Q_WS_WIN
}

#ifdef Q_WS_X11
QDBusInterface *LockAction::getQDBusInterface() {
	if (!m_qdbusInterface) {
		m_qdbusInterface = new QDBusInterface(
			"org.freedesktop.ScreenSaver",
			"/ScreenSaver",
			"org.freedesktop.ScreenSaver"
		);
		if (!m_qdbusInterface->isValid()) {
			delete m_qdbusInterface;
			
			U_DEBUG << "LockAction::getQDBusInterface(): using org.kde.krunner" U_END;
			m_qdbusInterface = new QDBusInterface(
				"org.kde.krunner",
				"/ScreenSaver",
				"org.freedesktop.ScreenSaver"
			);
		}
		else {
			U_DEBUG << "LockAction::getQDBusInterface(): using org.freedesktop.ScreenSaver" U_END;
		}
	}
	
	return m_qdbusInterface;
}
#endif // Q_WS_X11

// private

LockAction::LockAction() :
	Action(i18n("Lock Screen"), "system-lock-screen", "lock") {
	setShouldStopTimer(false);

	addCommandLineArg("k", "lock");
}
