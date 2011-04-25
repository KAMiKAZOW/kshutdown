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
#define KS_HOME_PAGE "http://kshutdown.sf.net"

#ifdef KS_PURE_QT
	class QComboBox;
#else
	class KCmdLineArgs;
	class KComboBox;
#endif // KS_PURE_QT

#include "kshutdown.h"

class QCheckBox;
class QGroupBox;

class InfoWidget;

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
		DISPLAY_STATUS_SIMPLE = 1 << 2
	};
	virtual ~MainWindow();
	static bool checkCommandLine();
	QString getDisplayStatus(const int options);
	static void init();
	inline static MainWindow *self() {
		if (!m_instance)
			m_instance = new MainWindow();

		return m_instance;
	}
	void maybeShow();
	void setTime(const QString &trigger, const QTime &time, const bool absolute);
public slots:
	Q_SCRIPTABLE QStringList actionList(const bool showDescription);
	Q_SCRIPTABLE QStringList triggerList(const bool showDescription);
	Q_SCRIPTABLE inline bool active() const { return m_active; }
	Q_SCRIPTABLE void setActive(const bool yes);
	void notify(const QString &id, const QString &text);
#ifdef KS_NATIVE_KDE
	Q_SCRIPTABLE void setExtrasCommand(const QString &command);
#endif // KS_NATIVE_KDE
	Q_SCRIPTABLE void setSelectedAction(const QString &id);
	Q_SCRIPTABLE void setSelectedTrigger(const QString &id);
	Q_SCRIPTABLE void setTime(const QString &trigger, const QString &time);
	Q_SCRIPTABLE void setWaitForProcess(const qlonglong pid);
protected:
	virtual void closeEvent(QCloseEvent *e);
private:
	Q_DISABLE_COPY(MainWindow)
#ifdef KS_NATIVE_KDE
	KActionCollection *m_actionCollection;
#endif // KS_NATIVE_KDE
	bool m_active;
	bool m_forceQuit;
	bool m_ignoreUpdateWidgets;
	bool m_showActiveWarning;
	bool m_showMinimizeInfo;
	ConfirmAction *m_confirmLockAction;
	InfoWidget *m_infoWidget;
	static MainWindow *m_instance;
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
	U_SYSTEM_TRAY *m_systemTray;
	MainWindow();
	static void addAction(Action *action);
	static void addTrigger(Trigger *trigger);
	Action *getSelectedAction() const;
	Trigger *getSelectedTrigger() const;
	void initMenuBar();
	void initSystemTray();
	void initTriggers();
	void initWidgets();
	void pluginConfig(const bool read);
	void readConfig();
	int selectById(U_COMBO_BOX *comboBox, const QString &id);
	void setTitle(const QString &title);
	void updateWidgets();
	void writeConfig();
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
#ifdef KS_PURE_QT
	void onRestore(QSystemTrayIcon::ActivationReason reason);
#endif // KS_PURE_QT
	void onStatusChange(const bool updateWidgets);
	void onTriggerActivated(int index);
};

#endif // KSHUTDOWN_MAINWINDOW_H
