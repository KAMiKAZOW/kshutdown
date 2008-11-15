//
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

class QProcess;

class Process: public QObject {
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
	QString getInfo() const;
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
	bool isSelectedProcessRunning() const;
	bool isValid() const;
private:
	QMap<int, Process> *m_processMap;
	QProcess *m_refreshProcess;
	QString m_refreshBuf;
	U_COMBO_BOX *m_processes;
public slots:
	void onRefresh();
private slots:
	void onProcessExit();
	void onReadStdout();
};

#endif // __PROCESSMONITOR_H__
