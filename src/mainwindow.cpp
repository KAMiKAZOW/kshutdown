//
// mainwindow.cpp - The main window
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

#include "pureqt.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLayout>
#include <QTimer>

#ifdef KS_PURE_QT
	#include <QComboBox>
	#include <QMenuBar>
	#include <QPushButton>

	#include "version.h" // for about()
#else
	#include <KComboBox>
	#include <KMenu>
	#include <KMenuBar>
	#include <KPushButton>
	#include <KStandardAction>
	#include <KStandardGuiItem>
#endif // KS_PURE_QT

#include "mainwindow.h"
#include "theme.h"

MainWindow *MainWindow::m_instance = 0;

// public

MainWindow::~MainWindow() {
	U_DEBUG << "MainWindow::~MainWindow()" U_END;
}

QWidget *MainWindow::getElementById(const QString &id) {
	if (id.isEmpty())
		return 0;

	if (!m_elements) {
		m_elements = new QMap<QString, QWidget*>();

		m_elements->insert("main-window", this);
		m_elements->insert("menu-bar", menuBar());
		m_elements->insert("main-widget", centralWidget());

		m_elements->insert("actions", m_actions);
		m_elements->insert("action-box", m_actionBox);

		m_elements->insert("triggers", m_triggers);
		m_elements->insert("trigger-box", m_triggerBox);

		m_elements->insert("ok-cancel-button", m_okCancelButton);
// TODO: date-time-edit
	}

	return m_elements->value(id);
}

void MainWindow::setActive(const bool yes) {
	U_DEBUG << "MainWindow::setActive( " << yes << " )" U_END;

	if (m_active == yes)
		return;

	m_active = yes;

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();

	U_DEBUG << "\tMainWindow::getSelectedAction() == " << action->id() U_END;
	U_DEBUG << "\tMainWindow::getSelectedTrigger() == " << trigger->id() U_END;

	if (m_active) {
		m_triggerTimer->start(trigger->checkTimeout());
		action->setState(Base::START);
		trigger->setState(Base::START);
	}
	else {
		m_triggerTimer->stop();
		action->setState(Base::STOP);
		trigger->setState(Base::STOP);
	}

	setTitle(QString::null);
	updateWidgets();
}

// protected

void MainWindow::closeEvent(QCloseEvent *e) {
	writeConfig();

	// hide in system tray instead of close
	if (!m_forceQuit && !Action::totalExit()) {
		e->ignore();
		hide();

		if (m_active) {
			if (m_showActiveWarning) {
				m_showActiveWarning = false;
// TODO: common code (showMessage)
				m_systemTray->showMessage("KShutdown", i18n("KShutdown is still active!"), QSystemTrayIcon::Warning, 2000);
			}
		}
		else {
			if (m_showMinimizeInfo) {
				m_showMinimizeInfo = false;
				m_systemTray->showMessage("KShutdown", i18n("KShutdown has been minimized"), QSystemTrayIcon::Information, 2000);
			}
		}
	}
}

// private

MainWindow::MainWindow() :
	U_MAIN_WINDOW(0, Qt::Dialog),
	m_active(false),
	m_forceQuit(false),
	m_showActiveWarning(true),
	m_showMinimizeInfo(true),
	m_actionHash(QHash<QString, Action*>()),
	m_triggerHash(QHash<QString, Trigger*>()),
	m_elements(0),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(0),
	m_currentTriggerWidget(0),
	m_theme(new Theme(this)) {

	U_DEBUG << "MainWindow::MainWindow()" U_END;

	setObjectName("main-window");
#ifdef KS_PURE_QT
	setWindowIcon(U_STOCK_ICON("kshutdown"));
#endif // KS_PURE_QT

	// NOTE: do not change the "init" order,
	// or your computer will explode
	initWidgets();
	initActions();
	initTriggers();
	initPlugins();
	initSystemTray();
	initMenuBar();

	readConfig();

	//m_theme->load(this, "dark");//!!!

	setTitle(QString::null);
	updateWidgets();
}

void MainWindow::addAction(Action *action) {
	m_actions->addItem(action->icon(), action->text(), action->id());
	m_actionHash[action->id()] = action;

	int index = m_actions->count() - 1;
	U_DEBUG << "\tMainWindow::addAction( " << action->text() << " ) [ id=" << action->id() << ", index=" << index << " ]" U_END;
}

