//
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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

// used in both main.cpp and mainwindow.cpp
#define KS_CONTACT "kdtonline@poczta.onet.pl"
#define KS_COPYRIGHT "(C) 2003-3000 Konrad Twardowski"
#define KS_HOME_PAGE "http://kshutdown.sf.net"

#ifdef KS_PURE_QT
	#include <QHash>
	#include <QMainWindow>
	#include <QSystemTrayIcon>
	class QComboBox;
#else
	#include <KMainWindow>
	#include <KSystemTrayIcon>
	class KCmdLineArgs;
	class KComboBox;
#endif // KS_PURE_QT

#include "kshutdown.h"

class QCheckBox;
class QGroupBox;

class Theme;

using namespace KShutdown;

class MainWindow: public U_MAIN_WINDOW {
	Q_OBJECT
public:
	virtual ~MainWindow();
	static bool checkCommandLine();
	QWidget *getElementById(const QString &id);
	static QString getOption(const QString &name);
	static void init();
	static bool isArg(const QString &name);
	inline static MainWindow *self() {
		if (!m_instance)
			m_instance = new MainWindow();

		return m_instance;
	}
	void maybeShow();
	void setActive(const bool yes);
protected:
	virtual void closeEvent(QCloseEvent *e);
private:
	bool m_active;
	bool m_forceQuit;
	bool m_showActiveWarning;
	bool m_showMinimizeInfo;
	ConfirmAction *m_confirmLockAction;
#ifdef KS_NATIVE_KDE
	static KCmdLineArgs *m_args;
#else
	static QStringList m_args;
#endif // KS_NATIVE_KDE
	static MainWindow *m_instance;
	QCheckBox *m_force;
	QGroupBox *m_actionBox;
	QGroupBox *m_triggerBox;
	static QHash<QString, Action*> m_actionHash;
	static QHash<QString, Trigger*> m_triggerHash;
	static QList<Action*> m_actionList;
	static QList<Trigger*> m_triggerList;
	QMap<QString, QWidget*> *m_elements;
	QTimer *m_triggerTimer;
	QWidget *m_currentActionWidget;
	QWidget *m_currentTriggerWidget;
	Theme *m_theme;
	U_COMBO_BOX *m_actions;
	U_COMBO_BOX *m_triggers;
	U_PUSH_BUTTON *m_okCancelButton;
	U_SYSTEM_TRAY *m_systemTray;
	MainWindow();
	static void addAction(Action *action);
	static void addTrigger(Trigger *trigger);
	Action *getSelectedAction() const;
	void setSelectedAction(const QString &id);
	Trigger *getSelectedTrigger() const;
	void setSelectedTrigger(const QString &id);
	void initMenuBar();
	void initPlugins();
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
	void onCheckTrigger();
	void onForceClick();
	void onOKCancel();
	void onPreferences();
	void onQuit();
#ifdef KS_PURE_QT
	void onRestore(QSystemTrayIcon::ActivationReason reason);
#endif // KS_PURE_QT
	void onTriggerActivated(int index);
};

#endif // __MAINWINDOW_H__
