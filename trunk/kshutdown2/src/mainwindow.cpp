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

#ifdef Q_WS_X11
	#include <QDBusConnection>
#endif // Q_WS_X11

#ifdef KS_PURE_QT
	#include <QPointer>
	#include <QWhatsThis>

	#include "version.h" // for about()
#else
	#include <KActionCollection>
	#include <KNotification>
	#include <KNotifyConfigWidget>
	#include <KShortcutsDialog>
	#include <KStandardAction>
#endif // KS_PURE_QT

#include "actions/extras.h"
#include "actions/lock.h"
#include "actions/test.h"
#include "triggers/idlemonitor.h"
#include "triggers/processmonitor.h"
#include "bookmarks.h"
#include "commandline.h"
#include "infowidget.h"
#include "mainwindow.h"
#include "password.h"
#include "preferences.h"
#include "progressbar.h"
#include "usystemtray.h"
#include "utils.h"

MainWindow *MainWindow::m_instance = 0;
QHash<QString, Action*> MainWindow::m_actionHash;
QHash<QString, Trigger*> MainWindow::m_triggerHash;
QList<Action*> MainWindow::m_actionList;
QList<Trigger*> MainWindow::m_triggerList;

// public

MainWindow::~MainWindow() {
	U_DEBUG << "MainWindow::~MainWindow()" U_END;

	foreach (Action *action, m_actionList)
		delete action;
	foreach (Trigger *trigger, m_triggerList)
		delete trigger;

	Config::shutDown();
	Utils::shutDown();
	if (m_progressBar) {
		delete m_progressBar;
		m_progressBar = 0;
	}
}

bool MainWindow::checkCommandLine() {
#ifdef KS_PURE_QT
	if (Utils::isHelpArg()) {
		QString table = "<table border=\"1\" cellpadding=\"5\" cellspacing=\"0\" width=\"800\">\n";
		
		table += "<tr><td colspan=\"2\"><b>" + i18n("Actions") + "</b></td></tr>\n";
		
		foreach (Action *action, m_actionList) {
			table += "<tr>";

			// args
			
			table += "<td width=\"50%\"><code>";
			QString argsCell = "";
			foreach (const QString &arg, action->getCommandLineArgs()) {
				if (!argsCell.isEmpty())
					argsCell += ", ";
				argsCell += ((arg.length() == 1) ? "-" : "--");
				argsCell += arg;
			}
			table += argsCell;
			table += "</code></td>";
			
			// name

			table += "<td>";
			table += action->originalText();
			table += "</td>";

			table += "</tr>\n";
		}
		
		// NOTE: sync. description with main.cpp/main
		
		table += "<tr><td colspan=\"2\"><b>" + i18n("Miscellaneous") + "</b></td></tr>\n";

		if (m_actionHash.contains("idle-monitor"))
			table += "<tr><td><code>-i, --inactivity</code></td><td>" + i18n("Detect user inactivity. Example: --logout --inactivity 90 - automatically logout after 90 minutes of user inactivity") + "</td></tr>\n";

		table += "<tr><td><code>--confirm</code></td><td>" + i18n("Confirm command line action") + "</td></tr>\n";

		table += "<tr><td><code>--hide-ui</code></td><td>" + i18n("Hide main window and system tray icon") + "</td></tr>\n";
		
		table += "<tr><td><code>--init</code></td><td>" + i18n("Do not show main window on startup") + "</td></tr>\n";
		
		table += "<tr><td>" + i18n("Optional parameter") + "</td><td>" + i18n("Activate countdown. Examples: 13:37 - absolute time (HH:MM), 10 - number of minutes from now") + "</td></tr>\n";

		// HACK: non-clickable links in Oxygen Style
		table += "<tr><td colspan=\"2\"><a href=\"http://sourceforge.net/apps/mediawiki/kshutdown/index.php?title=Command_Line\">http://sourceforge.net/apps/mediawiki/kshutdown/index.php?title=Command_Line</a></td></tr>\n";

		table += "</table>";
		
		//U_DEBUG << table U_END;

		QMessageBox::information( // krazy:exclude=qclasses
			0,
			i18n("Command Line Options"),
			"<qt>" +
			table +
			"</qt>"
		);

		return false;
	}
#endif // KS_PURE_QT
	
	TimeOption::init();
	
	Action *actionToActivate = 0;
	bool confirm = Utils::isArg("confirm");
	foreach (Action *action, m_actionList) {
		if (action->isCommandLineArgSupported()) {
			if (confirm && !action->showConfirmationMessage(0))
				return false;
			
			actionToActivate = action;

			break; // foreach
		}
	}
	if (actionToActivate) {
		if (actionToActivate == Extras::self()) {
			// NOTE: Sync. with extras.cpp (constructor)
			QString command = Utils::getOption("extra");
			if (command.isEmpty())
				command = Utils::getOption("e");
			Extras::self()->setCommand(command);
		}

		// setup main window and execute action later
		if (TimeOption::isValid()) {
			TimeOption::setAction(actionToActivate);
			
			return false;
		}
		else {
			if (TimeOption::isError()) {
				U_ERROR_MESSAGE(0, i18n("Invalid time: %0").arg(TimeOption::value()));
				
				return false;
			}
			
			// execute action and quit now
			if (actionToActivate->authorize(0))
				actionToActivate->activate(false);
			
			return true;
		}
	}
	
	return false;
}

