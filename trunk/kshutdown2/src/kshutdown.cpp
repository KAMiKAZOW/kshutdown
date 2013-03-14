// kshutdown.cpp - KShutdown base library
// Copyright (C) 2007  Konrad Twardowski
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

#include "pureqt.h"

#ifdef KS_UNIX
	#include <errno.h>
	#include <signal.h>
#endif // KS_UNIX

#include <QDateTimeEdit>
#include <QProcess>
#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#define _WIN32_WINNT 0x0500 // for LockWorkStation, etc
	#include <windows.h>
	#include <powrprof.h>
#else
	#include <unistd.h> // for sleep
#endif // Q_OS_WIN32

#ifdef KS_DBUS
	#include <QDBusInterface>
	#include <QDBusReply>
#endif // KS_DBUS

#ifdef KS_PURE_QT
	#include <QPointer>
#endif // KS_PURE_QT

#include "actions/lock.h"
#include "kshutdown.h"
#include "mainwindow.h"
#include "password.h"
#include "progressbar.h"
#include "utils.h"

using namespace KShutdown;

bool Action::m_totalExit = false;
#ifdef KS_DBUS
QDBusInterface *StandardAction::m_consoleKitInterface = 0;
QDBusInterface *StandardAction::m_halInterface = 0;
#endif // KS_DBUS

// Base

// public

Base::Base(const QString &id) :
	m_statusType(InfoWidget::InfoType),
	m_disableReason(QString::null),
	m_error(QString::null),
	m_id(id),
	m_originalText(QString::null),
	m_status(QString::null) {
}

Base::~Base() {
}

QWidget *Base::getWidget() {
	return 0;
}

void Base::readConfig(const QString &group, Config *config) {
	Q_UNUSED(group)
	Q_UNUSED(config)
}

void Base::setState(const State state) {
	Q_UNUSED(state)
}

void Base::writeConfig(const QString &group, Config *config) {
	Q_UNUSED(group)
	Q_UNUSED(config)
}

// protected

#ifdef Q_OS_WIN32
// CREDITS: http://lists.trolltech.com/qt-solutions/2005-05/msg00005.html
void Base::setLastError() {
	DWORD lastError = ::GetLastError();
	char *buffer = 0;
	::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		lastError,
		0,
		(char *)&buffer,
		0,
		0
	);
	m_error = QString::fromLocal8Bit(buffer) + " (error code: " + QString::number(lastError) + ", 0x" + QString::number(lastError, 16) + ")";
	if (buffer)
		::LocalFree(buffer);
}
#endif // Q_OS_WIN32

// Action

// public

