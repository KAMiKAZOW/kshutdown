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

#include <QDebug>
#include <QMessageBox>
#ifdef Q_OS_WIN32
	#include <QtWin>
#endif // Q_OS_WIN32

// Base

// public

Base::Base(const QString &id) :
	m_statusType(InfoWidget::Type::Info),
	m_disableReason(QString() ),
	m_error(QString() ),
	m_id(id),
	m_originalText(QString() ),
	m_status(QString() ),
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
void Base::setLastError() {
	DWORD lastError = ::GetLastError();

	m_error = QtWin::stringFromHresult(static_cast<HRESULT>(lastError)) +
		" (error code: " + QString::number(lastError) +
		" | 0x" + QString::number(lastError, 16).toUpper() + ')';
}
#endif // Q_OS_WIN32

// Action

// public

Action::Action(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
	m_force(false),
	m_shouldStopTimer(true) {
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
	auto *result = new QAction(this);

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
			(alwaysShowConfirmationMessage || Config::confirmActionVar) &&
			(this != LockAction::self()); // lock action - no confirmation

		if (!wantConfirmation || showConfirmationMessage()) {
			if (authorize(MainWindow::self()))
				activate(false);
		}
	});

	return result;
}

QString Action::getDisplayName(const ActionType type) {
	switch (type) {
		case ActionType::SHUT_DOWN:
			return Config::oldActionNamesVar ? i18n("Turn Off Computer") : i18n("Shut Down");

		case ActionType::REBOOT:
			return Config::oldActionNamesVar ? i18n("Restart Computer") : i18n("Restart");

		case ActionType::HIBERNATE:
			return Config::oldActionNamesVar ? i18n("Hibernate Computer") : i18n("Hibernate");

		case ActionType::SUSPEND:
			#ifdef Q_OS_WIN32
			return i18n("Sleep");
			#else
			// NOTE: Matches https://phabricator.kde.org/D19184
			return Config::oldActionNamesVar ? i18n("Suspend Computer") : i18n("Sleep");
			#endif // Q_OS_WIN32

		case ActionType::LOCK:
			return Config::oldActionNamesVar.getBool() ? i18n("Lock Screen") : i18n("Lock");

		case ActionType::LOGOUT:
			#ifdef Q_OS_WIN32
			return i18n("Log Off");
			#else
			return i18n("Logout");
			#endif // Q_OS_WIN32

		case ActionType::TEST:
			return i18n("Show Message (no shutdown)");

		default:
			return ""; // unused
	}
}

int Action::getUIGroup() const {
	if ((m_id == "shutdown") || (m_id == "reboot"))
		return 0;

	if ((m_id == "hibernate") || (m_id == "suspend"))
		return 1;

	if ((m_id == "lock") || (m_id == "logout"))
		return 2;

	return 3;
}

bool Action::isCommandLineOptionSet() const {
	return (m_commandLineOption != nullptr) && CLI::getArgs()->isSet(*m_commandLineOption);
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
	QScopedPointer<QMessageBox> message(new QMessageBox(
		QMessageBox::Warning,
		title,
		text,
		QMessageBox::Ok | QMessageBox::Cancel,
		parent
	));

	if (Config::cancelDefaultVar)
		message->setDefaultButton(QMessageBox::Cancel);
	else
		message->setDefaultButton(QMessageBox::Ok);

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

	return message->exec() == QMessageBox::Ok;
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

void Action::disable(const QString &reason) {
	m_uiAction->setEnabled(false);
	m_disableReason = reason;
}

#ifdef QT_DBUS_LIB
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
			qDebug() << "systemd/logind backend found...";
		else
			qDebug() << "systemd/logind backend NOT found...";
	}

	return m_loginInterface;
}
#endif // QT_DBUS_LIB

