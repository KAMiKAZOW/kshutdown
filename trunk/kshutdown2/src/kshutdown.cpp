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

#include "kshutdown.h"

#include "commandline.h"
#include "config.h"
#include "log.h"
#include "mainwindow.h"
#include "password.h"
#include "progressbar.h"
#include "utils.h"
#include "actions/bootentry.h"
#include "actions/lock.h"

#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <powrprof.h>
#else
	#include <csignal> // for ::kill

	#include <QThread>
#endif // Q_OS_WIN32

using namespace KShutdown;

bool Action::m_totalExit = false;
#ifdef QT_DBUS_LIB
QDBusInterface *Action::m_loginInterface = nullptr;

QDBusInterface *PowerAction::m_halDeviceInterface = nullptr;
QDBusInterface *PowerAction::m_halDeviceSystemPMInterface = nullptr;
QDBusInterface *PowerAction::m_upowerInterface = nullptr;

bool StandardAction::m_kdeShutDownAvailable = false;
QDBusInterface *StandardAction::m_consoleKitInterface = nullptr;
QDBusInterface *StandardAction::m_kdeSessionInterface = nullptr;
QDBusInterface *StandardAction::m_lxqtSessionInterface = nullptr;
QDBusInterface *StandardAction::m_razorSessionInterface = nullptr;
#endif // QT_DBUS_LIB

// DateTimeEdit

// public:

DateTimeEdit::DateTimeEdit()
	: QDateTimeEdit()
{
	connect(
		lineEdit(), SIGNAL(selectionChanged()),
		SLOT(onLineEditSelectionChange())
	);
}

DateTimeEdit::~DateTimeEdit() = default;

// private slots:

void DateTimeEdit::onLineEditSelectionChange() {
	if (displayedSections() != (HourSection | MinuteSection))
		return;

	QString selectedText = lineEdit()->selectedText();
	// HACK: Change selection from "15h" to "15" after double click.
	//       This fixes Up/Down keys and mouse wheel editing.
	if (selectedText == time().toString("hh'h'"))
		lineEdit()->setSelection(0, 2);
	else if (selectedText == time().toString("mm'm'"))
		lineEdit()->setSelection(6, 2);
}

// DateTimeTriggerBase

// public

DateTimeTriggerBase::DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id) :
	Trigger(text, iconName, id),
	m_dateTime(QDateTime()),
	m_endDateTime(QDateTime())
{
}

DateTimeTriggerBase::~DateTimeTriggerBase() {
	delete m_edit;
	m_edit = nullptr;
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
		m_edit = new DateTimeEdit();
		connect(m_edit, SIGNAL(dateChanged(const QDate &)), SLOT(syncDateTime()));
		connect(m_edit, SIGNAL(timeChanged(const QTime &)), SLOT(syncDateTime()));
		m_edit->setObjectName("date-time-edit");

		// larger font
		Utils::setFont(m_edit, 2, true);
	}

	return m_edit;
}

QString DateTimeTriggerBase::longDateTimeFormat() {
	QString dateFormat = "MMM d dddd";
	QString timeFormat = QLocale::system()
		.timeFormat(QLocale::ShortFormat);

	if (timeFormat == "h:mm:ss AP")
		timeFormat = "h:mm AP";
	else
		timeFormat = "hh:mm";
	
	return dateFormat + ' ' + timeFormat;
}

void DateTimeTriggerBase::readConfig(Config *config) {
	m_dateTime = config->read("Date Time", m_dateTime).toDateTime();
}

void DateTimeTriggerBase::writeConfig(Config *config) {
	if (m_edit)
		config->write("Date Time", m_edit->dateTime());
}

void DateTimeTriggerBase::setDateTime(const QDateTime &dateTime) {
	m_dateTime = dateTime;
	m_edit->setDateTime(dateTime);
}

