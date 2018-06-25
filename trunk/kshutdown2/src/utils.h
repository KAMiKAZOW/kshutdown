// utils.h - Misc. utilities
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

#ifndef KSHUTDOWN_UTILS_H
#define KSHUTDOWN_UTILS_H

#include "pureqt.h"

#include <QApplication>
#include <QMenu>
#include <QProcess>

#ifdef KS_PURE_QT
	#define i18n(text) QApplication::translate(0, (text))
#else
	#include <KLocalizedString> // for i18n
#endif // KS_PURE_QT

// TODO: use std::chrono and "hrs" literals #C++14 #experimental
inline int operator "" _px(unsigned long long int value) { return value; }

class Utils final {
public:
	static void addTitle(QMenu *menu, const QIcon &icon, const QString &text);
	static QString getUser();
	static void init();
	static bool isCinnamon();
	static bool isEnlightenment();
	static bool isGNOME();
	static bool isGTKStyle();
	static bool isHaiku();
	static bool isKDEFullSession();
	static bool isKDE();
	static bool isLXDE();
	static bool isLXQt();
	static bool isMATE();
	static bool isOpenbox();
	static bool isRazor();
	static bool isRestricted(const QString &action);
	static bool isTrinity();
	static bool isUnity();
	static bool isXfce();
	static QString read(QProcess &process, bool &ok);
	static void setFont(QWidget *widget, const int relativeSize, const bool bold);
	static void showMenuToolTip(QAction *action);
	static QString trim(QString &text, const int maxLength);
private:
	static QProcessEnvironment m_env;
	static QString m_desktopSession;
	static QString m_xdgCurrentDesktop;
	explicit Utils() { }
};

#endif // KSHUTDOWN_UTILS_H
