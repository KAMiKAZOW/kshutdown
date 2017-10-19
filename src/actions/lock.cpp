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

#include "../config.h"
#include "../utils.h"

#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <QDesktopWidget>
#endif // Q_OS_WIN32

// LockAction

// private

LockAction *LockAction::m_instance = nullptr;
#ifdef KS_DBUS
QDBusInterface *LockAction::m_qdbusInterface = nullptr;
#endif // KS_DBUS

// public

bool LockAction::onAction() {
#ifdef Q_OS_WIN32
	BOOL result = ::LockWorkStation();
	if (result == 0) {
		setLastError();

		return false;
	}

	return true;
#else
// TODO: support custom command in other actions
	Config *config = Config::user();
	config->beginGroup("KShutdown Action lock");
	QString customCommand = config->read("Custom Command", "").toString();
	config->endGroup();

	customCommand = customCommand.trimmed();
	if (!customCommand.isEmpty()) {
		U_DEBUG << "Using custom lock command: " << customCommand U_END;
		int exitCode = QProcess::execute(customCommand);
		if (exitCode != 0)
			U_DEBUG << "Lock command failed to start" U_END;
		else
			return true;
	}

// TODO: support org.freedesktop.DisplayManager Lock

	// HACK: This is a workaround for "lazy" initial kscreensaver repaint.
	// Now the screen content is hidden immediately.
	QWidget *blackScreen = nullptr;
	if (Utils::isKDE()) {
		blackScreen = new QWidget(
			nullptr,
			Qt::FramelessWindowHint |
			Qt::NoDropShadowWindowHint |
			Qt::WindowDoesNotAcceptFocus |
			Qt::WindowStaysOnTopHint |
			Qt::X11BypassWindowManagerHint |
			Qt::Tool
		);
		// set black background color
		blackScreen->setAutoFillBackground(true);
		QPalette p;
		p.setColor(QPalette::Window, Qt::black);
		blackScreen->setPalette(p);
		// set full screen size
		QRect screenGeometry = QApplication::desktop()->screenGeometry();
		blackScreen->resize(screenGeometry.size());
		// show and force repaint
		blackScreen->show();
		QApplication::processEvents();
	}

// TODO: systemd/logind

	#ifdef KS_DBUS
	// try DBus
	QDBusInterface *dbus = getQDBusInterface();
	dbus->call("Lock");
	QDBusError error = dbus->lastError();
	#endif // KS_DBUS

	delete blackScreen;

	#ifdef KS_DBUS
	if (error.type() == QDBusError::NoError)
		return true;
	#endif // KS_DBUS
	
	QStringList args;

	// Cinnamon
	
	if (Utils::isCinnamon()) {
		args.clear();
		args << "--lock";
		if (launch("cinnamon-screensaver-command", args))
			return true;
	}

	// Unity, GNOME Shell
	
	if (Utils::isGNOME() || Utils::isUnity()) {
		args.clear();
		args << "--activate";
		if (launch("gnome-screensaver-command", args))
			return true;
	}

	// try "gnome-screensaver-command" command

	if (Utils::isGNOME()) {
		args.clear();
		args << "--lock";
		if (launch("gnome-screensaver-command", args))
			return true;
	}

	// MATE
	
	if (Utils::isMATE()) {
		args.clear();
		args << "--lock";
		if (launch("mate-screensaver-command", args))
			return true;
	}

	// try "xflock4"
	if (Utils::isXfce()) {
		args.clear();
		if (launch("xflock4", args))
			return true;
	}

	// LXDE

	if (Utils::isLXDE()) {
		args.clear();
		if (launch("lxlock", args))
			return true;
	}
	
	// Enlightenment

	if (Utils::isEnlightenment()) {
// TODO: use D-Bus
		args.clear();
		args << "-desktop-lock";
		if (launch("enlightenment_remote", args))
			return true;
	}

	// LXQt

	if (Utils::isLXQt()) {
		args.clear();
		args << "--lockscreen";
		if (launch("lxqt-leave", args))
			return true;
	}

	// Trinity

	if (Utils::isTrinity()) {
		args.clear();
		args << "kdesktop";
		args << "KScreensaverIface";
		args << "lock";
		if (launch("dcop", args))
			return true;
	}

	// try "xdg-screensaver" command
	args.clear();
	args << "lock";
	if (launch("xdg-screensaver", args))
		return true;
	
	// try "xscreensaver-command" command
	args.clear();
	args << "-lock";
	if (launch("xscreensaver-command", args))
		return true;

	// do not set "m_error" because it may block auto shutdown
	U_ERROR << "Could not lock the screen" U_END;
	
	return false;
#endif // Q_OS_WIN32
}

#ifdef KS_DBUS
QDBusInterface *LockAction::getQDBusInterface() {
	if (!m_qdbusInterface) {
		m_qdbusInterface = new QDBusInterface(
			"org.freedesktop.ScreenSaver",
			"/ScreenSaver",
			"org.freedesktop.ScreenSaver"
		);
		if (!m_qdbusInterface->isValid() && Utils::isKDE()) {
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
#endif // KS_DBUS

// private

LockAction::LockAction() :
	Action(i18n("Lock Screen"), "system-lock-screen", "lock") {
	
	setCanBookmark(true);
	setShouldStopTimer(false);

	addCommandLineArg("k", "lock");
	
	#ifdef Q_OS_HAIKU
	disable("");
	#endif // Q_OS_HAIKU
}
