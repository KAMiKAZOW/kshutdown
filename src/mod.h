// mod.h - Mod Support
// Copyright (C) 2014  Konrad Twardowski
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

#ifndef KSHUTDOWN_MOD_H
#define KSHUTDOWN_MOD_H

#include <QHash>
#include <QObject>

class MainWindow;

class Mod: public QObject {
public:
	static void applyMainWindowColors(MainWindow *mainWindow);
	static QVariant get(const QString &name, const QVariant &defaultValue);
	static bool getBool(const QString &name, const bool defaultValue = false);
	static void init();
private:
	Q_DISABLE_COPY(Mod)
	static QHash<QString, QVariant> *m_map;
	static QColor getContrastBW(const QColor &base);
};

#endif // KSHUTDOWN_MOD_H
