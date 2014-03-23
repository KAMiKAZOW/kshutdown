// mainwindow.h - The main window
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

#ifndef KSHUTDOWN_MAINWINDOW_H
#define KSHUTDOWN_MAINWINDOW_H

// used in both main.cpp and mainwindow.cpp
#define KS_CONTACT "http://kshutdown.sourceforge.net/contact.html"
#define KS_COPYRIGHT "(C) 2003-3000 Konrad Twardowski"
#define KS_HOME_PAGE "http://kshutdown.sf.net/"

#ifdef KS_PURE_QT
	class QComboBox;
#else
	class KActionCollection;
	class KCmdLineArgs;
	class KComboBox;
#endif // KS_PURE_QT

#include "kshutdown.h"

class QCheckBox;
class QGroupBox;

class BookmarksMenu;
class InfoWidget;
class ProgressBar;
class USystemTray;

using namespace KShutdown;

class MainWindow: public U_MAIN_WINDOW {
	friend class TimeOption;
	Q_OBJECT
	#ifdef KS_PURE_QT
	// compatible with KDE
	Q_CLASSINFO("D-Bus Interface", "net.sf.kshutdown.MainWindow")
	#endif // KS_PURE_QT
public:
	enum DisplayStatus {
		DISPLAY_STATUS_HTML = 1 << 0,
		DISPLAY_STATUS_HTML_NO_ACTION = 1 << 1,
		DISPLAY_STATUS_SIMPLE = 1 << 2,
		DISPLAY_STATUS_APP_NAME = 1 << 3,
		DISPLAY_STATUS_OK_HINT = 1 << 4
	};
	virtual ~MainWindow();
	QHash<QString, Action*> actionHash() const { return m_actionHash; }
	QList<Action*> actionList() const { return m_actionList; }
	static bool checkCommandLine();
	QString getDisplayStatus(const int options);
	Action *getSelectedAction() const;
	Trigger *getSelectedTrigger() const;
	static void init();
	inline ProgressBar *progressBar() { return m_progressBar; }
	inline static MainWindow *self() {
		if (!m_instance)
			m_instance = new MainWindow();

		return m_instance;
	}
	void maybeShow();
	void setTime(const QString &trigger, const QTime &time, const bool absolute);
	QHash<QString, Trigger*> triggerHash() const { return m_triggerHash; }
public slots:
	Q_SCRIPTABLE QStringList actionList(const bool showDescription);
	Q_SCRIPTABLE QStringList triggerList(const bool showDescription);
	Q_SCRIPTABLE inline bool active() const { return m_active; }
	Q_SCRIPTABLE void setActive(const bool yes);
	void notify(const QString &id, const QString &text);
	Q_SCRIPTABLE void setExtrasCommand(const QString &command);
	Q_SCRIPTABLE void setSelectedAction(const QString &id);
	Q_SCRIPTABLE void setSelectedTrigger(const QString &id);
	Q_SCRIPTABLE void setTime(const QString &trigger, const QString &time);
	Q_SCRIPTABLE void setWaitForProcess(const qint64 pid);
	void writeConfig();
protected:
	virtual void closeEvent(QCloseEvent *e);
private:
	Q_DISABLE_COPY(MainWindow)
#ifdef KS_NATIVE_KDE
	KActionCollection *m_actionCollection;
#endif // KS_NATIVE_KDE
	BookmarksMenu *m_bookmarksMenu;
	bool m_active;
	bool m_forceQuit;
	bool m_ignoreUpdateWidgets;
	bool m_showActiveWarning;
	bool m_showMinimizeInfo;
	ConfirmAction *m_confirmLockAction;
	InfoWidget *m_infoWidget;
	static MainWindow *m_instance;
	ProgressBar *m_progressBar;
	QCheckBox *m_force;
	QGroupBox *m_actionBox;
	QGroupBox *m_triggerBox;
	static QHash<QString, Action*> m_actionHash;
	static QHash<QString, Trigger*> m_triggerHash;
	static QList<Action*> m_actionList;
	static QList<Trigger*> m_triggerList;
	QString m_lastNotificationID;
	QTimer *m_triggerTimer;
	QWidget *m_currentActionWidget;
	QWidget *m_currentTriggerWidget;
	U_ACTION *m_cancelAction;
	U_COMBO_BOX *m_actions;
	U_COMBO_BOX *m_triggers;
	U_PUSH_BUTTON *m_okCancelButton;
	USystemTray *m_systemTray;
	MainWindow();
	static void addAction(Action *action);
	static void addTrigger(Trigger *trigger);
	void initMenuBar();
	void initTriggers();
	void initWidgets();
	void pluginConfig(const bool read);
	void readConfig();
	void setTitle(const QString &plain, const QString &html);
	void updateWidgets();
private slots:
#ifdef KS_PURE_QT
	void onAbout();
#endif // KS_PURE_QT
	void onActionActivated(int index);
	void onCancel();
	void onCheckTrigger();
#ifdef KS_NATIVE_KDE
	void onConfigureNotifications();
	void onConfigureShortcuts();
#endif // KS_NATIVE_KDE
	void onFocusChange(QWidget *old, QWidget *now);
	void onForceClick();
	void onOKCancel();
	void onPreferences();
	void onQuit();
	void onStatusChange(const bool updateWidgets);
	void onTriggerActivated(int index);
};

#endif // KSHUTDOWN_MAINWINDOW_H
