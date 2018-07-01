// kshutdown.h - KShutdown base library
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

#ifndef KSHUTDOWN_KSHUTDOWN_H
#define KSHUTDOWN_KSHUTDOWN_H

#include "infowidget.h"
#include "plugins.h"

#include <QAction>
#include <QDateTimeEdit>

#ifdef QT_DBUS_LIB
	#include <QDBusInterface>
	#include <QDBusReply>
#endif // QT_DBUS_LIB

#ifndef Q_OS_WIN32
	// HACK: fixes some compilation error (?)
	#include <sys/types.h>
#endif // !Q_OS_WIN32

class BootEntryComboBox;
class Config;
class MainWindow;

typedef int UShutdownType;
// This is mapped to the values in kworkspace.h
const UShutdownType U_SHUTDOWN_TYPE_LOGOUT = 0; // KDE: ShutdownTypeNone
const UShutdownType U_SHUTDOWN_TYPE_REBOOT = 1; // KDE: ShutdownTypeReboot
const UShutdownType U_SHUTDOWN_TYPE_HALT = 2; // KDE: ShutdownTypeHalt

namespace KShutdown {
	
const QString TIME_DISPLAY_FORMAT = "hh'h ':' 'mm'm'";
const QString LONG_TIME_DISPLAY_FORMAT = "hh'h ':' 'mm'm ':' 'ss's'";
const QString TIME_PARSE_FORMAT = "h:mm";

class DateTimeEdit: public QDateTimeEdit {
	Q_OBJECT
public:
	explicit DateTimeEdit();
	virtual ~DateTimeEdit();
private slots:
	void onLineEditSelectionChange();
};

class DateTimeTriggerBase: public Trigger {
	Q_OBJECT
public:
	explicit DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id);
	virtual ~DateTimeTriggerBase();
	virtual bool canActivateAction() override;
	virtual QWidget *getWidget() override;
	static QString longDateTimeFormat();
	virtual void readConfig(Config *config) override;
	virtual void writeConfig(Config *config) override;
	void setDateTime(const QDateTime &dateTime);
	virtual void setState(const State state) override;
protected:
	DateTimeEdit *m_edit = nullptr;
	QDateTime m_dateTime;
	QDateTime m_endDateTime;
	virtual QDateTime calcEndTime() = 0;
	virtual void updateStatus();
private slots:
	void syncDateTime();
private:
	Q_DISABLE_COPY(DateTimeTriggerBase)
	QString createStatus(const QDateTime &now, int &secsTo);
};

class DateTimeTrigger: public DateTimeTriggerBase {
public:
	explicit DateTimeTrigger();
	QDateTime dateTime();
	virtual QString getStringOption() override;
	virtual void setStringOption(const QString &option) override;
	virtual QWidget *getWidget() override;
	virtual void setState(const State state) override;
protected:
	virtual QDateTime calcEndTime() override;
private:
	Q_DISABLE_COPY(DateTimeTrigger)
};

class NoDelayTrigger: public Trigger {
public:
	explicit NoDelayTrigger();
	virtual bool canActivateAction() override { return true; }
private:
	Q_DISABLE_COPY(NoDelayTrigger)
};

class TimeFromNowTrigger: public DateTimeTriggerBase {
public:
	explicit TimeFromNowTrigger();
	virtual QString getStringOption() override;
	virtual void setStringOption(const QString &option) override;
	virtual QWidget *getWidget() override;
protected:
	virtual QDateTime calcEndTime() override;
private:
	Q_DISABLE_COPY(TimeFromNowTrigger)
};

class PowerAction: public Action {
public:
	explicit PowerAction(const QString &text, const QString &iconName, const QString &id);
	#ifdef QT_DBUS_LIB
	static QDBusInterface *getHalDeviceInterface();
	static QDBusInterface *getHalDeviceSystemPMInterface();
	static QDBusInterface *getUPowerInterface();
	#endif // QT_DBUS_LIB
	virtual bool onAction() override;
	enum class PowerActionType { Suspend, Hibernate };
protected:
	QString m_methodName;
	bool isAvailable(const PowerActionType feature) const;
private:
	Q_DISABLE_COPY(PowerAction)
	#ifdef QT_DBUS_LIB
	static QDBusInterface *m_halDeviceInterface;
	static QDBusInterface *m_halDeviceSystemPMInterface;
	static QDBusInterface *m_upowerInterface;
	#endif // QT_DBUS_LIB
};

class HibernateAction: public PowerAction {
public:
	explicit HibernateAction();
private:
	Q_DISABLE_COPY(HibernateAction)
};

class SuspendAction: public PowerAction {
public:
	explicit SuspendAction();
private:
	Q_DISABLE_COPY(SuspendAction)
};

class StandardAction: public Action {
public:
	explicit StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type);
	virtual bool onAction() override;
protected:
	#ifdef QT_DBUS_LIB
	static QDBusInterface *m_consoleKitInterface;
	static QDBusInterface *m_kdeSessionInterface;
	static QDBusInterface *m_lxqtSessionInterface;
	static QDBusInterface *m_razorSessionInterface;
	void checkAvailable(const QString &consoleKitName);
	#endif // QT_DBUS_LIB
private:
	Q_DISABLE_COPY(StandardAction)
	#ifdef QT_DBUS_LIB
	static bool m_kdeShutDownAvailable;
	#endif // QT_DBUS_LIB
	#ifndef Q_OS_WIN32
	pid_t m_lxsession;
	#endif // !Q_OS_WIN32
	UShutdownType m_type;
};

class LogoutAction: public StandardAction {
public:
	explicit LogoutAction();
private:
	Q_DISABLE_COPY(LogoutAction)
};

class RebootAction: public StandardAction {
public:
	explicit RebootAction();
	virtual QWidget *getWidget() override;
private:
	Q_DISABLE_COPY(RebootAction)
	BootEntryComboBox *m_bootEntryComboBox;
};

class ShutDownAction: public StandardAction {
public:
	explicit ShutDownAction();
private:
	Q_DISABLE_COPY(ShutDownAction)
};

} // KShutdown

#endif // KSHUTDOWN_KSHUTDOWN_H
