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

#include <QDateTimeEdit>
#include <QProcess>
#ifdef Q_WS_WIN
	#define _WIN32_WINNT 0x0500 // for LockWorkStation, etc
	#include <windows.h>
	#include <powrprof.h>
#else
	#include <QDBusInterface>
	#include <QDBusReply>

	#include <unistd.h> // for sleep
#endif // Q_WS_WIN

#include "actions/lock.h"
#include "kshutdown.h"
#include "mainwindow.h"
#include "progressbar.h"
#include "utils.h"

using namespace KShutdown;

bool Action::m_totalExit = false;

// Base

// public

Base::Base(const QString &id) :
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

#ifdef Q_WS_WIN
// CREDITS: http://lists.trolltech.com/qt-solutions/2005-05/msg00005.html
void Base::setLastError() {
	char *buffer = 0;
	::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		::GetLastError(),
		0,
		(char *)&buffer,
		0,
		0
	);
	m_error = QString::fromLocal8Bit(buffer);
	if (buffer)
		::LocalFree(buffer);
}
#endif // Q_WS_WIN

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
	setText(text);
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

void Action::activate(const bool force) {
	m_force = force;
	U_ACTION::trigger();
}

bool Action::isCommandLineArgSupported() {
	foreach (QString i, m_commandLineArgs) {
		if (Utils::isArg(i))
			return true;
	}
	
	return false;
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
	U_DEBUG << "Exit code: " << exitCode U_END;

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

	m_error = QString::null;
	if (!onAction()) {
		m_totalExit = false;
		if (!m_error.isNull()) {
			QString text = m_error.isEmpty() ? i18n("Unknown error") : m_error;
			U_ERROR_MESSAGE(0, text);
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
	setText(action->text());
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

// private

void ConfirmAction::slotFire() {
// FIXME: remove MainWindow deps.
	if (m_impl->shouldStopTimer())
		MainWindow::self()->setActive(false);

// TODO: show confirmation message near the system tray icon
// if main window is hidden
	if (
		!Config::confirmAction() ||
		(m_impl == LockAction::self()) || // lock action - no confirmation
		U_CONFIRM(0, m_impl->originalText(), i18n("Are you sure?"))
	)
		m_impl->activate(false);
}

// Trigger

// public

Trigger::Trigger(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
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
		if (ProgressBar::isInstance())
			ProgressBar::self()->setProgress(secsTo);

		MainWindow *mainWindow = MainWindow::self();
		
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
		
		if (Config::user()->progressBarEnabled()) {
			QDateTime now = QDateTime::currentDateTime();
			int secsTo = now.secsTo(m_endDateTime);
			ProgressBar::self()->setTotal(secsTo);
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
	secsTo = now.secsTo(m_endDateTime);
	if (secsTo > 0) {
		const int DAY = 86400;
		
		QString result;
		if (secsTo < DAY) {
			result = "+" + QTime().addSecs(secsTo).toString(TIME_FORMAT + ":ss");
		}
		else {
			result += "24:00+";
		}
		
		result += " (";
// TODO: do not bold effective time
		result += m_endDateTime.toString(DATE_TIME_FORMAT);		
		result += ")";
		
		return result;
	}
	else if (secsTo == 0) {
		return QString::null;
	}
	else /* if (secsTo < 0) */ {
		return i18n("Invalid date/time");
	}
}

// DateTimeTrigger

// public

DateTimeTrigger::DateTimeTrigger() :
	DateTimeTriggerBase(i18n("At Date/Time"), "view-pim-calendar", "date-time") {
}

QWidget *DateTimeTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setCalendarPopup(true);
	
	// Fix for BUG #2444169 - remeber the previous shutdown settings
	m_edit->setDateTime(QDateTime(QDate::currentDate(), m_dateTime.time()));
	
	m_edit->setDisplayFormat(DATE_TIME_FORMAT);
	m_edit->setMinimumDate(QDate::currentDate());
	//m_edit->setMinimumDateTime(QDateTime::currentDateTime());
	m_edit->setToolTip(i18n("Enter date and time"));

	return m_edit;
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
	DateTimeTriggerBase(i18n("Time From Now (HH:MM)"), "chronometer", "time-from-now") {
}

QWidget *TimeFromNowTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setDisplayFormat(TIME_FORMAT);
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
#ifdef Q_WS_WIN
	BOOL hibernate = (m_methodName == "Hibernate");
	BOOL result = ::SetSuspendState(hibernate, TRUE, FALSE);
	if (result == 0) {
		setLastError();

		return false;
	}

	return true;
#else
	// lock screen before hibernate/suspend
	if (Config::lockScreenBeforeHibernate()) {
		LockAction::self()->activate(false);

		// wait for screensaver
		::sleep(2);
	}

	// DOC: http://people.freedesktop.org/~david/hal-spec/hal-spec.html
	QDBusInterface i(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device.SystemPowerManagement",
		QDBusConnection::systemBus()
	);
	if (m_methodName == "Suspend")
		i.call(m_methodName, 0);
	else
		i.call(m_methodName); // no args

	QDBusError error = i.lastError();
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
#endif // Q_WS_WIN
}

// protected

bool PowerAction::isAvailable(const QString &feature) const {
#ifdef Q_WS_WIN
	if (feature == "power_management.can_hibernate")
		return ::IsPwrHibernateAllowed();

	if (feature == "power_management.can_suspend")
		return ::IsPwrSuspendAllowed();

	return false;
#else
	QDBusInterface *i = new QDBusInterface(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device",
		QDBusConnection::systemBus()
	);
	QDBusReply<bool> reply = i->call("GetProperty", feature);
	delete i;

	if (reply.isValid())
		return reply.value();
		
	// try old property name for backward compat.
	U_DEBUG << "Using old HAL property names..." U_END;
	
	QString oldFeatureName = QString::null;
	if (feature == "power_management.can_hibernate")
		oldFeatureName = "power_management.can_suspend_to_disk";
	else if (feature == "power_management.can_suspend")
		oldFeatureName = "power_management.can_suspend_to_ram";
	if (!oldFeatureName.isNull()) {
		reply = i->call("GetProperty", oldFeatureName);
	
		if (reply.isValid())
			return reply.value();
	}

	U_ERROR << reply.error() U_END;

	return false;
#endif // Q_WS_WIN
}

// HibernateAction

// public

HibernateAction::HibernateAction() :
	PowerAction(i18n("Hibernate Computer"), "system-suspend-hibernate", "hibernate") {
	m_methodName = "Hibernate";
	if (!isAvailable("power_management.can_hibernate"))
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
	PowerAction(i18n("Suspend Computer (Sleep)"), "system-suspend", "suspend"
	) {
	m_methodName = "Suspend";
	if (!isAvailable("power_management.can_suspend"))
		disable(i18n("Cannot suspend computer"));

	addCommandLineArg("S", "suspend");
	
	setWhatsThis(i18n("Enter in a low-power state."));
}

// StandardAction

// public

StandardAction::StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type) :
	Action(text, iconName, id),
	m_type(type) {
}

bool StandardAction::onAction() {
	m_totalExit = true;
#ifdef Q_WS_WIN
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
			FALSE,
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

	return true;
#else
	
	// GNOME

	if (Utils::isGNOME()) {
		QStringList args;
		args << "--kill";
		args << "--silent";
		if (launch("gnome-session-save", args))
			return true;
	}
	
	// Xfce
	
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

	#ifdef KS_NATIVE_KDE
		if (KWorkSpace::requestShutDown(
			KWorkSpace::ShutdownConfirmNo,
			m_type,
			KWorkSpace::ShutdownModeForceNow
		))
			return true;

		m_error = i18n(
			"Could not logout properly.\n" \
			"The session manager cannot be contacted."
		);

		return false;
	#else
		return unsupportedAction();
	#endif // KS_NATIVE_KDE
#endif // Q_WS_WIN
}

// LogoutAction

// public

LogoutAction::LogoutAction() :
	StandardAction(i18n("Logout"), "system-log-out", "logout", U_SHUTDOWN_TYPE_LOGOUT) {

	addCommandLineArg("l", "logout");
}

// RebootAction

// public

RebootAction::RebootAction() :
	StandardAction(i18n("Restart Computer"), QString::null, "reboot", U_SHUTDOWN_TYPE_REBOOT) {

#ifdef KS_NATIVE_KDE
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
#else
	setIcon(U_STOCK_ICON("system-reboot"));
#endif // KS_NATIVE_KDE

	addCommandLineArg("r", "reboot");
}

// ShutDownAction

// public

ShutDownAction::ShutDownAction() :
	StandardAction(i18n("Turn Off Computer"), "system-shutdown", "shutdown", U_SHUTDOWN_TYPE_HALT) {
/* TODO: IsPwrShutdownAllowed()
#ifdef Q_WS_WIN
	setEnabled();
#endif // Q_WS_WIN
*/

	addCommandLineArg("h", "halt");
	addCommandLineArg("s", "shutdown");
}
