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
#include <QPointer>
#include <QTimer>

#ifdef KS_DBUS
	#include <QDBusConnection>
#endif // KS_DBUS

#ifdef KS_PURE_QT
	#include "version.h" // for about()

	#include <QLabel>
#else
	#include <KActionCollection>
	#include <KHelpMenu>
	#include <KNotification>
	#include <KNotifyConfigWidget>
	#include <KShortcutsDialog>
	#include <KStandardAction>
	#include <KStandardGuiItem>
#endif // KS_PURE_QT

#ifdef KS_KF5
	#include <KGlobalAccel>
#endif // KS_KF5

#include "actions/bootentry.h"
#include "actions/extras.h"
#include "actions/lock.h"
#include "actions/test.h"
#include "triggers/idlemonitor.h"
#include "triggers/processmonitor.h"
#include "bookmarks.h"
#include "commandline.h"
#include "infowidget.h"
#include "log.h"
#include "mainwindow.h"
#include "mod.h"
#include "password.h"
#include "preferences.h"
#include "progressbar.h"
#include "stats.h"
#include "usystemtray.h"
#include "utils.h"

MainWindow *MainWindow::m_instance = nullptr;
QHash<QString, Action*> MainWindow::m_actionHash;
QHash<QString, Trigger*> MainWindow::m_triggerHash;
QList<Action*> MainWindow::m_actionList;
QList<Trigger*> MainWindow::m_triggerList;

// public

MainWindow::~MainWindow() {
	//U_DEBUG << "MainWindow::~MainWindow()" U_END;

	foreach (Action *action, m_actionList)
		delete action;
	foreach (Trigger *trigger, m_triggerList)
		delete trigger;

	Config::shutDown();
	Utils::shutDown();
	Log::shutDown();

	if (m_progressBar) {
		delete m_progressBar;
		m_progressBar = nullptr;
	}
}

