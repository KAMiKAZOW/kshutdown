//
// plainqt.h - Compile for Qt only or for KDE
// Copyright (C) 2007  Konrad Twardowski
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

#ifndef __PUREQT_H__
#define __PUREQT_H__

#ifdef KS_PURE_QT
	// common includes
	#include <QAction>
	#include <QDebug>
	#include <QMessageBox>
	#include <QSettings>

	#undef KS_NATIVE_KDE
	#define U_ACTION QAction
	#define U_COMBO_BOX QComboBox
	#define U_CONFIG QSettings
	#define U_DEBUG qDebug()
	#define U_ERROR qCritical()
	#define U_ERROR_MESSAGE(parent, text) \
		QMessageBox::critical((parent), i18n("Error"), (text));
	#define U_EXPORT Q_DECL_EXPORT
	#define U_ICON QIcon
	#define U_MAIN_WINDOW QMainWindow
	#define U_MENU QMenu
	#define U_MENU_BAR QMenuBar
	#define U_PUSH_BUTTON QPushButton
	#define U_SYSTEM_TRAY QSystemTrayIcon

// FIXME: fake i18n
	#define i18n QString

	// shutdown types

	typedef int UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT 1
	#define U_SHUTDOWN_TYPE_REBOOT 2
	#define U_SHUTDOWN_TYPE_HALT 3
#else
	// common includes
	#include <KAction>
	#include <KConfig>
	#include <KLocale>
	#include <KMessageBox>
	#include <kdemacros.h> // for KDE_EXPORT
	#include <libworkspace/kworkspace.h>

	#define KS_NATIVE_KDE
	#define U_ACTION KAction
	#define U_COMBO_BOX KComboBox
	#define U_CONFIG KConfig
	#define U_DEBUG() kDebug()
	#define U_ERROR kError()
	#define U_ERROR_MESSAGE(parent, text) \
		KMessageBox::error((parent), (text));
	#define U_EXPORT KDE_EXPORT
	#define U_ICON KIcon
	#define U_MAIN_WINDOW KMainWindow
	#define U_MENU KMenu
	#define U_MENU_BAR KMenuBar
	#define U_PUSH_BUTTON KPushButton

	// shutdown types

	typedef KWorkSpace::ShutdownType UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT KWorkSpace::ShutdownTypeNone
	#define U_SHUTDOWN_TYPE_REBOOT KWorkSpace::ShutdownTypeReboot
	#define U_SHUTDOWN_TYPE_HALT KWorkSpace::ShutdownTypeHalt
#endif // KS_PURE_QT

#endif // __PUREQT_H__
