// plugins.h - Plugins
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

#ifndef KSHUTDOWN_PLUGINS_H
#define KSHUTDOWN_PLUGINS_H

#include "infowidget.h"

#include <QAction>
#include <QCommandLineOption>

#ifdef QT_DBUS_LIB
	#include <QDBusInterface>
	#include <QDBusReply>
#endif // QT_DBUS_LIB

class Config;
class MainWindow;

class Base: public QObject {
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
	virtual bool isEnabled() const { return true; }
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

enum class ActionType { SHUT_DOWN, REBOOT, HIBERNATE, SUSPEND, LOCK, LOGOUT, TEST };

class Action: public Base {
	friend class PluginManager;
	Q_OBJECT
public:
	explicit Action(const QString &text, const QString &iconName, const QString &id);
	void activate(const bool force);
	bool authorize(QWidget *parent);
	QAction *createConfirmAction(const bool alwaysShowConfirmationMessage);
	static QString getDisplayName(const ActionType type);
	int getUIGroup() const;
	QIcon icon() const { return m_uiAction->icon(); }
	bool isCommandLineOptionSet() const;
	virtual bool isEnabled() const override { return m_uiAction->isEnabled(); }
	virtual bool onAction() = 0;
	inline bool shouldStopTimer() const {
		return m_shouldStopTimer;
	}
	inline void setShouldStopTimer(const bool value) {
		m_shouldStopTimer = value;
	}
	bool showConfirmationMessage();
	inline static bool totalExit() {
		return m_totalExit;
	}
	virtual void updateMainWindow(MainWindow *mainWindow);
	QAction *uiAction() const { return m_uiAction; }

	bool visibleInMainMenu() const { return m_visibleInMainMenu; }
	void setVisibleInMainMenu(const bool value) { m_visibleInMainMenu = value; }

	bool visibleInSystemTrayMenu() const { return m_visibleInSystemTrayMenu; }
	void setVisibleInSystemTrayMenu(const bool value) { m_visibleInSystemTrayMenu = value; }

	bool visibleInWindow() const { return m_visibleInWindow; }
	void setVisibleInWindow(const bool value) { m_visibleInWindow = value; }
protected:
	bool m_force;
	static bool m_totalExit;
	void disable(const QString &reason);
	#ifdef QT_DBUS_LIB
	static QDBusInterface *getLoginInterface();
	#endif // QT_DBUS_LIB
	bool launch(const QString &program, const QStringList &args, const bool detached = false);
	void setCommandLineOption(const QCommandLineOption *option);
	void setCommandLineOption(const QStringList &names, const QString &description = "");
	bool unsupportedAction();
private:
	Q_DISABLE_COPY(Action)
	#ifdef QT_DBUS_LIB
	static QDBusInterface *m_loginInterface;
	#endif // QT_DBUS_LIB
	bool m_shouldStopTimer;
	bool m_visibleInMainMenu = true;
	bool m_visibleInSystemTrayMenu = true;
	bool m_visibleInWindow = true;
	QAction *m_uiAction = nullptr;

// TODO: use std::optional (C++17)
	const QCommandLineOption *m_commandLineOption = nullptr;

	void readProperties(Config *config);
private slots:
	void slotFire();
signals:
	void statusChanged(const bool updateWidgets);
};

class Trigger: public Base {
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

class PluginManager final {
public:
	static Action *action(const QString &id) { return m_actionMap[id]; }
	static QList<Action*> actionList() { return m_actionList; }
	static QHash<QString, Action*> actionMap() { return m_actionMap; }
	static void add(Action *action);
	static void add(Trigger *trigger);
	static void initActionsAndTriggers();
	static void readConfig();
	static void shutDown();
	static Trigger *trigger(const QString &id) { return m_triggerMap[id]; }
	static QList<Trigger*> triggerList() { return m_triggerList; }
	static QHash<QString, Trigger*> triggerMap() { return m_triggerMap; }
	static void writeConfig();
private:
	static QHash<QString, Action*> m_actionMap;
	static QHash<QString, Trigger*> m_triggerMap;
	static QList<Action*> m_actionList;
	static QList<Trigger*> m_triggerList;
};

#endif // KSHUTDOWN_PLUGINS_H
