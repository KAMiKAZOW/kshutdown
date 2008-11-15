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

ProcessMonitor::ProcessMonitor()
	: KShutdown::Trigger(i18n("When selected application exit"), "application-exit", "process-monitor"),
	m_processMap(new QMap<int, Process>()),
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
	return !isSelectedProcessRunning();//!!!
}

QString ProcessMonitor::getInfo() const {
/*
	int index = cb_processes->currentItem();
	QMapIterator<int, Process> i = _processesMap->find(index);

	if (i == _processesMap->end())
		return QString::null;

	return i18n("Waiting for \"%1\"").arg(i.data().command);
	*/
	return QString::null;//!!!
}

QWidget *ProcessMonitor::getWidget() {
	if (!m_processes) {
		m_processes = new U_COMBO_BOX();
	}

	return m_processes;
}

bool ProcessMonitor::isSelectedProcessRunning() const
{
/*	int index = cb_processes->currentItem();
	QMapIterator<int, Process> i = _processesMap->find(index);

	if (i == _processesMap->end())
		return false;

	if (::kill(i.data().pid, 0)) { // check if process exists
		switch (errno) {
			case EINVAL: return false;
			case ESRCH: return false;
			case EPERM: return true;
		}
	}*/

	return true;
}

bool ProcessMonitor::isValid() const
{
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
/*	m_refresh->setEnabled(false);
	m_processes->setEnabled(false);

	_buf = QString::null;
	cb_processes->clear();
	_processesMap->clear();

	if (!_process) {
		_process = new QProcess(this);
		connect(_process, SIGNAL(processExited()), SLOT(slotProcessExit()));
		connect(_process, SIGNAL(readyReadStdout()), SLOT(slotReadStdout()));
	}
	else {
		if (_process->isRunning())
			_process->kill();
		_process->clearArguments();
	}
	_process->addArgument("ps");
	// show all processes
	_process->addArgument("-A");
	// order: user pid command
	_process->addArgument("-o");
	_process->addArgument("user=");
	_process->addArgument("-o");
	_process->addArgument("pid=");
	_process->addArgument("-o");
	_process->addArgument("comm=");
	// sort by command
	_process->addArgument("--sort");
	_process->addArgument("comm");

	// start process
	if (!_process->start()) {
		// error
		KMessageBox::error(
			ks_main,
			MiscUtils::HTML(i18n("Could not execute command<br><br><b>%1</b>").arg(_process->arguments().join(" ")))
		);
		cb_processes->setEnabled(false);
		cb_processes->insertItem(SmallIcon("messagebox_warning"), i18n("Error"));
		b_refresh->setEnabled(true);
	}*/
}

// private slots

void ProcessMonitor::onProcessExit() {
/*
	int index = 0;
	int line = 0;
	Process p;
	QStringList list = QStringList::split(" ", _buf.simplifyWhiteSpace());
	for (QStringList::Iterator i = list.begin(); i != list.end(); ++i)
	{
		switch (line)
		{
			// user
			case 0:
				line++; // next is pid
				p = Process();
				p.user = *i;
				break;
			// pid
			case 1:
				line++; // next is command
				p.pid = (*i).toLong();
				break;
			// command
			case 2:
				line = 0; // next is user (wrap around)
				p.command = *i;
				_processesMap->insert(index, p);
				// kdDebug() << "USER=" << p.user << " PID=" << p.pid << " COMMAND=" << p.command << endl;
				QString text = QString("%1 (pid %2, %3)")
					.arg(p.command)
					.arg(p.pid)
					.arg(p.user);
				cb_processes->insertItem(SmallIcon(p.command), text);
				index++;
				break;
		}
	}
	if (cb_processes->count() == 0)
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

void ProcessMonitor::onReadStdout() {
//	m_refreshBuf.append(m_refreshProcess->readStdout());
}
