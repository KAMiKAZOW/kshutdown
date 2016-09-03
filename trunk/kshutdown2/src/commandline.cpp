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

#include "pureqt.h"

#include "commandline.h"
#include "mainwindow.h"
#include "utils.h"

// private

bool TimeOption::m_absolute = false;
bool TimeOption::m_relative = false;
KShutdown::Action *TimeOption::m_action = nullptr;
QString TimeOption::m_option = QString::null;
QTime TimeOption::m_time = QTime();

// public

void TimeOption::init() {
	m_absolute = false;
	m_relative = false;
	m_option = Utils::getTimeOption();
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
	if (Utils::isArg("inactivity") || Utils::isArg("i")) {
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
