// preferences.h - Preferences Dialog
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

#ifndef KSHUTDOWN_PREFERENCES_H
#define KSHUTDOWN_PREFERENCES_H

#include "pureqt.h"
#include "udialog.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QTabWidget>

class PasswordPreferences;

class Preferences: public UDialog {
	Q_OBJECT
public:
	explicit Preferences(QWidget *parent);
	virtual ~Preferences() = default;
	void apply();
private:
	Q_DISABLE_COPY(Preferences)
	bool m_oldProgressBarVisible;
	PasswordPreferences *m_passwordPreferences;
	QCheckBox *m_bwTrayIcon;
	QCheckBox *m_confirmAction;
	QCheckBox *m_lockScreenBeforeHibernate;
	QCheckBox *m_noMinimizeToSystemTrayIcon;
	QCheckBox *m_progressBarEnabled;
	QCheckBox *m_systemTrayIconEnabled;
	#ifndef KS_KF5
	QCheckBox *m_useThemeIconInSystemTray;
	#endif // !KS_KF5
	#ifdef Q_OS_LINUX
	QLineEdit *m_lockCommand;
	#endif // Q_OS_LINUX
	QTabWidget *m_tabs;
	//QWidget *createActionsWidget();
	QWidget *createGeneralWidget();
	QWidget *createSystemTrayWidget();
	//QWidget *createTriggersWidget();
private slots:
	void onFinish(int result);
	void onProgressBarEnabled(bool enabled);
	#ifndef KS_KF5
	void onUseThemeIconInSystemTraySelected(bool selected);
	#endif // !KS_KF5
};

#endif // KSHUTDOWN_PREFERENCES_H
