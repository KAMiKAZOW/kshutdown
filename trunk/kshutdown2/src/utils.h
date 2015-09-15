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

#include <QProcessEnvironment>
#include <QString>

class QAction;
class QWidget;

class KCmdLineArgs;

class Utils {
public:
	static QString getOption(const QString &name);
	static QString getTimeOption();
	static QString getUser();
	static void init();
	static void initArgs();
	static bool isArg(const QString &name);
	static bool isCinnamon();
	static bool isEnlightenment();
	static bool isGUI();
	static bool isHelpArg();
	static bool isGNOME();
	static bool isGTKStyle();
	static bool isHaiku();
	static bool isKDEFullSession();
	static bool isKDE_4();
	static bool isLXDE();
	static bool isMATE();
	static bool isRazor();
	static bool isRestricted(const QString &action);
	static bool isTrinity();
	static bool isUnity();
	static bool isXfce();
	static void setFont(QWidget *widget, const int relativeSize, const bool bold);
	static void showMenuToolTip(QAction *action);
	static void shutDown();
	static QString trim(QString &text, const int maxLength);
private:
	static bool m_gui;
#ifdef KS_NATIVE_KDE
	static KCmdLineArgs *m_args;
#else
	static QStringList m_args;
#endif // KS_NATIVE_KDE
	static QProcessEnvironment m_env;
	static QString m_desktopSession;
	static QString m_xdgCurrentDesktop;
	Utils() { }
};

#endif // KSHUTDOWN_UTILS_H