Action::Action(const QString &text, const QString &iconName, const QString &id) :
	U_ACTION(0),
	Base(id),
	m_force(false),
	m_shouldStopTimer(true),
	m_showInMenu(true),
	m_commandLineArgs(QStringList()) {
	m_originalText = text;
	if (!iconName.isNull())
		setIcon(U_STOCK_ICON(iconName));
	
	if (Utils::isRestricted("kshutdown/action/" + id))
		disable(i18n("Disabled by Administrator"));

	setText(text);
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

void Action::activate(const bool force) {
	m_force = force;
	U_ACTION::trigger();
}

bool Action::authorize(QWidget *parent) {
	return PasswordDialog::authorize(
		parent,
		m_originalText,
		"kshutdown/action/" + m_id
	);
}

bool Action::isCommandLineArgSupported() {
	foreach (const QString &i, m_commandLineArgs) {
		if (Utils::isArg(i))
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
		parent = 0;

	#ifdef KS_NATIVE_KDE
	return KMessageBox::warningYesNo(
		parent,
		text,
		title,
		KGuiItem(originalText(), KIcon(icon())),
		KStandardGuiItem::cancel()
	) == KMessageBox::Yes;
	#else
	QPointer<QMessageBox> message = new QMessageBox( // krazy:exclude=qclasses
		QMessageBox::Warning,
		title,
		text,
		QMessageBox::Ok | QMessageBox::Cancel, // krazy:exclude=qclasses
		parent
	);
	message->setDefaultButton(QMessageBox::Cancel);
	QAbstractButton *ok = message->button(QMessageBox::Ok);
	ok->setIcon(icon());
	ok->setText(originalText());
	bool accepted = message->exec() == QMessageBox::Ok; // krazy:exclude=qclasses
	delete message;
	
	return accepted;
	#endif // KS_NATIVE_KDE
}

// protected

// NOTE: Sync. with "KCmdLineOptions" in main.cpp
void Action::addCommandLineArg(const QString &shortArg, const QString &longArg) {
	if (!shortArg.isEmpty())
		m_commandLineArgs.append(shortArg);
	if (!longArg.isEmpty())
		m_commandLineArgs.append(longArg);
}

void Action::disable(const QString &reason) {
	setEnabled(false);
	m_disableReason = reason;
}

bool Action::launch(const QString &program, const QStringList &args) {
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
// TODO: ?
	return (exitCode != 255);
}

bool Action::unsupportedAction() {
	m_error = i18n("Unsupported action: %0")
		.arg(originalText());

	return false;
}

// private slots

void Action::slotFire() {
	U_DEBUG << "Action::slotFire() [ id=" << m_id << " ]" U_END;

	if (!isEnabled()) {
		QString s = m_disableReason.isEmpty() ? i18n("Unknown error") : m_disableReason;
		U_ERROR_MESSAGE(0, text() + ": " + s);
		
		return;
	}

	m_error = QString::null;
	if (!onAction()) {
		m_totalExit = false;
		if (!m_error.isNull()) {
			QString s = m_error.isEmpty() ? i18n("Unknown error") : m_error;
			U_ERROR_MESSAGE(0, text() + ": " + s);
		}
	}
}

// ConfirmAction

// public

ConfirmAction::ConfirmAction(QObject *parent, Action *action) :
	U_ACTION(parent),
	m_impl(action) {

	// clone basic properties
	setEnabled(action->isEnabled());
	setIcon(action->icon());
	setMenu(action->menu());
	setShortcut(action->shortcut());
	setText(action->text());
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

// private

void ConfirmAction::slotFire() {
	if (m_impl->shouldStopTimer())
		MainWindow::self()->setActive(false);

	if (
		!Config::confirmAction() ||
		(m_impl == LockAction::self()) || // lock action - no confirmation
		m_impl->showConfirmationMessage()
	) {
		if (m_impl->authorize(0))
			m_impl->activate(false);
	}
}

// Trigger

// public

Trigger::Trigger(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
	m_supportsProgressBar(false),
	m_checkTimeout(500),
	m_icon(U_STOCK_ICON(iconName)),
	m_text(text),
	m_whatsThis(QString::null) {
}

// DateTimeTriggerBase

// public

DateTimeTriggerBase::DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id) :
	Trigger(text, iconName, id),
	m_dateTime(QDateTime()),
	m_endDateTime(QDateTime()),
	m_edit(0) {
}

DateTimeTriggerBase::~DateTimeTriggerBase() {
	if (m_edit) {
		delete m_edit;
		m_edit = 0;
	}
}

bool DateTimeTriggerBase::canActivateAction() {
	QDateTime now = QDateTime::currentDateTime();
	int secsTo;
	m_status = createStatus(now, secsTo);

	if (secsTo > 0) {
		MainWindow *mainWindow = MainWindow::self();
		mainWindow->progressBar()->setValue(secsTo);
		
		QString id = QString::null;
		if ((secsTo < 60) && (secsTo > 55))
			id = "1m";
		else if ((secsTo < 300) && (secsTo > 295))
			id = "5m";
		else if ((secsTo < 1800) && (secsTo > 1795))
			id = "30m";
		else if ((secsTo < 3600) && (secsTo > 3595))
			id = "1h";
		else if ((secsTo < 7200) && (secsTo > 7195))
			id = "2h";
		if (!id.isNull()) {
			emit notify(id, mainWindow->getDisplayStatus(MainWindow::DISPLAY_STATUS_HTML));
		}
	}
	
	return now >= m_endDateTime;
}

QWidget *DateTimeTriggerBase::getWidget() {
	if (!m_edit) {
		m_edit = new QDateTimeEdit();
		connect(m_edit, SIGNAL(dateChanged(const QDate &)), SLOT(syncDateTime()));
		connect(m_edit, SIGNAL(timeChanged(const QTime &)), SLOT(syncDateTime()));
		m_edit->setObjectName("date-time-edit");

		// larger font
		Utils::setFont(m_edit, 2, true);
	}

	return m_edit;
}

void DateTimeTriggerBase::readConfig(const QString &group, Config *config) {
	config->beginGroup(group);
	m_dateTime = config->read("Date Time", m_dateTime).toDateTime();
	config->endGroup();
}

void DateTimeTriggerBase::writeConfig(const QString &group, Config *config) {
	if (!m_edit)
		return;

	config->beginGroup(group);
	config->write("Date Time", m_edit->dateTime());
	config->endGroup();
}

void DateTimeTriggerBase::setDateTime(const QDateTime &dateTime) {
	m_dateTime = dateTime;
	m_edit->setDateTime(dateTime);
}

void DateTimeTriggerBase::setState(const State state) {
	if (state == StartState) {
		m_endDateTime = calcEndTime();
		
		// reset progress bar
		if (m_supportsProgressBar) {
			QDateTime now = QDateTime::currentDateTime();
			int secsTo = now.secsTo(m_endDateTime);
			ProgressBar *progressBar = MainWindow::self()->progressBar();
			progressBar->setTotal(secsTo);
			progressBar->setValue(0);
		}
	}
}

// protected

void DateTimeTriggerBase::updateStatus() {
	QDateTime now = QDateTime::currentDateTime();
	int secsTo;

	m_dateTime = m_edit->dateTime();
	m_endDateTime = calcEndTime();
	m_status = createStatus(now, secsTo);
}

// private slots

void DateTimeTriggerBase::syncDateTime() {
	updateStatus();
	emit statusChanged(false);
}

// private

QString DateTimeTriggerBase::createStatus(const QDateTime &now, int &secsTo) {
	m_statusType = InfoWidget::InfoType;
	
	secsTo = now.secsTo(m_endDateTime);
	if (secsTo > 0) {
		const int DAY = 86400;
		
		QString result;
		if (secsTo < DAY) {
			result = '+' + QTime(0, 0).addSecs(secsTo).toString(TIME_DISPLAY_FORMAT + "' ':' 'ss's'");
		}
		else {
			result += "24:00+";
		}
		
		result += " (";
// TODO: do not bold effective time
		result += i18n("selected time: %0").arg(m_endDateTime.toString(DATE_TIME_DISPLAY_FORMAT));
		result += ')';
		
		return result;
	}
	else if (secsTo == 0) {
		return QString::null;
	}
	else /* if (secsTo < 0) */ {
		m_statusType = InfoWidget::WarningType;
	
		return i18n("Invalid date/time");
	}
}

// DateTimeTrigger

// public

DateTimeTrigger::DateTimeTrigger() :
	DateTimeTriggerBase(i18n("At Date/Time"), "view-pim-calendar", "date-time")
{
	m_dateTime = QDateTime::currentDateTime().addSecs(60 * 60/* hour */); // set default
	m_supportsProgressBar = true;
}

QDateTime DateTimeTrigger::dateTime() {
	DateTimeTriggerBase::getWidget();

	return m_edit->dateTime();
}

QWidget *DateTimeTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setCalendarPopup(true);
	
	// Fix for BUG #2444169 - remember the previous shutdown settings
	m_edit->setDateTime(QDateTime(QDate::currentDate(), m_dateTime.time()));
	
	m_edit->setDisplayFormat(DATE_TIME_DISPLAY_FORMAT);
	m_edit->setMinimumDate(QDate::currentDate());
	//m_edit->setMinimumDateTime(QDateTime::currentDateTime());
	m_edit->setToolTip(i18n("Enter date and time"));

	return m_edit;
}

void DateTimeTrigger::setState(const State state) {
	DateTimeTriggerBase::setState(state);
	
	if (state == InvalidStatusState) {
		// show warning if selected date/time is invalid
		if (QDateTime::currentDateTime() >= dateTime()) {
			m_status = i18n("Invalid date/time");
			m_statusType = InfoWidget::WarningType;
		}
		else {
			updateStatus();
		}
	}
}

// protected

QDateTime DateTimeTrigger::calcEndTime() {
	return m_edit->dateTime();
}

// NoDelayTrigger

// public

NoDelayTrigger::NoDelayTrigger() :
	Trigger(i18n("No Delay"), "dialog-warning", "no-delay") {
}

bool NoDelayTrigger::canActivateAction() {
	return true;
}

// protected

QDateTime NoDelayTrigger::calcEndTime() {
	return QDateTime::currentDateTime();
}

// TimeFromNowTrigger

// public

TimeFromNowTrigger::TimeFromNowTrigger() :
	DateTimeTriggerBase(i18n("Time From Now (HH:MM)"), "chronometer", "time-from-now")
{
	m_dateTime.setTime(QTime(1, 0, 0)); // set default
	m_supportsProgressBar = true;
}

QWidget *TimeFromNowTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setDisplayFormat(TIME_DISPLAY_FORMAT);
	m_edit->setTime(m_dateTime.time());
	m_edit->setToolTip(i18n("Enter delay in \"HH:MM\" format (Hour:Minute)"));

	return m_edit;
}

// protected

QDateTime TimeFromNowTrigger::calcEndTime() {
	QTime time = m_dateTime.time();
	int m = (time.hour() * 60) + time.minute();
	
	return QDateTime::currentDateTime().addSecs(m * 60);
}

// PowerAction

// public

PowerAction::PowerAction(const QString &text, const QString &iconName, const QString &id) :
	Action(text, iconName, id),
	m_methodName(QString::null) {
}

bool PowerAction::onAction() {
// TODO: KDE: use Solid API (?)
#ifdef Q_OS_WIN32
	BOOL hibernate = (m_methodName == "Hibernate");
	BOOL result = ::SetSuspendState(hibernate, TRUE, FALSE); // krazy:exclude=captruefalse
	if (result == 0) {
		setLastError();

		return false;
	}

	return true;
#elif defined(Q_OS_HAIKU)
	return false;
#else
	// lock screen before hibernate/suspend
	if (Config::lockScreenBeforeHibernate()) {
		LockAction::self()->activate(false);

		// HACK: wait for screensaver
		::sleep(2);
	}
	
	// DOC: http://upower.freedesktop.org/docs/UPower.html
	QDBusInterface i_upower(
		"org.freedesktop.UPower",
		"/org/freedesktop/UPower",
		"org.freedesktop.UPower",
		QDBusConnection::systemBus()
	);
	
	// DOC: http://people.freedesktop.org/~david/hal-spec/hal-spec.html
	QDBusInterface i_hal(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device.SystemPowerManagement",
		QDBusConnection::systemBus()
	);
	
	QDBusError error;
	
	if (i_upower.isValid())
	{
		i_upower.call(m_methodName);
		
		error = i_upower.lastError();
	}
	else if (i_hal.isValid())
	{
	  	if (m_methodName == "Suspend")
			i_hal.call(m_methodName, 0);
		else
			i_hal.call(m_methodName); // no args
		
		error = i_hal.lastError();
	}
	
	if (
		(error.type() != QDBusError::NoError) &&
		// ignore missing reply after resume from suspend/hibernation
		(error.type() != QDBusError::NoReply) &&
		// ignore unknown errors
		(error.type() != QDBusError::Other)
	) {
		m_error = error.message();

		return false;
	}

	return true;
#endif // Q_OS_WIN32
}

// protected

bool PowerAction::isAvailable(const PowerActionType feature) const {
#ifdef Q_OS_WIN32
	if (feature == Hibernate)
		return ::IsPwrHibernateAllowed();

	if (feature == Suspend)
		return ::IsPwrSuspendAllowed();

	return false;
#elif defined(Q_OS_HAIKU)
	Q_UNUSED(feature)

	return false;
#else
	
	// Use the UPower backend if available
	QDBusInterface i_upower(
		"org.freedesktop.UPower",
		"/org/freedesktop/UPower",
		"org.freedesktop.UPower",
		QDBusConnection::systemBus()
	);

	// Use the legacy HAL backend if UPower not available
	QDBusInterface i_hal(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device",
		QDBusConnection::systemBus()
	);

	if (i_upower.isValid())
	{
		U_DEBUG << "UPower backend found ..." U_END;

		switch (feature) {
			case Suspend:
				return i_upower.property("CanSuspend").toBool();
			case Hibernate:
				return i_upower.property("CanHibernate").toBool();
		}
	}	
	else if (i_hal.isValid())
	{
		U_DEBUG << "HAL backend found ..." U_END;
			
		// try old property name as well, for backward compat.
		QList<QString> featureNames;
		switch (feature) {
			case Suspend:
				featureNames.append("power_management.can_suspend");
				featureNames.append("power_management.can_suspend_to_ram");
				break;
			case Hibernate:
				featureNames.append("power_management.can_hibernate");
				featureNames.append("power_management.can_suspend_to_disk");
				break;
		}
		
		QDBusReply<bool> reply;
		
		// Try the alternative feature names in order; if we get a valid answer, return it
		foreach (const QString &featureName, featureNames) {
			reply =  i_hal.call("GetProperty", featureName);
			if (reply.isValid())
				return reply.value();	
		}

		U_ERROR << reply.error() U_END;
	}
	
	return false;
#endif // Q_OS_WIN32
}

// HibernateAction

// public

HibernateAction::HibernateAction() :
	PowerAction(i18n("Hibernate Computer"), "system-suspend-hibernate", "hibernate") {
	m_methodName = "Hibernate";
	if (!isAvailable(Hibernate))
		disable(i18n("Cannot hibernate computer"));

	addCommandLineArg("H", "hibernate");
	
	// NOTE: Copied from http://en.wikipedia.org/wiki/Hibernate_(OS_feature)
	// Do not remove "Source" info.
	setWhatsThis("<qt>" + i18n(
"<p>Hibernate (or Suspend-to-Disk) is a feature of many computer operating systems where the contents of RAM are written to non-volatile storage such as a hard disk, as a file or on a separate partition, before powering off the computer.</p>" \
"<p>When the computer is restarted it reloads the content of memory and is restored to the state it was in when hibernation was invoked.</p>" \
"<p>Hibernating and later restarting is usually faster than closing down, later starting up, and starting all the programs that were running.</p>" \
"<p>Source: http://en.wikipedia.org/wiki/Hibernate_(OS_feature)</p>"
	) + "</qt>");
}

// SuspendAction

// public

SuspendAction::SuspendAction() :
	PowerAction(
		#ifdef Q_OS_WIN32
		i18n("Sleep"),
		#else
		i18n("Suspend Computer"),
		#endif // Q_OS_WIN32
		"system-suspend", "suspend") {
	m_methodName = "Suspend";
	if (!isAvailable(Suspend))
		disable(i18n("Cannot suspend computer"));

	addCommandLineArg("S", "suspend");
// TODO: menu item tool tips
	setWhatsThis(i18n("Enter in a low-power state mode."));
}

// StandardAction

// public

StandardAction::StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type) :
	Action(text, iconName, id),
	#ifdef KS_NATIVE_KDE
	m_kdeShutDownAvailable(false),
	#endif // KS_NATIVE_KDE
	m_type(type) {

// TODO: clean up kshutdown.cpp, move this to LogoutAction
	#ifdef KS_UNIX
	m_lxsession = 0;
	if (Utils::isLXDE() && (type == U_SHUTDOWN_TYPE_LOGOUT)) {
		bool ok = false;
		int i = qgetenv("_LXSESSION_PID").toInt(&ok);
		if (ok) {
			m_lxsession = i;
			U_DEBUG << "LXDE session found: " << m_lxsession U_END;
		}
		else {
			disable("No lxsession found");
		}
	}
	#endif // KS_UNIX
}

