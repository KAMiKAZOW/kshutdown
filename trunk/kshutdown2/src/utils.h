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

#include <QString>

class QWidget;

class KCmdLineArgs;

class Utils {
public:
	static QString getOption(const QString &name);
	static QString getTimeOption();
	static QString getUser();
	static void init();
	static bool isArg(const QString &name);
	static bool isHelpArg();
	static bool isGDM();
	static bool isGNOME();
	static bool isKDEFullSession();
	static bool isKDE_3();
	static bool isKDE_4();
	static bool isKDM();
	static bool isXfce();
	static void setFont(QWidget *widget, const int relativeSize, const bool bold);
	static void shutDown();
private:
#ifdef KS_NATIVE_KDE
	static KCmdLineArgs *m_args;
#else
	static QStringList m_args;
#endif // KS_NATIVE_KDE
	Utils() { }
};

#endif // KSHUTDOWN_UTILS_H
