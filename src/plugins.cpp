// plugins.cpp - Plugins
// Copyright (C) 2018  Konrad Twardowski
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

#include "plugins.h"

#include "commandline.h"
#include "config.h"
#include "log.h"
#include "mainwindow.h"
#include "password.h"
#include "utils.h"
#include "actions/extras.h"
#include "actions/lock.h"
#include "actions/test.h"
#include "triggers/idlemonitor.h"
#include "triggers/processmonitor.h"

#include <QMessageBox>

// Base

// public

Base::Base(const QString &id) :
	m_statusType(InfoWidget::Type::Info),
	m_disableReason(QString::null),
	m_error(QString::null),
	m_id(id),
	m_originalText(QString::null),
	m_status(QString::null),
	m_canBookmark(false) {
}

Base::~Base() = default;

QWidget *Base::getWidget() {
	return nullptr;
}

void Base::readConfig(Config *config) {
	Q_UNUSED(config)
}

void Base::setState(const State state) {
	Q_UNUSED(state)
}

void Base::writeConfig(Config *config) {
	Q_UNUSED(config)
}

// protected

#ifdef Q_OS_WIN32
// CREDITS: http://lists.trolltech.com/qt-solutions/2005-05/msg00005.html
void Base::setLastError() {
	DWORD lastError = ::GetLastError();
	wchar_t *buffer = 0;
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		lastError,
		0,
		(wchar_t *)&buffer,
		0,
		0
	);
	m_error = QString::fromWCharArray(buffer) + " (error code: " + QString::number(lastError) + ", 0x" + QString::number(lastError, 16) + ')';
	if (buffer)
		::LocalFree(buffer);
}
#endif // Q_OS_WIN32

// Action

// public