void MainWindow::addTrigger(Trigger *trigger) {
	m_triggers->addItem(trigger->icon(), trigger->text(), trigger->id());
	m_triggerHash[trigger->id()] = trigger;

	int index = m_triggers->count() - 1;
	U_DEBUG << "\tMainWindow::addTrigger( " << trigger->text() << " ) [ id=" << trigger->id() << ", index=" << index << " ]" U_END;
}

Action *MainWindow::getSelectedAction() const {
	return m_actionHash[m_actions->itemData(m_actions->currentIndex()).toString()];
}

void MainWindow::setSelectedAction(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedAction( " << id << " )" U_END;

	onActionActivated(selectById(m_actions, id));
}

Trigger *MainWindow::getSelectedTrigger() const {
	return m_triggerHash[m_triggers->itemData(m_triggers->currentIndex()).toString()];
}

void MainWindow::setSelectedTrigger(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedTrigger( " << id << " )" U_END;

	onTriggerActivated(selectById(m_triggers, id));
}

void MainWindow::initActions() {
	U_DEBUG << "MainWindow::initActions()" U_END;

	addAction(new ShutDownAction());
	addAction(new RebootAction());
	addAction(new HibernateAction());
	addAction(new SuspendAction());
	addAction(LockAction::self());
	addAction(new LogoutAction());
}

// TODO: customizable action/trigger presets

void MainWindow::initMenuBar() {
	U_DEBUG << "MainWindow::initMenuBar()" U_END;

	U_MENU_BAR *menuBar = new U_MENU_BAR();

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("&File"));
#ifdef KS_NATIVE_KDE
	fileMenu->addTitle(U_STOCK_ICON("dialog-warning"), i18n("No Delay"));
#else
// FIXME: Qt: warning title
#endif // KS_NATIVE_KDE
	QString id;
	for (int i = 0; i < m_actions->count(); ++i) {
		id = m_actions->itemData(i).toString();
		fileMenu->addAction(new ConfirmAction(m_actionHash[id]));
		if ((id == "reboot") || (id == "suspend"))
			fileMenu->addSeparator();
	}
	fileMenu->addSeparator();
#ifdef KS_NATIVE_KDE
	fileMenu->addAction(KStandardAction::quit(this, SLOT(onQuit()), this));
#else
	fileMenu->addAction(i18n("Quit"), this, SLOT(onQuit()), QKeySequence("Ctrl+Q"));
#endif // KS_NATIVE_KDE
	menuBar->addMenu(fileMenu);
	m_systemTray->setContextMenu(fileMenu);

	// settings menu

	U_MENU *settingsMenu = new U_MENU(i18n("&Settings"));
	//!!!
	menuBar->addMenu(settingsMenu);

	// help menu

#ifdef KS_NATIVE_KDE
// FIXME: KDE: too many menu items
	menuBar->addMenu(helpMenu());
#else
	U_MENU *helpMenu = new U_MENU(i18n("&Help"));
	helpMenu->addAction(i18n("About"), this, SLOT(onAbout()));
	helpMenu->addAction(i18n("About Qt"), U_APP, SLOT(aboutQt()));
	menuBar->addMenu(helpMenu);
#endif // KS_NATIVE_KDE

	setMenuBar(menuBar);
}

// TODO: plugins
void MainWindow::initPlugins() {
	U_DEBUG << "MainWindow::initPlugins()" U_END;
}

void MainWindow::initSystemTray() {
	U_DEBUG << "MainWindow::initSystemTray()" U_END;

	m_systemTray = new U_SYSTEM_TRAY(this);
	m_systemTray->setIcon(windowIcon());
	m_systemTray->show();
#ifdef KS_PURE_QT
	connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(onRestore(QSystemTrayIcon::ActivationReason)));
#endif // KS_PURE_QT
}

void MainWindow::initTriggers() {
	U_DEBUG << "MainWindow::initTriggers()" U_END;

	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));

	addTrigger(new NoDelayTrigger());
	addTrigger(new TimeFromNowTrigger());
	addTrigger(new DateTimeTrigger());
}

