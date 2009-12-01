// idlemonitor.cpp - An inactivity monitor
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

#include "idlemonitor.h"

// public

IdleMonitor::IdleMonitor()
	: KShutdown::Trigger(i18n("Monitor user inactivity"), "user-away", "idle-monitor")
//	m_widget(0),
{
	m_checkTimeout = 2000;
}

bool IdleMonitor::canActivateAction() {
	return false;
/* !!!	if (m_processList.isEmpty())
		return false;

	int index = m_processes->currentIndex();
	Process *p = m_processList.value(index);
	m_status = i18n("Waiting for \"%0\"")
		.arg(p->toString());

	return !p->isRunning();*/
}

QWidget *IdleMonitor::getWidget() {
	/* !!! if (!m_widget) {
		m_widget = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(m_widget);
		layout->setMargin(0);
		layout->setSpacing(5);

		m_processes = new U_COMBO_BOX(m_widget);
		m_processes->setFocusPolicy(Qt::StrongFocus);
		m_processes->setToolTip(i18n("List of the running processes"));
		layout->addWidget(m_processes);
		
		U_PUSH_BUTTON *refreshButton = new U_PUSH_BUTTON(m_widget);
#ifdef KS_NATIVE_KDE
		refreshButton->setIcon(U_STOCK_ICON("view-refresh"));
#else
		refreshButton->setText(i18n("Refresh"));
#endif // KS_NATIVE_KDE
		refreshButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
		refreshButton->setToolTip(i18n("Refresh the list of processes"));
		connect(
			refreshButton, SIGNAL(clicked()),
			SLOT(onRefresh())
		);
		layout->addWidget(refreshButton);
	}
	onRefresh();

	return m_widget;*/
	return 0;
}

// public slots

/* !!! 
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
*/