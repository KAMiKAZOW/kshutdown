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

#include "kshutdown.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QMainWindow>
#include <QPushButton>

#ifdef Q_OS_WIN32
	#include <QWinTaskbarButton>
#endif // Q_OS_WIN32

#ifdef KS_KF5
	#include <KActionCollection>
#endif // KS_KF5

class BookmarksMenu;
class ProgressBar;
class USystemTray;

using namespace KShutdown;

class MainWindow: public QMainWindow {
	friend class Mod;
	friend class TimeOption;
	friend class USystemTray;
	Q_OBJECT
	#ifdef KS_PURE_QT
	// compatible with KDE
	Q_CLASSINFO("D-Bus Interface", "net.sf.kshutdown.MainWindow")
	#endif // KS_PURE_QT
public:
	enum/* non-class */DisplayStatus {
		DISPLAY_STATUS_HTML = 1 << 0,
		DISPLAY_STATUS_HTML_NO_ACTION = 1 << 1,
		DISPLAY_STATUS_SIMPLE = 1 << 2,
		DISPLAY_STATUS_APP_NAME = 1 << 3,
		DISPLAY_STATUS_OK_HINT = 1 << 4
	};
	virtual ~MainWindow();
	QAction *cancelAction() const { return m_cancelAction; }
	QString getDisplayStatus(const int options);
	Action *getSelectedAction() const;
	Trigger *getSelectedTrigger() const;
	inline InfoWidget *infoWidget() { return m_infoWidget; }
	inline QPushButton *okCancelButton() { return m_okCancelButton; }
	inline ProgressBar *progressBar() { return m_progressBar; }
	static MainWindow *self() {
		if (!m_instance)
			m_instance = new MainWindow();

		return m_instance;
	}
	bool maybeShow(const bool forceShow = false);
	void setTime(const QString &selectTrigger, const QTime &time, const bool absolute);
	#ifdef Q_OS_WIN32
	QWinTaskbarButton *winTaskbarButton() { return m_winTaskbarButton; }
	#endif // Q_OS_WIN32
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
	virtual void closeEvent(QCloseEvent *e) override;
	#ifdef Q_OS_WIN32
	virtual void showEvent(QShowEvent *e) override;
	#endif // Q_OS_WIN32
private:
	Q_DISABLE_COPY(MainWindow)
#ifdef KS_KF5
	KActionCollection *m_actionCollection;
#endif // KS_KF5
	BookmarksMenu *m_bookmarksMenu;
	bool m_active;
	bool m_forceQuit;
	bool m_ignoreUpdateWidgets;
	bool m_showActiveWarning;
	bool m_showMinimizeInfo;
	InfoWidget *m_infoWidget;
	static MainWindow *m_instance;
	ProgressBar *m_progressBar;
	QAction *m_cancelAction;
	QAction *m_confirmLockAction;
	QCheckBox *m_force;
	QComboBox *m_actions;
	QComboBox *m_triggers;
	QGroupBox *m_actionBox;
	QGroupBox *m_triggerBox;
	QPushButton *m_okCancelButton;
	QString m_lastNotificationID;
	QTimer *m_triggerTimer;
	QWidget *m_currentActionWidget;
	QWidget *m_currentTriggerWidget;
	#ifdef Q_OS_WIN32
	QWinTaskbarButton *m_winTaskbarButton = nullptr;
	#endif // Q_OS_WIN32
	USystemTray *m_systemTray;
	explicit MainWindow();
	QAction *createQuitAction();
	void initFileMenu(QMenu *fileMenu, const bool mainMenu);
	void initMenuBar();
	void initTriggers();
	void initWidgets();
	void readConfig();
	void setActive(const bool yes, const bool needAuthorization);
	void setTitle(const QString &plain, const QString &html);
	void updateWidgets();
private slots:
#ifdef KS_PURE_QT
	void onAbout();
#endif // KS_PURE_QT
	void onActionActivated(int index);
	void onCheckTrigger();
#ifdef KS_KF5
	void onConfigureNotifications();
	void onConfigureShortcuts();
#endif // KS_KF5
	void onFocusChange(QWidget *old, QWidget *now);
	void onForceClick();
	void onMenuHovered(QAction *action);
	void onOKCancel();
	void onPreferences();
	void onQuit();
	void onStatusChange(const bool forceUpdateWidgets);
	void onTriggerActivated(int index);
};

#endif // KSHUTDOWN_MAINWINDOW_H
