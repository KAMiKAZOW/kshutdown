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
	#include <QPainter>

	#include <KActionCollection>
	#include <KIconEffect>
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
#include "commandline.h"
#include "infowidget.h"
#include "mainwindow.h"
#include "password.h"
#include "preferences.h"
#include "progressbar.h"
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

		table += "<tr><td><code>-i, --inactivity</code></td><td>" + i18n("Detect user inactivity. Example: --logout --inactivity 90 - automatically logout after 90 minutes of user inactivity") + "</td></tr>\n";

		table += "<tr><td><code>--confirm</code></td><td>" + i18n("Confirm command line action") + "</td></tr>\n";

		table += "<tr><td><code>--hide-ui</code></td><td>" + i18n("Hide main window and system tray icon") + "</td></tr>\n";
		
		table += "<tr><td><code>--init</code></td><td>" + i18n("Do not show main window on startup") + "</td></tr>\n";
		
		table += "<tr><td>" + i18n("Optional parameter") + "</td><td>" + i18n("Activate countdown. Examples: 13:37 - absolute time (HH:MM), 10 - number of minutes from now") + "</td></tr>\n";
		
		table += "<tr><td colspan=\"2\"><a href=\"http://sourceforge.net/apps/mediawiki/kshutdown/index.php?title=Command_Line\">" + i18n("More Info...") + "</a></td></tr>\n";

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
#ifdef KS_NATIVE_KDE
		if (actionToActivate == Extras::self()) {
			// NOTE: Sync. with extras.cpp (constructor)
			QString command = Utils::getOption("extra");
			if (command.isEmpty())
				command = Utils::getOption("e");
			Extras::self()->setCommand(command);
		}
#endif // KS_NATIVE_KDE

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
	
	bool html = (options & DISPLAY_STATUS_HTML) != 0;
	bool noAction = (options & DISPLAY_STATUS_HTML_NO_ACTION) != 0;

	QString actionText = action->originalText();
	QString triggerStatus = trigger->status();
	
	QString s = "";
	
	if (html)
		s += "<qt>";

	if (html) {
		if (!noAction)
			s += "<b>" + actionText + "</b><br>";

		if (!triggerStatus.isEmpty())
			s += i18n("Remaining time: %0").arg("<b>" + triggerStatus + "</b>");
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
	Utils::init();

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
		m_systemTray->hide();
		
		return;
	}

	if (!U_SYSTEM_TRAY::isSystemTrayAvailable()) {
		show();

		return;
	}
	else {
		m_systemTray->show();
	}

	if (!Utils::isArg("init") && !U_APP->isSessionRestored())
		show();
}

void MainWindow::setTime(const QString &selectTrigger, const QTime &time, const bool absolute) {
	setActive(false);

	setSelectedTrigger(selectTrigger);
	DateTimeTriggerBase *dateTimeTrigger = dynamic_cast<DateTimeTriggerBase *>(
		getSelectedTrigger()
	);
	
	if (!dateTimeTrigger)
		return;

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
	m_active = yes;
	
#ifdef KS_NATIVE_KDE
// TODO: Qt 4.6: http://doc.qt.nokia.com/4.6/qgraphicscolorizeeffect.html
	if (m_active) {
		U_ICON defaultIcon = U_STOCK_ICON("system-shutdown");

		QRect iconSize = m_systemTray->geometry();
		if (iconSize.size().isEmpty()) {
			U_DEBUG << "MainWindow::setActive: empty system tray icon size: " << iconSize U_END;
		}
		else {
			U_DEBUG << "MainWindow::setActive: system tray icon size: " << iconSize U_END;
			int iconW = qBound(24, iconSize.width(), 64);
			int iconH = qBound(24, iconSize.height(), 64);
			QImage i = defaultIcon.pixmap(iconW, iconH).toImage();
			KIconEffect::colorize(i, Qt::yellow, 0.5f);
		
			// show icons of the active action/trigger
			QPainter *p = new QPainter(&i);
			p->setOpacity(0.8);
			// left/bottom
			iconW /= 2;
			iconH /= 2;
			QPixmap actionOverlay = action->icon().pixmap(iconW, iconH);
			p->drawPixmap(0, iconH, actionOverlay);
			// right/bottom
			QPixmap triggerOverlay = trigger->icon().pixmap(iconW, iconH);
			p->drawPixmap(iconW, iconH, triggerOverlay);
			delete p;
		
			m_systemTray->setIcon(QPixmap::fromImage(i));
		}
	}
	else {
		m_systemTray->setIcon(U_STOCK_ICON("system-shutdown"));
	}
#endif // KS_NATIVE_KDE

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
	}
	else {
#ifdef Q_WS_WIN
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(true);
#endif // Q_WS_WIN
		m_triggerTimer->stop();
		action->setState(Base::StopState);
		trigger->setState(Base::StopState);
		
		ProgressBar::freeInstance();
	}

	setTitle(QString::null);
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
	m_systemTray->showMessage(
		"KShutdown",
		noHTML,
		QSystemTrayIcon::Warning,
		4000
	);
