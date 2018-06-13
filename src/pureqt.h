// pureqt.h - Allows you to compile for Qt-only or for native KDE
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

// krazy:excludeall=qclasses

#ifndef KSHUTDOWN_PUREQT_H
#define KSHUTDOWN_PUREQT_H

//#define KS_V5

#ifdef KS_PURE_QT

// Q-Files
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

#else

	#ifdef KS_KF5
		#include <QApplication>
		#include <QDebug>
		#include <QMessageBox>
	#else
		// #kde4
		#include <KApplication>
		#include <KDebug>
		#include <KMessageBox>
	#endif // KS_KF5

#endif // KS_PURE_QT

// HACK: Q_OS_FREEBSD undefined (?) <http://sourceforge.net/p/kshutdown/bugs/18/>
#if defined(Q_OS_LINUX) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(Q_OS_HURD)
	#define KS_DBUS
#endif

#if defined(Q_OS_LINUX) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(Q_OS_HURD) || defined(Q_OS_HAIKU)
	#define KS_UNIX
#endif

#ifdef KS_PURE_QT
	#undef KS_KF5
	#undef KS_NATIVE_KDE

	#define U_CONFIRM(parent, title, text) \
		(QMessageBox::question( \
			(parent), \
			(title), \
			(text), \
			QMessageBox::Ok | QMessageBox::Cancel, \
			QMessageBox::Ok \
		) == QMessageBox::Ok)
	#define U_ERROR_MESSAGE(parent, text) \
		QMessageBox::critical((parent), i18n("Error"), (text));
	#define U_INFO_MESSAGE(parent, text) \
		QMessageBox::information((parent), i18n("Information"), (text));

	#define U_APP qApp
	#define U_DEBUG qDebug()
	#define U_END
	#define U_ERROR qCritical()

	#define i18n(text) QApplication::translate(0, (text))
#else
	#define KS_NATIVE_KDE

	#ifdef KS_KF5
		#include <KLocalizedString> // for i18n

		#define U_CONFIRM(parent, title, text) \
			(QMessageBox::question( \
				(parent), \
				(title), \
				(text), \
				QMessageBox::Ok | QMessageBox::Cancel, \
				QMessageBox::Ok \
			) == QMessageBox::Ok)
		#define U_ERROR_MESSAGE(parent, text) \
			QMessageBox::critical((parent), i18n("Error"), (text));
		#define U_INFO_MESSAGE(parent, text) \
			QMessageBox::information((parent), i18n("Information"), (text));

		#define U_APP qApp
		#define U_DEBUG qDebug()
		#define U_END
		#define U_ERROR qCritical()
	#else
		#include <KLocale> // for i18n

		#define U_CONFIRM(parent, title, text) \
			(KMessageBox::questionYesNo( \
				(parent), \
				(text), \
				(title) \
			) == KMessageBox::Yes)
		#define U_ERROR_MESSAGE(parent, text) \
			KMessageBox::error((parent), (text));
		#define U_INFO_MESSAGE(parent, text) \
			KMessageBox::information((parent), (text));

		#define U_APP kapp
		#define U_DEBUG kDebug()
		#define U_END << endl
		#define U_ERROR kError()
	#endif // KS_KF5

	// use i18n from KLocale
#endif // KS_PURE_QT

#endif // KSHUTDOWN_PUREQT_H
