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
#endif // Q_WS_WIN

// LockAction

// private

LockAction *LockAction::m_instance = 0;

// public

bool LockAction::onAction() {
#ifdef Q_WS_WIN
	BOOL result = ::LockWorkStation();
	if (result == 0) {
// TODO: test error message
		setLastError();

		return false;
	}

	return true;
#else
	// try DBus
	QDBusInterface i("org.freedesktop.ScreenSaver", "/ScreenSaver");
	i.call("Lock");
	QDBusError error = i.lastError();
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

// private

LockAction::LockAction() :
	Action(i18n("Lock Screen"), "system-lock-screen", "lock") {
	setShouldStopTimer(false);

	addCommandLineArg("k", "lock");
}
