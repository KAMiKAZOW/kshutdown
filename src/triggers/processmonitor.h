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

#ifndef KSHUTDOWN_PROCESSMONITOR_H
#define KSHUTDOWN_PROCESSMONITOR_H

#if defined(Q_OS_LINUX) || defined(__FreeBSD_kernel__) || defined(Q_OS_HURD)
	#define KS_TRIGGER_PROCESS_MONITOR
#endif // Q_OS_LINUX

#ifdef KS_TRIGGER_PROCESS_MONITOR
	#include <sys/types.h>
#endif // KS_TRIGGER_PROCESS_MONITOR

#include "../kshutdown.h"

#include <QProcess>

class Process: public QObject {
public:
	explicit Process(QObject *parent);
	bool isRunning();
	QString toString();
private:
	Q_DISABLE_COPY(Process)
	friend class ProcessMonitor;
#ifdef KS_TRIGGER_PROCESS_MONITOR
	pid_t m_pid;
#endif // KS_TRIGGER_PROCESS_MONITOR
	QString m_command; // a process command (e.g. "firefox")
	QString m_user; // an owner of the process (e.g. "root")
};

class ProcessMonitor: public KShutdown::Trigger {
	Q_OBJECT
public:
	ProcessMonitor();
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
	void setPID(const pid_t pid);
private:
	Q_DISABLE_COPY(ProcessMonitor)
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

#endif // KSHUTDOWN_PROCESSMONITOR_H
