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

#include "pureqt.h"

#ifdef KS_UNIX
	#include <sys/types.h>
#endif // KS_UNIX

#include <QDateTime>

#include "config.h"
#include "infowidget.h"

#ifdef KS_NATIVE_KDE
	#include <kworkspace/kworkspace.h>
	typedef KWorkSpace::ShutdownType UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT KWorkSpace::ShutdownTypeNone
	#define U_SHUTDOWN_TYPE_REBOOT KWorkSpace::ShutdownTypeReboot
	#define U_SHUTDOWN_TYPE_HALT KWorkSpace::ShutdownTypeHalt
#else
	typedef int UShutdownType;
	#define U_SHUTDOWN_TYPE_LOGOUT 1
	#define U_SHUTDOWN_TYPE_REBOOT 2
	#define U_SHUTDOWN_TYPE_HALT 3
#endif // KS_NATIVE_KDE

class QDateTimeEdit;
#ifdef KS_DBUS
class QDBusInterface;
#endif // KS_DBUS

namespace KShutdown {
	
const QString DATE_TIME_DISPLAY_FORMAT = "MMM d dddd hh:mm";
const QString TIME_DISPLAY_FORMAT = "hh'h ':' 'mm'm'";
const QString TIME_PARSE_FORMAT = "h:mm";

class U_EXPORT Base {
public:
	enum State { StartState, StopState, InvalidStatusState };
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
	virtual void readConfig(const QString &group, Config *config);
	virtual void setState(const State state);
	inline QString status() const {
		return m_status;
	}
	inline InfoWidget::Type statusType() const {
		return m_statusType;
	}
	virtual void writeConfig(const QString &group, Config *config);
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

class U_EXPORT Action: public U_ACTION, public Base {
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
protected:
	bool m_force;
	static bool m_totalExit;
	void addCommandLineArg(const QString &shortArg, const QString &longArg);
	void disable(const QString &reason);
	bool launch(const QString &program, const QStringList &args);
	bool unsupportedAction();
private:
	Q_DISABLE_COPY(Action)
	bool m_shouldStopTimer;
	bool m_showInMenu;
	QStringList m_commandLineArgs;
private slots:
	void slotFire();
signals:
	void statusChanged(const bool updateWidgets);
};

class ConfirmAction: public U_ACTION {
	Q_OBJECT
public:
	explicit ConfirmAction(QObject *parent, Action *action);
private:
	Q_DISABLE_COPY(ConfirmAction)
	Action *m_impl;
private slots:
	void slotFire();
};

class U_EXPORT Trigger: public QObject, public Base {
	Q_OBJECT
public:
	explicit Trigger(const QString &text, const QString &iconName, const QString &id);
	inline U_ICON icon() const {
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
	inline QString whatsThis() const { return m_whatsThis; }
	inline void setWhatsThis(const QString &value) { m_whatsThis = value; }
protected:
	bool m_supportsProgressBar;
	int m_checkTimeout;
private:
	Q_DISABLE_COPY(Trigger)
	U_ICON m_icon;
	QString m_text;
	QString m_whatsThis;
signals:
	void notify(const QString &id, const QString &text);
	void statusChanged(const bool updateWidgets);
};

class U_EXPORT DateTimeTriggerBase: public Trigger {
	Q_OBJECT
public:
	explicit DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id);
	virtual ~DateTimeTriggerBase();
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
	virtual void readConfig(const QString &group, Config *config);
	virtual void writeConfig(const QString &group, Config *config);
	void setDateTime(const QDateTime &dateTime);
	virtual void setState(const State state);
protected:
	QDateTime m_dateTime;
	QDateTime m_endDateTime;
	QDateTimeEdit *m_edit;
	virtual QDateTime calcEndTime() = 0;
	virtual void updateStatus();
private slots:
	void syncDateTime();
private:
	Q_DISABLE_COPY(DateTimeTriggerBase)
	QString createStatus(const QDateTime &now, int &secsTo);
};

class U_EXPORT DateTimeTrigger: public DateTimeTriggerBase {
public:
	DateTimeTrigger();
	QDateTime dateTime();
	virtual QString getStringOption();
	virtual void setStringOption(const QString &option);
	virtual QWidget *getWidget();
	virtual void setState(const State state);
protected:
	virtual QDateTime calcEndTime();
private:
	Q_DISABLE_COPY(DateTimeTrigger)
};

class U_EXPORT NoDelayTrigger: public Trigger {
public:
	NoDelayTrigger();
	virtual bool canActivateAction();
protected:
	virtual QDateTime calcEndTime();
private:
	Q_DISABLE_COPY(NoDelayTrigger)
};

class U_EXPORT TimeFromNowTrigger: public DateTimeTriggerBase {
public:
	TimeFromNowTrigger();
	virtual QString getStringOption();
	virtual void setStringOption(const QString &option);
	virtual QWidget *getWidget();
protected:
	virtual QDateTime calcEndTime();
private:
	Q_DISABLE_COPY(TimeFromNowTrigger)
};

class U_EXPORT PowerAction: public Action {
public:
	explicit PowerAction(const QString &text, const QString &iconName, const QString &id);
	virtual bool onAction();
	enum PowerActionType { Suspend, Hibernate };
protected:
	QString m_methodName;
	bool isAvailable(const PowerActionType feature) const;
private:
	Q_DISABLE_COPY(PowerAction)
};

class U_EXPORT HibernateAction: public PowerAction {
public:
	HibernateAction();
private:
	Q_DISABLE_COPY(HibernateAction)
};

class U_EXPORT SuspendAction: public PowerAction {
public:
	SuspendAction();
private:
	Q_DISABLE_COPY(SuspendAction)
};

class U_EXPORT StandardAction: public Action {
public:
	explicit StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type);
	virtual bool onAction();
protected:
	#ifdef KS_DBUS
	static QDBusInterface *m_consoleKitInterface;
	static QDBusInterface *m_halInterface;
	static QDBusInterface *m_razorSessionInterface;
	#endif // KS_DBUS
	void checkAvailable(const UShutdownType type, const QString &consoleKitName);
private:
	Q_DISABLE_COPY(StandardAction)
	#ifdef KS_NATIVE_KDE
	bool m_kdeShutDownAvailable;
	#endif // KS_NATIVE_KDE
	#ifdef KS_UNIX
	pid_t m_lxsession;
	#endif // KS_UNIX
	UShutdownType m_type;
};

class U_EXPORT LogoutAction: public StandardAction {
public:
	LogoutAction();
private:
	Q_DISABLE_COPY(LogoutAction)
};

class U_EXPORT RebootAction: public StandardAction {
public:
	RebootAction();
private:
	Q_DISABLE_COPY(RebootAction)
};

class U_EXPORT ShutDownAction: public StandardAction {
public:
	ShutDownAction();
private:
	Q_DISABLE_COPY(ShutDownAction)
};

} // KShutdown

#endif // KSHUTDOWN_KSHUTDOWN_H
