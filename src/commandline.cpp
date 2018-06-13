// commandline.cpp - Command Line
// Copyright (C) 2009  Konrad Twardowski
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

#include "commandline.h"

#include "config.h"
#include "mainwindow.h"
#include "actions/extras.h"

// private

QCommandLineParser *CLI::m_args = nullptr;
bool TimeOption::m_absolute = false;
bool TimeOption::m_relative = false;
KShutdown::Action *TimeOption::m_action = nullptr;
QString TimeOption::m_option = QString::null;
QTime TimeOption::m_time = QTime();

// public

// CLI

bool CLI::check() {
	if (
		isArg("help")
// TODO: win32: "/?"
	) {
		QString moreInfo =
			"\n" +
			i18n("More Info...") + "\n" +
			"https://sourceforge.net/p/kshutdown/wiki/Command%20Line/";

		#ifdef KS_UNIX
		QTextStream out(stdout);
		out << m_args->helpText();
		out << moreInfo << endl;

		return true;
		#else
		QString html = m_args->helpText()
			.toHtmlEscaped();

// FIXME: clickable link opens two invalid browser tabs
// (https://sourceforge.net/auth/?return_to=%2Fp%2Fkshutdown%2Fwiki%2FCommand and http://www.line.com/)
// because for some reason "%20" is interpreted as " " :/
// TODO: rename wiki page to fix the above problem ;)
		//html += moreInfo;
		//html += "\n<a href=\"https://sourceforge.net/p/kshutdown/wiki/Command%20Line/\">https://sourceforge.net/p/kshutdown/wiki/Command%20Line/</a>";

		qDebug() << "HTML:" << html;

// FIXME: need scroll pane (?)
		QMessageBox::information( // krazy:exclude=qclasses
			nullptr,
			i18n("Command Line Options"),
			"<qt><pre>" +
			html +
			"</pre></qt>"
		);

		return false;
		#endif // KS_UNIX
	}

	TimeOption::init();
	
	Action *actionToActivate = nullptr;
	bool confirm = isConfirm();
	foreach (Action *action, MainWindow::actionList()) {
		if (action->isCommandLineArgSupported()) {
			if (confirm && !action->showConfirmationMessage())
				return false; // user cancel
			
			actionToActivate = action;

			break; // foreach
		}
	}

	if (actionToActivate) {
		if (actionToActivate == Extras::self()) {
// TODO: move to KShutdown::Action::onCommandLine #api
			Extras::self()->setStringOption(getOption("extra"));
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
			if (actionToActivate->authorize(nullptr))
				actionToActivate->activate(false);

			return true;
		}
	}
	
	return false;
}

QString CLI::getOption(const QString &name) {
	qDebug() << "CLI::getOption:" << name;

	QString option = m_args->value(name);

	return option.isEmpty() ? QString::null : option;
}

QString CLI::getTimeOption() {
	QStringList pa = m_args->positionalArguments();

	if (pa.count())
		return pa.front();
	
	return QString::null;
}

void CLI::init(const QString &appDescription) {
	m_args = new QCommandLineParser();
	m_args->setApplicationDescription(appDescription);
	m_args->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

// TODO: Ideally this should be in KShutdown::Action to avoid code duplication.
// Unfortunatelly due to chicked-egg circular deps it's not possible (?)
	m_args->addOption({ { "h", "halt" }, i18n("Turn Off Computer") });
	m_args->addOption({ { "s", "shutdown" }, i18n("Turn Off Computer") });
	m_args->addOption({ { "r", "reboot" }, i18n("Restart Computer") });

	m_args->addOption({ { "H", "hibernate" }, i18n("Hibernate Computer") });
	m_args->addOption({ { "S", "suspend" }, i18n("Suspend Computer") });

	m_args->addOption({ { "k", "lock" }, i18n("Lock Screen") });
	m_args->addOption({ { "l", "logout" }, i18n("Logout") });

	m_args->addOption({
		{ "e", "extra" },
		i18n("Run executable file (example: Desktop shortcut or Shell script)"),
		"file"
	});
	
	m_args->addOption({ "test", i18n("Test Action (does nothing)") });

	m_args->addOption({
		{ "i", "inactivity" },
		i18n(
			"Detect user inactivity. Example:\n"
			"--logout --inactivity 90 - automatically logout after 90 minutes of user inactivity"
		)
	});

// TODO: plain text? options.add(":", ki18n("Other Options:"));

	m_args->addOption({ "help", i18n("Show this help") });
	m_args->addOption({ "cancel", i18n("Cancel an active action") });
	m_args->addOption({ "confirm", i18n("Show confirmation message") });
	m_args->addOption({ "confirm-auto", i18n("Show confirmation message only if the \"Confirm Action\" option is enabled") });
	m_args->addOption({ "hide-ui", i18n("Hide main window and system tray icon") });
	m_args->addOption({ "init", i18n("Do not show main window on startup") });
	m_args->addOption({ "mod", i18n("A list of modifications"), "value" });

// TODO: docs
	m_args->addOption({ "style", i18n("Widget Style"), "value" });

	#if defined(KS_PURE_QT) && !defined(KS_PORTABLE)
	m_args->addOption({ "portable", i18n("Run in \"portable\" mode") });
	#endif

	m_args->addOption({ "ui-menu", i18n("Show custom popup menu instead of main window"), "value" });
	m_args->addPositionalArgument(
		"time",
		i18n(
			"Activate countdown. Examples:\n"
			"13:37 (HH:MM) or \"1:37 PM\" - absolute time\n"
			"10 or 10m - number of minutes from now\n"
			"2h - two hours"
		)
	);
}