void DateTimeTriggerBase::setState(const State state) {
	if (state == State::Start) {
		m_endDateTime = calcEndTime();
		
		// reset progress bar
		if (m_supportsProgressBar) {
			QDateTime now = QDateTime::currentDateTime();
			int secsTo = now.secsTo(m_endDateTime);
			ProgressBar *progressBar = MainWindow::self()->progressBar();
			progressBar->setTotal(secsTo);
			progressBar->setValue(-1);
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
	m_statusType = InfoWidget::Type::Info;
	
	secsTo = now.secsTo(m_endDateTime);
	if (secsTo > 0) {
		const int DAY = 86'400;
		
		QString result;
		if (secsTo < DAY) {
			result = '+' + QTime(0, 0).addSecs(secsTo).toString(LONG_TIME_DISPLAY_FORMAT);
		}
		else {
			result += "24h+";
		}
		
		result += " (";

		if (secsTo >= DAY)
			result += i18n("not recommended") + ", ";

// TODO: do not bold effective time
		result += i18n("selected time: %0").arg(m_endDateTime.toString(longDateTimeFormat()));
		result += ')';
		
		return result;
	}
	else if (secsTo == 0) {
		return QString::null;
	}
	else /* if (secsTo < 0) */ {
		m_statusType = InfoWidget::Type::Warning;
	
		return i18n("Invalid date/time");
	}
}

// DateTimeTrigger

// public

DateTimeTrigger::DateTimeTrigger() :
	DateTimeTriggerBase(i18n("At Date/Time"), "view-pim-calendar", "date-time")
{
	setCanBookmark(true);

	m_dateTime = QDateTime::currentDateTime().addSecs(60 * 60/* hour */); // set default
	m_supportsProgressBar = true;
}

QDateTime DateTimeTrigger::dateTime() {
	DateTimeTriggerBase::getWidget();

	return m_edit->dateTime();
}

QString DateTimeTrigger::getStringOption() {
	if (!m_edit)
		return QString::null;
	
	return m_edit->time().toString(TIME_PARSE_FORMAT);
}

void DateTimeTrigger::setStringOption(const QString &option) {
	if (!m_edit)
		return;

	QDate date;
	QTime time = QTime::fromString(option, TIME_PARSE_FORMAT);

	// select next day if time is less than current time
	if (time < QTime::currentTime())
		date = QDate::currentDate().addDays(1);
	else
		date = m_edit->date();
	m_edit->setDateTime(QDateTime(date, time));
}

QWidget *DateTimeTrigger::getWidget() {
	bool initDateTime = !m_edit;

	DateTimeTriggerBase::getWidget();

	m_edit->setCalendarPopup(true);
	
	// Fix for BUG #2444169 - remember the previous shutdown settings
	if (initDateTime) {
		QDate date = m_dateTime.date();
		QTime time = m_dateTime.time();

		// select next day if time is less than current time
		if ((date == QDate::currentDate()) && (time < QTime::currentTime()))
			date = date.addDays(1);
		m_edit->setDateTime(QDateTime(date, time));
	}
	
	m_edit->setDisplayFormat(DateTimeTriggerBase::longDateTimeFormat());
	m_edit->setMinimumDate(QDate::currentDate());
	//m_edit->setMinimumDateTime(QDateTime::currentDateTime());
	m_edit->setToolTip(i18n("Enter date and time"));

	return m_edit;
}

void DateTimeTrigger::setState(const State state) {
	DateTimeTriggerBase::setState(state);
	
	if (state == State::InvalidStatus) {
		// show warning if selected date/time is invalid
		if (QDateTime::currentDateTime() >= dateTime()) {
			m_status = i18n("Invalid date/time");
			m_statusType = InfoWidget::Type::Warning;
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
	
	setCanBookmark(true);
}

// TimeFromNowTrigger

// public

TimeFromNowTrigger::TimeFromNowTrigger() :
	DateTimeTriggerBase(i18n("Time From Now (HH:MM)"), "chronometer", "time-from-now")
{
	setCanBookmark(true);

	m_dateTime.setTime(QTime(1, 0, 0)); // set default
	m_supportsProgressBar = true;
}

QString TimeFromNowTrigger::getStringOption() {
	if (!m_edit)
		return QString::null;

	return m_edit->time().toString(TIME_PARSE_FORMAT);
}

void TimeFromNowTrigger::setStringOption(const QString &option) {
	if (!m_edit)
		return;

	QTime time = QTime::fromString(option, TIME_PARSE_FORMAT);
	m_edit->setTime(time);
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
	
	setCanBookmark(true);
}

#ifdef QT_DBUS_LIB
QDBusInterface *PowerAction::getHalDeviceInterface() {
	if (!m_halDeviceInterface) {
		// DOC: http://people.freedesktop.org/~dkukawka/hal-spec-git/hal-spec.html
		m_halDeviceInterface = new QDBusInterface(
			"org.freedesktop.Hal",
			"/org/freedesktop/Hal/devices/computer",
			"org.freedesktop.Hal.Device",
			QDBusConnection::systemBus()
		);
	}
	if (m_halDeviceInterface->isValid())
		qDebug() << "HAL backend found...";
	else
		qCritical() << "HAL backend NOT found: " << m_halDeviceInterface->lastError().message();

	return m_halDeviceInterface;
}

QDBusInterface *PowerAction::getHalDeviceSystemPMInterface() {
	if (!m_halDeviceSystemPMInterface) {
		// DOC: http://people.freedesktop.org/~dkukawka/hal-spec-git/hal-spec.html
		m_halDeviceSystemPMInterface = new QDBusInterface(
			"org.freedesktop.Hal",
			"/org/freedesktop/Hal/devices/computer",
			"org.freedesktop.Hal.Device.SystemPowerManagement",
			QDBusConnection::systemBus()
		);
	}
	if (!m_halDeviceSystemPMInterface->isValid())
		qCritical() << "No valid org.freedesktop.Hal interface found: " << m_halDeviceSystemPMInterface->lastError().message();

	return m_halDeviceSystemPMInterface;
}

QDBusInterface *PowerAction::getUPowerInterface() {
	// DOC: http://upower.freedesktop.org/docs/UPower.html
	if (!m_upowerInterface) {
		m_upowerInterface = new QDBusInterface(
			"org.freedesktop.UPower",
			"/org/freedesktop/UPower",
			"org.freedesktop.UPower",
			QDBusConnection::systemBus()
		);

		if (!m_upowerInterface->isValid()) {
			// HACK: wait for service start
			// BUG: https://sourceforge.net/p/kshutdown/bugs/34/
			qDebug() << "UPower: Trying again...";
			delete m_upowerInterface;
			m_upowerInterface = new QDBusInterface(
				"org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				QDBusConnection::systemBus()
			);
		}

		if (m_upowerInterface->isValid())
			qDebug() << "UPower backend found...";
		else
			qDebug() << "UPower backend NOT found...";
	}
	
	return m_upowerInterface;
}
#endif // QT_DBUS_LIB

bool PowerAction::onAction() {
#ifdef Q_OS_WIN32
	BOOL hibernate = (m_methodName == "Hibernate");
	BOOL result = ::SetSuspendState(hibernate, TRUE, FALSE);
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
		QThread::sleep(1);
	}
	
	#ifdef QT_DBUS_LIB
	QDBusInterface *login = getLoginInterface();
	QDBusInterface *upower = getUPowerInterface();
	
	QDBusError error;
	
	// systemd

	if (login->isValid()) {
// TODO: test interactivity option
		login->call(m_methodName, false);
		
		error = login->lastError();
	}

	// UPower

	else if (upower->isValid()) {
		upower->call(m_methodName);
		
		error = upower->lastError();
	}

	// HAL (lazy init)

	else {
		QDBusInterface *hal = getHalDeviceSystemPMInterface();
		if (hal->isValid()) {
			if (m_methodName == "Suspend")
				hal->call(m_methodName, 0);
			else
				hal->call(m_methodName); // no args
		
			error = hal->lastError();
		}
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
	#endif // QT_DBUS_LIB

	return true;
#endif // Q_OS_WIN32
}

// protected

bool PowerAction::isAvailable(const PowerActionType feature) const {
#ifdef Q_OS_WIN32
	if (feature == PowerActionType::Hibernate)
		return ::IsPwrHibernateAllowed();

	if (feature == PowerActionType::Suspend)
		return ::IsPwrSuspendAllowed();

	return false;
#elif defined(Q_OS_HAIKU)
	Q_UNUSED(feature)

	return false;
#else
	#ifdef QT_DBUS_LIB
	QDBusInterface *login = getLoginInterface();
	if (login->isValid()) {
		switch (feature) {
			case PowerActionType::Suspend: {
				QDBusReply<QString> reply = login->call("CanSuspend");
				qDebug() << "systemd: CanSuspend: " << reply;
				
				return reply.isValid() && (reply.value() == "yes");
			} break;
			case PowerActionType::Hibernate: {
				QDBusReply<QString> reply = login->call("CanHibernate");
				qDebug() << "systemd: CanHibernate: " << reply;
				
				return reply.isValid() && (reply.value() == "yes");
			} break;
		}
	}

	// Use the UPower backend if available
	QDBusInterface *upower = getUPowerInterface();
	if (upower->isValid()) {
		switch (feature) {
			case PowerActionType::Suspend:
				return upower->property("CanSuspend").toBool();
			case PowerActionType::Hibernate:
				return upower->property("CanHibernate").toBool();
		}
	}

	// Use the legacy HAL backend if systemd or UPower not available
	QDBusInterface *hal = getHalDeviceInterface();
	if (hal->isValid()) {
		// try old property name as well, for backward compat.
		QList<QString> featureNames;
		switch (feature) {
			case PowerActionType::Suspend:
				featureNames.append("power_management.can_suspend");
				featureNames.append("power_management.can_suspend_to_ram");
				break;
			case PowerActionType::Hibernate:
				featureNames.append("power_management.can_hibernate");
				featureNames.append("power_management.can_suspend_to_disk");
				break;
		}
		
		QDBusReply<bool> reply;
		
		// Try the alternative feature names in order; if we get a valid answer, return it
		foreach (const QString &featureName, featureNames) {
			reply = hal->call("GetProperty", featureName);
			if (reply.isValid())
				return reply.value();
		}

		qCritical() << reply.error();
	}
	#else
	Q_UNUSED(feature)
	#endif // QT_DBUS_LIB
	
	return false;
#endif // Q_OS_WIN32
}

// HibernateAction

// public

HibernateAction::HibernateAction() :
	PowerAction(
		Config::oldActionNamesVar.getBool() ? i18n("Hibernate Computer") : i18n("Hibernate"),
		"system-suspend-hibernate", "hibernate"
) {
	m_methodName = "Hibernate";
	if (!isAvailable(PowerActionType::Hibernate))
		disable(i18n("Cannot hibernate computer"));

	addCommandLineArg("H", "hibernate");

	uiAction()->setStatusTip(
		i18n("Hibernate Computer") + "\n" +
		i18n("Save the contents of RAM to disk\nthen turn off the computer.")
	);
}

// SuspendAction

// public

SuspendAction::SuspendAction() :
	PowerAction(
		#ifdef Q_OS_WIN32
		i18n("Sleep"),
		#else
		// NOTE: Matches https://phabricator.kde.org/D19184
		Config::oldActionNamesVar.getBool() ? i18n("Suspend Computer") : i18n("Sleep"),
		#endif // Q_OS_WIN32
		"system-suspend", "suspend") {
	m_methodName = "Suspend";
	if (!isAvailable(PowerActionType::Suspend))
		disable(i18n("Cannot suspend computer"));

	addCommandLineArg("S", "suspend");

	uiAction()->setStatusTip(
		i18n("Suspend Computer") + "\n" +
		i18n("Enter in a low-power state mode.")
	);
}

// StandardAction

// public

StandardAction::StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type) :
	Action(text, iconName, id),
	m_type(type) {
	
	setCanBookmark(true);

// TODO: clean up kshutdown.cpp, move this to LogoutAction
	#ifdef QT_DBUS_LIB
	if (!m_kdeSessionInterface) {
		m_kdeSessionInterface = new QDBusInterface(
			"org.kde.ksmserver",
			"/KSMServer",
			"org.kde.KSMServerInterface"
		);
		QDBusReply<bool> reply = m_kdeSessionInterface->call("canShutdown");
		m_kdeShutDownAvailable = reply.isValid() && reply.value();

		if (Utils::isKDEFullSession() && !m_kdeShutDownAvailable && (type != U_SHUTDOWN_TYPE_LOGOUT))
			qDebug() << "No KDE shutdown API available (check \"Offer shutdown options\" in the \"Session Management\" settings)";
	}

	if (Utils::isRazor() && (type == U_SHUTDOWN_TYPE_LOGOUT)) {
		m_razorSessionInterface = new QDBusInterface(
			"org.razorqt.session",
			"/RazorSession",
			"org.razorqt.session"
		);
		QDBusReply<bool> reply = m_razorSessionInterface->call("canLogout");
		if (!reply.isValid() || !reply.value()) {
			delete m_razorSessionInterface;
			m_razorSessionInterface = nullptr;
			disable("No Razor-qt session found");
		}
	}

	if (Utils::isLXQt() && (type == U_SHUTDOWN_TYPE_LOGOUT)) {
		m_lxqtSessionInterface = new QDBusInterface(
			"org.lxqt.session",
			"/LXQtSession",
			"org.lxqt.session"
		);
		QDBusReply<bool> reply = m_lxqtSessionInterface->call("canLogout");
		if (!reply.isValid() || !reply.value()) {
			delete m_lxqtSessionInterface;
			m_lxqtSessionInterface = nullptr;
			disable("No LXQt session found");
		}
	}
	#endif // QT_DBUS_LIB

	#ifndef Q_OS_WIN32
	m_lxsession = 0;
	if (Utils::isLXDE() && (type == U_SHUTDOWN_TYPE_LOGOUT)) {
		bool ok = false;
		int i = qgetenv("_LXSESSION_PID").toInt(&ok);
		if (ok) {
			m_lxsession = i;
			qDebug() << "LXDE session found: " << m_lxsession;
		}
		else {
			disable("No lxsession found");
		}
	}
	#endif // !Q_OS_WIN32
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
			qCritical() << "WTF? Unknown m_type: " << m_type;

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

	// Cinnamon

	if (Utils::isCinnamon()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "--logout";
			args << "--no-prompt";

			if (launch("cinnamon-session-quit", args, true))
				return true;
		}
	}
	
	// GNOME Shell, Unity

	else if (Utils::isGNOME() || Utils::isUnity()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
// TODO: --force
			args << "--logout";
			args << "--no-prompt";

			if (launch("gnome-session-quit", args, true))
				return true;
		}
	}

	// LXDE

	else if (Utils::isLXDE()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			#ifndef Q_OS_WIN32
			if (m_lxsession && (::kill(m_lxsession, SIGTERM) == 0))
				return true;
			#else
				return false;
			#endif // !Q_OS_WIN32
		}
	}

	// LXQt

	#ifdef QT_DBUS_LIB
	else if (Utils::isLXQt()) {
		if ((m_type == U_SHUTDOWN_TYPE_LOGOUT) && m_lxqtSessionInterface && m_lxqtSessionInterface->isValid()) {
			QDBusReply<void> reply = m_lxqtSessionInterface->call("logout");

			if (reply.isValid())
				return true;
		}
	}
/* DEAD: interactive
			QStringList args;
			args << "--logout";

			if (launch("lxqt-leave", args))
				return true;
*/
	#endif // QT_DBUS_LIB

	// MATE

	else if (Utils::isMATE()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "--logout";

			if (launch("mate-session-save", args, true))
				return true;
		}
	}

	// Razor-qt
	
	#ifdef QT_DBUS_LIB
	else if (Utils::isRazor()) {
		if ((m_type == U_SHUTDOWN_TYPE_LOGOUT) && m_razorSessionInterface && m_razorSessionInterface->isValid()) {
			QDBusReply<void> reply = m_razorSessionInterface->call("logout");

			if (reply.isValid())
				return true;
		}
	}
	#endif // QT_DBUS_LIB

	// Trinity

	else if (Utils::isTrinity()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "ksmserver";
			args << "ksmserver"; // not a paste bug
			args << "logout";

			// DOC: http://api.kde.org/4.x-api/kde-workspace-apidocs/plasma-workspace/libkworkspace/html/namespaceKWorkSpace.html
			args << "0"; // no confirm
			args << "0"; // logout
			args << "2"; // force now

			if (launch("dcop", args))
				return true;
		}
	}

	// Xfce

	else if (Utils::isXfce()) {
		switch (m_type) {
			case U_SHUTDOWN_TYPE_LOGOUT: {
				QStringList args;
				args << "--logout";

				if (launch("xfce4-session-logout", args, true))
					return true;
			} break;
/*
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
				qCritical() << "WTF? Unknown m_type: " << m_type;

				return false; // do nothing
*/
		}
	}

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

	#ifdef QT_DBUS_LIB
	if (
		Utils::isKDEFullSession() &&
		(m_kdeShutDownAvailable || (m_type == U_SHUTDOWN_TYPE_LOGOUT))
	) {
		/*QDBusReply<void> reply = */m_kdeSessionInterface->call(
			"logout",
			0, // KWorkSpace::ShutdownConfirmNo
			m_type,
			2 // KWorkSpace::ShutdownModeForceNow
		);

/* TODO: error checking?
		if (reply.isValid())
			return true;
*/

		return true;
	}
	#endif // QT_DBUS_LIB

	// Openbox

	// NOTE: Put this after m_kdeSessionInterface call
	// because in case of Openbox/KDE session KShutdown
	// will detect both Openbox and KDE at the same time.
	// ISSUE: https://sourceforge.net/p/kshutdown/feature-requests/20/
	if (Utils::isOpenbox()) {
		if (m_type == U_SHUTDOWN_TYPE_LOGOUT) {
			QStringList args;
			args << "--exit";

			if (launch("openbox", args))
				return true;
		}
	}

	// fallback to systemd/logind/ConsoleKit/HAL/whatever
	
	#ifdef QT_DBUS_LIB

