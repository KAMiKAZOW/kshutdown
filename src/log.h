// log.h - Log
// Copyright (C) 2015  Konrad Twardowski
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

#ifndef KSHUTDOWN_LOG_H
#define KSHUTDOWN_LOG_H

#include <QFile>
#include <QTextStream>

class Log final {
public:
	static void error(const QString &text);
	static void info(const QString &text);
	static void init();
	static void shutDown();
	static void warning(const QString &text);
private:
	explicit Log() { }
	static QFile *logFile;
	static QTextStream *output;
	static void log(const QString &category, const QString &text);
};

#endif // KSHUTDOWN_LOG_H
