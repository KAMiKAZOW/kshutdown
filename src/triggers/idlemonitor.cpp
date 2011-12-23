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

#include <QDateTimeEdit>

#include "idlemonitor.h"

#ifdef Q_WS_WIN
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#define _WIN32_WINNT 0x0500 // for LockWorkStation, etc
	#include <windows.h>
#else
	#include <QDBusInterface>
 	#include <QDBusReply>
 	
 	#include "../utils.h"
 	#include "../actions/lock.h"
#endif // Q_WS_WIN

// public

IdleMonitor::IdleMonitor()
	: KShutdown::DateTimeTriggerBase(
		i18n("On User Inactivity (HH:MM)"),
// TODO: better icon - user-idle?
		"user-away-extended",
		"idle-monitor"
	),
	m_idleTime(0)
{
	m_dateTime.setTime(QTime(1, 0, 0)); // set default
	m_checkTimeout = 5000;
	
#ifdef Q_WS_WIN
	m_supported = true;
#else
	m_supported = LockAction::getQDBusInterface()->isValid() && Utils::isKDE_4();
#endif // Q_WS_WIN

	setWhatsThis("<qt>" + i18n("Use this trigger to detect user inactivity (example: no mouse clicks).") + "</qt>");
}

IdleMonitor::~IdleMonitor() { }

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
	m_status = '~' + t.addSecs(remainingTime).toString("HH:mm:ss");
	
	//m_status += (" {DEBUG:" + QString::number(m_idleTime) + "}");

	return false;
}

QWidget *IdleMonitor::getWidget() {
	if (!m_edit) {
		DateTimeTriggerBase::getWidget();

		m_edit->setDisplayFormat(KShutdown::TIME_DISPLAY_FORMAT);
		m_edit->setTime(m_dateTime.time()); // 1.
		m_edit->setMinimumTime(QTime(0, 1)); // 2.
		m_edit->setToolTip(i18n("Enter a maximum user inactivity in \"HH:MM\" format (Hours:Minutes)"));
	}

	return m_edit;
}

void IdleMonitor::setState(const State state) {
	if (state == StartState) {
		m_idleTime = 0;

#ifdef Q_WS_X11
		if (m_supported)
			LockAction::getQDBusInterface()->call("SimulateUserActivity");
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
	m_dateTime = m_edit->dateTime();
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
	BOOL result = ::GetLastInputInfo(&lii);
	if (result) {
		qint32 tickCount = ::GetTickCount();
		qint32 lastTick = lii.dwTime;

		m_idleTime = (tickCount - lastTick) / 1000;
		//U_ERROR_MESSAGE(0, QString::number(m_idleTime));
	}
	else {
		m_idleTime = 0;
	}
#else
	QDBusReply<quint32> reply = LockAction::getQDBusInterface()->call("GetSessionIdleTime");
	
	if (reply.isValid()) {
		U_DEBUG << "org.freedesktop.ScreenSaver: reply=" << reply.value() U_END;

		m_idleTime = reply.value();
	}
	else {
		m_idleTime = 0;
	}
#endif // Q_WS_WIN
}
