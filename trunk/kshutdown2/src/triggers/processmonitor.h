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

#include "../kshutdown.h"

#ifdef KS_UNIX
	#define KS_TRIGGER_PROCESS_MONITOR
	#define KS_TRIGGER_PROCESS_MONITOR_UNIX
#endif // KS_UNIX

#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#define KS_TRIGGER_PROCESS_MONITOR
	#define KS_TRIGGER_PROCESS_MONITOR_WIN
#endif // Q_OS_WIN32

#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	#include <sys/types.h>
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

class Process final: public QObject {
public:
	explicit Process(QObject *parent, const QString &command);
	U_ICON icon() const;
	bool isRunning() const;
	#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	inline bool own() const { return m_own; }
	#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX
	#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN
	inline void setPID(const DWORD value) { m_pid = value; }
	inline bool visible() const { return m_visible; }
	inline void setVisible(const bool value) { m_visible = value; }
	inline HWND windowHandle() const { return m_windowHandle; }
	inline void setWindowHandle(const HWND windowHandle) { m_windowHandle = windowHandle; }
	#endif // KS_TRIGGER_PROCESS_MONITOR_WIN
	inline QString toString() const { return m_stringCache; }
private:
	Q_DISABLE_COPY(Process)
	friend class ProcessMonitor;
	QString m_command; // a process command or window title (e.g. "firefox")
	QString m_stringCache = "<THEDAILYWTF>";
	
	#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	bool m_own = false;
	pid_t m_pid = 0;
	QString m_user = QString::null; // an owner of the process (e.g. "root")
	#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX
	
	#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN
	DWORD m_pid = 0;
	bool m_visible = false;
	HWND m_windowHandle = NULL;
	#endif // KS_TRIGGER_PROCESS_MONITOR_WIN

	void makeStringCache();
};

class ProcessMonitor: public KShutdown::Trigger {
	Q_OBJECT
public:
	explicit ProcessMonitor();
	void addProcess(Process *process);
	virtual bool canActivateAction() override;
	virtual QWidget *getWidget() override;
	virtual void readConfig(Config *config) override;
	virtual void writeConfig(Config *config) override;
	void setPID(const qint64 pid);
private:
	Q_DISABLE_COPY(ProcessMonitor)
	QList<Process*> m_processList;
	QString m_recentCommand = "";
	QWidget *m_widget = nullptr;
	U_COMBO_BOX *m_processesComboBox = nullptr;
	void clearAll();
	void errorMessage(const QString &message);
	void refreshProcessList();
	void updateStatus(const Process *process);
public slots:
	void onRefresh();
private slots:
	void onProcessSelect(const int index);
};

#endif // KSHUTDOWN_PROCESSMONITOR_H