QString MainWindow::getDisplayStatus(const int options) {
	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();
	
	bool appName = (options & DISPLAY_STATUS_APP_NAME) != 0;
	bool html = (options & DISPLAY_STATUS_HTML) != 0;
	bool noAction = (options & DISPLAY_STATUS_HTML_NO_ACTION) != 0;

	QString actionText = action->originalText();
	QString triggerStatus = trigger->status();
	
	QString s = "";
	
	if (html)
		s += "<qt>";

	if (html) {
		if (appName)
			s += "KShutdown<br><br>";
	
		if (!noAction)
			s += i18n("Action: %0").arg("<b>" + actionText + "</b>") + "<br>";

		if (!triggerStatus.isEmpty()) {
			s += i18n("Remaining time: %0").arg("<b>" + triggerStatus + "</b>");
			// HACK: wrap long text
			int i = s.indexOf(" (");
			if (i != -1)
				s.insert(i, "<br>");
		}
	}
	// simple - single line, no HTML
	else {
		s += actionText + " - ";

		if (!triggerStatus.isEmpty())
			s += i18n("Remaining time: %0").arg(triggerStatus);
	}
	
	if (html)
		s += "</qt>";

	//U_DEBUG << s U_END;

	return s;
}

void MainWindow::init() {
	Utils::initArgs();

	U_DEBUG << "MainWindow::init(): Actions" U_END;
	m_actionHash = QHash<QString, Action*>();
	m_actionList = QList<Action*>();
	addAction(new ShutDownAction());
	addAction(new RebootAction());
	addAction(new HibernateAction());
	addAction(new SuspendAction());
	addAction(LockAction::self());
	addAction(new LogoutAction());
	addAction(Extras::self());
	addAction(new TestAction());

	U_DEBUG << "MainWindow::init(): Triggers" U_END;
	m_triggerHash = QHash<QString, Trigger*>();
	m_triggerList = QList<Trigger*>();
	addTrigger(new NoDelayTrigger());
	addTrigger(new TimeFromNowTrigger());
	addTrigger(new DateTimeTrigger());
#ifdef KS_TRIGGER_PROCESS_MONITOR
	addTrigger(new ProcessMonitor());
#endif // KS_TRIGGER_PROCESS_MONITOR

	IdleMonitor *idleMonitor = new IdleMonitor();
	if (idleMonitor->isSupported())
		addTrigger(idleMonitor);
	else
		delete idleMonitor;
}

void MainWindow::maybeShow() {
	if (Utils::isArg("hide-ui")) {
		hide();
		m_systemTray->setVisible(false);
		
		return;
	}

	if (!m_systemTray->isSupported()) {
		show();

		return;
	}

	bool trayIconEnabled = Config::systemTrayIconEnabled();
	if (trayIconEnabled)
		m_systemTray->setVisible(true);

	if (Utils::isArg("init") || U_APP->isSessionRestored()) {
		if (!trayIconEnabled)
			showMinimized();
	}
	else {
		show();
	}
}

void MainWindow::setTime(const QString &selectTrigger, const QTime &time, const bool absolute) {
	setActive(false);

	setSelectedTrigger(selectTrigger);
	Trigger *trigger = getSelectedTrigger();
	DateTimeTriggerBase *dateTimeTrigger = dynamic_cast<DateTimeTriggerBase *>(trigger);
	
	// ensure the trigger exists and is valid
	if (!dateTimeTrigger || (trigger->id() != selectTrigger)) {
		U_ERROR_MESSAGE(this, i18n("Unsupported action: %0").arg(selectTrigger));
	
		return;
	}

	QDate date = QDate::currentDate();

	// select next day if time is less than current time
	if (absolute && (time < QTime::currentTime()))
		date = date.addDays(1);
	dateTimeTrigger->setDateTime(QDateTime(date, time));

	setActive(true);
}

