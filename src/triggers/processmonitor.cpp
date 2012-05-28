// processmonitor.cpp - A process monitor
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

#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	#include <sys/types.h>
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

#include "../utils.h"
#include "processmonitor.h"

#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	#include <errno.h>
	#include <signal.h>
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

#include <QAbstractItemView>
#include <QHBoxLayout>

// public

Process::Process(QObject *parent, const QString &command)
	: QObject(parent),
	m_command(command),
#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	m_pid(0),
	m_user(QString::null)
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX
#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN
	m_pid(0),
	m_visible(false),
	m_windowHandle(NULL)
#endif // KS_TRIGGER_PROCESS_MONITOR_WIN
{
}

bool Process::isRunning() const {
#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	if (::kill(m_pid, 0)) { // check if process exists
		switch (errno) {
			case EINVAL: return false;
			case ESRCH: return false;
			case EPERM: return true;
		}
	}

	return true;
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN
	return ::IsWindow(m_windowHandle) != 0;
#endif // KS_TRIGGER_PROCESS_MONITOR_WIN
}

QString Process::toString() const {
#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	return QString("%0 (pid %1, %2)")
		.arg(m_command)
		.arg(m_pid)
		.arg(m_user);
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN
	return QString("%0 (pid %1)")
		.arg(m_command)
		.arg(m_pid);
#endif // KS_TRIGGER_PROCESS_MONITOR_WIN
}

// public

ProcessMonitor::ProcessMonitor()
	: KShutdown::Trigger(i18n("When selected application exit"), "application-exit", "process-monitor"),
	m_processList(QList<Process*>()),
	m_refreshProcess(0),
	m_refreshBuf(QString::null),
	m_widget(0),
	m_processesComboBox(0)
{
	m_checkTimeout = 2000;
}

void ProcessMonitor::addProcess(Process *process) {
	m_processList.append(process);
}

bool ProcessMonitor::canActivateAction() {
	if (m_processList.isEmpty())
		return false;

	int index = m_processesComboBox->currentIndex();
	Process *p = m_processList.value(index);
	updateStatus(p);

	return !p->isRunning();
}

QWidget *ProcessMonitor::getWidget() {
	if (!m_widget) {
		m_widget = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(m_widget);
		layout->setMargin(0);
		layout->setSpacing(5);

		m_processesComboBox = new U_COMBO_BOX(m_widget);
		m_processesComboBox->view()->setAlternatingRowColors(true);
		m_processesComboBox->setFocusPolicy(Qt::StrongFocus);
		m_processesComboBox->setToolTip(i18n("List of the running processes"));
		connect(m_processesComboBox, SIGNAL(activated(int)), SLOT(onProcessSelect(const int)));
		layout->addWidget(m_processesComboBox);
		
		U_PUSH_BUTTON *refreshButton = new U_PUSH_BUTTON(m_widget);
		refreshButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
		refreshButton->setText(i18n("Refresh"));
		connect(
			refreshButton, SIGNAL(clicked()),
			SLOT(onRefresh())
		);
		layout->addWidget(refreshButton);
	}
	onRefresh();

	return m_widget;
}

void ProcessMonitor::setPID(const pid_t pid) {
#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	m_processesComboBox->clear();
	m_processList.clear();
	
	Process *p = new Process(this, "?");
	p->m_pid = pid;
	p->m_user = '?';
	addProcess(p);
	m_processesComboBox->addItem(U_ICON(), p->toString());
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX
}

// private

void ProcessMonitor::errorMessage(const QString &message) {
	U_APP->restoreOverrideCursor();

	m_processesComboBox->clear();
	m_processesComboBox->setEnabled(false);
	m_processList.clear();

	m_processesComboBox->addItem(
		U_STOCK_ICON("dialog-error"),
		message
	);
}

void ProcessMonitor::updateStatus(const Process *process) {
	if (process) {
		m_status = i18n("Waiting for \"%0\"")
			.arg(process->toString());
	}
	else {
		m_status = QString::null;
	}
}

// public slots

#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
void ProcessMonitor::onRefresh() {
	U_APP->setOverrideCursor(Qt::WaitCursor);
	
	m_processesComboBox->clear();
	m_processesComboBox->setEnabled(true);
	m_processList.clear();
	m_refreshBuf = QString::null;

	if (!m_refreshProcess) {
		m_refreshProcess = new QProcess(this);
		connect(
			m_refreshProcess, SIGNAL(error(QProcess::ProcessError)),
			SLOT(onError(QProcess::ProcessError))
		);
		connect(
			m_refreshProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
			SLOT(onFinished(int, QProcess::ExitStatus))
		);
		connect(
			m_refreshProcess, SIGNAL(readyReadStandardOutput()),
			SLOT(onReadyReadStandardOutput())
		);
	}
	else {
		if (m_refreshProcess->state() == QProcess::Running)
			m_refreshProcess->terminate();
	}
	
	QStringList args;
	// show all processes
	args << "-A";
	// order: user pid command
// TODO: args << "-o" << "user=,pid=,command=";
// http://sourceforge.net/tracker/?func=detail&aid=3292203&group_id=93707&atid=605270
	args << "-o" << "user=,pid=,comm=";
	// sort by command
	//args << "--sort" << "command";
	args << "--sort" << "comm";

	// start process
	m_refreshProcess->start("ps", args);
}
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX

#ifdef KS_TRIGGER_PROCESS_MONITOR_WIN

// CREDITS: http://stackoverflow.com/questions/7001222/enumwindows-pointer-error
BOOL CALLBACK EnumWindowsCallback(HWND windowHandle, LPARAM param) {
	// exclude KShutdown...
	DWORD pid = 0;
	::GetWindowThreadProcessId(windowHandle, &pid);

	if (pid == U_APP->applicationPid())
		return TRUE;

	ProcessMonitor *processMonitor = (ProcessMonitor *)param;

	int textLength = ::GetWindowTextLengthW(windowHandle) + 1;
	wchar_t textBuf[textLength];
	int result = ::GetWindowTextW(windowHandle, textBuf, textLength);
	if (result > 0) {
// TODO: common code: trim
		int max = 30;
		QString title = QString::fromWCharArray(textBuf);
		if (title.length() > max) {
			title.truncate(max);
			title = title.trimmed();
			title.append("...");
		}

		Process *p = new Process(processMonitor, title);
		p->setPID(pid);
		p->setVisible(::IsWindowVisible(windowHandle));
		p->setWindowHandle(windowHandle);
		processMonitor->addProcess(p);
	}
	
	return TRUE;
}

// sort alphabetically, visible first
bool compareProcess(const Process *p1, const Process *p2) {
	bool v1 = p1->visible();
	bool v2 = p2->visible();
	
	if (v1 && !v2)
		return true;
	
	if (!v1 && v2)
		return false;

	QString s1 = p1->toString();
	QString s2 = p2->toString();
		
	return QString::compare(s1, s2, Qt::CaseInsensitive) < 0;
}

void ProcessMonitor::onRefresh() {
	qDeleteAll(m_processList);//!!!review all
	m_processesComboBox->clear();
	m_processList.clear();
	
	::EnumWindows(EnumWindowsCallback, (LPARAM)this);

// TODO: error message
	if (m_processList.isEmpty()) {
		errorMessage(i18n("Error"));
	}
	else {
		qSort(m_processList.begin(), m_processList.end(), compareProcess);
		bool firstHidden = true;
		foreach (Process *i, m_processList) {
			// CREDITS: http://stackoverflow.com/questions/5542423/wm-geticon-sometimes-returns-no-icon-handle
			DWORD iconHandle = ::GetClassLongPtr(i->windowHandle(), GCLP_HICONSM);
			if (iconHandle != 0) {
				m_processesComboBox->addItem(QPixmap::fromWinHICON((HICON)iconHandle), i->toString());

				// separate hidden windows
				if (firstHidden && !i->visible()) {
					firstHidden = false;
					m_processesComboBox->insertSeparator(m_processesComboBox->count() - 1);
				}
			}
		}
	}

	updateStatus(m_processList.isEmpty() ? 0 : m_processList.value(0));
	emit statusChanged(false);
}
#endif // KS_TRIGGER_PROCESS_MONITOR_WIN

// private slots

void ProcessMonitor::onError(QProcess::ProcessError error) {
	errorMessage(i18n("Error: %0").arg(error));
}

void ProcessMonitor::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
#ifdef KS_TRIGGER_PROCESS_MONITOR_UNIX
	U_DEBUG << "ProcessMonitor::onFinished( exitCode=" << exitCode << ", exitStatus=" << exitStatus << " )" U_END;
	
	if (!m_processList.isEmpty()) {
		U_APP->restoreOverrideCursor();
		
		return;
	}
	
	if (exitStatus == QProcess::NormalExit) {
		QString user = Utils::getUser();
		QStringList processLines = m_refreshBuf.split('\n');
		foreach (const QString &i, processLines) {
			QStringList processInfo = i.simplified().split(' ');
			if (processInfo.count() >= 3) {
				Process *p = new Process(this, processInfo[2]/* command */);
				p->m_user = processInfo[0];
				p->m_pid = processInfo[1].toLong();
				addProcess(p);
				m_processesComboBox->addItem(
					// show icons for the current user only (faster)
					(p->m_user == user) ? U_STOCK_ICON(p->m_command) : U_ICON(),
					p->toString()
				);
			}
		}
		
		if (m_processList.isEmpty())
			errorMessage(i18n("Error, exit code: %0").arg(exitCode));
	}
	else { // QProcess::CrashExit
		errorMessage(i18n("Error, exit code: %0").arg(exitCode));
	}
	
	U_APP->restoreOverrideCursor();
	
	updateStatus(m_processList.isEmpty() ? 0 : m_processList.value(0));
	emit statusChanged(false);
#endif // KS_TRIGGER_PROCESS_MONITOR_UNIX
}

void ProcessMonitor::onProcessSelect(const int index) {
	#ifdef KS_TRIGGER_PROCESS_MONITOR
	updateStatus(m_processList.value(index));
	emit statusChanged(false);
	#endif // KS_TRIGGER_PROCESS_MONITOR
}

void ProcessMonitor::onReadyReadStandardOutput() {
	m_refreshBuf.append(m_refreshProcess->readAllStandardOutput());
}
