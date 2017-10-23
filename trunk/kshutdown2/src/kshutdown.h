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

#include <QAction>
#include <QDateTimeEdit>

#ifdef KS_DBUS
	#include <QDBusInterface>
	#include <QDBusReply>
#endif // KS_DBUS

#ifdef KS_UNIX
	// HACK: fixes some compilation error (?)
	#include <sys/types.h>
#endif // KS_UNIX

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

class Base {
public:
	enum class State { Start, Stop, InvalidStatus };
	explicit Base(const QString &id);
	virtual ~Base();
	
	bool canBookmark() const { return m_canBookmark; }
	
	void setCanBookmark(const bool value) { m_canBookmark = value; }
	
	inline QString disableReason() const {
		return m_disableReason;
	}
	inline QString error() const {
		return m_error;
	}
	
	virtual QString getStringOption() { return QString::null; }
	
	virtual void setStringOption(const QString &option) {
		Q_UNUSED(option)
	}
	
	virtual QWidget *getWidget();
	inline QString id() const {
		return m_id;
	}
	inline QString originalText() const {
		return m_originalText;
	}
	virtual void readConfig(Config *config);
	virtual void setState(const State state);
	inline QString status() const {
		return m_status;
	}
	inline InfoWidget::Type statusType() const {
		return m_statusType;
	}
	virtual void writeConfig(Config *config);
protected:
	InfoWidget::Type m_statusType;
	QString m_disableReason;
	QString m_error;
	QString m_id;

	/**
	 * A text without "&" shortcut.
	 */
	QString m_originalText;

	QString m_status;
#ifdef Q_OS_WIN32
	void setLastError();
#endif // Q_OS_WIN32
private:
	bool m_canBookmark;
};

class Action: public QAction, public Base {
	Q_OBJECT
public:
	explicit Action(const QString &text, const QString &iconName, const QString &id);
	void activate(const bool force);
	bool authorize(QWidget *parent);
	inline QStringList getCommandLineArgs() const {
		return m_commandLineArgs;
	}
	bool isCommandLineArgSupported();
	virtual bool onAction() = 0;
	inline bool shouldStopTimer() const {
		return m_shouldStopTimer;
	}
	inline void setShouldStopTimer(const bool value) {
		m_shouldStopTimer = value;
	}
	inline bool showInMenu() const {
		return m_showInMenu;
	}
	inline void setShowInMenu(const bool value) {
		m_showInMenu = value;
	}
	bool showConfirmationMessage();
	inline static bool totalExit() {
		return m_totalExit;
	}
	virtual void updateMainWindow(MainWindow *mainWindow);
protected:
	bool m_force;
	static bool m_totalExit;
	void addCommandLineArg(const QString &shortArg, const QString &longArg);
	void disable(const QString &reason);
	#ifdef KS_DBUS
	static QDBusInterface *getLoginInterface();
	#endif // KS_DBUS
	bool launch(const QString &program, const QStringList &args, const bool detached = false);
	bool unsupportedAction();
private:
	Q_DISABLE_COPY(Action)
	#ifdef KS_DBUS
	static QDBusInterface *m_loginInterface;
	#endif // KS_DBUS
	bool m_shouldStopTimer;
	bool m_showInMenu;
	QStringList m_commandLineArgs;
private slots:
	void slotFire();
signals:
	void statusChanged(const bool updateWidgets);
};

class ConfirmAction: public QAction {
	Q_OBJECT
public:
	explicit ConfirmAction(QObject *parent, Action *action);
private:
	Q_DISABLE_COPY(ConfirmAction)
	Action *m_impl;
private slots:
	void slotFire();
};

class Trigger: public QObject, public Base {
	Q_OBJECT
public:
	explicit Trigger(const QString &text, const QString &iconName, const QString &id);
	inline QIcon icon() const {
		return m_icon;
	}
	virtual bool canActivateAction() = 0;
	inline int checkTimeout() const {
		return m_checkTimeout;
	}
	inline bool supportsProgressBar() { return m_supportsProgressBar; }
	inline QString text() const {
		return m_text;
	}
	inline QString toolTip() const { return m_toolTip; }
	inline void setToolTip(const QString &value) { m_toolTip = value; }
protected:
	bool m_supportsProgressBar;
	int m_checkTimeout;
private:
	Q_DISABLE_COPY(Trigger)
	QIcon m_icon;
	QString m_text;
	QString m_toolTip = QString::null;
signals:
	void notify(const QString &id, const QString &text);
	void statusChanged(const bool updateWidgets);
};

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
	#ifdef KS_DBUS
	static QDBusInterface *getHalDeviceInterface();
	static QDBusInterface *getHalDeviceSystemPMInterface();
	static QDBusInterface *getUPowerInterface();
	#endif // KS_DBUS
	virtual bool onAction() override;
	enum class PowerActionType { Suspend, Hibernate };
protected:
	QString m_methodName;
	bool isAvailable(const PowerActionType feature) const;
private:
	Q_DISABLE_COPY(PowerAction)
	#ifdef KS_DBUS
	static QDBusInterface *m_halDeviceInterface;
	static QDBusInterface *m_halDeviceSystemPMInterface;
	static QDBusInterface *m_upowerInterface;
	#endif // KS_DBUS
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
	#ifdef KS_DBUS
	static QDBusInterface *m_consoleKitInterface;
	static QDBusInterface *m_kdeSessionInterface;
	static QDBusInterface *m_lxqtSessionInterface;
	static QDBusInterface *m_razorSessionInterface;
	void checkAvailable(const QString &consoleKitName);
	#endif // KS_DBUS
private:
	Q_DISABLE_COPY(StandardAction)
	#ifdef KS_DBUS
	static bool m_kdeShutDownAvailable;
	#endif // KS_DBUS
	#ifdef KS_UNIX
	pid_t m_lxsession;
	#endif // KS_UNIX
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
