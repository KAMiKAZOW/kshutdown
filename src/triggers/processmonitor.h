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

#include <QComboBox>

#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	// HACK: fixes some compilation error (?)
	#include <sys/types.h>
#endif // Q_OS_WIN32

class Process final: public QObject {
public:
	explicit Process(QObject *parent, const QString &command);
	QIcon icon() const;
	bool isRunning() const;

	#ifdef Q_OS_WIN32
	inline void setPID(const DWORD value) { m_pid = value; }
	inline bool visible() const { return m_visible; }
	inline void setVisible(const bool value) { m_visible = value; }
	inline HWND windowHandle() const { return m_windowHandle; }
	inline void setWindowHandle(const HWND windowHandle) { m_windowHandle = windowHandle; }
	#else
	inline bool own() const { return m_own; }
	#endif // Q_OS_WIN32

	inline QString toString() const { return m_stringCache; }
private:
	Q_DISABLE_COPY(Process)
	friend class ProcessMonitor;
	QString m_command; // a process command or window title (e.g. "firefox")
	QString m_stringCache = "<THEDAILYWTF>";

	#ifndef Q_OS_WIN32
	bool m_own = false;
	pid_t m_pid = 0;
	QString m_user = QString(); // an owner of the process (e.g. "root")
	#else
	DWORD m_pid = 0;
	bool m_visible = false;
	HWND m_windowHandle = NULL;
	#endif // !Q_OS_WIN32

	void makeStringCache();
};

class ProcessMonitor: public Trigger {
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
	QComboBox *m_processesComboBox = nullptr;
	QList<Process*> m_processList;
	QString m_errorMessage = "";
	QString m_recentCommand = "";
	QWidget *m_widget = nullptr;
	void clearAll();
	void errorMessage();
	void refreshProcessList();
	void updateStatus(const Process *process);
public slots:
	void onRefresh();
private slots:
	void onProcessSelect(const int index);
};

#endif // KSHUTDOWN_PROCESSMONITOR_H