// public slots

QStringList MainWindow::actionList(const bool showDescription) {
	if (!showDescription)
		return m_actionHash.keys();

	QStringList sl;
	foreach (const Action *i, m_actionHash)
		sl.append(i->id() + " - " + i->originalText());

	return sl;
}

QStringList MainWindow::triggerList(const bool showDescription) {
	if (!showDescription)
		return m_triggerHash.keys();

	QStringList sl;
	foreach (const Trigger *i, m_triggerHash)
		sl.append(i->id() + " - " + i->text());

	return sl;
}

void MainWindow::setActive(const bool yes) {
	U_DEBUG << "MainWindow::setActive( " << yes << " )" U_END;

	if (m_active == yes)
		return;

	Action *action = getSelectedAction();
	
	if (yes && !action->isEnabled()) {
// TODO: GUI
		U_DEBUG << "MainWindow::setActive: action disabled: " << action->text() << ", " << action->disableReason() U_END;
	
		return;
	}

	if (yes && !action->authorize(this)) {
		return;
	}

	Trigger *trigger = getSelectedTrigger();
	if (yes && !m_triggerHash.contains(trigger->id())) {
// TODO: GUI
		U_DEBUG << "MainWindow::setActive: trigger disabled: " << trigger->text() << ", " << trigger->disableReason() U_END;
	
		return;
	}
	
	m_active = yes;
	m_systemTray->setActive(m_active, action, trigger);

	U_DEBUG << "\tMainWindow::getSelectedAction() == " << action->id() U_END;
	U_DEBUG << "\tMainWindow::getSelectedTrigger() == " << trigger->id() U_END;

	// reset notifications
	m_lastNotificationID = QString::null;

	if (m_active) {
#ifdef Q_WS_WIN
		// HACK: disable "Lock Screen" action if countdown is active
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(false);
#endif // Q_WS_WIN
		m_triggerTimer->start(trigger->checkTimeout());
		action->setState(Base::StartState);
		trigger->setState(Base::StartState);

		if (trigger->supportsProgressBar() && Config::user()->progressBarEnabled())
			m_progressBar->show();
	}
	else {
#ifdef Q_WS_WIN
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(true);
#endif // Q_WS_WIN
		m_triggerTimer->stop();
		action->setState(Base::StopState);
		trigger->setState(Base::StopState);
		
		m_progressBar->hide();
	}

	setTitle(QString::null, QString::null);
	updateWidgets();
}

void MainWindow::notify(const QString &id, const QString &text) {
	// do not display the same notification twice
	if (m_lastNotificationID == id)
		return;

	m_lastNotificationID = id;
	
	QString noHTML = QString(text);
#ifdef KS_NATIVE_KDE
	// HACK: some tags are not supported in Xfce
	if (Utils::isXfce()) {
		noHTML.replace("<br>", "\n");
		noHTML.remove("<qt>");
		noHTML.remove("</qt>");
	}
// TODO: show error messages using notification
	KNotification::event(
		id,
		noHTML,
		QPixmap(),
		this,
		KNotification::CloseOnTimeout
	);
#endif // KS_NATIVE_KDE
#ifdef KS_PURE_QT
	noHTML.replace("<br>", "\n");
	noHTML.remove(QRegExp("\\<\\w+\\>"));
	noHTML.remove(QRegExp("\\</\\w+\\>"));
	m_systemTray->warning(noHTML);
#endif // KS_PURE_QT
}

void MainWindow::setExtrasCommand(const QString &command) {
	setSelectedAction("extras");
	Extras::self()->setCommand(command);
}

void MainWindow::setSelectedAction(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedAction( " << id << " )" U_END;

	int index = m_actions->findData(id);
	if (index == -1)
		index = m_actions->findData("test");
	if (index == -1)
		index = 0;
	m_actions->setCurrentIndex(index);
	onActionActivated(index);
}

void MainWindow::setSelectedTrigger(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedTrigger( " << id << " )" U_END;

	int index = m_triggers->findData(id);
	if (index == -1)
		index = 0;
	m_triggers->setCurrentIndex(index);
	onTriggerActivated(index);
}

void MainWindow::setTime(const QString &trigger, const QString &time) {
	if (!trigger.isEmpty())
		setSelectedTrigger(trigger);
	
	QTime t = QTime::fromString(time, KShutdown::TIME_PARSE_FORMAT);
	bool absolute = (trigger == "date-time") ? true : false;
	setTime(trigger, t, absolute);
}