bool MainWindow::checkCommandLine() {
#if defined(KS_PURE_QT) || defined(KS_KF5)
	if (Utils::isHelpArg()) {
// TODO: get the info directly from QCommandLineParser
		#ifdef KS_KF5
		//Utils::parser()->showHelp(0);
		#endif // KS_KF5

		QString table = "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" width=\"800\">\n";
		
		table += "<tr><td colspan=\"2\"><b>" + i18n("Actions") + "</b></td></tr>\n";
		
		foreach (Action *action, m_actionList) {
			table += "<tr>";

			// args
			
			table += "<td width=\"30%\"><code>";
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

		if (m_triggerHash.contains("idle-monitor")) {
			table += "<tr><td><code>-i, --inactivity</code></td><td>" +
			i18n( // NOTE: copied from main.cpp
			"Detect user inactivity. Example:\n"
			"--logout --inactivity 90 - automatically logout after 90 minutes of user inactivity"
			).replace('\n', "<br />") +
			"</td></tr>\n";
		}

		table += "<tr><td><code>--confirm</code></td><td>" + i18n("Confirm command line action") + "</td></tr>\n";

		table += "<tr><td><code>--hide-ui</code></td><td>" + i18n("Hide main window and system tray icon") + "</td></tr>\n";
		
		table += "<tr><td><code>--init</code></td><td>" + i18n("Do not show main window on startup") + "</td></tr>\n";
		
		table += "<tr><td><code>--mod</code></td><td>" + i18n("A list of modifications") + "</td></tr>\n";

		table += "<tr><td><code>--ui-menu</code></td><td>" + i18n("Show custom popup menu instead of main window") + "</td></tr>\n";
		
		#ifndef KS_PORTABLE
		table += "<tr><td><code>--portable</code></td><td>" + i18n("Run in \"portable\" mode") + "</td></tr>\n";
		#endif // KS_PORTABLE
		
		table += "<tr><td>" + i18n("Optional parameter") + "</td><td>" +
		i18n( // NOTE: copied from main.cpp
		"Activate countdown. Examples:\n"
		"13:37 (HH:MM) or \"1:37 PM\" - absolute time\n"
		"10 or 10m - number of minutes from now\n"
		"2h - two hours"
		).replace('\n', "<br />") +
		"</td></tr>\n";

		// HACK: non-clickable links in Oxygen Style
		table += "<tr><td colspan=\"2\"><a href=\"https://sourceforge.net/p/kshutdown/wiki/Command%20Line/\">https://sourceforge.net/p/kshutdown/wiki/Command%20Line/</a></td></tr>\n";

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
	
	Action *actionToActivate = nullptr;
	bool confirm = Utils::isArg("confirm");
	foreach (Action *action, m_actionList) {
		if (action->isCommandLineArgSupported()) {
			if (confirm && !action->showConfirmationMessage())
				return false;
			
			actionToActivate = action;

			break; // foreach
		}
	}
	if (actionToActivate) {
		QString extrasCommand = QString::null;
	
		if (actionToActivate == Extras::self()) {
			// NOTE: Sync. with extras.cpp (constructor)
			extrasCommand = Utils::getOption("extra");
			if (extrasCommand.isEmpty())
				extrasCommand = Utils::getOption("e");
			Extras::self()->setStringOption(extrasCommand);
		}

		// setup main window and execute action later
		if (TimeOption::isValid()) {
			TimeOption::setAction(actionToActivate);
			
			return false;
		}
		else {
			if (
				TimeOption::isError() &&
				// HACK: fix command line: -e <extrasCommand> <TimeOption::value()>
				(TimeOption::value() != extrasCommand)
			) {
				U_ERROR_MESSAGE(0, i18n("Invalid time: %0").arg(TimeOption::value()));
				
				return false;
			}

			// execute action and quit now
			if (actionToActivate->authorize(nullptr))
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
	bool okHint = (options & DISPLAY_STATUS_OK_HINT) != 0;

	QString actionText = action->originalText();
	QString triggerStatus = trigger->status();
	
	QString s = "";

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
				s.replace(i, 1, "<br>");
		}
		
		if (okHint && m_okCancelButton->isEnabled()) {
			#ifdef KS_NATIVE_KDE
			QString okText = KStandardGuiItem::ok().text();
			okText.remove('&');
			#else
			QString okText = i18n("OK");
			#endif // KS_NATIVE_KDE
			if (!s.isEmpty())
				s += "<br><br>";
			s += i18n("Press %0 to activate KShutdown").arg("<b>" + okText + "</b>");
		}
	}
	// simple - single line, no HTML
	else {
		s += actionText + " - ";

		if (!triggerStatus.isEmpty())
			s += i18n("Remaining time: %0").arg(triggerStatus);
	}
	
	if (html)
		s = "<qt>" + s + "</qt>";

	//U_DEBUG << s U_END;

	return s;
}

void MainWindow::init() {
	Utils::initArgs(); // 1.
	Mod::init(); // 2.

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

	auto *idleMonitor = new IdleMonitor();
	if (idleMonitor->isSupported())
		addTrigger(idleMonitor);
	else
		delete idleMonitor;

	pluginConfig(true); // read
}

bool MainWindow::maybeShow() {
	if (Utils::isArg("hide-ui")) {
		hide();
		m_systemTray->setVisible(false);
		
		return true;
	}

	QString menuLayout = Utils::getOption("ui-menu");
	if (!menuLayout.isEmpty()) {
		QStringList menuActions = menuLayout.split(':');

		auto *menu = new U_MENU();
		#if QT_VERSION < 0x050100
		//connect(menu, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
		#else
		connect(menu, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
		menu->setToolTipsVisible(true);
		#endif // QT_VERSION

		bool confirm = Utils::isArg("confirm");

		foreach (const QString &id, menuActions) {
			if (id == "-") {
				menu->addSeparator();
			}
			else if (id == "cancel") {
				menu->addAction(m_cancelAction);
			}
			else if (id == "quit") {
				menu->addAction(createQuitAction());
			}
			else if (id == "title") {
				Utils::addTitle(menu, U_APP->windowIcon(), "KShutdown");
			}
			else {
				auto *action = m_actionHash[id];
// TODO: show confirmation dialog at cursor position (?)
				if (action) {
					if (confirm)
						menu->addAction(new ConfirmAction(menu, action));
					else
						menu->addAction(action);
				}
				else {
					U_DEBUG << "Unknown ui-menu element: " << id U_END;
				}
			}
		}

// FIXME: this does not work in all configurations (e.g. in Qt4 Build only?)
		menu->exec(QCursor::pos());

		return false;
	}

	if (!m_systemTray->isSupported()) {
		show();

		return true;
	}

	bool trayIconEnabled = Config::systemTrayIconEnabled();
	if (trayIconEnabled)
		m_systemTray->setVisible(true);

	if (Utils::isArg("init") || U_APP->isSessionRestored()) {
		if (!trayIconEnabled)
			showMinimized(); // krazy:exclude=qmethods
	}
	else {
		show();
	}

	return true;
}

void MainWindow::setTime(const QString &selectTrigger, const QTime &time, const bool absolute) {
	setActive(false);

	setSelectedTrigger(selectTrigger);
	Trigger *trigger = getSelectedTrigger();
	auto *dateTimeTrigger = dynamic_cast<DateTimeTriggerBase *>(trigger);
	
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
	QStringList sl;
	foreach (const Action *i, m_actionList) {
		if (showDescription)
			sl.append(i->id() + " - " + i->originalText());
		else
			sl.append(i->id());
	}

	return sl;
}

QStringList MainWindow::triggerList(const bool showDescription) {
	QStringList sl;
	foreach (const Trigger *i, m_triggerList) {
		if (showDescription)
			sl.append(i->id() + " - " + i->text());
		else
			sl.append(i->id());
	}

	return sl;
}

void MainWindow::setActive(const bool yes) {
	if (m_active == yes)
		return;

	U_DEBUG << "MainWindow::setActive( " << yes << " )" U_END;

	if (!yes && !PasswordDialog::authorize(this, i18n("Cancel"), "kshutdown/action/cancel"))
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
	m_systemTray->updateIcon(this);

	//U_DEBUG << "\tMainWindow::getSelectedAction() == " << action->id() U_END;
	//U_DEBUG << "\tMainWindow::getSelectedTrigger() == " << trigger->id() U_END;

	// reset notifications
	m_lastNotificationID = QString::null;

	if (m_active) {
#ifdef Q_OS_WIN32
		// HACK: disable "Lock Screen" action if countdown is active
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(false);
#endif // Q_OS_WIN32
		m_triggerTimer->start(trigger->checkTimeout());
		action->setState(Base::State::Start);
		trigger->setState(Base::State::Start);

		if (trigger->supportsProgressBar() && Config::user()->progressBarEnabled())
			m_progressBar->show();
	}
	else {
#ifdef Q_OS_WIN32
		if (action != LockAction::self())
			m_confirmLockAction->setEnabled(true);
#endif // Q_OS_WIN32
		m_triggerTimer->stop();
		action->setState(Base::State::Stop);
		trigger->setState(Base::State::Stop);
		
		m_progressBar->hide();
	}

	setTitle(QString::null, QString::null);
	onStatusChange(true);
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
	noHTML.remove(QRegExp(R"(\<\w+\>)"));
	noHTML.remove(QRegExp(R"(\</\w+\>)"));
	m_systemTray->warning(noHTML);
#endif // KS_PURE_QT

	// flash taskbar button
	if ((id == "1m") || (id == "5m"))
		U_APP->alert(this, 10000);
}

void MainWindow::setExtrasCommand(const QString &command) {
	setSelectedAction("extras");
	Extras::self()->setStringOption(command);
}

void MainWindow::setSelectedAction(const QString &id) {
	//U_DEBUG << "MainWindow::setSelectedAction( " << id << " )" U_END;

	int index = m_actions->findData(id);
	if (index == -1)
		index = m_actions->findData("test");
	if (index == -1)
		index = 0;
	m_actions->setCurrentIndex(index);
	onActionActivated(index);
}

void MainWindow::setSelectedTrigger(const QString &id) {
	//U_DEBUG << "MainWindow::setSelectedTrigger( " << id << " )" U_END;

	int index = m_triggers->findData(id);
	if (index == -1)
		index = 0;
	m_triggers->setCurrentIndex(index);
	onTriggerActivated(index);
}

void MainWindow::setTime(const QString &trigger, const QString &time) {
	if (!trigger.isEmpty())
		setSelectedTrigger(trigger);
	
	QTime t = TimeOption::parseTime(time);
	bool absolute = (trigger == "date-time");
	setTime(trigger, t, absolute);
}

void MainWindow::setWaitForProcess(const qint64 pid) {
	//U_DEBUG << "MainWindow::setWaitForProcess( " << pid << " )" U_END;

	setActive(false);

	setSelectedTrigger("process-monitor");
	auto *processMonitor = dynamic_cast<ProcessMonitor *>(
		getSelectedTrigger()
	);
	
	if (!processMonitor)
		return;

	processMonitor->setPID(pid);
	
	setActive(true);
}

// protected

void MainWindow::closeEvent(QCloseEvent *e) {
	// normal close
	bool hideInTray = Config::minimizeToSystemTrayIcon() && Config::systemTrayIconEnabled();
	if (!e->spontaneous() || m_forceQuit || Action::totalExit() || !hideInTray) {
		writeConfig();
	
		e->accept();

		// HACK: ?
		if (!hideInTray)
			U_APP->quit();

		return;
	}
	
	e->ignore();

	// no system tray, minimize instead
	if (!m_systemTray->isSupported()) {
		showMinimized(); // krazy:exclude=qmethods
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
	U_MAIN_WINDOW(
		0,
		Qt::Window |
		// disable "Maximize" button; no Qt::WindowMaximizeButtonHint
		Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint
	),
	m_active(false),
	m_forceQuit(false),
	m_ignoreUpdateWidgets(true),
	m_showActiveWarning(true),
	m_showMinimizeInfo(true),
	m_progressBar(nullptr),
	m_lastNotificationID(QString::null),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(nullptr),
	m_currentTriggerWidget(nullptr) {

	//U_DEBUG << "MainWindow::MainWindow()" U_END;

#ifdef KS_NATIVE_KDE
	m_actionCollection = new KActionCollection(this);
#endif // KS_NATIVE_KDE

	// HACK: It seems that the "quit on last window closed"
	// does not work correctly if main window is hidden
	// "in" the system tray..
	U_APP->setQuitOnLastWindowClosed(false);

	setObjectName("main-window");
#ifdef KS_KF5
// FIXME: ? setWindowIcon(U_ICON(":/images/kshutdown.png"));
#endif // KF_KF5
#ifdef KS_PURE_QT
	// HACK: delete this on quit
	setAttribute(Qt::WA_DeleteOnClose, true);
// TODO: wayland <http://blog.martin-graesslin.com/blog/2015/07/porting-qt-applications-to-wayland/>
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
		
		// insert separator like in menu
		if ((id == "reboot") || (id == "suspend") || (id == "logout"))
			m_actions->insertSeparator(m_actions->count());

		//U_DEBUG << "\tMainWindow::addAction( " << action->text() << " ) [ id=" << id << ", index=" << index << " ]" U_END;
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
		
		// insert separator
		if (id == "date-time")
			m_triggers->insertSeparator(m_triggers->count());

		//U_DEBUG << "\tMainWindow::addTrigger( " << trigger->text() << " ) [ id=" << id << ", index=" << index << " ]" U_END;	
	}
	m_triggers->setMaxVisibleItems(m_triggers->count());
	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));
	
	m_systemTray = new USystemTray(this);
	initMenuBar();

	readConfig();

	setTitle(QString::null, QString::null);
	m_ignoreUpdateWidgets = false;
	updateWidgets();
	
	// HACK: avoid insane window stretching
	// (dunno how to use setFixedSize correctly w/o breaking layout)
	QSize hint = sizeHint();
	setMaximumSize(hint.width() * 2, hint.height() * 2);
	
	connect(
		U_APP, SIGNAL(focusChanged(QWidget *, QWidget *)),
		this, SLOT(onFocusChange(QWidget *, QWidget *))
	);

#ifdef KS_DBUS
	QDBusConnection dbus = QDBusConnection::sessionBus();
	#ifdef KS_PURE_QT
// TODO: allow only one application instance
	dbus.registerService("net.sf.kshutdown");
	#endif // KS_PURE_QT
	dbus.registerObject(
		"/kshutdown",
		this,
		QDBusConnection::ExportScriptableSlots
	);
#endif // KS_DBUS
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

U_ACTION *MainWindow::createQuitAction() {
	#ifdef KS_NATIVE_KDE
	auto *quitAction = KStandardAction::quit(this, SLOT(onQuit()), this);
	quitAction->setEnabled(!Utils::isRestricted("action/file_quit"));
	#else
	auto *quitAction = new U_ACTION(this);
	quitAction->setIcon(U_STOCK_ICON("application-exit"));
	quitAction->setShortcut(QKeySequence("Ctrl+Q"));
	connect(quitAction, SIGNAL(triggered()), SLOT(onQuit()));
	#endif // KS_NATIVE_KDE
	// NOTE: Use "Quit KShutdown" instead of "Quit" because
	// it may be too similar to "Turn Off" in some language translations.
	quitAction->setText(i18n("Quit KShutdown"));

	return quitAction;
}

void MainWindow::initFileMenu(U_MENU *fileMenu, const bool addQuitAction) {
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
		#ifdef KS_KF5
		KGlobalAccel::setGlobalShortcut(confirmAction, QList<QKeySequence>());
		#else
		confirmAction->setGlobalShortcut(KShortcut());
		#endif // KS_KF5
// TODO: show global shortcuts: confirmAction->setShortcut(confirmAction->globalShortcut());
#endif // KS_NATIVE_KDE

		fileMenu->addAction(confirmAction);

		if (id == "reboot") {
/* TODO: boot menu
			#ifdef Q_OS_LINUX
			QStringList bootEntryList = BootEntry::getList();
			if (!bootEntryList.isEmpty())
				fileMenu->addMenu(new BootEntryMenu(this));
			#endif // Q_OS_LINUX
*/
			fileMenu->addSeparator();
		}
		else if (id == "suspend") {
			fileMenu->addSeparator();
		}
	}
	fileMenu->addSeparator();
	fileMenu->addAction(m_cancelAction);
	//fileMenu->addSeparator();

	if (addQuitAction)
		fileMenu->addAction(createQuitAction());
}

void MainWindow::initMenuBar() {
	//U_DEBUG << "MainWindow::initMenuBar()" U_END;

	auto *menuBar = new U_MENU_BAR();

	// HACK: Fixes Bookmarks menu and key shortcuts
	//       (Global app menus in Unity are all f*** - it's a fact)
	// BUG: https://bugs.launchpad.net/appmenu-qt5/+bug/1449373
	//      https://bugs.launchpad.net/appmenu-qt5/+bug/1380702
	menuBar->setNativeMenuBar(false);

	// file menu

	U_MENU *fileMenu = new U_MENU(i18n("A&ction"), menuBar);

	connect(fileMenu, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
	#if QT_VERSION >= 0x050100
	fileMenu->setToolTipsVisible(true);
	#endif // QT_VERSION

	Utils::addTitle(fileMenu, /*U_STOCK_ICON("dialog-warning")*/U_ICON(), i18n("No Delay"));
	initFileMenu(fileMenu, true);
	menuBar->addMenu(fileMenu);

	auto *systemTrayFileMenu = new U_MENU(); // need copy

	#if QT_VERSION < 0x050100
	// HACK: wrong tool tip location <https://sourceforge.net/p/kshutdown/feature-requests/21/>
	// connect(systemTrayFileMenu, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
	#else
	connect(systemTrayFileMenu, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
	systemTrayFileMenu->setToolTipsVisible(true);
	#endif // QT_VERSION

	initFileMenu(
		systemTrayFileMenu,
		#ifdef KS_KF5
		!Utils::isKDE()
		#else
		true
		#endif // KS_KF5
	);
	m_systemTray->setContextMenu(systemTrayFileMenu);

	// bookmarks menu

	if (!Mod::getBool("ui-hide-bookmarks-menu"))
		menuBar->addMenu(m_bookmarksMenu);

	// tools menu

	U_MENU *toolsMenu = new U_MENU(i18n("&Tools"), menuBar);

	#ifndef Q_OS_WIN32
	toolsMenu->addAction(i18n("Statistics"), this, SLOT(onStats()));
	toolsMenu->addSeparator();
	#endif // Q_OS_WIN32

#ifdef KS_PURE_QT
	toolsMenu->addAction(i18n("Preferences"), this, SLOT(onPreferences()))
		->setIcon(U_STOCK_ICON("configure"));
#endif // KS_PURE_QT

#ifdef KS_NATIVE_KDE
	U_ACTION *configureShortcutsAction = KStandardAction::keyBindings(this, SLOT(onConfigureShortcuts()), this);
	configureShortcutsAction->setEnabled(!Utils::isRestricted("action/options_configure_keybinding"));
	toolsMenu->addAction(configureShortcutsAction);

	U_ACTION *configureNotificationsAction = KStandardAction::configureNotifications(this, SLOT(onConfigureNotifications()), this);
	configureNotificationsAction->setEnabled(!Utils::isRestricted("action/options_configure_notifications"));
	toolsMenu->addAction(configureNotificationsAction);
	
	toolsMenu->addSeparator();
	
	U_ACTION *preferencesAction = KStandardAction::preferences(this, SLOT(onPreferences()), this);
	preferencesAction->setEnabled(!Utils::isRestricted("action/options_configure"));
	toolsMenu->addAction(preferencesAction);
#endif // KS_NATIVE_KDE

	menuBar->addMenu(toolsMenu);

	// help menu

#ifdef KS_NATIVE_KDE
	Config *config = Config::user();
	config->beginGroup("KDE Action Restrictions");
	// unused
	config->write("action/help_whats_this", false);
	// KDE version is in About dialog too
	config->write("action/help_about_kde", false);
	// no KShutdown Handbook yet
	config->write("action/help_contents", false);
	// mail bug report does not work (known bug)
	config->write("action/help_report_bug", false);
	config->endGroup();
	#ifdef KS_KF5
	KHelpMenu *helpMenu = new KHelpMenu(this);
	menuBar->addMenu(helpMenu->menu());
	#else
	menuBar->addMenu(helpMenu());
	#endif // KS_KF5
#else
	U_MENU *helpMenu = new U_MENU(i18n("&Help"), menuBar);
	helpMenu->addAction(i18n("About"), this, SLOT(onAbout()));
	menuBar->addMenu(helpMenu);
#endif // KS_NATIVE_KDE

	setMenuBar(menuBar);

	if (Mod::getBool("ui-hide-menu-bar"))
		menuBar->hide();
}

void MainWindow::initWidgets() {
	m_progressBar = new ProgressBar();

	m_bookmarksMenu = new BookmarksMenu(this);

	QWidget *mainWidget = new QWidget();
	mainWidget->setObjectName("main-widget");
	auto *mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setMargin(5_px);
	mainLayout->setSpacing(10_px);

	m_actionBox = new QGroupBox(i18n("Select an &action"));
	m_actionBox->setObjectName("action-box");

	auto *actionLayout = new QVBoxLayout(m_actionBox);
	actionLayout->setMargin(5_px);
	actionLayout->setSpacing(10_px);

	m_actions = new U_COMBO_BOX();
	m_actions->setObjectName("actions");
	connect(m_actions, SIGNAL(activated(int)), SLOT(onActionActivated(int)));
	actionLayout->addWidget(m_actions);

	m_force = new QCheckBox(i18n("Do not save session / Force shutdown"));
	m_force->setObjectName("force");
	connect(m_force, SIGNAL(clicked()), SLOT(onForceClick()));
	actionLayout->addWidget(m_force);

	m_triggerBox = new QGroupBox(i18n("Se&lect a time/event"));
	m_triggerBox->setObjectName("trigger-box");

	auto *triggerLayout = new QVBoxLayout(m_triggerBox);
	triggerLayout->setMargin(5_px);
	triggerLayout->setSpacing(10_px);

	m_triggers = new U_COMBO_BOX();
	m_triggers->setObjectName("triggers");
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	triggerLayout->addWidget(m_triggers);
	
	m_infoWidget = new InfoWidget(this);

	m_okCancelButton = new U_PUSH_BUTTON();
	m_okCancelButton->setObjectName("ok-cancel-button");
	
	m_okCancelButton->setDefault(true);
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	// add extra margin around OK/Cancel button
	//m_okCancelButton->setContentsMargins(10_px, 10_px, 10_px, 10_px); NOP
	auto *okCancelWrapper = new QWidget(mainWidget);
	auto *okCancelLayout = new QVBoxLayout(okCancelWrapper);
	okCancelLayout->setMargin(5_px);
	okCancelLayout->addWidget(m_okCancelButton);

	mainLayout->addWidget(m_actionBox);
	mainLayout->addWidget(m_triggerBox);
	mainLayout->addStretch();
	mainLayout->addWidget(m_infoWidget);
	mainLayout->addStretch();
	mainLayout->addWidget(okCancelWrapper);

	setCentralWidget(mainWidget);
	
	m_cancelAction = new U_ACTION(this);
#ifdef KS_NATIVE_KDE
	m_cancelAction->setIcon(KStandardGuiItem::cancel().icon());
	m_actionCollection->addAction("kshutdown/cancel", m_cancelAction);
	#ifdef KS_KF5
	KGlobalAccel::setGlobalShortcut(m_cancelAction, QList<QKeySequence>());
	#else
	m_cancelAction->setGlobalShortcut(KShortcut());
	#endif // KS_KF5
// TODO: show global shortcut: m_cancelAction->setShortcut(m_cancelAction->globalShortcut());
#else
	m_cancelAction->setIcon(U_STOCK_ICON("dialog-cancel"));
#endif // KS_NATIVE_KDE
	connect(m_cancelAction, SIGNAL(triggered()), SLOT(onCancel()));
}

void MainWindow::pluginConfig(const bool read) {
	Config *config = Config::user();

	foreach (Action *i, m_actionList) {
		config->beginGroup("KShutdown Action " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, m_triggerList) {
		config->beginGroup("KShutdown Trigger " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
		config->endGroup();
	}
}

void MainWindow::readConfig() {
	//U_DEBUG << "MainWindow::readConfig()" U_END;

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
#ifdef KS_KF5
	setWindowTitle(plain);
#elif defined(KS_NATIVE_KDE)
	setCaption(plain);
#else
	#if QT_VERSION >= 0x050200
	setWindowTitle(plain);
	// NOTE: the " - KShutdown" suffix is appended automatically
	// if QApplication::applicationDispayName is set
	#else
	if (plain.isEmpty())
		setWindowTitle("KShutdown");
	else
		setWindowTitle(plain + " - KShutdown");
	#endif // QT_VERSION
#endif // KS_NATIVE_KDE
	QString s = html.isEmpty() ? "KShutdown" : html;

	#if defined(Q_OS_WIN32) || defined(Q_OS_HAIKU)
	m_systemTray->setToolTip(plain.isEmpty() ? "KShutdown" : (plain + " - KShutdown"));
	#else
	m_systemTray->setToolTip(s);
	#endif // Q_OS_WIN32

	// HACK: add "PB" prefix
	if (s.startsWith("<qt>"))
		s.insert(4, i18n("Progress Bar") + " - ");

	m_progressBar->setToolTip(s);
}

void MainWindow::updateWidgets() {
	if (m_ignoreUpdateWidgets) {
		//U_DEBUG << "MainWindow::updateWidgets(): IGNORE" U_END;
	
		return;
	}

	//U_DEBUG << "MainWindow::updateWidgets()" U_END;

	bool enabled = !m_active;
	m_actions->setEnabled(enabled);
	if (m_currentActionWidget)
		m_currentActionWidget->setEnabled(enabled);
	m_triggers->setEnabled(enabled);
	if (m_currentTriggerWidget)
		m_currentTriggerWidget->setEnabled(enabled);

	m_bookmarksMenu->setEnabled(enabled);
	m_force->setEnabled(enabled);

	Mod::applyMainWindowColors(this);

#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
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
	#else
	m_okCancelButton->setGuiItem(
		m_active
		? KStandardGuiItem::cancel()
		: KStandardGuiItem::ok()
	);
	#endif // KS_KF5
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
	m_okCancelButton->setToolTip(i18n("Click to activate/cancel the selected action"));

	Action *action = getSelectedAction();
	action->updateMainWindow(this);

	// update "Cancel" action
	if (m_active) {
		m_cancelAction->setEnabled(!Utils::isRestricted("kshutdown/action/cancel"));
		m_cancelAction->setText(i18n("Cancel: %0").arg(action->originalText()));
	}
	else {
		m_cancelAction->setEnabled(false);
		m_cancelAction->setText(i18n("Cancel"));
	}
}

void MainWindow::writeConfig() { // public
	//U_DEBUG << "MainWindow::writeConfig()" U_END;

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
	//U_DEBUG << "MainWindow::onQuit()" U_END;
	
	if (!PasswordDialog::authorize(this, i18n("Quit KShutdown"), "action/file_quit"))
		return;

	m_forceQuit = true;
	m_systemTray->setVisible(false);
	close();
	U_APP->quit();
}

// private slots

#ifdef KS_PURE_QT
void MainWindow::onAbout() {
	auto *iconLabel = new QLabel();
	iconLabel->setAlignment(Qt::AlignCenter);
	iconLabel->setPixmap(U_ICON(":/images/hi64-app-kshutdown.png").pixmap(64, 64));
	// TEST: iconLabel->setPixmap(U_ICON(":/images/hi128-app-kshutdown.png").pixmap(128, 128));

	QString titleText = "KShutdown " KS_FULL_VERSION;
	QString buildText = KS_BUILD;
	if (Config::isPortable())
		buildText += " | Portable";
	auto *titleLabel = new QLabel(
		"<qt>" \
		"<h1 style=\"margin: 0px; white-space: nowrap\">" + titleText + "</h1>" +
		"<b style=\"white-space: nowrap\">" + buildText + "</b>" \
		"</qt>"
	);
	titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	QString releaseNotes =
		"https://kshutdown.sourceforge.io/releases/" KS_FILE_VERSION ".html";
	auto *aboutLabel = new QLabel(
		"<qt>" +
		i18n("A graphical shutdown utility") + "<br />" \
		"<a href=\"" KS_HOME_PAGE "\">kshutdown.sourceforge.io</a><br />" \
		"<br />" \
		"<a href=\"" + releaseNotes + "\">" + i18n("What's New?") + "</a><br />" \
		"</qt>"
	);
	aboutLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	aboutLabel->setOpenExternalLinks(true);

	auto *titleWidget = new QWidget();
	auto *titleLayout = new QHBoxLayout(titleWidget);
	titleLayout->setMargin(0_px);
	titleLayout->setSpacing(20_px);
	titleLayout->addWidget(iconLabel);
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();

	auto *aboutTab = new QWidget();
	auto *aboutLayout = new QVBoxLayout(aboutTab);
	aboutLayout->setMargin(20_px);
	aboutLayout->setSpacing(20_px);
	aboutLayout->addWidget(titleWidget);
	aboutLayout->addWidget(aboutLabel);
	aboutLayout->addStretch();

QString licenseText =
R"(<qt>This program is <b>free software</b>; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but <b>WITHOUT ANY WARRANTY</b>; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
<a href="http://www.gnu.org/licenses/gpl.html">GNU General Public License</a> for more details. (<a href="http://tldrlegal.com/l/gpl2">tl;dr</a>)</qt>)";

	auto *licenseLabel = new QLabel(licenseText.replace("\n", "<br />"));
	licenseLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	licenseLabel->setOpenExternalLinks(true);

	auto *aboutQtButton = new U_PUSH_BUTTON(i18n("About Qt"));
	aboutQtButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
	connect(aboutQtButton, SIGNAL(clicked()), U_APP, SLOT(aboutQt()));

	auto *licenseTab = new QWidget();
	auto *licenseLayout = new QVBoxLayout(licenseTab);
	licenseLayout->setMargin(20_px);
	licenseLayout->setSpacing(20_px);
	licenseLayout->addWidget(new QLabel(KS_COPYRIGHT));
// TODO: https://sourceforge.net/p/kshutdown/wiki/Credits/
	licenseLayout->addWidget(licenseLabel);
	licenseLayout->addStretch();
	licenseLayout->addWidget(aboutQtButton);

	QPointer<UDialog> dialog = new UDialog(this, i18n("About"), true);
	#ifdef Q_OS_WIN32
	dialog->rootLayout()->setMargin(5_px);
	dialog->rootLayout()->setSpacing(5_px);
	#endif // Q_OS_WIN32

	auto *tabs = new U_TAB_WIDGET();
	tabs->addTab(aboutTab, i18n("About"));
	tabs->addTab(licenseTab, i18n("License"));
	dialog->mainLayout()->addWidget(tabs);

	dialog->exec();
	delete dialog;
}
#endif // KS_PURE_QT

void MainWindow::onActionActivated(int index) {
	Q_UNUSED(index)
	//U_DEBUG << "MainWindow::onActionActivated( " << index << " )" U_END;

	if (m_currentActionWidget) {
		m_actionBox->layout()->removeWidget(m_currentActionWidget);
		m_currentActionWidget->hide();
	}

	Action *action = getSelectedAction();
	
	#ifdef Q_OS_WIN32
	QString id = action->id();
	m_force->setVisible((id == "shutdown") || (id == "reboot") || (id == "logout"));
	#else
	m_force->hide();
	#endif // Q_OS_WIN32

	m_currentActionWidget = action->getWidget();
	if (m_currentActionWidget) {
		m_actionBox->layout()->addWidget(m_currentActionWidget);
		m_currentActionWidget->show();
	}

	m_actions->setToolTip(action->statusTip()/*toolTip()*/);

	onStatusChange(true);
}

void MainWindow::onCancel() {
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
		
		// update date/time after resume from suspend, etc.
		onStatusChange(false);
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
	if (!PasswordDialog::authorizeSettings(this))
		return;

	KNotifyConfigWidget::configure(this);
}

void MainWindow::onConfigureShortcuts() {
	if (!PasswordDialog::authorizeSettings(this))
		return;

	KShortcutsDialog dialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, this);
	dialog.addCollection(m_actionCollection);
	dialog.configure();
}
#endif // KS_NATIVE_KDE

void MainWindow::onFocusChange(QWidget *old, QWidget *now) {
	Q_UNUSED(old)

	// update trigger status info on focus gain
	if (now && (now == m_currentTriggerWidget)) {
		//U_DEBUG << "MainWindow::onFocusChange()" U_END;
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

void MainWindow::onMenuHovered(QAction *action) {
	Utils::showMenuToolTip(action);
}

void MainWindow::onOKCancel() {
	//U_DEBUG << "MainWindow::onOKCancel()" U_END;
	
	// show error message if selected date/time is invalid
	if (!m_active) {
		auto *dateTimeTrigger = dynamic_cast<DateTimeTrigger *>(getSelectedTrigger());
		if (
			dateTimeTrigger &&
			(dateTimeTrigger->dateTime() <= QDateTime::currentDateTime())
		) {
			m_infoWidget->setText(
				i18n("Invalid time: %0").arg(dateTimeTrigger->dateTime().toString(DateTimeTriggerBase::longDateTimeFormat())),
				InfoWidget::Type::Error
			);
		
			return;
		}
	}
	
	setActive(!m_active);
}

void MainWindow::onPreferences() {
	//U_DEBUG << "MainWindow::onPreferences()" U_END;
	
	if (!PasswordDialog::authorizeSettings(this))
		return;

	// DOC: http://www.kdedevelopers.org/node/3919
	QPointer<Preferences> dialog = new Preferences(this);
	if ((dialog->exec() == Preferences::Accepted) || Utils::isGTKStyle()) {
		dialog->apply();
		
		m_progressBar->setVisible(m_active && getSelectedTrigger()->supportsProgressBar() && Config::progressBarEnabled());
		m_systemTray->setVisible(Config::systemTrayIconEnabled());
		m_systemTray->updateIcon(this); // update colors
	}
	delete dialog;
}

void MainWindow::onStats() {
	QPointer<Stats> dialog = new Stats(this);
	dialog->exec();
	delete dialog;
}

void MainWindow::onStatusChange(const bool forceUpdateWidgets) {
	//U_DEBUG << "onStatusChange(" << forceUpdateWidgets << ")" U_END;

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();
	
	action->setState(Action::State::InvalidStatus);
	trigger->setState(Trigger::State::InvalidStatus);

	QString displayStatus =
		m_active
		? QString::null
		: getDisplayStatus(DISPLAY_STATUS_HTML | DISPLAY_STATUS_HTML_NO_ACTION | DISPLAY_STATUS_OK_HINT);
	
	InfoWidget::Type type;
	if (action->statusType() != InfoWidget::Type::Info)
		type = action->statusType();
	else if (trigger->statusType() != InfoWidget::Type::Info)
		type = trigger->statusType();
	else
		type = InfoWidget::Type::Info;
	
	if (forceUpdateWidgets) {
		updateWidgets();
		
		// do not override status set in "updateWidgets()"
		if (!displayStatus.isEmpty() && !m_infoWidget->isVisible())
			m_infoWidget->setText(displayStatus, type);
	}
	else {
		m_infoWidget->setText(displayStatus, type);
	}
}

void MainWindow::onTriggerActivated(int index) {
	Q_UNUSED(index)
	//U_DEBUG << "MainWindow::onTriggerActivated( " << index << " )" U_END;

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

	m_triggers->setToolTip(trigger->toolTip());

	onStatusChange(true);
}
