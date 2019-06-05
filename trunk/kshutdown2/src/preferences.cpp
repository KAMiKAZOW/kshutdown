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

#include "preferences.h"

#include "config.h"
#include "mainwindow.h"
#include "password.h"
#include "progressbar.h"
#include "utils.h"

#include <QPushButton>

#ifdef KS_KF5
	#include <KStandardGuiItem>
#endif // KS_KF5

// public

Preferences::Preferences(QWidget *parent) :
	UDialog(parent, i18n("Preferences"), false) {

	#ifdef Q_OS_WIN32
	rootLayout()->setMargin(5_px);
	rootLayout()->setSpacing(5_px);
	#endif // Q_OS_WIN32

	ProgressBar *progressBar = MainWindow::self()->progressBar();
	m_oldProgressBarVisible = progressBar->isVisible();

	m_tabs = new QTabWidget();
	m_tabs->addTab(createGeneralWidget(), i18n("General"));
	m_tabs->addTab(createSystemTrayWidget(), i18n("System Tray"));
	
	m_passwordPreferences = new PasswordPreferences(this);
	m_tabs->addTab(m_passwordPreferences, i18n("Password"));

// TODO: "Tweaks/Advanced" tab

// TODO: actions/triggers config.
	//tabs->addTab(createActionsWidget(), i18n("Actions"));
	//tabs->addTab(createTriggersWidget(), i18n("Triggers"));

	#ifdef KS_KF5
	if (Utils::isKDE()) {
		int iconSize = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
		m_tabs->setIconSize(QSize(iconSize, iconSize));
		m_tabs->setTabIcon(0, QIcon::fromTheme(KStandardGuiItem::configure().iconName())); // General
		m_tabs->setTabIcon(1, QIcon::fromTheme("user-desktop")); // System Tray
		m_tabs->setTabIcon(2, QIcon::fromTheme("dialog-password")); // Password
	}
	#endif // KS_KF5

	mainLayout()->addWidget(m_tabs);
	m_tabs->setFocus();

	// show recently used tab
	Config *config = Config::user();
	config->beginGroup("Preferences");
	int currentTabIndex = qBound(0, config->read("Current Tab Index", 0).toInt(), m_tabs->count() - 1);
	config->endGroup();
	m_tabs->setCurrentIndex(currentTabIndex);
	
	connect(this, SIGNAL(finished(int)), SLOT(onFinish(int)));
}

void Preferences::apply() {
	Config *config = Config::user();

	Config::blackAndWhiteSystemTrayIconVar.write(m_bwTrayIcon);
	Config::confirmActionVar.write(m_confirmAction);
	Config::lockScreenBeforeHibernateVar.write(m_lockScreenBeforeHibernate);

	Config::minimizeToSystemTrayIconVar.setBool(!m_noMinimizeToSystemTrayIcon->isChecked());
	Config::minimizeToSystemTrayIconVar.write();

	Config::progressBarEnabledVar.write(m_progressBarEnabled);
	Config::systemTrayIconEnabledVar.write(m_systemTrayIconEnabled);
	#ifndef KS_KF5
	Config::write("General", "Use Theme Icon In System Tray", m_useThemeIconInSystemTray->isChecked());
	#endif // !KS_KF5

	m_passwordPreferences->apply();

	#ifdef Q_OS_LINUX
	config->beginGroup("KShutdown Action lock");
	config->write("Custom Command", m_lockCommand->text());
	config->endGroup();
	#endif // Q_OS_LINUX

	Config::cancelDefaultVar.write(m_cancelDefault);
	Config::oldActionNamesVar.write(m_oldActionNames);

	config->sync();
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
	auto *l = new QVBoxLayout(w);
	l->setMargin(10);

	m_confirmAction = Config::confirmActionVar.newCheckBox(i18n("Confirm Action"));
	l->addWidget(m_confirmAction);

	m_cancelDefault = Config::cancelDefaultVar.newCheckBox(i18n("Select \"Cancel\" button by default"));
	QString attr = QApplication::isRightToLeft() ? "margin-right" : "margin-left";
	m_cancelDefault->setStyleSheet("QCheckBox { " + attr + ": 20px; }");
	l->addWidget(m_cancelDefault);

	l->addSpacing(20_px);

	m_oldActionNames = Config::oldActionNamesVar.newCheckBox(i18n("Use old action names"));
	m_oldActionNames->setToolTip(
		i18n("Old: %0").arg(i18n("Turn Off Computer") + ", " + i18n("Suspend Computer") + ", ...") + "\n" +
		i18n("New: %0").arg(i18n("Shut Down") + ", " + i18n("Sleep") + ", ...")
	);
	l->addWidget(m_oldActionNames);

	m_progressBarEnabled = Config::progressBarEnabledVar.newCheckBox(i18n("Show progress bar on top/bottom of the screen"));
// TODO: connect(m_progressBarEnabled, SIGNAL(toggled(bool)), SLOT(onProgressBarEnabled(bool)));
	l->addWidget(m_progressBarEnabled);

	l->addSpacing(20_px);
	l->addStretch();

	m_lockScreenBeforeHibernate = Config::lockScreenBeforeHibernateVar.newCheckBox(i18n("Lock Screen Before Hibernate/Suspend"));
	l->addWidget(m_lockScreenBeforeHibernate);
#if defined(Q_OS_WIN32) || defined(Q_OS_HAIKU)
	m_lockScreenBeforeHibernate->hide();
#endif // Q_OS_WIN32

	#ifdef Q_OS_LINUX
	if (m_lockScreenBeforeHibernate->isVisible())
		l->addSpacing(10_px);

	m_lockCommand = new QLineEdit();
	m_lockCommand->setClearButtonEnabled(true);

	Config *config = Config::user();
	config->beginGroup("KShutdown Action lock");
	m_lockCommand->setText(config->read("Custom Command", "").toString());
	config->endGroup();

// TODO: "Test/Presets" button
	QLabel *lockCommandLabel = new QLabel(i18n("Custom Lock Screen Command:"));
	lockCommandLabel->setBuddy(m_lockCommand);
	l->addSpacing(10_px);
	l->addWidget(lockCommandLabel);
	l->addWidget(m_lockCommand);
	#endif // Q_OS_LINUX

	return w;
}