void MainWindow::setWaitForProcess(const qlonglong pid) {
	U_DEBUG << "MainWindow::setWaitForProcess( " << pid << " )" U_END;

	setActive(false);

	setSelectedTrigger("process-monitor");
	ProcessMonitor *processMonitor = dynamic_cast<ProcessMonitor *>(
		getSelectedTrigger()
	);
	
	if (!processMonitor)
		return;

	processMonitor->setPID(pid);
	
	setActive(true);
}

// protected

void MainWindow::closeEvent(QCloseEvent *e) {
	writeConfig();
	
	// normal close
	bool hideInTray = Config::minimizeToSystemTrayIcon() && Config::systemTrayIconEnabled();
	if (!e->spontaneous() || m_forceQuit || Action::totalExit() || !hideInTray) {
		e->accept();

		// HACK: ?
		if (!hideInTray)
			U_APP->quit();

		return;
	}
	
	e->ignore();

	// no system tray, minimize instead
	if (!m_systemTray->isSupported()) {
		showMinimized();
	}
	// hide in system tray instead of close
	else {
		hide();
	}

	if (m_active) {
		if (m_showActiveWarning) {
			m_showActiveWarning = false;
			m_systemTray->warning(i18n("KShutdown is still active!"));
		}
	}
	else {
		if (m_showMinimizeInfo) {
			m_showMinimizeInfo = false;
			m_systemTray->info(i18n("KShutdown has been minimized"));
		}
	}
}

// private

MainWindow::MainWindow() :
	U_MAIN_WINDOW(0, Qt::Window),
	m_active(false),
	m_forceQuit(false),
	m_ignoreUpdateWidgets(true),
	m_showActiveWarning(true),
	m_showMinimizeInfo(true),
	m_progressBar(0),
	m_lastNotificationID(QString::null),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(0),
	m_currentTriggerWidget(0) {

	U_DEBUG << "MainWindow::MainWindow()" U_END;

#ifdef KS_NATIVE_KDE
	m_actionCollection = new KActionCollection(this);
#endif // KS_NATIVE_KDE

	// HACK: It seems that the "quit on last window closed"
	// does not work correctly if main window is hidden
	// "in" the system tray..
	U_APP->setQuitOnLastWindowClosed(false);

	setObjectName("main-window");
#ifdef KS_PURE_QT
	// HACK: delete this on quit
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowIcon(U_ICON(":/images/kshutdown.png"));
#endif // KS_PURE_QT

	// NOTE: do not change the "init" order,
	// or your computer will explode
	initWidgets();
	
	// init actions
	foreach (Action *action, m_actionList) {
		connect(
			action, SIGNAL(statusChanged(const bool)),
			this, SLOT(onStatusChange(const bool))
		);

		QString id = action->id();
		m_actions->addItem(action->icon(), action->text(), id);
		int index = m_actions->count() - 1;
		
		// insert separator like in menu
		if ((id == "reboot") || (id == "suspend") || (id == "logout"))
			m_actions->insertSeparator(m_actions->count());

		U_DEBUG << "\tMainWindow::addAction( " << action->text() << " ) [ id=" << id << ", index=" << index << " ]" U_END;
	}
	m_actions->setMaxVisibleItems(m_actions->count());

	// init triggers
	foreach (Trigger *trigger, m_triggerList) {
		connect(
			trigger, SIGNAL(notify(const QString &, const QString &)),
			this, SLOT(notify(const QString &, const QString &))
		);
		connect(
			trigger, SIGNAL(statusChanged(const bool)),
			this, SLOT(onStatusChange(const bool))
		);

		QString id = trigger->id();
		m_triggers->addItem(trigger->icon(), trigger->text(), id);
		int index = m_triggers->count() - 1;
		
		// insert separator
		if (id == "date-time")
			m_triggers->insertSeparator(m_triggers->count());

		U_DEBUG << "\tMainWindow::addTrigger( " << trigger->text() << " ) [ id=" << id << ", index=" << index << " ]" U_END;	
	}
	m_triggers->setMaxVisibleItems(m_triggers->count());
	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));
	
	m_systemTray = new USystemTray(this);
	initMenuBar();

	readConfig();

	setTitle(QString::null, QString::null);
	m_ignoreUpdateWidgets = false;
	updateWidgets();
	
	connect(
		U_APP, SIGNAL(focusChanged(QWidget *, QWidget *)),
		this, SLOT(onFocusChange(QWidget *, QWidget *))
	);
	
