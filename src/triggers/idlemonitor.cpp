// idlemonitor.cpp - An inactivity monitor
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

#ifdef Q_WS_WIN
	// !!!
#else
	#include <QDBusInterface>
 	#include <QDBusReply>
#endif // Q_WS_WIN

#include <QDateTimeEdit>

#include "idlemonitor.h"

// public

IdleMonitor::IdleMonitor()
	: KShutdown::DateTimeTriggerBase(
		i18n("On User Inactivity (HH:MM)") + " [Experimental]",
		"user-away-extended",//!!!user-idle?
		"idle-monitor"
	),
	m_idleTime(0)
{
	//!!!m_checkTimeout = 5000;
	
	m_checkTimeout = 1000;
	
#ifdef Q_WS_WIN
	m_supported = true;
#else
	m_dbus = new QDBusInterface(
		"org.freedesktop.ScreenSaver",
		"/ScreenSaver",
		"org.freedesktop.ScreenSaver"
	);
	m_supported = m_dbus->isValid();
#endif // Q_WS_WIN

	setWhatsThis(i18n("Use this trigger to detect user inactivity (no mouse or keyboard input)."));
}

IdleMonitor::~IdleMonitor() {
	if (m_dbus) {
		delete m_dbus;
		m_dbus = 0;
	}
}

bool IdleMonitor::canActivateAction() {
	getSessionIdleTime();
	
	if (m_idleTime == 0) {
		m_status = i18n("Unknown");
		
		return false;
	}

	QTime time = m_dateTime.time();
	quint32 timeout = ((time.hour() * 60) + time.minute()) * 60;
	U_DEBUG << "timeout=" << timeout U_END;
	
	if (m_idleTime >= timeout)
		return true;

	quint32 remainingTime = timeout - m_idleTime;
	QTime t = QTime();
	m_status = "~" + t.addSecs(remainingTime).toString("HH:mm:ss");
	
	m_status += (" {DEBUG:" + QString::number(m_idleTime) + "}");

	return false;
}

QWidget *IdleMonitor::getWidget() {
	if (!m_edit) {
		DateTimeTriggerBase::getWidget();

		m_edit->setDisplayFormat(KShutdown::TIME_FORMAT);
		m_edit->setTime(m_dateTime.time());
		m_edit->setToolTip(i18n("Enter a maximum user inactivity in \"HH:MM\" format (Hour:Minute)"));
	}

	return m_edit;
}

void IdleMonitor::setState(const State state) {
	Q_UNUSED(state)
	
	if (state == StartState) {
		m_idleTime = 0;

#ifdef Q_WS_X11
		if (m_supported)
			m_dbus->call("SimulateUserActivity");//!!!
#endif // Q_WS_X11
	}
	else if (state == StopState) {
		m_idleTime = 0;
	}
}

// protected

/**
 * The returned value is unused in this trigger;
 * just return something...
 */
QDateTime IdleMonitor::calcEndTime() {
	return m_edit->dateTime();
}

/**
 * Sets @c null status.
 */
void IdleMonitor::updateStatus() {
	m_status = QString::null;
}

// private

/**
 * Sets the @c m_idleTime to the current session idle time (in seconds).
 * Sets @c zero if this information is unavailable.
 */
void IdleMonitor::getSessionIdleTime() {
	if (!m_supported) {
		m_idleTime = 0;
		
		return;
	}

#ifdef Q_WS_WIN
	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);
	BOOL result = ::GetLastInputInfo();
	if (result) {
		qulong64 tickCount = ::GetTickCount();
		qulong64 lastTick = lii.dwTime;

		// HACK: avoid rollback <http://en.wikipedia.org/wiki/GetTickCount#Rollback>
		m_idleTime = ((tickCount - lastTick) & 0xFFFFFFFF) / 1000;
	}
	else {
		m_idleTime = 0;
	}
#else
	QDBusReply<quint32> reply = m_dbus->call("GetSessionIdleTime");
	
	if (reply.isValid()) {
		U_DEBUG << "org.freedesktop.ScreenSaver: reply=" << reply.value() U_END;

		m_idleTime = reply.value();
	}
	else {
		m_idleTime = 0;
	}
#endif // Q_WS_WIN
}
