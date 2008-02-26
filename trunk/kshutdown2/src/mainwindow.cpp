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

	#include "version.h" // for about()
#else
	#include <KCmdLineArgs>
	#include <KComboBox>
	#include <KStandardAction>
#endif // KS_PURE_QT

#include "actions/extras.h"
#include "mainwindow.h"
#include "preferences.h"
#include "theme.h"

#ifdef KS_NATIVE_KDE
	KCmdLineArgs *MainWindow::m_args = 0;
#else
	QStringList MainWindow::m_args;
#endif // KS_NATIVE_KDE
MainWindow *MainWindow::m_instance = 0;
QHash<QString, Action*> MainWindow::m_actionHash;
QHash<QString, Trigger*> MainWindow::m_triggerHash;
QList<Action*> MainWindow::m_actionList;
QList<Trigger*> MainWindow::m_triggerList;

// public

MainWindow::~MainWindow() {
	U_DEBUG << "MainWindow::~MainWindow()" U_END;
#ifdef KS_NATIVE_KDE
	if (m_args)
		m_args->clear();
#endif // KS_NATIVE_KDE
}

bool MainWindow::checkCommandLine() {
	Action *actionToActivate = 0;
	foreach (Action *action, m_actionList) {
		if (action->isCommandLineArgSupported()) {
			actionToActivate = action;

			break; // foreach
		}
	}
	if (actionToActivate) {
		actionToActivate->activate(false); //!!!force
		
		return true;
	}
	
	return false;
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
// TODO: date-time-edit, force
	}

	return m_elements->value(id);
}

QString MainWindow::getOption(const QString &name) {
#ifdef KS_NATIVE_KDE
	return m_args->getOption(name.toAscii());
#else
	int i = m_args.indexOf("-" + name, 1);
	if (i == -1) {
		i = m_args.indexOf("--" + name, 1);
		if (i == -1) {
			U_DEBUG << "Argument not found: " << name U_END;

			return QString::null;
		}
	}

	int argIndex = (i + 1);

	if (argIndex < m_args.size()) {
		U_DEBUG << "Value of " << name << " is " << m_args[argIndex] U_END;

		return m_args[argIndex];
	}

	U_DEBUG << "Argument value is not set: " << name U_END;

	return QString::null;
#endif // KS_NATIVE_KDE
}

void MainWindow::init() {
#ifdef KS_NATIVE_KDE
	m_args = KCmdLineArgs::parsedArgs();
#else
	m_args = U_APP->arguments();
#endif // KS_NATIVE_KDE

	U_DEBUG << "MainWindow::init(): Actions" U_END;
	m_actionHash = QHash<QString, Action*>();
	m_actionList = QList<Action*>();
	addAction(new ShutDownAction());
	addAction(new RebootAction());
	addAction(new HibernateAction());
	addAction(new SuspendAction());
	addAction(LockAction::self());
	addAction(new LogoutAction());
#ifdef KS_NATIVE_KDE
	addAction(Extras::self());
#endif // KS_NATIVE_KDE

	U_DEBUG << "MainWindow::init(): Triggers" U_END;
	m_triggerHash = QHash<QString, Trigger*>();
	m_triggerList = QList<Trigger*>();
	addTrigger(new NoDelayTrigger());
	addTrigger(new TimeFromNowTrigger());
	addTrigger(new DateTimeTrigger());
}

bool MainWindow::isArg(const QString &name) {
#ifdef KS_NATIVE_KDE
	return m_args->isSet(name.toAscii());
#else
	return (m_args.contains("-" + name) || m_args.contains("--" + name));
#endif // KS_NATIVE_KDE
}