// TODO: move
	MainWindow::self()->writeConfig();
	
	// try systemd/logind

	QDBusInterface *login = getLoginInterface();
	if ((m_type == U_SHUTDOWN_TYPE_HALT) && login->isValid()) {
// TODO: check CanPowerOff/CanReboot
		QDBusReply<void> reply = login->call("PowerOff", false);

		if (reply.isValid())
			return true;
	}
	else if ((m_type == U_SHUTDOWN_TYPE_REBOOT) && login->isValid()) {
		QDBusReply<void> reply = login->call("Reboot", false);

		if (reply.isValid())
			return true;
	}

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

	// try HAL (lazy init)
	
	QDBusInterface *hal = PowerAction::getHalDeviceSystemPMInterface();

	if ((m_type == U_SHUTDOWN_TYPE_HALT) && hal->isValid()) {
		QDBusReply<int> reply = hal->call("Shutdown");

		if (reply.isValid())
			return true;
	}
	else if ((m_type == U_SHUTDOWN_TYPE_REBOOT) && hal->isValid()) {
		QDBusReply<int> reply = hal->call("Reboot");

		if (reply.isValid())
			return true;
	}
	#endif // QT_DBUS_LIB
	
	// show error
	
	return unsupportedAction();
#endif // Q_OS_WIN32
}

