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

#include "processmonitor.h"

#include "../config.h"
#include "../utils.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QPushButton>

#ifndef Q_OS_WIN32
	#include <csignal> // for ::kill
#endif // !Q_OS_WIN32

// public

Process::Process(QObject *parent, const QString &command)
	: QObject(parent),
	m_command(command) {
}

QIcon Process::icon() const {
	#ifndef Q_OS_WIN32
	// show icons for own processes only (faster)
// FIXME: laggy/slow combo box
	return own() ? QIcon::fromTheme(m_command) : QIcon();
	#else
/* FIXME: still crashy?
	if (!visible())
		return QIcon();

	ULONG_PTR iconHandle = ::GetClassLongPtr(windowHandle(), GCLP_HICONSM);

	if (iconHandle != 0)
		return QPixmap::fromWinHICON((HICON)iconHandle);
*/
	return QIcon();
	#endif // !Q_OS_WIN32
}

bool Process::isRunning() const {
	#ifndef Q_OS_WIN32
	if (::kill(m_pid, 0)) { // check if process exists
		switch (errno) {
			case EINVAL: return false;
			case ESRCH: return false;
			case EPERM: return true;
		}
	}

	return true;
	#else
	return ::IsWindow(m_windowHandle) != 0;
	#endif // !Q_OS_WIN32
}

// private:

void Process::makeStringCache() {
	#ifndef Q_OS_WIN32
	m_stringCache = QString("%0 (pid %1, %2)")
		.arg(m_command, QString::number(m_pid), m_user);
	#else
	m_stringCache = QString("%0 (pid %1)")
		.arg(m_command, QString::number(m_pid));
	#endif // !Q_OS_WIN32
}

// public

ProcessMonitor::ProcessMonitor()
	: KShutdown::Trigger(i18n("When selected application exit"), "application-exit", "process-monitor"),
	m_processList(QList<Process*>())
{
	m_checkTimeout = 2000;
}