void MainWindow::maybeShow() {
	if (!U_SYSTEM_TRAY::isSystemTrayAvailable()) {
		show();

		return;
	}

	if (!isArg("init") && !U_APP->isSessionRestored())
		show();
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
#ifdef Q_WS_WIN
		// HACK: disable "Lock Screen" action if countdown is active
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(false);
#endif // Q_WS_WIN
		m_triggerTimer->start(trigger->checkTimeout());
		action->setState(Base::START);
		trigger->setState(Base::START);
	}
	else {
#ifdef Q_WS_WIN
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(true);
#endif // Q_WS_WIN
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
	m_elements(0),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(0),
	m_currentTriggerWidget(0),
	m_theme(new Theme(this)) {

	U_DEBUG << "MainWindow::MainWindow()" U_END;

	// HACK: It seems that the "quit on last window closed"
	// does not work correctly if main window is hidden
	// "in" the system tray..
	U_APP->setQuitOnLastWindowClosed(false);

	setObjectName("main-window");
#ifdef KS_PURE_QT
	setWindowIcon(U_STOCK_ICON("kshutdown"));
#endif // KS_PURE_QT

	// NOTE: do not change the "init" order,
	// or your computer will explode
	initWidgets();
	
	// init actions
	foreach (Action *action, m_actionList) {
		m_actions->addItem(action->icon(), action->text(), action->id());
		int index = m_actions->count() - 1;
		U_DEBUG << "\tMainWindow::addAction( " << action->text() << " ) [ id=" << action->id() << ", index=" << index << " ]" U_END;
	}
	
	// init triggers
	foreach (Trigger *trigger, m_triggerList) {
		m_triggers->addItem(trigger->icon(), trigger->text(), trigger->id());
		int index = m_triggers->count() - 1;
		U_DEBUG << "\tMainWindow::addTrigger( " << trigger->text() << " ) [ id=" << trigger->id() << ", index=" << index << " ]" U_END;	
	}
	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));
	
	initPlugins();
	initSystemTray();
	initMenuBar();

	readConfig();

	// load theme
	QString theme = getOption("theme");
	if (!theme.isEmpty())
		m_theme->load(this, theme);

	setTitle(QString::null);
	updateWidgets();
	
	checkCommandLine();
}

void MainWindow::addAction(Action *action) {
	m_actionHash[action->id()] = action;
	m_actionList.append(action);
}