bool StandardAction::onAction() {
	m_totalExit = true;
#ifdef Q_OS_WIN32
	UINT flags = 0;
	switch (m_type) {
		case U_SHUTDOWN_TYPE_LOGOUT:
			flags = EWX_LOGOFF;
			break;
		case U_SHUTDOWN_TYPE_REBOOT:
			flags = EWX_REBOOT;
			break;
		case U_SHUTDOWN_TYPE_HALT:
			flags = EWX_POWEROFF;
			break;
		default:
			U_ERROR << "WTF? Unknown m_type: " << m_type U_END;

			return false; // do nothing
	}

	// adjust privileges

	HANDLE hToken = 0;
	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		TOKEN_PRIVILEGES tp;
		if (!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid)) {
			setLastError();

			return false;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!::AdjustTokenPrivileges(
			hToken,
			FALSE, // krazy:exclude=captruefalse
			&tp,
			sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)NULL,
			(PDWORD)NULL
		)) {
			setLastError();

			return false;
		}
/*
		if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			m_error = "ERROR_NOT_ALL_ASSIGNED";

			return false;
		}
*/
	}
	else {
		setLastError();

		return false;
	}

/*
	OSVERSIONINFO os;
	::ZeroMemory(&os, sizeof(OSVERSIONINFO));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (
		::GetVersionEx(&os) &&
		((os.dwMajorVersion == 5) && (os.dwMinorVersion == 1)) // win xp
	) {
	}
*/
		
	if (
		(m_type == U_SHUTDOWN_TYPE_HALT) ||
		(m_type == U_SHUTDOWN_TYPE_REBOOT)
	) {
		BOOL bForceAppsClosed = m_force ? TRUE : FALSE;
		m_force = false;
		
		BOOL bRebootAfterShutdown = (m_type == U_SHUTDOWN_TYPE_REBOOT) ? TRUE : FALSE;
		
		if (::InitiateSystemShutdown(
			NULL, NULL, 0, // unused
			bForceAppsClosed,
			bRebootAfterShutdown
		) == 0) {
// TODO: handle ERROR_NOT_READY
			if (::GetLastError() == ERROR_MACHINE_LOCKED) {
				
				bForceAppsClosed = TRUE;
				
				if (::InitiateSystemShutdown(
					NULL, NULL, 0, // unused
					bForceAppsClosed,
					bRebootAfterShutdown
				) == 0) {
					setLastError();
					
					return false;
				}
			}
			else {
				setLastError();
				
				return false;
			}
		}
	}
	else {
		flags += EWX_FORCEIFHUNG;
		if (m_force) {
			flags += EWX_FORCE;
			m_force = false;
		}

		#define SHTDN_REASON_MAJOR_APPLICATION 0x00040000
		#define SHTDN_REASON_FLAG_PLANNED 0x80000000
		if (::ExitWindowsEx(flags, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_FLAG_PLANNED) == 0) {
			setLastError();

			return false;
		}
	}

	return true;

