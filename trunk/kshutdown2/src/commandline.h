// commandline.h - Command Line
// Copyright (C) 2009  Konrad Twardowski
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

// TODO: merge all command line related APIs

#ifndef KSHUTDOWN_COMMANDLINE_H
#define KSHUTDOWN_COMMANDLINE_H

#include "kshutdown.h"

#include <QCommandLineParser>

class CLI final {
public:
	static bool check();
	static QCommandLineParser *getArgs() { return m_args; }
	static QString getOption(const QString &name);
	static QString getTimeOption();
	static void init(const QString &appDescription);
	static void initOptions();
	static bool isArg(const QString &name);
	static bool isConfirm();
	static void showHelp(QWidget *parent);
private:
	static QCommandLineParser *m_args;
};

class TimeOption final {
public:
	inline static Action *action() { return m_action; }
	inline static void setAction(Action *action) { m_action = action; }
	static void init();
	static bool isError();
	static bool isValid();
	static QTime parseTime(const QString &time);
	inline static QString value() { return m_option; }
	static void setupMainWindow();
private:
	Q_DISABLE_COPY(TimeOption)
	static Action *m_action;
	static bool m_absolute;
	static bool m_relative;
	static QString m_option;
	static QTime m_time;
	explicit TimeOption() { }
};

#endif // KSHUTDOWN_COMMANDLINE_H