QWidget *Preferences::createSystemTrayWidget() {
	QWidget *w = new QWidget();
	auto *l = new QVBoxLayout(w);
	l->setMargin(10);

// TODO: show info if not supported

	#ifdef Q_OS_LINUX
	auto *systemTrayWarning = new InfoWidget(this);
	systemTrayWarning->setText(
		i18n("May not work correctly with some Desktop Environments"),
		InfoWidget::Type::Warning
	);
	l->addWidget(systemTrayWarning);
	l->addSpacing(10_px);
	#endif // Q_OS_LINUX

	m_systemTrayIconEnabled = Config::systemTrayIconEnabledVar.newCheckBox(i18n("Enable System Tray Icon"));
	l->addWidget(m_systemTrayIconEnabled);

	m_noMinimizeToSystemTrayIcon = new QCheckBox(i18n("Quit instead of minimizing to System Tray Icon"));
	m_noMinimizeToSystemTrayIcon->setChecked(!Config::minimizeToSystemTrayIconVar);
	l->addWidget(m_noMinimizeToSystemTrayIcon);

	m_noMinimizeToSystemTrayIcon->setEnabled(m_systemTrayIconEnabled->isChecked());
	connect(m_systemTrayIconEnabled, SIGNAL(toggled(bool)), m_noMinimizeToSystemTrayIcon, SLOT(setEnabled(bool)));

	l->addSpacing(10_px);

	#ifndef KS_KF5
	m_useThemeIconInSystemTray = new QCheckBox(i18n("Use System Icon Theme"));
	m_useThemeIconInSystemTray->setChecked(Config::readBool("General", "Use Theme Icon In System Tray", true));
	connect(m_useThemeIconInSystemTray, &QCheckBox::toggled, [this](const bool selected) {
		m_bwTrayIcon->setEnabled(!selected);
	});
	l->addWidget(m_useThemeIconInSystemTray);
	#endif // !KS_KF5

	m_bwTrayIcon = Config::blackAndWhiteSystemTrayIconVar.newCheckBox(i18n("Black and White System Tray Icon"));
	#ifdef KS_KF5
// TODO: redesign this option
	m_bwTrayIcon->hide();
	#else
		#ifndef Q_OS_WIN32
		m_bwTrayIcon->setEnabled(!m_useThemeIconInSystemTray->isChecked());
		#endif // !Q_OS_WIN32
	#endif // KS_KF5
	l->addWidget(m_bwTrayIcon);

	#ifdef Q_OS_WIN32
	m_useThemeIconInSystemTray->hide();
	#endif // Q_OS_WIN32

// TODO: system tray icon preview

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