void MainWindow::initWidgets() {
	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("main-widget");
	QVBoxLayout *mainWidgetLayout = new QVBoxLayout();
	mainWidgetLayout->setMargin(5);
	mainWidgetLayout->setSpacing(5);
	mainWidget->setLayout(mainWidgetLayout);

	m_actionBox = new QGroupBox(i18n("Select an &action"), mainWidget);
	m_actionBox->setObjectName("action-box");
	m_actionBox->setLayout(new QVBoxLayout());

	m_actions = new U_COMBO_BOX();
	m_actions->setObjectName("actions");
	connect(m_actions, SIGNAL(activated(int)), SLOT(onActionActivated(int)));
	m_actionBox->layout()->addWidget(m_actions);

#ifdef Q_WS_WIN
	m_force = new QCheckBox(i18n("Force"), m_actionBox);
	m_force->setObjectName("force");
	m_actionBox->layout()->addWidget(m_force);
#else
	m_force = 0;
#endif // Q_WS_WIN

	m_triggerBox = new QGroupBox(i18n("S&elect a time"), mainWidget);
	m_triggerBox->setObjectName("trigger-box");
	m_triggerBox->setLayout(new QVBoxLayout());

	m_triggers = new U_COMBO_BOX();
	m_triggers->setObjectName("triggers");
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	m_triggerBox->layout()->addWidget(m_triggers);

	m_okCancelButton = new U_PUSH_BUTTON(mainWidget);
	m_okCancelButton->setObjectName("ok-cancel-button");
	m_okCancelButton->setDefault(true);
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	mainWidgetLayout->addWidget(m_actionBox);
	mainWidgetLayout->addWidget(m_triggerBox);
	mainWidgetLayout->addStretch();
	mainWidgetLayout->addWidget(m_okCancelButton);
	setCentralWidget(mainWidget);
}

void MainWindow::pluginConfig(const bool read) {
	U_CONFIG *config = U_CONFIG_USER;
	QString group;

	foreach (Action *i, m_actionHash) {
		group = "KShutdown Action " + i->id();
		if (read)
			i->readConfig(group, config);
		else
			i->writeConfig(group, config);
	}

	foreach (Trigger *i, m_triggerHash) {
		group = "KShutdown Trigger " + i->id();
		if (read)
			i->readConfig(group, config);
		else
			i->writeConfig(group, config);
	}
}

void MainWindow::readConfig() {
	U_DEBUG << "MainWindow::readConfig()" U_END;

	pluginConfig(true); // read

	U_CONFIG *config = U_CONFIG_USER;

#ifdef KS_NATIVE_KDE
	KConfigGroup g = config->group("General");
	setSelectedAction(g.readEntry("Selected Action", "shutdown"));
	setSelectedTrigger(g.readEntry("Selected Trigger", "time-from-now"));
#else
	config->beginGroup("General");
	setSelectedAction(config->value("Selected Action", "shutdown").toString());
	setSelectedTrigger(config->value("Selected Trigger", "time-from-now").toString());
	config->endGroup();
#endif // KS_NATIVE_KDE
}

int MainWindow::selectById(U_COMBO_BOX *comboBox, const QString &id) {
	int index = comboBox->findData(id);
	if (index == -1)
		index = 0;
	comboBox->setCurrentIndex(index);

	return index;
}

void MainWindow::setTitle(const QString &title) {
#ifdef KS_NATIVE_KDE
	setCaption(title);
	m_systemTray->setToolTip(title);
#else
	QString newTitle;
	if (title.isEmpty())
		newTitle = "KShutdown";
	else
		newTitle = (title + " - KShutdown");
	setWindowTitle(newTitle);
	m_systemTray->setToolTip(newTitle);
#endif // KS_NATIVE_KDE
}

void MainWindow::updateWidgets() {
	U_DEBUG << "MainWindow::updateWidgets()" U_END;

	bool enabled = !m_active;
	m_actions->setEnabled(enabled);
	if (m_currentActionWidget)
		m_currentActionWidget->setEnabled(enabled);
	m_triggers->setEnabled(enabled);
	if (m_currentTriggerWidget)
		m_currentTriggerWidget->setEnabled(enabled);

	if (m_force)
		m_force->setEnabled(enabled);

	Action *action = getSelectedAction();
	if (action->isEnabled()) {
		m_okCancelButton->setEnabled(true);
#ifdef KS_NATIVE_KDE
		m_okCancelButton->setGuiItem(
			m_active
			? KStandardGuiItem::cancel()
			: KStandardGuiItem::ok()
		);
#else
		m_okCancelButton->setText(
			m_active
			? i18n("Cancel")
			: i18n("OK")
		);
#endif // KS_NATIVE_KDE
	}
	else {
		m_okCancelButton->setEnabled(false);
		m_okCancelButton->setIcon(U_STOCK_ICON("dialog-warning"));
// TODO: show solution dialog
		m_okCancelButton->setText(i18n("Action not available: %0").arg(action->originalText()));
	}
// FIXME: adjust window to its minimum/preferred size
}