// protected

#ifdef QT_DBUS_LIB
void StandardAction::checkAvailable(const QString &consoleKitName) {
	bool available = false;
	QString error = "";
// TODO: clean up; return bool
// TODO: win32: check if shutdown/reboot action is available

	// try systemd

	QDBusInterface *login = getLoginInterface();
	if (login->isValid()) {
// TODO: CanPowerOff, etc.
		available = true;
	}

	// try ConsoleKit

	if (!consoleKitName.isEmpty()) {
		if (!m_consoleKitInterface) {
			m_consoleKitInterface = new QDBusInterface(
				"org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				QDBusConnection::systemBus()
			);
		}
		if (!available && !m_consoleKitInterface->isValid()) {
			// HACK: wait for service start
			qDebug() << "ConsoleKit: Trying again...";
			delete m_consoleKitInterface;
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
				qCritical() << reply.error().message();
				if (error.isEmpty())
					error = reply.error().name();
			}
			else {
				available = reply.value();
			}
		}
		else {
			qCritical() << "ConsoleKit Error: " << m_consoleKitInterface->lastError().message();
			if (error.isEmpty())
				error = "No valid org.freedesktop.ConsoleKit interface found";
		}
	}
	
	// try HAL (lazy init)

	if (!available) {
		QDBusInterface *hal = PowerAction::getHalDeviceSystemPMInterface();
		available = hal->isValid();
	}
	
	// BUG #19 - disable only if both ConsoleKit and native KDE API is unavailable
	if (!available && !m_kdeShutDownAvailable)
		disable(error);
}
#endif // QT_DBUS_LIB

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
	uiAction()->setStatusTip(i18n("End Session"));

	addCommandLineArg("l", "logout");

	#ifdef Q_OS_HAIKU
	disable("");
	#endif // Q_OS_HAIKU
}