#ifdef Q_WS_X11
	QDBusConnection dbus = QDBusConnection::sessionBus();
	#ifdef KS_PURE_QT
	dbus.registerService("net.sf.kshutdown");
	#endif // KS_PURE_QT
	dbus.registerObject(
		"/kshutdown",
		this,
		QDBusConnection::ExportScriptableSlots
	);
#endif // Q_WS_X11
}

void MainWindow::addAction(Action *action) {
	m_actionHash[action->id()] = action;
	m_actionList.append(action);
}

void MainWindow::addTrigger(Trigger *trigger) {
	m_triggerHash[trigger->id()] = trigger;
	m_triggerList.append(trigger);
}

Action *MainWindow::getSelectedAction() const { // public
	return m_actionHash[m_actions->itemData(m_actions->currentIndex()).toString()];
}

Trigger *MainWindow::getSelectedTrigger() const { // public
	return m_triggerHash[m_triggers->itemData(m_triggers->currentIndex()).toString()];
}

void MainWindow::initMenuBar() {
	U_DEBUG << "MainWindow::initMenuBar()" U_END;

	U_MENU_BAR *menuBar = new U_MENU_BAR();

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("A&ction"), menuBar);

	// "No Delay" warning
	QString warningText = i18n("No Delay");
#ifdef KS_NATIVE_KDE
	fileMenu->addTitle(U_STOCK_ICON("dialog-warning"), warningText);
#else
	U_ACTION *warningAction = new U_ACTION(menuBar);
	QFont warningActionFont = warningAction->font();
	warningActionFont.setBold(true);
	warningAction->setEnabled(false);
	warningAction->setFont(warningActionFont);
	warningAction->setIcon(U_STOCK_ICON("dialog-warning"));
	warningAction->setText(warningText);
	fileMenu->addAction(warningAction);
#endif // KS_NATIVE_KDE

	Action *a;
	QString id;
	for (int i = 0; i < m_actions->count(); ++i) {
		QVariant data = m_actions->itemData(i);
		
		// skip separator
		if (data.type() == QVariant::Invalid)
			continue; // for
		
		id = data.toString();
		
		a = m_actionHash[id];

		if (!a->showInMenu())
			continue; // for

		ConfirmAction *confirmAction = new ConfirmAction(this, a);
		if (a == LockAction::self())
			m_confirmLockAction = confirmAction;

#ifdef KS_NATIVE_KDE
		m_actionCollection->addAction("kshutdown/" + id, confirmAction);
		confirmAction->setGlobalShortcut(KShortcut());
// TODO: show global shortcuts: confirmAction->setShortcut(confirmAction->globalShortcut());
#endif // KS_NATIVE_KDE

		fileMenu->addAction(confirmAction);
		if ((id == "reboot") || (id == "suspend"))
			fileMenu->addSeparator();
	}
	fileMenu->addSeparator();
	fileMenu->addAction(m_cancelAction);
	//fileMenu->addSeparator();
#ifdef KS_NATIVE_KDE
	U_ACTION *quitAction = KStandardAction::quit(this, SLOT(onQuit()), this);
	quitAction->setEnabled(!Utils::isRestricted("action/file_quit"));
	fileMenu->addAction(quitAction);
#else
	fileMenu->addAction(i18n("Quit"), this, SLOT(onQuit()), QKeySequence("Ctrl+Shift+Q"))
		->setIcon(U_STOCK_ICON("application-exit"));
#endif // KS_NATIVE_KDE
	menuBar->addMenu(fileMenu);
	m_systemTray->setContextMenu(fileMenu);

	// bookmarks menu
	
// TODO: menuBar->addMenu(m_bookmarksMenu);

	// settings menu

	U_MENU *settingsMenu = new U_MENU(
		Utils::isGTKStyle() ? i18n("&Edit") : i18n("&Settings"),
		menuBar
	);
#ifdef KS_NATIVE_KDE
// FIXME: Details -> Shortcut Schemes
	U_ACTION *configureShortcutsAction = KStandardAction::keyBindings(this, SLOT(onConfigureShortcuts()), this);
	configureShortcutsAction->setEnabled(!Utils::isRestricted("action/options_configure_keybinding"));
	settingsMenu->addAction(configureShortcutsAction);

	U_ACTION *configureNotificationsAction = KStandardAction::configureNotifications(this, SLOT(onConfigureNotifications()), this);
	configureNotificationsAction->setEnabled(!Utils::isRestricted("action/options_configure_notifications"));
	settingsMenu->addAction(configureNotificationsAction);
	
	settingsMenu->addSeparator();
	
	U_ACTION *preferencesAction = KStandardAction::preferences(this, SLOT(onPreferences()), this);
	preferencesAction->setEnabled(!Utils::isRestricted("action/options_configure"));
	settingsMenu->addAction(preferencesAction);
