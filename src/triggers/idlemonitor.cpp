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

#include "idlemonitor.h"

#include "../mainwindow.h"
#include "../progressbar.h"
#include "../utils.h"

#ifdef Q_OS_WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include "../actions/lock.h"
#endif // Q_OS_WIN32

#ifdef KS_KF5
	#include <KIdleTime>
#else
	#include <QDebug>
#endif // KS_KF5

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
	setCanBookmark(true);

	m_dateTime.setTime(QTime(1, 0, 0)); // set default
	m_checkTimeout = 5000;
	m_supportsProgressBar = true;

	#if defined(KS_KF5) || defined(Q_OS_WIN32)
	m_supported = true;
	#elif defined(Q_OS_HAIKU)
	m_supported = false;
	#elif defined(QT_DBUS_LIB)
// FIXME: returns invalid time on KDE (known bug)
	m_supported = LockAction::getQDBusInterface()->isValid() && !Utils::isKDE();

	// HACK: Check if it's actually implemented... (GNOME Shell)
	if (m_supported) {
		QDBusReply<quint32> reply = LockAction::getQDBusInterface()->call("GetSessionIdleTime");
		if (!reply.isValid()) {
			qDebug() << "GetSessionIdleTime not implemented";
			m_supported = false;
		}
	}
	#else
	m_supported = false;
	#endif

	setToolTip(i18n("Use this trigger to detect user inactivity\n(example: no mouse clicks)."));
}

IdleMonitor::~IdleMonitor() = default;

bool IdleMonitor::canActivateAction() {
	getSessionIdleTime();
	
	if (m_idleTime == 0) {
		m_status = i18n("Unknown");
		
		return false;
	}

	quint32 maximumIdleTime = getMaximumIdleTime();
	//qDebug() << "maximumIdleTime=" << maximumIdleTime;
	
	if (m_idleTime >= maximumIdleTime)
		return true;

	quint32 remainingTime = maximumIdleTime - m_idleTime;
	QTime time = QTime(0, 0);
	m_status = '~' + time.addSecs(remainingTime).toString("HH:mm:ss");

	MainWindow *mainWindow = MainWindow::self();
	mainWindow->progressBar()->setValue(remainingTime);

	//m_status += (" {DEBUG:" + QString::number(m_idleTime) + "}");

	return false;
}

QString IdleMonitor::getStringOption() {
	if (!m_edit)
		return QString();

	return m_edit->time().toString(KShutdown::TIME_PARSE_FORMAT);
}

void IdleMonitor::setStringOption(const QString &option) {
	if (!m_edit)
		return;

	QTime time = QTime::fromString(option, KShutdown::TIME_PARSE_FORMAT);
	m_edit->setTime(time);
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
	if (state == State::Start) {
		m_idleTime = 0;

		ProgressBar *progressBar = MainWindow::self()->progressBar();
		progressBar->setTotal(getMaximumIdleTime());
		progressBar->setValue(-1);

#ifdef KS_KF5
		KIdleTime::instance()->simulateUserActivity();
#else
		#ifdef QT_DBUS_LIB
		if (m_supported)
			LockAction::getQDBusInterface()->call("SimulateUserActivity");
		#endif // QT_DBUS_LIB
#endif // KS_KF5
	}
	else if (state == State::Stop) {
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
	m_status = QString();
}

// private

quint32 IdleMonitor::getMaximumIdleTime() {
	QTime time = m_dateTime.time();

	return ((time.hour() * 60) + time.minute()) * 60;
}

/**
 * Sets the @c m_idleTime to the current session idle time (in seconds).
 * Sets @c zero if this information is unavailable.
 */
void IdleMonitor::getSessionIdleTime() {
	if (!m_supported) {
		m_idleTime = 0;
		
		return;
	}

#ifdef Q_OS_WIN32
	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);
	BOOL result = ::GetLastInputInfo(&lii);
	if (result) {
		qint32 tickCount = ::GetTickCount();
		qint32 lastTick = lii.dwTime;

		m_idleTime = (tickCount - lastTick) / 1000;
		//UDialog::error(0, QString::number(m_idleTime));
	}
	else {
		m_idleTime = 0;
	}
#elif defined(KS_KF5)
	m_idleTime = KIdleTime::instance()->idleTime() / 1000;
#elif defined(Q_OS_HAIKU)
	m_idleTime = 0;
#elif defined(QT_DBUS_LIB)
	QDBusReply<quint32> reply = LockAction::getQDBusInterface()->call("GetSessionIdleTime");
	
	if (reply.isValid()) {
		//qDebug() << "org.freedesktop.ScreenSaver: reply=" << reply.value();

		m_idleTime = reply.value();
	}
	else {
		//qDebug() << reply.error();
		m_idleTime = 0;
	}
#else
	m_idleTime = 0;
#endif // Q_OS_WIN32
}