void MainWindow::writeConfig() {
	U_DEBUG << "MainWindow::writeConfig()" U_END;

	pluginConfig(false); // write

	U_CONFIG *config = U_CONFIG_USER;

#ifdef KS_NATIVE_KDE
	KConfigGroup g = config->group("General");
	g.writeEntry("Selected Action", getSelectedAction()->id());
	g.writeEntry("Selected Trigger", getSelectedTrigger()->id());
#else
	config->beginGroup("General");
	config->setValue("Selected Action", getSelectedAction()->id());
	config->setValue("Selected Trigger", getSelectedTrigger()->id());
	config->endGroup();
#endif // KS_NATIVE_KDE

	config->sync();
}

// private slots

#ifdef KS_PURE_QT
void MainWindow::onAbout() {
	QMessageBox::about(
		this,
		i18n("About"),
		"<qt>" \
		"<h1>KShutdown " KS_VERSION "</h1>" \
		KS_COPYRIGHT "<br>" \
		"<br>" \
		"<a href=\"" KS_HOME_PAGE "\">" KS_HOME_PAGE "</a><br>" \
		"<a href=\"mailto:" KS_CONTACT "\">" KS_CONTACT "</a>" \
		"</qt>"
	);
}
#endif // KS_PURE_QT

void MainWindow::onActionActivated(int index) {
	U_DEBUG << "MainWindow::onActionActivated( " << index << " )" U_END;

	if (m_currentActionWidget) {
		m_actionBox->layout()->removeWidget(m_currentActionWidget);
		m_currentActionWidget->hide();
	}

	m_currentActionWidget = getSelectedAction()->getWidget();
	if (m_currentActionWidget) {
		m_actionBox->layout()->addWidget(m_currentActionWidget);
		m_currentActionWidget->show();
	}

	updateWidgets();
}

void MainWindow::onCheckTrigger() {
	if (!m_active) {
		U_DEBUG << "MainWindow::onCheckTrigger(): INTERNAL ERROR"  U_END;

		return;
	}

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();

	// activate action
	if (trigger->canActivateAction()) {
		setActive(false);
		if (action->isEnabled()) {
			bool force = (m_force && m_force->isChecked());
			U_DEBUG << "Activate action: force=" << force U_END;
			action->activate(force);
		}
	}
	// update status
	else {
		QString actionStatus = action->originalText();
		QString triggerStatus = trigger->status();
		QString title = triggerStatus;
		if (!actionStatus.isEmpty()) {
			if (!title.isEmpty())
				title += " - ";
			title += actionStatus;
		}
		setTitle(title);
	}
}

void MainWindow::onOKCancel() {
	U_DEBUG << "MainWindow::onOKCancel()" U_END;

	setActive(!m_active);
}

void MainWindow::onQuit() {
	U_DEBUG << "MainWindow::onQuit()" U_END;

	m_forceQuit = true;
	m_systemTray->hide();
	close();
}

#ifdef KS_PURE_QT
void MainWindow::onRestore(QSystemTrayIcon::ActivationReason reason) {
	U_DEBUG << "MainWindow::onRestore()" U_END;

	switch (reason) {
		case QSystemTrayIcon::Trigger:
// TODO: bring to front and show on the current desktop
			if (isVisible()) {
				hide();
			}
			else {
				show();
			}
			break;
		default:
			break;
			// ignore
	}
}
#endif // KS_PURE_QT

void MainWindow::onTriggerActivated(int index) {
	U_DEBUG << "MainWindow::onTriggerActivated( " << index << " )" U_END;

	if (m_currentTriggerWidget) {
		m_triggerBox->layout()->removeWidget(m_currentTriggerWidget);
		m_currentTriggerWidget->hide();
	}

	m_currentTriggerWidget = getSelectedTrigger()->getWidget();
	if (m_currentTriggerWidget) {
		m_triggerBox->layout()->addWidget(m_currentTriggerWidget);
		m_currentTriggerWidget->show();
	}

	updateWidgets();
}
