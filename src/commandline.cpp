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
#include "plugins.h"
#include "udialog.h"
#include "utils.h"

#include <QDebug>
#include <QDesktopServices>
#include <QTextEdit>

// private

QCommandLineParser *CLI::m_args = nullptr;
bool TimeOption::m_absolute = false;
bool TimeOption::m_relative = false;
Action *TimeOption::m_action = nullptr;
QString TimeOption::m_option = "";
QTime TimeOption::m_time = QTime();

// public

// CLI

bool CLI::check() {
	if (isArg("help")) {
		showHelp(nullptr);

		return true;
	}

	TimeOption::init();
	
	Action *actionToActivate = nullptr;
	bool confirm = isConfirm();
	for (Action *action : PluginManager::actionList()) {
		if (action->isCommandLineOptionSet()) {
			if (confirm && !action->showConfirmationMessage())
				return false; // user cancel
			
			actionToActivate = action;

			break; // for
		}
	}

	if (actionToActivate != nullptr) {
		if (!actionToActivate->onCommandLineOption())
			return false;

		// setup main window and execute action later
		if (TimeOption::isValid()) {
			TimeOption::setAction(actionToActivate);
			
			return false;
		}
		else {
			if (TimeOption::isError()) {
// TODO: show valid example
				UDialog::error(nullptr, i18n("Invalid time: %0").arg(TimeOption::value()));
				
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

	return option.isEmpty() ? QString() : option;
}

QString CLI::getTimeOption() {
	QStringList pa = m_args->positionalArguments();

	return pa.isEmpty() ? "" : pa.front();
}

void CLI::init(const QString &appDescription) {
	m_args = new QCommandLineParser();
	m_args->setApplicationDescription(appDescription);
	m_args->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
}

void CLI::initOptions() {
// TODO: plain text? options.add(":", ki18n("Other Options:"));

	m_args->addOptions({
		{
			{ "i", "inactivity" },
			i18n(
				"Detect user inactivity. Example:\n"
				"--logout --inactivity 90 - automatically logout after 90 minutes of user inactivity"
			)
		},

		{ "help", i18n("Show this help") },
		#ifdef KS_KF5
		{ "cancel", i18n("Cancel an active action") },
		#endif // KS_KF5
		{ "confirm", i18n("Show confirmation message") },
		{ "confirm-auto", i18n("Show confirmation message only if the \"Confirm Action\" option is enabled") },
		{ "hide-ui", i18n("Hide main window and system tray icon") },
		{ "init", i18n("Do not show main window on startup") },
		{ "mod", i18n("A list of modifications"), "value" },

// TODO: docs
		{ "style", i18n("Widget Style"), "value" },

		#if defined(KS_PURE_QT) && !defined(KS_PORTABLE)
		{ "portable", i18n("Run in \"portable\" mode") },
		#endif

		{ "ui-menu", i18n("Show custom popup menu instead of main window"), "value" },
	});

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

	return isArg("confirm-auto") && Config::confirmActionVar;
}

void CLI::showHelp(QWidget *parent) {
	auto *htmlWidget = new QTextEdit();
	htmlWidget->setReadOnly(true);
	htmlWidget->setWordWrapMode(QTextOption::NoWrap);

	auto palette = htmlWidget->palette();
// TODO: test dark themes
	QColor bg = palette.color(QPalette::Base);
	QColor fg = palette.color(QPalette::Text);
	QColor bg2 = bg.darker(105);

	QString plainTextTrimmed = m_args->helpText()
		.trimmed();

	QString html = "<qt>\n";

	// HACK: on Windows/Qt 5.11 "monospace" font is mapped to some sans-serif... WTF?
	#ifdef Q_OS_WIN32
	QString fontFamily = "Consolas";
	#else
	QString fontFamily = "monospace";
	#endif // Q_OS_WIN32
	html += "<table cellspacing=\"0\" cellpadding=\"1\" style=\"background-color: " + bg.name() + "; color: " + fg.name() + "; font-family: " + fontFamily + "; font-size: large\">\n";

	int rowNum = 0;

	for (QString &rawLine : plainTextTrimmed.split('\n')) {
		QString trimmedLine = rawLine.trimmed();

		html += "<tr>\n";

		int sep = trimmedLine.indexOf("  "/* 2 spaces */);
		if (sep != -1) {
			QString name = trimmedLine.left(sep)
				.trimmed();
			QString desc = trimmedLine.mid(sep)
				.trimmed();

			QString rowStyle = ((rowNum % 2) == 0)
				? "background-color: " + bg2.name()
				: "";
			rowNum++;

// FIXME: bold font weight is ignored in some monospaced fonts #linux
			html += "\t<td style=\"padding-right: 20px; " + rowStyle + "\"><b>" + name.toHtmlEscaped() + "</b></td>\n";
			html += "\t<td style=\"" + rowStyle + "\">" + desc.toHtmlEscaped() + "</td>\n";
		}
		else {
			if (rawLine.startsWith("    "/* 4 spaces; assume wrapped text continuation */)) {
				html += "\t<td></td>\n";
				html += "\t<td>" + trimmedLine.toHtmlEscaped() + "</td>\n";
			}
			else if (trimmedLine.isEmpty()) {
				html += "\t<td colspan=\"2\"><hr /></td>\n";
			}
			else {
				html += "\t<td colspan=\"2\">" + trimmedLine.toHtmlEscaped() + "</td>\n";
			}
		}

		html += "</tr>\n";
	}

	html += "</table>\n";
	html += "</qt>\n";

	htmlWidget->setText(html);

/* DEBUG:
	QTextStream out(stdout);
	out << html;
*/

	QScopedPointer<UDialog> dialog(new UDialog(parent, i18n("Command Line Options"), true));

	auto *closeButton = dialog->buttonBox()->button(QDialogButtonBox::Close);
	closeButton->setDefault(true);

	auto *wikiButton = dialog->buttonBox()->addButton("Wiki", QDialogButtonBox::ActionRole);
	QString wikiURL = "https://sourceforge.net/p/kshutdown/wiki/Command%20Line/";
	wikiButton->setToolTip(wikiURL);
	QObject::connect(wikiButton, &QPushButton::clicked, [wikiURL]() {
		QDesktopServices::openUrl(wikiURL);
	});

	dialog->mainLayout()->addWidget(htmlWidget);
	htmlWidget->setFocus();

// FIXME: is there any easy way to avoid hardcoded dialog sizes in Qt?
	dialog->resize(1000_px, 600_px);
	dialog->exec();
}

// TimeOption

void TimeOption::init() {
	m_absolute = false;
	m_relative = false;
	m_option = CLI::getTimeOption();
	m_time = QTime();
	
	if (m_option.isEmpty())
		return;
	
	qDebug() << "Time option: " << m_option;
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
	//qDebug() << "Absolute: " << m_absolute;
	//qDebug() << "Relative: " << m_relative;
	//qDebug() << "QTime: " << m_time;
	//qDebug() << "QTime.isNull(): " << m_time.isNull();
	//qDebug() << "QTime.isValid(): " << m_time.isValid();
	//qDebug() << "TimeOption::isError(): " << isError();
	//qDebug() << "TimeOption::isValid(): " << isValid();
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
	//qDebug() << "TimeOption::setupMainWindow(): " << m_action->text();
	
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
