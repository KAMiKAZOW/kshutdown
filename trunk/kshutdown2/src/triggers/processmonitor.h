// processmonitor.h - A process monitor
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

#ifndef __PROCESSMONITOR_H__
#define __PROCESSMONITOR_H__

#include <sys/types.h>

#include "../kshutdown.h"

#include <QProcess>

class Process: public QObject {
public:
	Process(QObject *parent);
	bool isRunning();
	QString toString();
private:
	friend class ProcessMonitor;
	pid_t m_pid;
	QString m_command; // a process command (e.g. "firefox")
	QString m_user; // an owner of the process (e.g. "root")
};

class ProcessMonitor: public KShutdown::Trigger {
	Q_OBJECT
public:
	ProcessMonitor();
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
private:
	QList<Process*> m_processList;
	QProcess *m_refreshProcess;
	QString m_refreshBuf;
	QWidget *m_widget;
	U_COMBO_BOX *m_processes;
	void errorMessage(const QString &message);
public slots:
	void onRefresh();
private slots:
	void onError(QProcess::ProcessError error);
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onReadyReadStandardOutput();
};

#endif // __PROCESSMONITOR_H__
