// preferences.cpp - Preferences Dialog
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

#include <QCheckBox>
#include <QProcess>
#include <QVBoxLayout>

#include "config.h"
#include "mainwindow.h"
#include "password.h"
#include "preferences.h"
#include "progressbar.h"
#include "utils.h"

// public

Preferences::Preferences(QWidget *parent) :
	UDialog(parent, i18n("Preferences"), Utils::isGTKStyle()) {
	//U_DEBUG << "Preferences::Preferences()" U_END;

	#ifdef Q_OS_WIN32
	rootLayout()->setMargin(5_px);
	rootLayout()->setSpacing(5_px);
	#endif // Q_OS_WIN32

	ProgressBar *progressBar = MainWindow::self()->progressBar();
	m_oldProgressBarVisible = progressBar->isVisible();

	m_tabs = new U_TAB_WIDGET();
	m_tabs->addTab(createGeneralWidget(), i18n("General"));
	m_tabs->addTab(createSystemTrayWidget(), i18n("System Tray"));
	
	m_passwordPreferences = new PasswordPreferences(this);
	m_tabs->addTab(m_passwordPreferences, i18n("Password"));
	
// TODO: actions/triggers config.
	//tabs->addTab(createActionsWidget(), i18n("Actions"));
	//tabs->addTab(createTriggersWidget(), i18n("Triggers"));

	#ifdef KS_NATIVE_KDE
/*
	int iconSize = 24;
// TODO: int iconSize = KIconLoader::global()->currentSize(KIconLoader::Dialog);
	m_tabs->setIconSize(QSize(iconSize, iconSize));
	m_tabs->setTabIcon(0, U_STOCK_ICON("edit-bomb")); // General
	m_tabs->setTabIcon(1, U_STOCK_ICON("user-desktop")); // System Tray
	m_tabs->setTabIcon(2, U_STOCK_ICON("dialog-password")); // Password
*/
// FIXME: vertically misaligned icons. WTF?
	#endif // KS_NATIVE_KDE

	mainLayout()->addWidget(m_tabs);

	// show recently used tab
	Config *config = Config::user();
	config->beginGroup("Preferences");
	int currentTabIndex = qBound(0, config->read("Current Tab Index", 0).toInt(), m_tabs->count() - 1);
	config->endGroup();
	m_tabs->setCurrentIndex(currentTabIndex);
	
	connect(this, SIGNAL(finished(int)), SLOT(onFinish(int)));
}

Preferences::~Preferences() {
	//U_DEBUG << "Preferences::~Preferences()" U_END;
}

void Preferences::apply() {
	Config::setBlackAndWhiteSystemTrayIcon(m_bwTrayIcon->isChecked());
	Config::setConfirmAction(m_confirmAction->isChecked());
	Config::setLockScreenBeforeHibernate(m_lockScreenBeforeHibernate->isChecked());
	Config::setMinimizeToSystemTrayIcon(!m_noMinimizeToSystemTrayIcon->isChecked());
	Config::setProgressBarEnabled(m_progressBarEnabled->isChecked());
	#ifndef KS_KF5
	Config::setSystemTrayIconEnabled(m_systemTrayIconEnabled->isChecked());
	#endif // KS_KF5

	m_passwordPreferences->apply();

	Config::user()->sync();
}

// private

/*
QWidget *Preferences::createActionsWidget() {
	QWidget *w = createContainerWidget();

	return w;
}
*/

QWidget *Preferences::createGeneralWidget() {
	QWidget *w = new QWidget();
	QVBoxLayout *l = new QVBoxLayout(w);
	l->setMargin(10);

	m_confirmAction = new QCheckBox(i18n("Confirm Action"));
	m_confirmAction->setChecked(Config::confirmAction());
	l->addWidget(m_confirmAction);

	m_progressBarEnabled = new QCheckBox(i18n("Progress Bar"));
	m_progressBarEnabled->setChecked(Config::progressBarEnabled());
// TODO: connect(m_progressBarEnabled, SIGNAL(toggled(bool)), SLOT(onProgressBarEnabled(bool)));
	l->addWidget(m_progressBarEnabled);

	m_lockScreenBeforeHibernate = new QCheckBox(i18n("Lock Screen Before Hibernate"));
	m_lockScreenBeforeHibernate->setChecked(Config::lockScreenBeforeHibernate());
	l->addWidget(m_lockScreenBeforeHibernate);
#if defined(Q_OS_WIN32) || defined(Q_OS_HAIKU)
	m_lockScreenBeforeHibernate->hide();
#endif // Q_OS_WIN32

	l->addStretch();

#ifdef KS_NATIVE_KDE
	l->addSpacing(20_px);

	U_PUSH_BUTTON *kdeRelatedSettingsPushButton = new U_PUSH_BUTTON(U_STOCK_ICON("start-here-kde"), i18n("Related KDE Settings..."));
	l->addWidget(kdeRelatedSettingsPushButton);
	connect(kdeRelatedSettingsPushButton, SIGNAL(clicked()), SLOT(onKDERelatedSettings()));
#endif // KS_NATIVE_KDE

	return w;
}