#else
	settingsMenu->addAction(i18n("Preferences"), this, SLOT(onPreferences()))
		->setIcon(U_STOCK_ICON("configure"));
#endif // KS_NATIVE_KDE
	menuBar->addMenu(settingsMenu);

	// help menu

#ifdef KS_NATIVE_KDE
	Config *config = Config::user();
	config->beginGroup("KDE Action Restrictions");
	// KDE version is in About dialog too
	config->write("action/help_about_kde", false);
	// no KShutdown Handbook yet
	config->write("action/help_contents", false);
	// mail bug report does not work (known bug)
	config->write("action/help_report_bug", false);
	config->endGroup();
	menuBar->addMenu(helpMenu());
#else
	U_MENU *helpMenu = new U_MENU(i18n("&Help"), menuBar);
	helpMenu->addAction(QWhatsThis::createAction(this));
	helpMenu->addSeparator();
	helpMenu->addAction(i18n("About"), this, SLOT(onAbout()));
	helpMenu->addAction(i18n("About Qt"), U_APP, SLOT(aboutQt()));
	menuBar->addMenu(helpMenu);
#endif // KS_NATIVE_KDE

	setMenuBar(menuBar);
}

void MainWindow::initWidgets() {
	m_progressBar = new ProgressBar();

	m_bookmarksMenu = new BookmarksMenu(this);

	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("main-widget");
	QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(10);

	m_actionBox = new QGroupBox(i18n("Selec&t an action"));
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

	m_triggerBox = new QGroupBox(i18n("Se&lect a time/event"));
	m_triggerBox->setObjectName("trigger-box");
	m_triggerBox->setLayout(new QVBoxLayout());

	m_triggers = new U_COMBO_BOX();
	m_triggers->setObjectName("triggers");
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	m_triggerBox->layout()->addWidget(m_triggers);
	
	m_infoWidget = new InfoWidget(this);

	m_okCancelButton = new U_PUSH_BUTTON();
	m_okCancelButton->setObjectName("ok-cancel-button");
	m_okCancelButton->setDefault(true);
	m_okCancelButton->setToolTip(i18n("Click to activate/cancel the selected action"));
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	mainLayout->addWidget(m_actionBox);
	mainLayout->addWidget(m_triggerBox);
	mainLayout->addStretch();
	mainLayout->addWidget(m_infoWidget);
	mainLayout->addStretch();
	mainLayout->addWidget(m_okCancelButton);

	setCentralWidget(mainWidget);
	
	m_cancelAction = new U_ACTION(this);
#ifdef KS_NATIVE_KDE
	m_cancelAction->setIcon(KStandardGuiItem::cancel().icon());
	m_actionCollection->addAction("kshutdown/cancel", m_cancelAction);
	m_cancelAction->setGlobalShortcut(KShortcut());
// TODO: show global shortcut: m_cancelAction->setShortcut(m_cancelAction->globalShortcut());
#else
	m_cancelAction->setIcon(U_STOCK_ICON("dialog-cancel"));
#endif // KS_NATIVE_KDE
	connect(m_cancelAction, SIGNAL(triggered()), SLOT(onCancel()));
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

#ifdef KS_NATIVE_KDE
	m_actionCollection->readSettings();
#endif // KS_NATIVE_KDE
}

void MainWindow::setTitle(const QString &plain, const QString &html) {
#ifdef KS_NATIVE_KDE
	setCaption(plain);
#else
	if (plain.isEmpty())
		setWindowTitle("KShutdown");
	else
		setWindowTitle(plain + " - KShutdown");
#endif // KS_NATIVE_KDE
	QString s = html.isEmpty() ? "KShutdown" : html;

	#ifdef Q_WS_WIN
	m_systemTray->setToolTip(plain.isEmpty() ? "KShutdown" : (plain + " - KShutdown"));
	#else
	m_systemTray->setToolTip(s);
	#endif // Q_WS_WIN

	m_progressBar->setToolTip(s);
}