// RebootAction

// public

RebootAction::RebootAction() :
	StandardAction(
		Config::oldActionNamesVar.getBool() ? i18n("Restart Computer") : i18n("Reboot"),
		QString::null, "reboot", U_SHUTDOWN_TYPE_REBOOT),
	m_bootEntryComboBox(nullptr) {

	if (!Config::oldActionNamesVar.getBool())
		uiAction()->setStatusTip(i18n("Restart Computer"));

#ifdef KS_KF5
/* HACK: crash on KDE 4.5.0
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
		setIcon(QIcon::fromTheme("system-restart"));
	else
		setIcon(p);
*/
	// NOTE: follow the Icon Naming Specification and use "system-reboot" instead of "system-restart"
	// <http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html>
	uiAction()->setIcon(QIcon::fromTheme("system-reboot"));
#else
	if (Utils::isKDE())
		uiAction()->setIcon(QIcon::fromTheme("system-reboot"));
	// HACK: missing "system-reboot" in some icon themes
	else
		uiAction()->setIcon(QIcon::fromTheme("view-refresh"));
#endif // KS_KF5

	addCommandLineArg("r", "reboot");
	
	#ifdef QT_DBUS_LIB
	checkAvailable("CanRestart");
	#endif // QT_DBUS_LIB
}

QWidget *RebootAction::getWidget() {
/* TODO: boot entry combo box
	#ifdef Q_OS_LINUX
	if (!m_bootEntryComboBox)
		m_bootEntryComboBox = new BootEntryComboBox();
	#endif // Q_OS_LINUX
*/
	return m_bootEntryComboBox;
}

// ShutDownAction

// public

ShutDownAction::ShutDownAction() :
	StandardAction(
		Config::oldActionNamesVar.getBool() ? i18n("Turn Off Computer") : i18n("Shut Down"),
		"system-shutdown", "shutdown", U_SHUTDOWN_TYPE_HALT
) {
	if (!Config::oldActionNamesVar.getBool())
		uiAction()->setStatusTip(i18n("Turn Off Computer"));

	addCommandLineArg("h", "halt");
	addCommandLineArg("s", "shutdown");

	#ifdef QT_DBUS_LIB
	checkAvailable("CanStop");
	#endif // QT_DBUS_LIB
}