Action::Action(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
	m_force(false),
	m_shouldStopTimer(true),
	m_showInMenu(true),
	m_commandLineArgs(QStringList()) {
	m_originalText = text;

	m_uiAction = new QAction();
	if (!iconName.isNull())
		m_uiAction->setIcon(QIcon::fromTheme(iconName));
	m_uiAction->setIconVisibleInMenu(true);
	m_uiAction->setText(text);
	connect(m_uiAction, SIGNAL(triggered()), SLOT(slotFire()));

	if (Utils::isRestricted("kshutdown/action/" + id))
		disable(i18n("Disabled by Administrator"));
}

void Action::activate(const bool force) {
	m_force = force;
	m_uiAction->trigger();
}

bool Action::authorize(QWidget *parent) {
	return PasswordDialog::authorize(
		parent,
		m_originalText,
		"kshutdown/action/" + m_id
	);
}

QAction *Action::createConfirmAction(const bool alwaysShowConfirmationMessage) {
	QAction *result = new QAction(this);

	// clone basic properties
	result->setEnabled(m_uiAction->isEnabled());
	result->setIcon(m_uiAction->icon());
	result->setIconVisibleInMenu(m_uiAction->isIconVisibleInMenu());
	result->setMenu(m_uiAction->menu());
	result->setShortcut(m_uiAction->shortcut());
	result->setStatusTip(m_uiAction->statusTip());
	result->setText(m_uiAction->text());

	// HACK: action->toolTip() is unusable in Qt
	result->setToolTip(m_uiAction->statusTip()); // for QMenu

	connect(result, &QAction::triggered, [this, /*FU &*/alwaysShowConfirmationMessage] {
		if (shouldStopTimer())
			MainWindow::self()->setActive(false);

		bool wantConfirmation =
			(alwaysShowConfirmationMessage || Config::confirmAction()) &&
			(this != LockAction::self()); // lock action - no confirmation

		if (!wantConfirmation || showConfirmationMessage()) {
			if (authorize(MainWindow::self()))
				activate(false);
		}
	});

	return result;
}

bool Action::isCommandLineArgSupported() {
	foreach (const QString &i, m_commandLineArgs) {
		if (CLI::isArg(i))
			return true;
	}

	return false;
}

bool Action::showConfirmationMessage() {
	QString text = i18n("Are you sure?");
	QString title = i18n("Confirm Action");

	MainWindow *mainWindow = MainWindow::self();
	QWidget *parent;

	if (mainWindow->isVisible())
		parent = mainWindow;
	else
		parent = nullptr;

// TODO: KMessageBox
	QPointer<QMessageBox> message = new QMessageBox(
		QMessageBox::Warning,
		title,
		text,
		QMessageBox::Ok | QMessageBox::Cancel,
		parent
	);
	message->setDefaultButton(QMessageBox::Cancel);
	if (!icon().isNull()) {
		int size = qApp->style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
		message->setIconPixmap(icon().pixmap(size));
	}

	QAbstractButton *ok = message->button(QMessageBox::Ok);
	ok->setIcon(icon());

	#ifdef Q_OS_WIN32
	// HACK: add left/right margins
	ok->setText(' ' + originalText() + ' ');
	#else
	ok->setText(originalText());
	#endif // Q_OS_WIN32

	bool accepted = message->exec() == QMessageBox::Ok; // krazy:exclude=qclasses
	delete message;

	return accepted;
}

void Action::updateMainWindow(MainWindow *mainWindow) {
	if (isEnabled()) {
		if (mainWindow->active())
			mainWindow->okCancelButton()->setEnabled(!Utils::isRestricted("kshutdown/action/cancel"));
		else
			mainWindow->okCancelButton()->setEnabled(true);
		emit statusChanged(false);
	}
	else {
		mainWindow->okCancelButton()->setEnabled(false);
// TODO: show solution dialog
		QString s = i18n("Action not available: %0").arg(originalText());
		if (!disableReason().isEmpty())
			s += ("<br>" + disableReason());
		mainWindow->infoWidget()->setText("<qt>" + s + "</qt>", InfoWidget::Type::Error);
	}

}

// protected

// NOTE: Sync. with commandline.cpp/CLI::init
void Action::addCommandLineArg(const QString &shortArg, const QString &longArg) {
	if (!shortArg.isEmpty())
		m_commandLineArgs.append(shortArg);
	if (!longArg.isEmpty())
		m_commandLineArgs.append(longArg);
/* TODO:
	CLI::getArgs()
		->addOption(QCommandLineOption(m_commandLineArgs, originalText()));
*/
}

void Action::disable(const QString &reason) {
	m_uiAction->setEnabled(false);
	m_disableReason = reason;
}

#ifdef KS_DBUS
// DOC: http://www.freedesktop.org/wiki/Software/systemd/logind/
QDBusInterface *Action::getLoginInterface() {
	if (!m_loginInterface) {
		m_loginInterface = new QDBusInterface(
			"org.freedesktop.login1",
			"/org/freedesktop/login1",
			"org.freedesktop.login1.Manager",
			QDBusConnection::systemBus()
		);
		if (m_loginInterface->isValid())
			U_DEBUG << "systemd/logind backend found..." U_END;
		else
			U_DEBUG << "systemd/logind backend NOT found..." U_END;
	}

	return m_loginInterface;
}
#endif // KS_DBUS

bool Action::launch(const QString &program, const QStringList &args, const bool detached) {
	if (detached) {
		U_DEBUG << "Launching detached \"" << program << "\" with \"" << args << "\" arguments" U_END;

		// HACK: start detached to fix session manager hang in GNOME-based DEs
		bool ok = QProcess::startDetached(program, args);
		U_DEBUG << "Started OK: " << ok U_END;

		return ok;
	}

	U_DEBUG << "Launching \"" << program << "\" with \"" << args << "\" arguments" U_END;

	int exitCode = QProcess::execute(program, args);

	if (exitCode == -2) {
		U_DEBUG << "Process failed to start (-2)" U_END;

		return false;
	}

	if (exitCode == -1) {
		U_DEBUG << "Process crashed (-1)" U_END;

		return false;
	}

	U_DEBUG << "Exit code: " << exitCode U_END;

	return (exitCode == 0);
}

bool Action::unsupportedAction() {
	m_error = i18n("Unsupported action: %0")
		.arg(originalText());

	return false;
}

// private slots

void Action::slotFire() {
	Log::info("Execute action: " + m_id + " (" + m_originalText + ')');

	U_DEBUG << "Action::slotFire() [ id=" << m_id << " ]" U_END;

	if (!isEnabled()) {
		QString s = m_disableReason.isEmpty() ? i18n("Unknown error") : m_disableReason;
		UDialog::error(0, m_uiAction->text() + ": " + s);

		return;
	}

	m_error = QString::null;
	if (!onAction()) {
		m_totalExit = false;
		if (!m_error.isNull()) {
			QString s = m_error.isEmpty() ? i18n("Unknown error") : m_error;
			UDialog::error(0, m_uiAction->text() + ": " + s);
		}
	}
}

// Trigger

// public

Trigger::Trigger(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
	m_supportsProgressBar(false),
	m_checkTimeout(500),
	m_icon(QIcon::fromTheme(iconName)),
	m_text(text) {
}

// PluginManager

// private:

QHash<QString, Action*> PluginManager::m_actionMap = QHash<QString, Action*>();
QHash<QString, Trigger*> PluginManager::m_triggerMap = QHash<QString, Trigger*>();

QList<Action*> PluginManager::m_actionList = QList<Action*>();
QList<Trigger*> PluginManager::m_triggerList = QList<Trigger*>();

// public:

void PluginManager::add(Action *action) {
	//qDebug() << "Add Action:" << action->id();

	m_actionMap[action->id()] = action;
	m_actionList.append(action);
}

void PluginManager::add(Trigger *trigger) {
	//qDebug() << "Add Trigger:" << trigger->id();

	m_triggerMap[trigger->id()] = trigger;
	m_triggerList.append(trigger);
}

void PluginManager::init() {
	qDebug() << "PluginManager::init (Action)" U_END;

	add(new ShutDownAction());
	add(new RebootAction());
	add(new HibernateAction());
	add(new SuspendAction());
	add(LockAction::self());
	add(new LogoutAction());
	add(Extras::self());
	add(new TestAction());

	qDebug() << "PluginManager::init (Trigger)" U_END;

	add(new NoDelayTrigger());
	add(new TimeFromNowTrigger());
	add(new DateTimeTrigger());
	add(new ProcessMonitor());

	auto *idleMonitor = new IdleMonitor();
	if (idleMonitor->isSupported())
		add(idleMonitor);
	else
		delete idleMonitor;

	Config *config = Config::user();

	foreach (Action *i, actionList()) {
		config->beginGroup("KShutdown Action " + i->id());
		i->readConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, triggerList()) {
		config->beginGroup("KShutdown Trigger " + i->id());
		i->readConfig(config);
		config->endGroup();
	}
}

void PluginManager::writeConfig() {
	Config *config = Config::user();

	foreach (Action *i, actionList()) {
		config->beginGroup("KShutdown Action " + i->id());
		i->writeConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, triggerList()) {
		config->beginGroup("KShutdown Trigger " + i->id());
		i->writeConfig(config);
		config->endGroup();
	}
}