bool CLI::isArg(const QString &name) {
	//qDebug() << "CLI::isArg:" << name;

	return m_args->isSet(name);
}

bool CLI::isConfirm() {
	if (isArg("confirm"))
		return true;

	return isArg("confirm-auto") && Config::confirmAction();
}

// TimeOption

void TimeOption::init() {
	m_absolute = false;
	m_relative = false;
	m_option = CLI::getTimeOption();
	m_time = QTime();
	
	if (m_option.isEmpty())
		return;
	
	U_DEBUG << "Time option: " << m_option U_END;
	if ((m_option == "0") || (m_option.compare("NOW", Qt::CaseInsensitive) == 0)) {
		m_time = QTime(0, 0);
		m_relative = true;
	}
	else if (m_option.count(":") == 1) {
		m_time = parseTime(m_option);
		if (m_time.isValid())
			m_absolute = true;
	}
	else {
		bool ok;
		int minutes = 0;
		int size = m_option.size();

		if ((size > 1) && m_option.endsWith('H', Qt::CaseInsensitive)) {
			int hours = m_option.mid(0, size - 1).toInt(&ok);
			if (ok) {
				minutes = hours * 60;
				if (hours == 24)
					minutes--;
			}
		}
		else if ((size > 1) && m_option.endsWith('M', Qt::CaseInsensitive)) {
			minutes = m_option.mid(0, size - 1).toInt(&ok);
		}
		else {
			minutes = m_option.toInt(&ok);
		}

		if (ok && (minutes > 0) && (minutes < 60 * 24)) {
			m_time = QTime(0, 0).addSecs(minutes * 60);
			m_relative = true;
		}
	}
	//U_DEBUG << "Absolute: " << m_absolute U_END;
	//U_DEBUG << "Relative: " << m_relative U_END;
	//U_DEBUG << "QTime: " << m_time U_END;
	//U_DEBUG << "QTime.isNull(): " << m_time.isNull() U_END;
	//U_DEBUG << "QTime.isValid(): " << m_time.isValid() U_END;
	//U_DEBUG << "TimeOption::isError(): " << isError() U_END;
	//U_DEBUG << "TimeOption::isValid(): " << isValid() U_END;
}

bool TimeOption::isError() {
	return !isValid() && !m_option.isEmpty();
}

bool TimeOption::isValid() {
	return m_time.isValid() && (m_absolute || m_relative);
}

QTime TimeOption::parseTime(const QString &time) {
	QTime result = QTime::fromString(time, KShutdown::TIME_PARSE_FORMAT);

	// try alternate AM/PM format
	if (!result.isValid())
		result = QTime::fromString(time, "h:mm AP");

	return result;
}

void TimeOption::setupMainWindow() {
	//U_DEBUG << "TimeOption::setupMainWindow(): " << m_action->text() U_END;
	
	MainWindow *mainWindow = MainWindow::self();
	mainWindow->setActive(false);
	
	mainWindow->setSelectedAction(m_action->id());
	
	QString trigger;
	if (CLI::isArg("inactivity")) {
		// set error mode
		if (!m_relative) {
			m_absolute = false;
			
			return;
		}
			
		trigger = "idle-monitor";
	}
	else {
		trigger = m_absolute ? "date-time" : "time-from-now";
	}

	mainWindow->setTime(trigger, m_time, m_absolute);
}
