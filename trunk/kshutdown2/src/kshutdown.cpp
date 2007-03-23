
//!!!cleanup

#include <QDateTimeEdit>
#include <QDBusInterface>
#include <QDBusReply>

#include <KLocale>
#include <KMessageBox>

#include "kshutdown.h"
#include "kshutdown.moc"

using namespace KShutDown;

LockAction *LockAction::m_instance = 0;

// Base

// public

Base::Base(const QString &id) :
	m_error(QString::null),
	m_id(id),
	m_status(QString::null) {
}

Base::~Base() {
}

QWidget *Base::getWidget() {
	return 0;
}

void Base::readConfig(KConfig *config) {
	Q_UNUSED(config)
}

void Base::setState(const State state) {
	Q_UNUSED(state)
}

void Base::writeConfig(KConfig *config) {
	Q_UNUSED(config)
}

// Action

// public

Action::Action(const QString &text, const QString &iconName, const QString &id) :
// FIXME: 0 parent - is this OK?
	KAction(0),
	Base(id) {
	m_originalText = text;
	setIcon(KIcon(iconName));
	setText(text);
	connect(this, SIGNAL(activated()), SLOT(slotFire()));
}

// private slots

void Action::slotFire() {
	kDebug() << "Action::slotFire() [ id=" << m_id << " ]" << endl;

	m_error = QString::null;
	if (!onAction()) {
		QString text = m_error.isEmpty() ? i18n("Unknown error") : m_error;
		KMessageBox::error(0, m_error);
	}
}

// Trigger

// public

Trigger::Trigger(const QString &text, const QString &iconName, const QString &id) :
	Base(id),
	m_checkTimeout(500),
	m_icon(KIcon(iconName)),
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
	}

	return m_edit;
}

void DateTimeTriggerBase::readConfig(KConfig *config) {
	m_dateTime = config->readEntry("Date Time", m_dateTime);
}

void DateTimeTriggerBase::writeConfig(KConfig *config) {
	if (m_edit)
		config->writeEntry("Date Time", m_edit->dateTime());
}

// private slots

void DateTimeTriggerBase::syncDateTime() {
	m_dateTime = m_edit->dateTime();
}

// DateTimeTrigger

// public

DateTimeTrigger::DateTimeTrigger() :
	DateTimeTriggerBase(i18n("At Date/Time"), "date", "date-time") {
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
	Trigger(i18n("No Delay"), "messagebox_warning", "no-delay") {
}

bool NoDelayTrigger::canActivateAction() {
	return true;
}

// TimeFromNowTrigger

// public

TimeFromNowTrigger::TimeFromNowTrigger() :
	DateTimeTriggerBase(i18n("Time From Now (HH:MM)"), "clock", "time-from-now") {
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
	// lock screen before hibernate/suspend
	LockAction::self()->activate(KAction::Trigger);

	QDBusInterface i(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device.SystemPowerManagement",
		QDBusConnection::systemBus()
	);
	i.call(m_methodName);

	QDBusError error = i.lastError();
	if (error.type() != QDBusError::NoError) {
		m_error = error.message();

		return false;
	}

	return true;
}

// protected

bool PowerAction::isAvailable(const QString &feature) const {
	QDBusInterface *i = new QDBusInterface(
		"org.freedesktop.Hal",
		"/org/freedesktop/Hal/devices/computer",
		"org.freedesktop.Hal.Device",
		QDBusConnection::systemBus()
	);
	QDBusReply<bool> reply = i->call("GetProperty", feature);

	if (reply.isValid())
		return reply.value();

	//!!!
	kError() << reply.error() << endl;

	return false;
}

// HibernateAction

// public

HibernateAction::HibernateAction() :
	PowerAction(i18n("Hibernate Computer"), "hibernate", "hibernate") {
	m_methodName = "Hibernate";
	setEnabled(isAvailable("power_management.can_suspend_to_disk"));
}

// SuspendAction

// public

SuspendAction::SuspendAction() :
	PowerAction(i18n("Suspend Computer"), "suspend", "suspend") {
	m_methodName = "Suspend";
	setEnabled(isAvailable("power_management.can_suspend_to_ram"));
}

// LockAction

// public

LockAction::LockAction() :
	Action(i18n("Lock Session"), "lock", "lock") {
}

bool LockAction::onAction() {
	QDBusInterface i("org.kde.krunner", "/ScreenSaver");
	i.call("lock");

	QDBusError error = i.lastError();
	if (error.type() != QDBusError::NoError) {
		m_error = error.message();

		return false;
	}

	return true;
}

// StandardKDEAction

// public

StandardKDEAction::StandardKDEAction(const QString &text, const QString &iconName, const QString &id, const KWorkSpace::ShutdownType type) :
	Action(text, iconName, id),
	m_type(type) {
}

bool StandardKDEAction::onAction() {
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
}

// LogoutAction

// public

LogoutAction::LogoutAction() :
	StandardKDEAction(i18n("End Current Session"), "undo", "logout", KWorkSpace::ShutdownTypeNone) {
}

// RebootAction

// public

RebootAction::RebootAction() :
	StandardKDEAction(i18n("Restart Computer"), "reload", "reboot", KWorkSpace::ShutdownTypeReboot) {
}

// ShutDownAction

// public

ShutDownAction::ShutDownAction() :
	StandardKDEAction(i18n("Turn Off Computer"), "exit", "shutdown", KWorkSpace::ShutdownTypeHalt) {
}