#elif defined(Q_OS_HAIKU)

	if (m_type == U_SHUTDOWN_TYPE_REBOOT) {
		QStringList args;
		args << "-r";
		
		if (launch("shutdown", args))
			return true;
	}
	else if (m_type == U_SHUTDOWN_TYPE_HALT) {
		QStringList args;
		
		if (launch("shutdown", args))
			return true;
	}
	
	return false;
	
#else

	// GNOME Shell, Unity
	
	if (Utils::isGNOME_3() || Utils::isUnity()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "--logout";
			args << "--no-prompt";
			if (launch("gnome-session-quit", args)) {
				// HACK: session save issue?
				U_APP->quit();
			
				return true;
			}
		}
	}

/* TODO: GNOME shutdown
	if (Utils::isGNOME()) {
		QStringList args;
		args << "--kill";
		args << "--silent";
		if (launch("gnome-session-save", args))
			return true;
	}
*/

	// LXDE

	else if (Utils::isLXDE()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			#ifdef KS_UNIX
			if (m_lxsession && (::kill(m_lxsession, SIGTERM) == 0))
				return true;
			#else
				return false;
			#endif // KS_UNIX
		}
	}

	// Xfce

/* FIXME: causes IO error in session manager (?)
	else if (Utils::isXfce()) {
		switch (m_type) {
			case U_SHUTDOWN_TYPE_LOGOUT: {
				QStringList args;
				args << "--logout";
				if (launch("xfce4-session-logout", args))
					return true;
			} break;
			case U_SHUTDOWN_TYPE_REBOOT: {
				QStringList args;
				args << "--reboot";
				if (launch("xfce4-session-logout", args))
					return true;
			} break;
			case U_SHUTDOWN_TYPE_HALT: {
				QStringList args;
				args << "--halt";
				if (launch("xfce4-session-logout", args))
					return true;
			} break;
			default:
				U_ERROR << "WTF? Unknown m_type: " << m_type U_END;

				return false; // do nothing
		}
	}
*/

	// Enlightenment

	else if (Utils::isEnlightenment()) {
// TODO: use D-Bus
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "-exit";
			if (launch("enlightenment_remote", args))
				return true;
		}
	}

	// native KDE shutdown API

	#ifdef KS_NATIVE_KDE
	if (m_kdeShutDownAvailable || (m_type == U_SHUTDOWN_TYPE_LOGOUT)) {
// TODO: remove KWorkSpace API deps (?)
// TODO: check if logout is available
		// BUG #3467712: http://sourceforge.net/p/kshutdown/bugs/13/
		KWorkSpace::requestShutDown(
			KWorkSpace::ShutdownConfirmNo,
			m_type,
			KWorkSpace::ShutdownModeForceNow
		);

		return true;
	}
	#endif // KS_NATIVE_KDE
	
	// fallback to ConsoleKit or HAL
	
	#ifdef KS_DBUS
	MainWindow::self()->writeConfig();
	
	// try ConsoleKit
	
	if ((m_type == U_SHUTDOWN_TYPE_HALT) && m_consoleKitInterface && m_consoleKitInterface->isValid()) {
		QDBusReply<void> reply = m_consoleKitInterface->call("Stop");

		if (reply.isValid())
			return true;
	}
	else if ((m_type == U_SHUTDOWN_TYPE_REBOOT) && m_consoleKitInterface && m_consoleKitInterface->isValid()) {
		QDBusReply<void> reply = m_consoleKitInterface->call("Restart");

		if (reply.isValid())
			return true;
	}

	// try HAL

	else if ((m_type == U_SHUTDOWN_TYPE_HALT) && m_halInterface && m_halInterface->isValid()) {
		QDBusReply<int> reply = m_halInterface->call("Shutdown");

		if (reply.isValid())
			return true;
	}
	else if ((m_type == U_SHUTDOWN_TYPE_REBOOT) && m_halInterface && m_halInterface->isValid()) {
		QDBusReply<int> reply = m_halInterface->call("Reboot");

		if (reply.isValid())
			return true;
	}

	#endif // KS_DBUS
	
	// show error
	
	return unsupportedAction();