void MainWindow::updateWidgets() {
	if (m_ignoreUpdateWidgets) {
		U_DEBUG << "MainWindow::updateWidgets(): IGNORE" U_END;
	
		return;
	}

	U_DEBUG << "MainWindow::updateWidgets()" U_END;

	bool enabled = !m_active;
	m_actions->setEnabled(enabled);
	if (m_currentActionWidget)
		m_currentActionWidget->setEnabled(enabled);
	m_triggers->setEnabled(enabled);
	if (m_currentTriggerWidget)
		m_currentTriggerWidget->setEnabled(enabled);

	m_bookmarksMenu->setEnabled(enabled);
	m_force->setEnabled(enabled);

	bool canCancel = !Utils::isRestricted("kshutdown/action/cancel");

#ifdef KS_NATIVE_KDE
	m_okCancelButton->setGuiItem(
		m_active
		? KStandardGuiItem::cancel()
		: KStandardGuiItem::ok()
	);
#else
	bool hasIcon = m_okCancelButton->style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons);
	if (m_active) {
		if (hasIcon)
			m_okCancelButton->setIcon(U_STOCK_ICON("dialog-cancel"));
		m_okCancelButton->setText(i18n("Cancel"));
	}
	else {
		if (hasIcon)
			m_okCancelButton->setIcon(U_STOCK_ICON("dialog-ok"));
		m_okCancelButton->setText(i18n("OK"));
	}
#endif // KS_NATIVE_KDE

	Action *action = getSelectedAction();
	if (action->isEnabled()) {
		if ((action == Extras::self()) && Extras::self()->command().isEmpty()) {
			m_okCancelButton->setEnabled(false);
			m_infoWidget->setText(
				"<qt>" +
				i18n("Please select an Extras command<br>from the menu above.") +
				"</qt>",
				InfoWidget::WarningType
			);
		}
		else {
			if (m_active)
				m_okCancelButton->setEnabled(canCancel);
			else
				m_okCancelButton->setEnabled(true);
			m_infoWidget->setText(QString::null);
		}
	}
	else {
		m_okCancelButton->setEnabled(false);
// TODO: show solution dialog
		QString s = i18n("Action not available: %0").arg(action->originalText());
		if (!action->disableReason().isEmpty())
			s += ("<br>" + action->disableReason());
		m_infoWidget->setText("<qt>" + s + "</qt>", InfoWidget::ErrorType);
	}
	
	// update "Cancel" action
	if (m_active) {
		m_cancelAction->setEnabled(canCancel);
		m_cancelAction->setText(i18n("Cancel: %0").arg(action->originalText()));
	}
	else {
		m_cancelAction->setEnabled(false);
		m_cancelAction->setText(i18n("Cancel"));
	}
}

void MainWindow::writeConfig() { // public
	U_DEBUG << "MainWindow::writeConfig()" U_END;

	pluginConfig(false); // write

	Config *config = Config::user();
	config->beginGroup("General");
	config->write("Selected Action", getSelectedAction()->id());
	config->write("Selected Trigger", getSelectedTrigger()->id());
	config->endGroup();

#ifdef KS_NATIVE_KDE
	m_actionCollection->writeSettings();
#endif // KS_NATIVE_KDE

	config->sync();
}

// public slots

void MainWindow::onQuit() {
	U_DEBUG << "MainWindow::onQuit()" U_END;
	
	if (!PasswordDialog::authorize(this, i18n("Quit"), "action/file_quit"))
		return;

	m_forceQuit = true;
	m_systemTray->setVisible(false);
	close();
	U_APP->quit();
}

// private slots

