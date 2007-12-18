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
	#include <QApplication>
	#include <QDebug>
	#include <QMessageBox>
	#include <QSettings>

	#undef KS_NATIVE_KDE

	#define U_ACTION QAction
	#define U_APP qApp
	#define U_COMBO_BOX QComboBox
	#define U_CONFIG QSettings
	//!!!singleton factory
	#define U_CONFIG_USER new QSettings()
	#define U_CONFIRM(parent, title, text) \
		(QMessageBox::question( \
			(parent), \
			(title), \
			(text), \
			QMessageBox::Ok | QMessageBox::Cancel, \
			QMessageBox::Ok \
		) == QMessageBox::Ok)
	#define U_DEBUG qDebug()
	#define U_DIALOG QDialog
	#define U_END
	#define U_ERROR qCritical()
	#define U_ERROR_MESSAGE(parent, text) \
		QMessageBox::critical((parent), i18n("Error"), (text));
	#define U_EXPORT Q_DECL_EXPORT
	#define U_ICON QIcon
	#define U_MAIN_WINDOW QMainWindow
	#define U_MENU QMenu
	#define U_MENU_BAR QMenuBar
	#define U_PUSH_BUTTON QPushButton
	#define U_STOCK_ICON(name) QIcon(":/images/" + QString((name)) + ".png")
	#define U_SYSTEM_TRAY QSystemTrayIcon

// !!!: fake i18n
	#define i18n QString

	// shutdown types

	typedef int UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT 1
	#define U_SHUTDOWN_TYPE_REBOOT 2
	#define U_SHUTDOWN_TYPE_HALT 3
#else
	// common includes
	#include <KAction>
	#include <KApplication>
	#include <KConfig>
	#include <KConfigGroup>
	#include <KDebug>
	#include <KLocale>
	#include <KMessageBox>
	#include <kdemacros.h> // for KDE_EXPORT
	#include <kworkspace/kworkspace.h>

	#define KS_NATIVE_KDE

	#define U_ACTION KAction
	#define U_APP kapp
	#define U_COMBO_BOX KComboBox
	#define U_CONFIG KConfig
	#define U_CONFIG_USER KGlobal::config().data()
	#define U_CONFIRM(parent, title, text) \
		(KMessageBox::questionYesNo( \
			(parent), \
			(text), \
			(title) \
		) == KMessageBox::Yes)
	#define U_DEBUG kDebug()
	#define U_DIALOG KDialog
	#define U_END << endl
	#define U_ERROR kError()
	#define U_ERROR_MESSAGE(parent, text) \
		KMessageBox::error((parent), (text));
	#define U_EXPORT KDE_EXPORT
	#define U_ICON KIcon
	#define U_MAIN_WINDOW KMainWindow
	#define U_MENU KMenu
	#define U_MENU_BAR KMenuBar
	#define U_PUSH_BUTTON KPushButton
	#define U_STOCK_ICON(name) KIcon((name))
	#define U_SYSTEM_TRAY KSystemTrayIcon

	// shutdown types

	typedef KWorkSpace::ShutdownType UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT KWorkSpace::ShutdownTypeNone
	#define U_SHUTDOWN_TYPE_REBOOT KWorkSpace::ShutdownTypeReboot
	#define U_SHUTDOWN_TYPE_HALT KWorkSpace::ShutdownTypeHalt
#endif // KS_PURE_QT

#endif // __PUREQT_H__
