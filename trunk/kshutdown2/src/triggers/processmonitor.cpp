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

#include <sys/types.h>

#include "../mainwindow.h"
#include "processmonitor.h"

#include <errno.h>
#include <signal.h>

// public

Process::Process(QObject *parent)
	: QObject(parent),
	m_pid(0),
	m_command(QString::null),
	m_user(QString::null)
{
}

QString Process::toString() {
	return QString("%0 (pid %1, %2)")
		.arg(m_command)
		.arg(m_pid)
		.arg(m_user);
}

// public

ProcessMonitor::ProcessMonitor()
	: KShutdown::Trigger(i18n("When selected application exit"), "application-exit", "process-monitor"),
	m_processList(QList<Process*>()),
	m_refreshProcess(0),
	m_refreshBuf(QString::null),
	m_processes(0)
{
	m_checkTimeout = 2000;

	// refresh button
	/*
	b_refresh = new KPushButton(this, "KPushButton::b_refresh");
	b_refresh->setIconSet(SmallIcon("reload"));
	b_refresh->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
	MiscUtils::setHint(b_refresh, i18n("Refresh the list of processes"));
	connect(b_refresh, SIGNAL(clicked()), SLOT(onRefresh()));

	// processes combo box
	cb_processes = new QComboBox(this, "QComboBox::cb_processes");
	cb_processes->setFocusPolicy(StrongFocus);
	MiscUtils::setHint(cb_processes, i18n("List of the running processes"));
	*/
}

bool ProcessMonitor::canActivateAction() {
	return !isSelectedProcessRunning();
}

/*QString ProcessMonitor::getInfo() const {*/
/*
	int index = cb_processes->currentItem();
	QMapIterator<int, Process> i = _processesMap->find(index);

	if (i == _processesMap->end())
		return QString::null;

	return i18n("Waiting for \"%1\"").arg(i.data().command);
	return QString::null;//!!!
}*/

QWidget *ProcessMonitor::getWidget() {
	if (!m_processes) {
		m_processes = new U_COMBO_BOX();//!!!
	}
	onRefresh();

	return m_processes;
}

// private

bool ProcessMonitor::isSelectedProcessRunning() const {
	int index = m_processes->currentIndex();
	Process *p = m_processList.value(index);
	if (::kill(p->m_pid, 0)) { // check if process exists
		switch (errno) {
			case EINVAL: return false;
			case ESRCH: return false;
			case EPERM: return true;
		}
	}

	return true;
}

bool ProcessMonitor::isValid() const {//!!!
	if (!isSelectedProcessRunning()) {
		U_ERROR_MESSAGE(
			MainWindow::self(),
			i18n("The selected process does not exist!")
		);

		return false;
	}

	return true;
}

// public slots

void ProcessMonitor::onRefresh() {
	m_processes->clear();
	m_processList.clear();
	m_refreshBuf = QString::null;

	if (!m_refreshProcess) {
		m_refreshProcess = new QProcess(this);
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
	//m_refreshProcess->waitForFinished();
/*		// error
		U_ERROR_MESSAGE(
			MainWindow::self(),
			"<qt>" + i18n("Could not execute command<br><br><b>%0</b>").arg(args.join(" ")) + "</qt>"
		);
		//!!!cb_processes->insertItem(SmallIcon("messagebox_warning"), i18n("Error"));*/
}

// private slots

void ProcessMonitor::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	U_DEBUG << "ProcessMonitor::onFinished( exitCode=" << exitCode << ", exitStatus=" << exitStatus << " )" U_END;
	
	if (exitStatus == QProcess::NormalExit) {
		int index = 0;
		int line = 0;
		Process *p = 0;
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
						U_STOCK_ICON(p->m_command),
						p->toString(),
						index
					);
					break;
			}
			index++;
		}
/*!!!		if (cb_processes->count() == 0)
		{
			cb_processes->setEnabled(false);
			cb_processes->insertItem(SmallIcon("messagebox_warning"), i18n("Error"));
			b_kill->setEnabled(false);
		}
		else
		{
			cb_processes->setEnabled(true);
			b_kill->setEnabled(true);
		}
		b_refresh->setEnabled(true);*/
	}
	else { // QProcess::CrashExit
		//!!!
	}
}

void ProcessMonitor::onReadyReadStandardOutput() {
	m_refreshBuf.append(m_refreshProcess->readAllStandardOutput());
}
