//
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

#ifndef __KSHUTDOWN_H__
#define __KSHUTDOWN_H__

#include "pureqt.h"

// TODO: libkshutdown

#include <QDateTime>

#include "config.h"

class QDateTimeEdit;

namespace KShutdown {

class U_EXPORT Base {
public:
// TODO: ENABLE, DISABLE, SELECTED, UNSELECTED
	enum State { START, STOP };
	Base(const QString &id);
	virtual ~Base();
	inline QString disableReason() const {
		return m_disableReason;
	}
	inline QString error() const {
		return m_error;
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
	virtual void writeConfig(const QString &group, Config *config);
protected:
	QString m_disableReason;
	QString m_error;
	QString m_id;

	/**
	 * A text without "&" shortcut.
	 */
	QString m_originalText;

	QString m_status;
#ifdef Q_WS_WIN
	void setLastError();
#endif // Q_WS_WIN
};

class U_EXPORT Action: public U_ACTION, public Base {
	Q_OBJECT
public:
	Action(const QString &text, const QString &iconName, const QString &id);
	void activate(const bool force);
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
	bool m_shouldStopTimer;
	bool m_showInMenu;
	QStringList m_commandLineArgs;
private slots:
	void slotFire();
};

class ConfirmAction: public U_ACTION {
	Q_OBJECT
public:
	ConfirmAction(Action *action);
private:
	Action *m_impl;
private slots:
	void slotFire();
};

class U_EXPORT Trigger: public QObject, public Base {
	Q_OBJECT
public:
	Trigger(const QString &text, const QString &iconName, const QString &id);
	inline U_ICON icon() const {
		return m_icon;
	}
	virtual bool canActivateAction() = 0;
	inline int checkTimeout() const {
		return m_checkTimeout;
	}
	inline QString text() const {
		return m_text;
	}
protected:
	int m_checkTimeout;
private:
	U_ICON m_icon;
	QString m_text;
};

class U_EXPORT DateTimeTriggerBase: public Trigger {
	Q_OBJECT
public:
	DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id);
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
	virtual void readConfig(const QString &group, Config *config);
	virtual void writeConfig(const QString &group, Config *config);
protected:
	QDateTime m_dateTime;
	QDateTime m_endDateTime;
	QDateTimeEdit *m_edit;
private slots:
	void syncDateTime();
};

class U_EXPORT DateTimeTrigger: public DateTimeTriggerBase {
public:
	DateTimeTrigger();
	virtual QWidget *getWidget();
	virtual void setState(const State state);
};

class U_EXPORT NoDelayTrigger: public Trigger {
public:
	NoDelayTrigger();
	virtual bool canActivateAction();
};

class U_EXPORT TimeFromNowTrigger: public DateTimeTriggerBase {
public:
	TimeFromNowTrigger();
	virtual QWidget *getWidget();
	virtual void setState(const State state);
};

class U_EXPORT PowerAction: public Action {
public:
	PowerAction(const QString &text, const QString &iconName, const QString &id);
	virtual bool onAction();
protected:
	QString m_methodName;
	bool isAvailable(const QString &feature) const;
};

class U_EXPORT HibernateAction: public PowerAction {
public:
	HibernateAction();
};

class U_EXPORT SuspendAction: public PowerAction {
public:
	SuspendAction();
};

class U_EXPORT LockAction: public Action {
public:
	virtual bool onAction();
	inline static LockAction *self() {
		if (!m_instance)
			m_instance = new LockAction();

		return m_instance;
	}
private:
	static LockAction *m_instance;
	LockAction();
};

class U_EXPORT StandardAction: public Action {
public:
	StandardAction(const QString &text, const QString &iconName, const QString &id, const UShutdownType type);
	virtual bool onAction();
private:
	UShutdownType m_type;
};

class U_EXPORT LogoutAction: public StandardAction {
public:
	LogoutAction();
};

class U_EXPORT RebootAction: public StandardAction {
public:
	RebootAction();
};

class U_EXPORT ShutDownAction: public StandardAction {
public:
	ShutDownAction();
};

} // KShutdown

#endif // __KSHUTDOWN_H__