void ProcessMonitor::addProcess(Process *process) {
	process->makeStringCache();

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
		auto *layout = new QHBoxLayout(m_widget);
		layout->setMargin(0_px);
		layout->setSpacing(10_px);

		m_processesComboBox = new QComboBox(m_widget);
		m_processesComboBox->view()->setAlternatingRowColors(true);
		m_processesComboBox->setFocusPolicy(Qt::StrongFocus);
		m_processesComboBox->setToolTip(i18n("List of the running processes"));
		connect(m_processesComboBox, SIGNAL(activated(int)), SLOT(onProcessSelect(const int)));
		layout->addWidget(m_processesComboBox);
		
		auto *refreshButton = new QPushButton(m_widget);
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

void ProcessMonitor::readConfig(Config *config) {
	m_recentCommand = config->read("Recent Command", "").toString();
}

void ProcessMonitor::writeConfig(Config *config) {
	config->write("Recent Command", m_recentCommand);
}

void ProcessMonitor::setPID(const qint64 pid) {
	#ifndef Q_OS_WIN32
	clearAll();
	
	Process *p = new Process(this, "?");
	p->m_own = false;
	p->m_pid = pid;
	p->m_user = '?';
	addProcess(p);

	m_processesComboBox->addItem(p->icon(), p->toString());
	#else
	Q_UNUSED(pid)
	#endif // !Q_OS_WIN32
}

// private

void ProcessMonitor::clearAll() {
	qDeleteAll(m_processList);
	m_processesComboBox->clear();
	m_processList.clear();
}

void ProcessMonitor::errorMessage(const QString &message) {
	clearAll();
	m_processesComboBox->setEnabled(false);

	m_processesComboBox->addItem(
		QIcon::fromTheme("dialog-error"),
		message
	);
}

#ifndef Q_OS_WIN32
// sort alphabetically, own first
bool compareProcess(const Process *p1, const Process *p2) {
	bool o1 = p1->own();
	bool o2 = p2->own();
	
	if (o1 && !o2)
		return true;
	
	if (!o1 && o2)
		return false;

	QString s1 = p1->toString();
	QString s2 = p2->toString();
		
	return QString::compare(s1, s2, Qt::CaseInsensitive) < 0;
}
#endif // !Q_OS_WIN32

#ifdef Q_OS_WIN32
// CREDITS: http://stackoverflow.com/questions/7001222/enumwindows-pointer-error
BOOL CALLBACK EnumWindowsCallback(HWND windowHandle, LPARAM param) {
	// exclude KShutdown...
	DWORD pid = 0;
	::GetWindowThreadProcessId(windowHandle, &pid);

	if (pid == qApp->applicationPid())
		return TRUE; // krazy:exclude=captruefalse

	ProcessMonitor *processMonitor = (ProcessMonitor *)param;

	int textLength = ::GetWindowTextLengthW(windowHandle) + 1;
	wchar_t *textBuf = new wchar_t[textLength];
	int result = ::GetWindowTextW(windowHandle, textBuf, textLength);
	if (result > 0) {
		QString title = QString::fromWCharArray(textBuf);
	delete[] textBuf;

// TODO: show process name (*.exe)
		Process *p = new Process(processMonitor, Utils::trim(title, 30));
		p->setPID(pid);
		p->setVisible(::IsWindowVisible(windowHandle));
		p->setWindowHandle(windowHandle);
		processMonitor->addProcess(p);
	}
	
	return TRUE; // krazy:exclude=captruefalse
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
#endif // Q_OS_WIN32

void ProcessMonitor::refreshProcessList() {
	#ifndef Q_OS_WIN32
	QStringList args;
	// show all processes
	args << "-A";
	// order: user pid command
// TODO: args << "-o" << "user=,pid=,command=";
// http://sourceforge.net/p/kshutdown/bugs/11/
	args << "-o" << "user=,pid=,comm=";
	// sort by command
	//args << "--sort" << "command";
	args << "--sort" << "comm";

	QProcess process;
	process.start("ps", args);
	process.waitForStarted(-1);
	Q_PID psPID = process.pid();

	bool ok;
	QString text = Utils::read(process, ok);

	if (!ok)
		return;

	qint64 appPID = QApplication::applicationPid();
	QString user = Utils::getUser();
	QStringList processLines = text.split('\n');

	foreach (const QString &i, processLines) {
		QStringList processInfo = i.simplified().split(' ');
		if (processInfo.count() >= 3) {
			pid_t processID = processInfo[1].toLong();

			// exclude "ps" and self
			if (
				(processID == appPID) ||
				((processID == psPID) && (psPID != 0))
			)
				continue; // for

			QString command;
			if (processInfo.count() > 3) // HACK: fix a command name that contains spaces
				command = QStringList(processInfo.mid(2)).join(" ");
			else
				command = processInfo[2];

			auto *p = new Process(this, command);
			p->m_user = processInfo[0];
			p->m_pid = processID;
			p->m_own = (p->m_user == user);
			addProcess(p);
		}
	}
	#else
	::EnumWindows(EnumWindowsCallback, (LPARAM)this);
/* TODO: also use tasklist.exe
	QStringList args;
	args << "/NH"; // no header
	args << "/FO" << "CSV"; // CSV output format

	QProcess process;
	process.start("tasklist.exe", args);
	process.waitForStarted(-1);
// TODO: Q_PID psPID = process.pid(); - use QProcess::processId()

	bool ok;
	QString text = Utils::read(process, ok);

	if (!ok)
		return;

	qint64 appPID = QApplication::applicationPid();
	QStringList processLines = text.split('\n');

	foreach (const QString &i, processLines) {
		QStringList processInfo = i.simplified().split("\",\"");
		
		if (processInfo.count() >= 2) {
			qint64 processID = processInfo[1].toLongLong();

			// exclude "tasklist.exe" and self
			if (
				(processID == appPID)
				//((processID == psPID) && (psPID != 0))
			)
				continue; // for

			QString command = processInfo[0].remove(0, 1); // remove first "
			Process *p = new Process(this, command);
			p->m_pid = processID;
			p->setVisible(true);
			addProcess(p);
		}
	}
*/
	#endif // !Q_OS_WIN32
}

void ProcessMonitor::updateStatus(const Process *process) {
	if (process) {
		#ifndef Q_OS_WIN32
		m_recentCommand = process->m_command;
		#else
		m_recentCommand = "";
		#endif // !Q_OS_WIN32

// TODO: clean up status API
		if (process->isRunning()) {
			m_status = i18n("Waiting for \"%0\"")
				.arg(process->toString());
			m_statusType = InfoWidget::Type::Info;
		}
		else {
			m_status = i18n("Process or Window does not exist: %0")
				.arg(process->toString());
			m_statusType = InfoWidget::Type::Warning;
		}
	}
	else {
		m_recentCommand = "";
		m_status = QString::null;
	}
}

// public slots

void ProcessMonitor::onRefresh() {
	clearAll();
	m_processesComboBox->setEnabled(true);

	qApp->setOverrideCursor(Qt::WaitCursor);

	refreshProcessList();

	if (m_processList.isEmpty()) {
// TODO: error message
		errorMessage(i18n("Error"));
	}
	else {
		qSort(m_processList.begin(), m_processList.end(), compareProcess);

		bool separatorAdded = false;
		int dummyProcessIndex = -1;

		foreach (Process *i, m_processList) {
			// separate non-important processes
			if (
				#ifndef Q_OS_WIN32
				!i->own() &&
				#else
				!i->visible() &&
				#endif // !Q_OS_WIN32
				!separatorAdded
			) {
				separatorAdded = true;
				dummyProcessIndex = m_processesComboBox->count();
				m_processesComboBox->insertSeparator(m_processesComboBox->count());
			}

			// NOTE: insert dummy list entry to match combo box indexes
			if (dummyProcessIndex != -1) {
				m_processList.insert(dummyProcessIndex, new Process(this, ""));
				dummyProcessIndex = -1;
			}

			m_processesComboBox->addItem(i->icon(), i->toString(), i->m_command);
		}

// FIXME: also compare recent PID
		int i = m_processesComboBox->findData(m_recentCommand);
		if (i != -1)
			m_processesComboBox->setCurrentIndex(i);
	}

	qApp->restoreOverrideCursor();

	int index = m_processesComboBox->currentIndex();
	updateStatus((index == -1) ? nullptr : m_processList.value(index));
	emit statusChanged(false);
}

// private slots

void ProcessMonitor::onProcessSelect(const int index) {
	updateStatus(m_processList.value(index));
	emit statusChanged(false);
}
