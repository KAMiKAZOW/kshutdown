// stats.h - System Statistics
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

#ifndef KSHUTDOWN_STATS_H
#define KSHUTDOWN_STATS_H

#include "udialog.h"

#include <QPlainTextEdit>
#include <QProcess>

class Stats: public UDialog {
	Q_OBJECT
public:
	explicit Stats(QWidget *parent);
	virtual ~Stats();
private:
	Q_DISABLE_COPY(Stats)
	QPlainTextEdit *m_textView;
	QProcess *m_process;
	QString m_outputBuf = "";
private slots:
	void onError(QProcess::ProcessError error);
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onReadyReadStandardOutput();
};

#endif // KSHUTDOWN_STATS_H