#endif // KS_PURE_QT
}

#ifdef KS_NATIVE_KDE
void MainWindow::setExtrasCommand(const QString &command) {
	setSelectedAction("extras");
	Extras::self()->setCommand(command);
}
#endif // KS_NATIVE_KDE

void MainWindow::setSelectedAction(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedAction( " << id << " )" U_END;

	onActionActivated(selectById(m_actions, id));
}

void MainWindow::setSelectedTrigger(const QString &id) {
	U_DEBUG << "MainWindow::setSelectedTrigger( " << id << " )" U_END;

	onTriggerActivated(selectById(m_triggers, id));
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
	if (!e->spontaneous() || m_forceQuit || Action::totalExit()) {
		e->accept();

		return;
	}

	// hide in system tray instead of close
	e->ignore();
	hide();

	if (m_active) {
		if (m_showActiveWarning) {
			m_showActiveWarning = false;
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

// private

MainWindow::MainWindow() :
	U_MAIN_WINDOW(0, Qt::Dialog),
	m_active(false),
	m_forceQuit(false),
	m_ignoreUpdateWidgets(true),
	m_showActiveWarning(true),
	m_showMinimizeInfo(true),
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
	
	initSystemTray();
	initMenuBar();

	readConfig();

	setTitle(QString::null);
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

Action *MainWindow::getSelectedAction() const {
	return m_actionHash[m_actions->itemData(m_actions->currentIndex()).toString()];
}

Trigger *MainWindow::getSelectedTrigger() const {
	return m_triggerHash[m_triggers->itemData(m_triggers->currentIndex()).toString()];
}

// TODO: customizable action/trigger presets

void MainWindow::initMenuBar() {
	U_DEBUG << "MainWindow::initMenuBar()" U_END;

	U_MENU_BAR *menuBar = new U_MENU_BAR();

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("A&ction"), menuBar);

	// "No Delay" warning
	QString warningText = i18n("Select Action (no delay)");
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
	//fileMenu->addSeparator();
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
	fileMenu->addAction(i18n("Quit"), this, SLOT(onQuit()), QKeySequence("Ctrl+Q"))
		->setIcon(U_STOCK_ICON("application-exit"));
#endif // KS_NATIVE_KDE
	menuBar->addMenu(fileMenu);
	m_systemTray->setContextMenu(fileMenu);

	// settings menu

	U_MENU *settingsMenu = new U_MENU(i18n("&Settings"), menuBar);
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
	settingsMenu->addAction(i18n("Preferences"), this, SLOT(onPreferences()));
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

void MainWindow::initSystemTray() {
	U_DEBUG << "MainWindow::initSystemTray()" U_END;

	m_systemTray = new U_SYSTEM_TRAY(this);
#ifdef KS_NATIVE_KDE
// TODO: "KShutdown" caption in System Tray Settings dialog (Entries tab).
// Currently it's lower case "kshutdown".
	m_systemTray->setIcon(U_STOCK_ICON("system-shutdown"));
#endif // KS_NATIVE_KDE
#ifdef KS_PURE_QT
	#ifdef Q_WS_X11
	if (Utils::isKDE_4())
		m_systemTray->setIcon(U_STOCK_ICON("system-shutdown"));
	else
		m_systemTray->setIcon(windowIcon());
	#else
	m_systemTray->setIcon(windowIcon());
	#endif // Q_WS_X11

	connect(
		m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		SLOT(onRestore(QSystemTrayIcon::ActivationReason))
	);
#endif // KS_PURE_QT
}

void MainWindow::initWidgets() {
	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("main-widget");
	QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setMargin(10);
	mainLayout->setSpacing(10);

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

	m_triggerBox = new QGroupBox(i18n("S&elect a time/event"));
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
	if (ProgressBar::isInstance())
		ProgressBar::self()->setToolTip(windowTitle());
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

	m_force->setEnabled(enabled);

	bool canCancel = !Utils::isRestricted("kshutdown/action/cancel");

#ifdef KS_NATIVE_KDE
	m_okCancelButton->setGuiItem(
		m_active
		? KStandardGuiItem::cancel()
		: KStandardGuiItem::ok()
	);
#else
	if (m_active) {
		m_okCancelButton->setIcon(U_STOCK_ICON("dialog-cancel"));
		m_okCancelButton->setText(i18n("Cancel"));
	}
	else {
		m_okCancelButton->setIcon(U_STOCK_ICON("dialog-ok"));
		m_okCancelButton->setText(i18n("OK"));
	}
#endif // KS_NATIVE_KDE

	Action *action = getSelectedAction();
	if (action->isEnabled()) {
#ifdef KS_NATIVE_KDE
		if ((action == Extras::self()) && Extras::self()->command().isEmpty()) {
			m_okCancelButton->setEnabled(false);
			m_infoWidget->setText(
				"<qt>" +
				i18n("Please select an Extras command<br>from the menu above.") +
				"</qt>",
				InfoWidget::WarningType
			);
		}
		else
#endif // KS_NATIVE_KDE
		{
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
		m_infoWidget->setText(
			"<qt>" +
			i18n("Action not available: %0").arg(action->originalText()) + "<br>" +
			action->disableReason() +
			"</qt>",
			InfoWidget::ErrorType
		);
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

void MainWindow::writeConfig() {
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
	m_systemTray->hide();
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
	QMessageBox::about( // krazy:exclude=qclasses
		this,
		i18n("About"),
		"<qt>" \
		"<h1>KShutdown " + version + "</h1>" \
		KS_COPYRIGHT "<br>" \
		"<br>" \
		"<a href=\"" KS_HOME_PAGE "\">" KS_HOME_PAGE "</a><br>" \
		"<a href=\"" KS_CONTACT "\">" KS_CONTACT "</a><br>" \
		"<br>" \
"This program is free software; you can redistribute it and/or modify<br>" \
"it under the terms of the <a href=\"http://www.gnu.org/licenses/\">GNU General Public License</a> as published by<br>" \
"the Free Software Foundation; either version 2 of the License, or<br>" \
"(at your option) any later version.<br>" \
"<br>" \
"This program is distributed in the hope that it will be useful,<br>" \
"but WITHOUT ANY WARRANTY; without even the implied warranty of<br>" \
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>" \
"GNU General Public License for more details." \
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
		setTitle(getDisplayStatus(DISPLAY_STATUS_SIMPLE));
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
	if (dialog->exec() == Preferences::Accepted)
		dialog->apply();
	delete dialog;
}

#ifdef KS_PURE_QT
void MainWindow::onRestore(QSystemTrayIcon::ActivationReason reason) {
	U_DEBUG << "MainWindow::onRestore()" U_END;

	switch (reason) {
		case QSystemTrayIcon::Trigger:
			if (isVisible()) {
				hide();
			}
			else {
				show();
				activateWindow();
			}
			break;
		default:
			break;
			// ignore
	}
}
#endif // KS_PURE_QT

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