#ifdef KS_PURE_QT
void MainWindow::onAbout() {
	QString version = KS_FULL_VERSION;
#ifdef KS_PORTABLE
	version += " (portable)";
#endif // KS_PORTABLE

QString license =
"This program is free software; you can redistribute it and/or modify<br>" \
"it under the terms of the GNU General Public License as published by<br>" \
"the Free Software Foundation; either version 2 of the License, or<br>" \
"(at your option) any later version.<br>" \
"<br>" \
"This program is distributed in the hope that it will be useful,<br>" \
"but WITHOUT ANY WARRANTY; without even the implied warranty of<br>" \
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>" \
"GNU General Public License for more details.<br>" \
"&lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;";
license = license.replace(' ', "&nbsp;"); // no wrap

	QMessageBox::about( // krazy:exclude=qclasses
		this,
		i18n("About"),
		"<qt>" \
		"<h1>KShutdown " + version + "</h1>" +
		i18n("An advanced shutdown utility") + "<br>" \
		"<br>" \
		KS_COPYRIGHT "<br>" \
		"<br>" \
		"<a href=\"" KS_HOME_PAGE "\">" KS_HOME_PAGE "</a><br>" \
		"<br>" +
		license +
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

	Action *action = getSelectedAction();
	m_currentActionWidget = action->getWidget();
	if (m_currentActionWidget) {
		m_actionBox->layout()->addWidget(m_currentActionWidget);
		m_currentActionWidget->show();
	}
	
	m_actions->setWhatsThis(action->whatsThis());

	updateWidgets();
}

void MainWindow::onCancel() {
	if (!PasswordDialog::authorize(this, i18n("Cancel"), "kshutdown/action/cancel"))//!!!
		return;

	setActive(false);
}

void MainWindow::onCheckTrigger() {
	if (!m_active) {
		U_DEBUG << "MainWindow::onCheckTrigger(): INTERNAL ERROR"  U_END;

		return;
	}

	// activate action
	Trigger *trigger = getSelectedTrigger();
	if (trigger->canActivateAction()) {
		setActive(false);
		Action *action = getSelectedAction();
		if (action->isEnabled()) {
			U_DEBUG << "Activate action: force=" << m_force->isChecked() U_END;
			action->activate(m_force->isChecked());
		}
	}
	// update status
	else {
		QString html = getDisplayStatus(DISPLAY_STATUS_HTML | DISPLAY_STATUS_APP_NAME);
		QString simple = getDisplayStatus(DISPLAY_STATUS_SIMPLE);
		setTitle(simple, html);
	}
}

#ifdef KS_NATIVE_KDE
void MainWindow::onConfigureNotifications() {
	if (!PasswordDialog::authorize(this, i18n("Preferences"), "action/options_configure_notifications"))
		return;

	KNotifyConfigWidget::configure(this);
}

void MainWindow::onConfigureShortcuts() {
	if (!PasswordDialog::authorize(this, i18n("Preferences"), "action/options_configure_keybinding"))
		return;

	KShortcutsDialog dialog;
	dialog.addCollection(m_actionCollection);
	dialog.configure();
}
#endif // KS_NATIVE_KDE

void MainWindow::onFocusChange(QWidget *old, QWidget *now) {
	Q_UNUSED(old)

	// update trigger status info on focus gain
	if (now && (now == m_currentTriggerWidget)) {
		U_DEBUG << "MainWindow::onFocusChange()" U_END;
		onStatusChange(false);
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

	if (m_active && !PasswordDialog::authorize(this, i18n("Cancel"), "kshutdown/action/cancel"))//!!!cli
		return;
	
	setActive(!m_active);
}

void MainWindow::onPreferences() {
	U_DEBUG << "MainWindow::onPreferences()" U_END;
	
	if (!PasswordDialog::authorize(this, i18n("Preferences"), "action/options_configure"))
		return;

	// DOC: http://www.kdedevelopers.org/node/3919
	QPointer<Preferences> dialog = new Preferences(this);
	if ((dialog->exec() == Preferences::Accepted) || Utils::isGTKStyle()) {
		dialog->apply();
		
		m_progressBar->setVisible(m_active && getSelectedTrigger()->supportsProgressBar() && Config::progressBarEnabled());
		m_systemTray->setVisible(Config::systemTrayIconEnabled());
		// update icon colors
		if (!m_active)
			m_systemTray->updateIcon();
	}
	delete dialog;
}

void MainWindow::onStatusChange(const bool aUpdateWidgets) {
	U_DEBUG << "onStatusChange(" << aUpdateWidgets << ")" U_END;
	
	QString displayStatus = getDisplayStatus(DISPLAY_STATUS_HTML | DISPLAY_STATUS_HTML_NO_ACTION);
	
	if (aUpdateWidgets) {
		updateWidgets();
		
		// do not override status set in "updateWidgets()"
		if (!displayStatus.isEmpty() && !m_infoWidget->isVisible())
			m_infoWidget->setText(displayStatus);
	}
	else {
		m_infoWidget->setText(displayStatus);
	}
}

void MainWindow::onTriggerActivated(int index) {
	U_DEBUG << "MainWindow::onTriggerActivated( " << index << " )" U_END;

	if (m_currentTriggerWidget) {
		m_triggerBox->layout()->removeWidget(m_currentTriggerWidget);
		m_currentTriggerWidget->hide();
	}

	Trigger *trigger = getSelectedTrigger();
	m_currentTriggerWidget = trigger->getWidget();
	if (m_currentTriggerWidget) {
		static_cast<QVBoxLayout *>(m_triggerBox->layout())->insertWidget(1, m_currentTriggerWidget);
		m_currentTriggerWidget->show();
	}
	
	m_triggers->setWhatsThis(trigger->whatsThis());
	
	onStatusChange(true);
}