#endif // Q_OS_WIN32
}

// protected

void StandardAction::checkAvailable(const UShutdownType type, const QString &consoleKitName) {
	#ifdef KS_PURE_QT
	Q_UNUSED(type)
	#endif // KS_PURE_QT

	bool available = false;
	QString error = "";

	#ifdef Q_OS_HAIKU
	Q_UNUSED(consoleKitName)
	
	return;
	#endif //  Q_OS_HAIKU

	#ifdef Q_OS_WIN32
	Q_UNUSED(consoleKitName)
	
// TODO: win32: check if shutdown/reboot action is available
	return;
	#endif // Q_OS_WIN32

	#ifdef KS_NATIVE_KDE
	if (Utils::isKDEFullSession()) {
		m_kdeShutDownAvailable = KWorkSpace::canShutDown(
			KWorkSpace::ShutdownConfirmNo,
			type,
			KWorkSpace::ShutdownModeForceNow
		);
		
		U_DEBUG << "KDE ShutDown API available:" << m_kdeShutDownAvailable U_END;

		if (m_kdeShutDownAvailable)
			return;

		error = "Check \"Offer shutdown options\"<br>in the \"Session Management\" settings<br>(KDE System Settings).";
	}
	#endif // KS_NATIVE_KDE

	#ifdef KS_DBUS
	if (!consoleKitName.isEmpty()) {
		error = ""; // reset
	
		if (!m_consoleKitInterface) {
			m_consoleKitInterface = new QDBusInterface(
				"org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				QDBusConnection::systemBus()
			);
		}
		if (m_consoleKitInterface->isValid()) {
			QDBusReply<bool> reply = m_consoleKitInterface->call(consoleKitName);
			if (!reply.isValid()) {
				U_ERROR << reply.error().message() U_END;
				error = reply.error().name();
			}
			else {
				available = reply.value();
			}
		}
		else {
// FIXME: this sometimes returns error (service timeout?)
			U_ERROR << "ConsoleKit Error:" << m_consoleKitInterface->lastError().message() U_END;
			error = "No valid org.freedesktop.ConsoleKit interface found";
		}
	}

	if (!m_halInterface) {
		m_halInterface = new QDBusInterface(
			"org.freedesktop.Hal",
			"/org/freedesktop/Hal/devices/computer",
			"org.freedesktop.Hal.Device.SystemPowerManagement",
			QDBusConnection::systemBus()
		);
	}
	if (m_halInterface->isValid()) {
		available = true;
	}
	else {
		U_ERROR << "HAL Error:" << m_halInterface->lastError().message() U_END;
		if (error.isEmpty())
			error = "No valid org.freedesktop.Hal interface found";
	}
	#endif // KS_DBUS

	if (!available)
		disable(error);
}

