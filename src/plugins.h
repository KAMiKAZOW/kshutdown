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

#ifdef KS_DBUS
	#include <QDBusInterface>
	#include <QDBusReply>
#endif // KS_DBUS

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

class Action: public Base {
	Q_OBJECT
public:
	explicit Action(const QString &text, const QString &iconName, const QString &id);
	void activate(const bool force);
	bool authorize(QWidget *parent);
	QAction *createConfirmAction(const bool alwaysShowConfirmationMessage);
	inline QStringList getCommandLineArgs() const {
		return m_commandLineArgs;
	}
	QIcon icon() const { return m_uiAction->icon(); }
	bool isCommandLineArgSupported();
	virtual bool isEnabled() const override { return m_uiAction->isEnabled(); }
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
	QAction *uiAction() const { return m_uiAction; }
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
	QAction *m_uiAction = nullptr;
	QStringList m_commandLineArgs;
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
	static void init();
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