QWidget *Preferences::createSystemTrayWidget() {
	QWidget *w = new QWidget();
	QVBoxLayout *l = new QVBoxLayout(w);
	l->setMargin(10);

	m_systemTrayIconEnabled = new QCheckBox(i18n("Enable System Tray Icon"));
	m_systemTrayIconEnabled->setChecked(Config::systemTrayIconEnabled());
	#ifdef KS_KF5
	m_systemTrayIconEnabled->setEnabled(false); // always on
	#endif // KS_KF5
	l->addWidget(m_systemTrayIconEnabled);

	m_noMinimizeToSystemTrayIcon = new QCheckBox(
		i18n("Quit instead of minimizing to System Tray Icon")
		// HACK: For some reason the last letter "j" (Polish translation) is truncated
		// in Oxygen style/Qt 4.8. Add trailing spaces to fix the layout :)
		+ "  "
	);
	m_noMinimizeToSystemTrayIcon->setChecked(!Config::minimizeToSystemTrayIcon());
	l->addWidget(m_noMinimizeToSystemTrayIcon);

	m_noMinimizeToSystemTrayIcon->setEnabled(m_systemTrayIconEnabled->isChecked());
	connect(m_systemTrayIconEnabled, SIGNAL(toggled(bool)), m_noMinimizeToSystemTrayIcon, SLOT(setEnabled(bool)));

	m_bwTrayIcon = new QCheckBox(i18n("Black and White System Tray Icon"));
	m_bwTrayIcon->setChecked(Config::blackAndWhiteSystemTrayIcon());
	#ifdef KS_KF5
// TODO: redesign this option
	m_bwTrayIcon->hide();
	#endif // KS_KF5
	l->addWidget(m_bwTrayIcon);

	l->addStretch();

	return w;
}

/*
QWidget *Preferences::createTriggersWidget() {
	QWidget *w = createContainerWidget();

	return w;
}
*/

// private slots

void Preferences::onFinish(int result) {
	Q_UNUSED(result)
	//U_DEBUG << "Finish: " << result U_END;

/*
	ProgressBar *progressBar = MainWindow::self()->progressBar();
	progressBar->setDemo(false);
	progressBar->setVisible(m_oldProgressBarVisible);
*/

	// save recently used tab
	Config *config = Config::user();
	config->beginGroup("Preferences");
	config->write("Current Tab Index", m_tabs->currentIndex());
	config->endGroup();
}

void Preferences::onProgressBarEnabled(bool enabled) {
	ProgressBar *progressBar = MainWindow::self()->progressBar();
	progressBar->setDemo(enabled);
	progressBar->setVisible(enabled || m_oldProgressBarVisible);
}

#ifdef KS_NATIVE_KDE
void Preferences::onKDERelatedSettings() {
	QProcess *process = new QProcess(0);
	#ifdef KS_KF5
	process->start("kcmshell5 autostart kcmsmserver kcm_sddm keys powerdevilglobalconfig powerdevilprofilesconfig screenlocker");
	#else
// HACK: Ubuntu Natty, Qt 4.7.2, KDE 4.6.2: "kdm" module resets all fonts to "Ubuntu"
// <https://bugs.launchpad.net/ubuntu/+source/kdebase/+bug/766145>
	process->start("kcmshell4 screensaver kcmsmserver powerdevilglobalconfig powerdevilprofilesconfig autostart kcmkded kgrubeditor keys");
	#endif // KS_KF5
}
#endif // KS_NATIVE_KDE