void MainWindow::addTrigger(Trigger *trigger) {
	m_triggerHash[trigger->id()] = trigger;
	m_triggerList.append(trigger);
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

// TODO: customizable action/trigger presets

void MainWindow::initMenuBar() {
	U_DEBUG << "MainWindow::initMenuBar()" U_END;

	U_MENU_BAR *menuBar = new U_MENU_BAR();

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("&File"));
#ifdef KS_NATIVE_KDE
	fileMenu->addTitle(U_STOCK_ICON("dialog-warning"), i18n("No Delay"));
#else
// FIXME: Qt: warning menu title
#endif // KS_NATIVE_KDE
	Action *a;
	QString id;
	for (int i = 0; i < m_actions->count(); ++i) {
		id = m_actions->itemData(i).toString();
		a = m_actionHash[id];

		if (!a->showInMenu())
			continue; // for

		ConfirmAction *ca = new ConfirmAction(a);
		if (a == LockAction::self())
			m_confirmLockAction = ca;

		fileMenu->addAction(ca);
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
#ifdef KS_NATIVE_KDE
	settingsMenu->addAction(KStandardAction::preferences(this, SLOT(onPreferences()), this));
#else
	settingsMenu->addAction(i18n("Preferences..."), this, SLOT(onPreferences()));
#endif // KS_NATIVE_KDE
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

void MainWindow::initWidgets() {
	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("main-widget");
	QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(5);

	m_actionBox = new QGroupBox(i18n("Select an &action"));
	m_actionBox->setObjectName("action-box");
	m_actionBox->setLayout(new QVBoxLayout());

	m_actions = new U_COMBO_BOX();
	m_actions->setObjectName("actions");
	connect(m_actions, SIGNAL(activated(int)), SLOT(onActionActivated(int)));
	m_actionBox->layout()->addWidget(m_actions);

	m_force = new QCheckBox(i18n("Do not save session"));
	m_force->setObjectName("force");
	connect(m_force, SIGNAL(clicked()), SLOT(onForceClick()));
	m_actionBox->layout()->addWidget(m_force);
#ifndef Q_WS_WIN
	m_force->hide();
#endif // Q_WS_WIN

	m_triggerBox = new QGroupBox(i18n("S&elect a time"));
	m_triggerBox->setObjectName("trigger-box");
	m_triggerBox->setLayout(new QVBoxLayout());

	m_triggers = new U_COMBO_BOX();
	m_triggers->setObjectName("triggers");
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	m_triggerBox->layout()->addWidget(m_triggers);

	m_okCancelButton = new U_PUSH_BUTTON();
	m_okCancelButton->setObjectName("ok-cancel-button");
	m_okCancelButton->setDefault(true);
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	mainLayout->addWidget(m_actionBox);
	mainLayout->addWidget(m_triggerBox);
	mainLayout->addStretch();
	mainLayout->addWidget(m_okCancelButton);
	setCentralWidget(mainWidget);
}

void MainWindow::pluginConfig(const bool read) {
	Config *config = Config::user();
	QString group;

	foreach (Action *i, m_actionList) {
		group = "KShutdown Action " + i->id();
		if (read)
			i->readConfig(group, config);
		else
			i->writeConfig(group, config);
	}

	foreach (Trigger *i, m_triggerList) {
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

	Config *config = Config::user();
	config->beginGroup("General");
	setSelectedAction(config->read("Selected Action", "shutdown").toString());
	setSelectedTrigger(config->read("Selected Trigger", "time-from-now").toString());
	config->endGroup();
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
#else
	if (title.isEmpty())
		setWindowTitle("KShutdown");
	else
		setWindowTitle(title + " - KShutdown");
#endif // KS_NATIVE_KDE
	m_systemTray->setToolTip(windowTitle());
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
		m_okCancelButton->setToolTip(i18n("Click to active the selected action"));
#endif // KS_NATIVE_KDE
	}
	else {
		m_okCancelButton->setEnabled(false);
		m_okCancelButton->setIcon(U_STOCK_ICON("dialog-warning"));
// TODO: show solution dialog
		QString text = i18n("Action not available: %0")
			.arg(action->originalText());
		m_okCancelButton->setText(text);
		m_okCancelButton->setToolTip(action->disableReason());
	}
// FIXME: adjust window to its minimum/preferred size
}

void MainWindow::writeConfig() {
	U_DEBUG << "MainWindow::writeConfig()" U_END;

	pluginConfig(false); // write

	Config *config = Config::user();
	config->beginGroup("General");
	config->write("Selected Action", getSelectedAction()->id());
	config->write("Selected Trigger", getSelectedTrigger()->id());
	config->endGroup();
	config->sync();
}

// private slots

#ifdef KS_PURE_QT
void MainWindow::onAbout() {
	QString version = KS_VERSION;
#ifdef KS_PORTABLE
	version += " (portable)";
#endif // KS_PORTABLE
	QMessageBox::about(
		this,
		i18n("About"),
		"<qt>" \
		"<h1>KShutdown " + version + "</h1>" \
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
			U_DEBUG << "Activate action: force=" << m_force->isChecked() U_END;
			action->activate(m_force->isChecked());
		}
	}
	// update status
	else {
		QString actionStatus = action->originalText();
		
/* TODO: extras command name
		if (!action->status().isEmpty())
			actionStatus += (" - " + action->status());
*/

		QString triggerStatus = trigger->status();

		QString title = triggerStatus;
		if (!actionStatus.isEmpty()) {
			if (!title.isEmpty()) {
// FIXME: i18n
				title = (i18n("Remaining time:") + " " + title);
				title += " - ";
			}
			title += actionStatus;
		}
		setTitle(title);
	}
}

void MainWindow::onForceClick() {
	if (
		m_force->isChecked() &&
		!U_CONFIRM(this, i18n("Confirm"), i18n("Are you sure you want to enable this option?\n\nData in all unsaved documents will be lost!"))
	)
		m_force->setChecked(false);
}

void MainWindow::onOKCancel() {
	U_DEBUG << "MainWindow::onOKCancel()" U_END;

// TODO: confirm "Start"
	setActive(!m_active);
}

void MainWindow::onPreferences() {
	U_DEBUG << "MainWindow::onPreferences()" U_END;

	Preferences *p = new Preferences(this);
// TODO: ok/cancel panel
	p->exec();
	p->apply();
	delete p;
}

void MainWindow::onQuit() {
	U_DEBUG << "MainWindow::onQuit()" U_END;

	m_forceQuit = true;
	m_systemTray->hide();
	close();
	U_APP->quit();
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
