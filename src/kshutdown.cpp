//
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

#include "kshutdown.h"
#include "mainwindow.h"

using namespace KShutdown;

bool Action::m_totalExit = false;
LockAction *LockAction::m_instance = 0;

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
// FIXME: 0 parent - is this OK?
	U_ACTION(0),
	Base(id),
	m_force(false),
	m_shouldStopTimer(true),
	m_showInMenu(true) {
	m_originalText = text;
	setIcon(U_STOCK_ICON(iconName));
	setText(text);
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

void Action::activate(const bool force) {
	m_force = force;
	U_ACTION::activate(Trigger);
}

// protected

void Action::disable(const QString &reason) {
	setEnabled(false);
	m_disableReason = reason;
}

bool Action::launch(const QString &program, const QStringList &args) {
	U_DEBUG << "Launching \"" << program << "\" with \"" << args << "\" arguments" U_END;

	int exitCode = QProcess::execute(program, args);
	U_DEBUG << "Exit code: " << exitCode U_END;

	return (exitCode != 255);//!!!
}

// private slots

void Action::slotFire() {
	U_DEBUG << "Action::slotFire() [ id=" << m_id << " ]" U_END;

	m_error = QString::null;
	if (!onAction()) {
		m_totalExit = false;
		QString text = m_error.isEmpty() ? i18n("Unknown error") : m_error;
		U_ERROR_MESSAGE(0, m_error);
	}
}

// ConfirmAction

// public

ConfirmAction::ConfirmAction(Action *action) :
	U_ACTION(0),
	m_impl(action) {
// TODO: find a better way to clone action
	setEnabled(action->isEnabled());
	setIcon(action->icon());
	setMenu(action->menu());
	setText(action->text());
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

// private

void ConfirmAction::slotFire() {
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
	m_text(text) {
}

// DateTimeTriggerBase

// public

DateTimeTriggerBase::DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id) :
	Trigger(text, iconName, id),
	m_dateTime(QDateTime()),
	m_endDateTime(QDateTime()),
	m_edit(0) {
}

bool DateTimeTriggerBase::canActivateAction() {
	QDateTime now = QDateTime::currentDateTime();

	int secsTo = now.secsTo(m_endDateTime);
	if (secsTo > 0) {
		#define KS_DAY 86400
		if (secsTo < KS_DAY)
			m_status = QTime().addSecs(secsTo).toString(Qt::ISODate);
		else
			m_status = "24+";
	}
	else {
		m_status = QString::null;
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
		QFont newFont(m_edit->font());
		newFont.setBold(true);
		int size = newFont.pointSize();
		if (size != -1) {
			newFont.setPointSize(size + 2);
		}
		else {
			size = newFont.pixelSize();
			newFont.setPixelSize(size + 2);
		}
		m_edit->setFont(newFont);
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

// private slots

void DateTimeTriggerBase::syncDateTime() {
	m_dateTime = m_edit->dateTime();
}

// DateTimeTrigger

// public

DateTimeTrigger::DateTimeTrigger() :
	DateTimeTriggerBase(i18n("At Date/Time"), "calendar-today", "date-time") {
}

QWidget *DateTimeTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setCalendarPopup(true);
	m_edit->setDateTime(QDateTime::currentDateTime());
	m_edit->setDisplayFormat("yyyy MM dddd hh:mm");
	m_edit->setMinimumDate(QDate::currentDate());

	return m_edit;
}

void DateTimeTrigger::setState(const State state) {
	if (state == START) {
		m_endDateTime = m_edit->dateTime();
	}
}

// NoDelayTrigger

// public

NoDelayTrigger::NoDelayTrigger() :
	Trigger(i18n("No Delay"), "dialog-warning", "no-delay") {
}

bool NoDelayTrigger::canActivateAction() {
	return true;
}

// TimeFromNowTrigger

// public

TimeFromNowTrigger::TimeFromNowTrigger() :
	DateTimeTriggerBase(i18n("Time From Now (HH:MM)"), "alarmclock", "time-from-now") {
}

QWidget *TimeFromNowTrigger::getWidget() {
	DateTimeTriggerBase::getWidget();

	m_edit->setDisplayFormat("hh:mm");
	m_edit->setTime(m_dateTime.time());

	return m_edit;
}

void TimeFromNowTrigger::setState(const State state) {
	if (state == START) {
		QTime time = m_dateTime.time();
		int m = (time.hour() * 60) + time.minute();
		m_endDateTime = QDateTime::currentDateTime().addSecs(m * 60);
	}
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
	LockAction::self()->activate(false);

	// wait for screensaver
	::sleep(2);

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
	if (feature == "power_management.can_suspend_to_disk")
		return ::IsPwrHibernateAllowed();

	if (feature == "power_management.can_suspend_to_ram")
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

	if (reply.isValid())
		return reply.value();

	U_ERROR << reply.error() U_END;

	return false;
#endif // Q_WS_WIN
}

// HibernateAction

// public

HibernateAction::HibernateAction() :
	PowerAction(i18n("Hibernate Computer"), "system-hibernate", "hibernate") {
	m_methodName = "Hibernate";
	if (!isAvailable("power_management.can_suspend_to_disk"))
		disable(i18n("Cannot suspend to disk"));
}

// SuspendAction

// public

SuspendAction::SuspendAction() :
	PowerAction(i18n("Suspend Computer"), "system-suspend", "suspend") {
	m_methodName = "Suspend";
	if (!isAvailable("power_management.can_suspend_to_ram"))
		disable(i18n("Cannot suspend to RAM"));
}

// LockAction

// public

LockAction::LockAction() :
	Action(i18n("Lock Screen"), "system-lock-screen", "lock") {
	setShouldStopTimer(false);
}

bool LockAction::onAction() {
#ifdef Q_WS_WIN
	BOOL result = ::LockWorkStation();
	if (result == 0) {
// TODO: test error message
		setLastError();

		return false;
	}

	return true;
#else
	// 1. try DBus
	QDBusInterface i("org.freedesktop.ScreenSaver", "/ScreenSaver");
	i.call("Lock");
	QDBusError error = i.lastError();
	if (error.type() != QDBusError::NoError) {
		// 2. try "xdg-screensaver" command
		QStringList args;
		args << "lock";
		if (!launch("xdg-screensaver", args)) {
			// 3. try "xscreensaver-command" command
			args.clear();
			args << "-lock";
			if (!launch("xscreensaver-command", args)) {
				// do not set "m_error" because it may block auto shutdown
				U_ERROR << "Could not lock the screen" U_END;

				return false;
			}
		}
	}

	return true;
#endif // Q_WS_WIN
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
		return false;//!!!
	#endif // KS_NATIVE_KDE
#endif // Q_WS_WIN
}

// LogoutAction

// public

LogoutAction::LogoutAction() :
// FIXME: "system-log-out" icon?
	StandardAction(i18n("Logout"), "edit-undo", "logout", U_SHUTDOWN_TYPE_LOGOUT) {
}

// RebootAction

// public

RebootAction::RebootAction() :
	StandardAction(i18n("Restart Computer"), "view-refresh", "reboot", U_SHUTDOWN_TYPE_REBOOT) {
}

// ShutDownAction

// public

ShutDownAction::ShutDownAction() :
	StandardAction(i18n("Turn Off Computer"), "application-exit", "shutdown", U_SHUTDOWN_TYPE_HALT) {
/* TODO: IsPwrShutdownAllowed()
#ifdef Q_WS_WIN
	setEnabled();
#endif // Q_WS_WIN
*/
}
