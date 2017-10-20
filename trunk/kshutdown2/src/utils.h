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

#include <QProcess>

#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
		#include <QCommandLineParser>
	#else
		#include <KCmdLineArgs> // #kde4
	#endif // KS_KF5
#endif // KS_NATIVE_KDE

// TODO: use std::chrono and "hrs" literals #C++14 #experimental
inline int operator "" _px(unsigned long long int value) { return value; }

class Utils final {
public:
	static void addTitle(QMenu *menu, const QIcon &icon, const QString &text);
	static QString getOption(const QString &name);
	static QString getTimeOption();
	static QString getUser();
	static void init();
	static void initArgs();
	static bool isArg(const QString &name);
	static bool isCinnamon();
	static bool isEnlightenment();
	static bool isHelpArg();
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
	#ifdef KS_KF5
	static inline QCommandLineParser *parser() { return m_args; }
	static inline void setParser(QCommandLineParser *value) { m_args = value; }
	#endif // KS_KF5
	static QString read(QProcess &process, bool &ok);
	static void setFont(QWidget *widget, const int relativeSize, const bool bold);
	static void showMenuToolTip(QAction *action);
	static void shutDown();
	static QString trim(QString &text, const int maxLength);
private:
#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
	static QCommandLineParser *m_args;
	#else
	static KCmdLineArgs *m_args;
	#endif // KS_KF5
#else
	static QStringList m_args;
#endif // KS_NATIVE_KDE
	static QProcessEnvironment m_env;
	static QString m_desktopSession;
	static QString m_xdgCurrentDesktop;
	explicit Utils() { }
};

#endif // KSHUTDOWN_UTILS_H
