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

#include <QAbstractItemView>
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
		m_processes->view()->setAlternatingRowColors(true);
		m_processes->setFocusPolicy(Qt::StrongFocus);
		m_processes->setToolTip(i18n("List of the running processes"));
		layout->addWidget(m_processes);
		
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
	m_processes->clear();
	m_processList.clear();
	
	Process *p = new Process(this);
	p->m_command = '?';
#ifdef KS_TRIGGER_PROCESS_MONITOR
	p->m_pid = pid;
#endif // KS_TRIGGER_PROCESS_MONITOR
	p->m_user = '?';
	m_processList.append(p);
	m_processes->addItem(U_ICON(), p->toString());
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
// TODO: args << "-o" << "user=,pid=,command=";
// http://sourceforge.net/tracker/?func=detail&aid=3292203&group_id=93707&atid=605270
	args << "-o" << "user=,pid=,comm=";
	// sort by command
	//args << "--sort" << "command";
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
	
	if (!m_processList.isEmpty())
		return;
	
	if (exitStatus == QProcess::NormalExit) {
		QString user = Utils::getUser();
		QStringList processLines = m_refreshBuf.split('\n');
		foreach (const QString &i, processLines) {
			QStringList processInfo = i.simplified().split(' ');
			if (processInfo.count() >= 3) {
				Process *p = new Process(this);
				p->m_user = processInfo[0];
				p->m_pid = processInfo[1].toLong();
				p->m_command = processInfo[2];
				m_processList.append(p);
				m_processes->addItem(
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
#endif // KS_TRIGGER_PROCESS_MONITOR
}

void ProcessMonitor::onReadyReadStandardOutput() {
	m_refreshBuf.append(m_refreshProcess->readAllStandardOutput());
}
