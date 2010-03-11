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

#ifdef KS_TRIGGER_PROCESS_MONITOR
	#include <sys/types.h>
#endif // KS_TRIGGER_PROCESS_MONITOR

#include "../utils.h"
#include "processmonitor.h"

#ifdef KS_TRIGGER_PROCESS_MONITOR
	#include <errno.h>
	#include <signal.h>
#endif // KS_TRIGGER_PROCESS_MONITOR

#include <QHBoxLayout>

// public

Process::Process(QObject *parent)
	: QObject(parent),
#ifdef KS_TRIGGER_PROCESS_MONITOR
	m_pid(0),
#endif // KS_TRIGGER_PROCESS_MONITOR
	m_command(QString::null),
	m_user(QString::null)
{
}

bool Process::isRunning() {
#ifdef KS_TRIGGER_PROCESS_MONITOR
	if (::kill(m_pid, 0)) { // check if process exists
		switch (errno) {
			case EINVAL: return false;
			case ESRCH: return false;
			case EPERM: return true;
		}
	}

	return true;
#else
	return false;
#endif // KS_TRIGGER_PROCESS_MONITOR
}

QString Process::toString() {
#ifdef KS_TRIGGER_PROCESS_MONITOR
	return QString("%0 (pid %1, %2)")
		.arg(m_command)
		.arg(m_pid)
		.arg(m_user);
#else
	return QString();
#endif // KS_TRIGGER_PROCESS_MONITOR
}

// public

ProcessMonitor::ProcessMonitor()
	: KShutdown::Trigger(i18n("When selected application exit"), "application-exit", "process-monitor"),
	m_processList(QList<Process*>()),
	m_refreshProcess(0),
	m_refreshBuf(QString::null),
	m_widget(0),
	m_processes(0)
{
	m_checkTimeout = 2000;
}

bool ProcessMonitor::canActivateAction() {
	if (m_processList.isEmpty())
		return false;

	int index = m_processes->currentIndex();
	Process *p = m_processList.value(index);
	m_status = i18n("Waiting for \"%0\"")
		.arg(p->toString());

	return !p->isRunning();
}

QWidget *ProcessMonitor::getWidget() {
	if (!m_widget) {
		m_widget = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(m_widget);
		layout->setMargin(0);
		layout->setSpacing(5);

		m_processes = new U_COMBO_BOX(m_widget);
		m_processes->setFocusPolicy(Qt::StrongFocus);
		m_processes->setToolTip(i18n("List of the running processes"));
		layout->addWidget(m_processes);
		
		U_PUSH_BUTTON *refreshButton = new U_PUSH_BUTTON(m_widget);
#ifdef Q_WS_X11
		refreshButton->setIcon(U_STOCK_ICON("view-refresh"));
#else
		refreshButton->setText(i18n("Refresh"));
#endif // Q_WS_X11
		refreshButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
		refreshButton->setToolTip(i18n("Refresh the list of processes"));
		connect(
			refreshButton, SIGNAL(clicked()),
			SLOT(onRefresh())
		);
		layout->addWidget(refreshButton);
	}
	onRefresh();

	return m_widget;
}

// private

void ProcessMonitor::errorMessage(const QString &message) {
	m_processes->clear();
	m_processes->setEnabled(false);
	m_processList.clear();

	m_processes->addItem(
		U_STOCK_ICON("dialog-error"),
		message
	);
}

// public slots

void ProcessMonitor::onRefresh() {
#ifdef KS_TRIGGER_PROCESS_MONITOR
	m_processes->clear();
	m_processes->setEnabled(true);
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
	args << "-o" << "user=";
	args << "-o" << "pid=";
	args << "-o" << "comm=";
	// sort by command
	args << "--sort" << "comm";

	// start process
	m_refreshProcess->start("ps", args);
#endif // KS_TRIGGER_PROCESS_MONITOR
}

// private slots

void ProcessMonitor::onError(QProcess::ProcessError error) {
	errorMessage(i18n("Error: %0").arg(error));
}

void ProcessMonitor::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
#ifdef KS_TRIGGER_PROCESS_MONITOR
	U_DEBUG << "ProcessMonitor::onFinished( exitCode=" << exitCode << ", exitStatus=" << exitStatus << " )" U_END;
	
	if (exitStatus == QProcess::NormalExit) {
		int index = 0;
		int line = 0;
		Process *p = 0;
		QString user = Utils::getUser();
		foreach (QString i, m_refreshBuf.simplified().split(" ")) {
			switch (line) {
				// user
				case 0:
					line++; // next is pid
					p = new Process(this);
					p->m_user = i;
					break;
				// pid
				case 1:
					line++; // next is command
					p->m_pid = i.toLong();
					break;
				// command
				case 2:
					line = 0; // next is user (wrap around)
					p->m_command = i;
					m_processList.append(p);
					m_processes->addItem(
						// show icons for the current user only (faster)
						(p->m_user == user) ? U_STOCK_ICON(p->m_command) : U_ICON(),
						p->toString(),
						index
					);
					break;
			}
			index++;
		}
		
		if (m_processList.isEmpty())
			errorMessage(i18n("Error, exit code: %0").arg(exitCode));
	}
	else { // QProcess::CrashExit
		errorMessage(i18n("Error, exit code: %0").arg(exitCode));
	}
#endif // KS_TRIGGER_PROCESS_MONITOR
}

void ProcessMonitor::onReadyReadStandardOutput() {
	m_refreshBuf.append(m_refreshProcess->readAllStandardOutput());
}