bool Action::launch(const QString &program, const QStringList &args, const bool detached) {
	if (detached) {
		qDebug() << "Launching detached \"" << program << "\" with \"" << args << "\" arguments";

		// HACK: start detached to fix session manager hang in GNOME-based DEs
		bool ok = QProcess::startDetached(program, args);
		qDebug() << "Started OK: " << ok;

		return ok;
	}

	qDebug() << "Launching \"" << program << "\" with \"" << args << "\" arguments";

// TODO: use startDetached
	int exitCode = QProcess::execute(program, args);

	if (exitCode == -2) {
		qDebug() << "Process failed to start (-2)";

		return false;
	}

	if (exitCode == -1) {
		qDebug() << "Process crashed (-1)";

		return false;
	}

	qDebug() << "Exit code: " << exitCode;

	return (exitCode == 0);
}

void Action::setCommandLineOption(const QCommandLineOption *option) {
	//qDebug() << "Action::setCommandLineOption: " << option->names() << option->description();
	m_commandLineOption = option;
	CLI::getArgs()->addOption(*m_commandLineOption);
}

void Action::setCommandLineOption(const QStringList &names, const QString &description) {
	setCommandLineOption(new QCommandLineOption(
		names,
		description.isEmpty() ? originalText() : description
	));
}

bool Action::unsupportedAction() {
	m_error = i18n("Unsupported action: %0")
		.arg(originalText());

	return false;
}

// private:

void Action::readProperties(Config *config) {
	m_visibleInMainMenu = config->read("Visible In Main Menu", m_visibleInMainMenu).toBool();
	m_visibleInSystemTrayMenu = config->read("Visible In System Tray Menu", m_visibleInSystemTrayMenu).toBool();
	m_visibleInWindow = config->read("Visible In Window", m_visibleInWindow).toBool();
}

// private slots

void Action::slotFire() {
	Log::info("Execute action: " + m_id + " (" + m_originalText + ')');

	qDebug() << "Action::slotFire() [ id=" << m_id << " ]";

	if (!isEnabled()) {
		QString s = m_disableReason.isEmpty() ? i18n("Unknown error") : m_disableReason;
		UDialog::error(nullptr, m_uiAction->text() + ": " + s);

		return;
	}

	m_error = QString();
	if (!onAction()) {
		m_totalExit = false;
		if (!m_error.isNull()) {
			QString s = m_error.isEmpty() ? i18n("Unknown error") : m_error;
			UDialog::error(nullptr, m_uiAction->text() + ": " + s);
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

void PluginManager::initActionsAndTriggers() {
	qDebug() << "Init Actions";

	add(new ShutDownAction());
	add(new RebootAction());
	add(new HibernateAction());
	add(new SuspendAction());
	add(LockAction::self());
	add(new LogoutAction());
	add(Extras::self());
	add(new TestAction());

	qDebug() << "Init Triggers";

	add(new NoDelayTrigger());
	add(new TimeFromNowTrigger());
	add(new DateTimeTrigger());
	add(new ProcessMonitor());

	auto *idleMonitor = new IdleMonitor();
	if (idleMonitor->isSupported())
		add(idleMonitor);
	else
		delete idleMonitor;
}

void PluginManager::readConfig() {
	//qDebug() << "PluginManager::readConfig";

	Config *config = Config::user();

// TODO: foreach -> for (new C++ syntax)
	foreach (Action *i, actionList()) {
		config->beginGroup("KShutdown Action " + i->id());
		i->readProperties(config);
		i->readConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, triggerList()) {
		config->beginGroup("KShutdown Trigger " + i->id());
		i->readConfig(config);
		config->endGroup();
	}
}

void PluginManager::shutDown() {
	//qDebug() << "PluginManager::shutDown (Action)";

	foreach (Action *i, actionList())
		delete i;

	m_actionList.clear();
	m_actionMap.clear();

	//qDebug() << "PluginManager::shutDown (Trigger)";

	foreach (Trigger *i, triggerList())
		delete i;

	m_triggerList.clear();
	m_triggerMap.clear();
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
