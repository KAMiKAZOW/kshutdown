// pureqt.h - Compile for Qt only or for KDE
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

#ifndef KSHUTDOWN_PUREQT_H
#define KSHUTDOWN_PUREQT_H

#ifdef KS_PURE_QT
	#define U(name) <Q##name>
#else
	#define U(name) <K##name>
#endif // KS_PURE_QT

#include U(Action)
#include U(Application)
#include U(ComboBox)
#include U(Debug)
#include U(Dialog)
#include U(DialogButtonBox)
#include U(Icon)
#include U(MainWindow)
#include U(Menu)
#include U(MenuBar)
#include U(MessageBox)
#include U(PushButton)
#include U(SystemTrayIcon)

#define U_DIALOG QDialog

#ifdef KS_PURE_QT
	#undef KS_NATIVE_KDE

	#define U_ACTION QAction
	#define U_APP qApp
	#define U_COMBO_BOX QComboBox
	#define U_CONFIRM(parent, title, text) \
		(QMessageBox::question( \
			(parent), \
			(title), \
			(text), \
			QMessageBox::Ok | QMessageBox::Cancel, \
			QMessageBox::Ok \
		) == QMessageBox::Ok)

	#define U_DEBUG qDebug()
	#define U_DIALOG_BUTTON_BOX QDialogButtonBox
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
	#define U_STOCK_ICON(name) QIcon::fromTheme((name))
	#define U_SYSTEM_TRAY QSystemTrayIcon

	#define i18n(text) QApplication::translate(0, (text))
#else
	#include <KLocale> // for i18n

	#define KS_NATIVE_KDE

	#define U_ACTION KAction
	#define U_APP kapp
	#define U_COMBO_BOX KComboBox
	#define U_CONFIRM(parent, title, text) \
		(KMessageBox::questionYesNo( \
			(parent), \
			(text), \
			(title) \
		) == KMessageBox::Yes)
	#define U_DEBUG kDebug()
	#define U_DIALOG_BUTTON_BOX KDialogButtonBox
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
	
	// use i18n from KLocale
#endif // KS_PURE_QT

#endif // KSHUTDOWN_PUREQT_H