// LogoutAction

// public

LogoutAction::LogoutAction() :
	StandardAction(
		#ifdef Q_OS_WIN32
		i18n("Log Off"),
		#else
		i18n("Logout"),
		#endif // Q_OS_WIN32
		"system-log-out", "logout", U_SHUTDOWN_TYPE_LOGOUT
) {
	addCommandLineArg("l", "logout");

	#ifdef Q_OS_HAIKU
	disable("");
	#endif // Q_OS_HAIKU
	
// TODO: KDE 4 logout, test
	#ifdef KS_PURE_QT
	#ifdef KS_UNIX
	if (Utils::isKDE_4())
		disable("");
	else if (Utils::isXfce())
		disable("");
	#endif // KS_UNIX
	#endif // KS_PURE_QT
}

// RebootAction

// public

RebootAction::RebootAction() :
	StandardAction(i18n("Restart Computer"), QString::null, "reboot", U_SHUTDOWN_TYPE_REBOOT) {

#ifdef KS_NATIVE_KDE
/* FIXME: crash on KDE 4.5.0
	QPixmap p = KIconLoader::global()->loadIcon(
		"system-reboot",
		//"dialog-ok",
		KIconLoader::NoGroup,
		0, // default size
		KIconLoader::DefaultState,
		QStringList(), // no emblems
		0L, // no path store
		true // return "null" if no icon
	);
	if (p.isNull())
		setIcon(U_STOCK_ICON("system-restart"));
	else
		setIcon(p);
*/
	setIcon(U_STOCK_ICON("system-reboot"));
#else
	if (Utils::isKDE_4())
		setIcon(U_STOCK_ICON("system-reboot"));
	// HACK: missing "system-reboot" icon
	else
		setIcon(U_STOCK_ICON("view-refresh"));
#endif // KS_NATIVE_KDE

	addCommandLineArg("r", "reboot");
	
	checkAvailable(U_SHUTDOWN_TYPE_REBOOT, "CanRestart");
}

// ShutDownAction

// public

ShutDownAction::ShutDownAction() :
	StandardAction(i18n("Turn Off Computer"), "system-shutdown", "shutdown", U_SHUTDOWN_TYPE_HALT) {
/* TODO: IsPwrShutdownAllowed()
#ifdef Q_OS_WIN32
	setEnabled();
#endif // Q_OS_WIN32
*/

	addCommandLineArg("h", "halt");
	addCommandLineArg("s", "shutdown");

	checkAvailable(U_SHUTDOWN_TYPE_HALT, "CanStop");
}
