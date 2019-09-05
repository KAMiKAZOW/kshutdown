// usystemtray.h - A system tray and notification area
// Copyright (C) 2012  Konrad Twardowski
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

#ifndef KSHUTDOWN_USYSTEMTRAY_H
#define KSHUTDOWN_USYSTEMTRAY_H

#ifdef KS_KF5
	#include <KStatusNotifierItem>
#else
	#include <QSystemTrayIcon>
#endif // KS_KF5

class MainWindow;

class USystemTray: public QObject {
	Q_OBJECT
public:
	explicit USystemTray(MainWindow *mainWindow);
	virtual ~USystemTray() = default;
	void info(const QString &message) const;
	bool isSupported() const;
	void setToolTip(const QString &toolTip);
	void setVisible(const bool visible);
	void updateIcon(MainWindow *mainWindow);
	void warning(const QString &message) const;
private:
	Q_DISABLE_COPY(USystemTray)
	#ifdef KS_KF5
	KStatusNotifierItem *m_trayIcon = nullptr;
	#else
	bool m_applyIconHack = true;
	bool m_sessionRestored;
	QSystemTrayIcon *m_trayIcon = nullptr;
	#endif // KS_KF5
	QString m_toolTip = QString();
#ifdef KS_PURE_QT
private slots:
	void onRestore(QSystemTrayIcon::ActivationReason reason);
#endif // KS_PURE_QT
};

#endif // KSHUTDOWN_USYSTEMTRAY_H
