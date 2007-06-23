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

#include <QCloseEvent>
#include <QFile>
#include <QGroupBox>
#include <QLayout>
#include <QTimer>

#ifdef KS_PURE_QT
	#include <QApplication> // for aboutQt()
	#include <QComboBox>
	#include <QMenuBar>
	#include <QPushButton>

	#include "version.h" // for about()
#else
	#include <KComboBox>
	#include <KMenuBar>
	#include <KPushButton>
	#include <KStandardAction>
	#include <KStandardGuiItem>
	//!!!#include "mainwindow.moc"
#endif // KS_PURE_QT

#include "mainwindow.h"

MainWindow *MainWindow::m_instance = 0;

// public

MainWindow::~MainWindow() {
	U_DEBUG << "MainWindow::~MainWindow()";

	writeConfig();
}

// protected

void MainWindow::closeEvent(QCloseEvent *e) {
	// hide in system tray instead of close
	if (!m_forceQuit) {
		e->ignore();
		hide();

		if (m_active) {
			if (m_showActiveWarning) {
				m_showActiveWarning = false;
				m_systemTray->showMessage("KShutdown", i18n("KShutdown is still active!"), QSystemTrayIcon::Warning);
			}
		}
		else {
			if (m_showMinimizeInfo) {
				m_showMinimizeInfo = false;
				m_systemTray->showMessage("KShutdown", i18n("KShutdown has been minimized"));
			}
		}
	}
}

// private

MainWindow::MainWindow() :
	U_MAIN_WINDOW(),
	m_active(false),
	m_forceQuit(false),
	m_showActiveWarning(true),
	m_showMinimizeInfo(true),
	m_actionHash(QHash<QString, Action*>()),
	m_triggerHash(QHash<QString, Trigger*>()),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(0),
	m_currentTriggerWidget(0) {

	U_DEBUG << "MainWindow::MainWindow()";

	setObjectName("mainWindow");
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

	//setTheme("dark");//!!!

	setTitle(QString::null);
	updateWidgets();
}

void MainWindow::addAction(Action *action) {
	m_actions->addItem(action->icon(), action->text(), action->id());
	m_actionHash[action->id()] = action;

	int index = m_actions->count() - 1;
	U_DEBUG << "\tMainWindow::addAction( " << action->text() << " ) [ id=" << action->id() << ", index=" << index << " ]";
}

void MainWindow::addTrigger(Trigger *trigger) {
	m_triggers->addItem(trigger->icon(), trigger->text(), trigger->id());
	m_triggerHash[trigger->id()] = trigger;

	int index = m_triggers->count() - 1;
	U_DEBUG << "\tMainWindow::addTrigger( " << trigger->text() << " ) [ id=" << trigger->id() << ", index=" << index << " ]";
}

Action *MainWindow::getSelectedAction() const {
	return m_actionHash[m_actions->itemData(m_actions->currentIndex()).toString()];
}

void MainWindow::setSelectedAction(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedAction( " << id << " )";

	onActionActivated(selectById(m_actions, id));
}

Trigger *MainWindow::getSelectedTrigger() const {
	return m_triggerHash[m_triggers->itemData(m_triggers->currentIndex()).toString()];
}

void MainWindow::setSelectedTrigger(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedTrigger( " << id << " )";

	onTriggerActivated(selectById(m_triggers, id));
}

void MainWindow::initActions() {
	U_DEBUG << "MainWindow::initActions()";

	addAction(LockAction::self());
	addAction(new LogoutAction());
	addAction(new ShutDownAction());
	addAction(new RebootAction());
	addAction(new SuspendAction());
	addAction(new HibernateAction());
}

// TODO: action/trigger presets

void MainWindow::initMenuBar() {
	U_DEBUG << "MainWindow::initMenuBar()";

	U_MENU_BAR *menuBar = new U_MENU_BAR();

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("&File"));
#ifdef KS_NATIVE_KDE
	fileMenu->addTitle(U_STOCK_ICON("messagebox_warning"), i18n("No Delay"));
#else
	//!!!
#endif // KS_NATIVE_KDE
	QString id;
	for (int i = 0; i < m_actions->count(); ++i) {
		id = m_actions->itemData(i).toString();
		fileMenu->addAction(m_actionHash[id]);
		if ((id == "logout") || (id == "reboot"))
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

// FIXME: too many menu items
#ifdef KS_NATIVE_KDE
	menuBar->addMenu(helpMenu());
#else
	U_MENU *helpMenu = new U_MENU(i18n("&Help"));
	helpMenu->addAction(i18n("About"), this, SLOT(onAbout()));
	helpMenu->addAction(i18n("About Qt"), qApp, SLOT(aboutQt()));
	menuBar->addMenu(helpMenu);
#endif // KS_NATIVE_KDE

	setMenuBar(menuBar);
}

// TODO: plugins
void MainWindow::initPlugins() {
	U_DEBUG << "MainWindow::initPlugins()";
}

void MainWindow::initSystemTray() {
	U_DEBUG << "MainWindow::initSystemTray()";

	m_systemTray = new U_SYSTEM_TRAY(this);
	//!!!m_systemTray->setIcon(windowIcon());
	m_systemTray->setToolTip("KShutdown");
	m_systemTray->show();
	connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(onRestore(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::initTriggers() {
	U_DEBUG << "MainWindow::initTriggers()";

	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));

	addTrigger(new NoDelayTrigger());
	addTrigger(new TimeFromNowTrigger());
	addTrigger(new DateTimeTrigger());
}

void MainWindow::initWidgets() {
	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("mainWidget");
	QVBoxLayout *mainWidgetLayout = new QVBoxLayout();
	mainWidgetLayout->setMargin(5);
	mainWidgetLayout->setSpacing(5);
	mainWidget->setLayout(mainWidgetLayout);

	m_actionBox = new QGroupBox(i18n("Select an &action"), mainWidget);
	m_actionBox->setObjectName("actionBox");
	m_actionBox->setLayout(new QVBoxLayout());

	QWidget *hBox = new QWidget();
	QHBoxLayout *hBoxLayout = new QHBoxLayout();
	hBoxLayout->setMargin(5);
	hBoxLayout->setSpacing(5);
	hBox->setLayout(hBoxLayout);
	m_actionBox->layout()->addWidget(hBox);

	m_actions = new U_COMBO_BOX(hBox);
	m_actions->setObjectName("actions");
	connect(m_actions, SIGNAL(activated(int)), SLOT(onActionActivated(int)));

	m_configureActionButton = new U_PUSH_BUTTON(hBox);
	m_configureActionButton->setObjectName("configureActionButton");
#ifdef KS_NATIVE_KDE
	m_configureActionButton->setGuiItem(KStandardGuiItem::configure());
#else
	m_configureActionButton->setText(i18n("Configure..."));
#endif // KS_NATIVE_KDE
	m_configureActionButton->setToolTip(i18n("Configure selected action"));
	connect(m_configureActionButton, SIGNAL(clicked()), SLOT(onConfigureAction()));

	hBoxLayout->addWidget(m_actions);
	hBoxLayout->addWidget(m_configureActionButton);

	m_triggerBox = new QGroupBox(i18n("S&elect a time"), mainWidget);
	m_triggerBox->setObjectName("triggerBox");
	m_triggerBox->setLayout(new QVBoxLayout());

	m_triggers = new U_COMBO_BOX();
	m_triggers->setObjectName("triggers");
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	m_triggerBox->layout()->addWidget(m_triggers);

// TODO: align center
	m_okCancelButton = new U_PUSH_BUTTON(mainWidget);
	m_okCancelButton->setObjectName("okCancelButton");
	m_okCancelButton->setDefault(true);
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	mainWidgetLayout->addWidget(m_actionBox);
	mainWidgetLayout->addWidget(m_triggerBox);
	mainWidgetLayout->addStretch();
	mainWidgetLayout->addWidget(m_okCancelButton);
	setCentralWidget(mainWidget);
}

void MainWindow::pluginConfig(const bool read) {
#ifdef KS_NATIVE_KDE
	KConfig *config = KGlobal::config().data();//!!!

	foreach (Action *i, m_actionHash) {
		config->setGroup("KShutdown Action " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
	}

	foreach (Trigger *i, m_triggerHash) {
		config->setGroup("KShutdown Trigger " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
	}
#else
	Q_UNUSED(read)
#endif // KS_NATIVE_KDE
}

void MainWindow::readConfig() {
	U_DEBUG << "MainWindow::readConfig()";
#ifdef KS_NATIVE_KDE
	KConfig *config = KGlobal::config().data();

	pluginConfig(true); // read

	config->setGroup("General");
	setSelectedAction(config->readEntry("Selected Action"));
	setSelectedTrigger(config->readEntry("Selected Trigger"));
#endif // KS_NATIVE_KDE
}

int MainWindow::selectById(U_COMBO_BOX *comboBox, const QString &id) {
	int index = comboBox->findData(id);
	if (index == -1)
		index = 0;
	comboBox->setCurrentIndex(index);

	return index;
}

void MainWindow::setActive(const bool yes) {
	U_DEBUG << "MainWindow::setActive( " << yes << " )";

	if (m_active == yes)
		return;

	m_active = yes;

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();

	U_DEBUG << "\tMainWindow::getSelectedAction() == " << action->id();
	U_DEBUG << "\tMainWindow::getSelectedTrigger() == " << trigger->id();

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

void MainWindow::setTheme(const QString &name) {
//!!!disable for 4.2-
	QFile style("themes/" + name + "/style.css");
	if (style.open(QFile::ReadOnly | QFile::Text))
		setStyleSheet(QTextStream(&style).readAll());
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
}

void MainWindow::updateWidgets() {
	U_DEBUG << "MainWindow::updateWidgets()";

	bool enabled = !m_active;
	m_actions->setEnabled(enabled);
	if (m_currentActionWidget)
		m_currentActionWidget->setEnabled(enabled);
	m_configureActionButton->setEnabled(enabled);
	m_triggers->setEnabled(enabled);
	if (m_currentTriggerWidget)
		m_currentTriggerWidget->setEnabled(enabled);


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
		m_okCancelButton->setIcon(U_STOCK_ICON("messagebox_critical"));
// TODO: show solution dialog
		m_okCancelButton->setText(i18n("Action not available: %0").arg(action->originalText()));
	}
// FIXME: adjust window to its minimum/preferred size
}

void MainWindow::writeConfig() {
	U_DEBUG << "MainWindow::writeConfig()";
#ifdef KS_NATIVE_KDE
	KConfig *config = KGlobal::config().data();

	pluginConfig(false); // write

	config->setGroup("General");
	config->writeEntry("Selected Action", getSelectedAction()->id());
	config->writeEntry("Selected Trigger", getSelectedTrigger()->id());

	config->sync();
#endif // KS_NATIVE_KDE
}

// private slots

#ifdef KS_PURE_QT
void MainWindow::onAbout() {
	QMessageBox::about(
		this,
		i18n("About"),
		"KShutdown " KS_VERSION "\n\n" \
		KS_HOME_PAGE "\n" \
		KS_CONTACT
	);
}
#endif // KS_PURE_QT

void MainWindow::onActionActivated(int index) {
	U_DEBUG << "MainWindow::onActionActivated( " << index << " )";

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
		U_DEBUG << "MainWindow::onCheckTrigger(): INTERNAL ERROR #1";

		return;
	}

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();

	if (trigger->canActivateAction()) {
		setActive(false);
		if (action->isEnabled())
			action->activate(Action::Trigger);
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

void MainWindow::onConfigureAction() {
	U_DEBUG << "MainWindow::onConfigureAction()";
	//!!!
}

void MainWindow::onOKCancel() {
	U_DEBUG << "MainWindow::onOKCancel()";

	setActive(!m_active);
}

void MainWindow::onQuit() {
	U_DEBUG << "MainWindow::onQuit()";

	m_forceQuit = true;
	m_systemTray->hide();
	close();
}

void MainWindow::onRestore(QSystemTrayIcon::ActivationReason reason) {
	U_DEBUG << "MainWindow::onRestore()";

	switch (reason) {
		case QSystemTrayIcon::Trigger:
// TODO: bring to front and show on the current desktop
			show();
			break;
		default:
			break;
			// ignore
	}
}

void MainWindow::onTriggerActivated(int index) {
	U_DEBUG << "MainWindow::onTriggerActivated( " << index << " )";

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
