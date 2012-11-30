// usystemtray.cpp - A system tray and notification area
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

#include "config.h"
#include "mainwindow.h"
#include "pureqt.h"
#include "usystemtray.h"
#include "utils.h"

#ifdef KS_NATIVE_KDE
	#include <KIconEffect>

	#include <QPainter>
#endif // KS_NATIVE_KDE

// public:

USystemTray::USystemTray(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	U_DEBUG << "USystemTray::USystemTray()" U_END;

#ifdef KS_NATIVE_KDE
	m_trayIcon = new KSystemTrayIcon(mainWindow);
// TODO: "KShutdown" caption in System Tray Settings dialog (Entries tab).
// Currently it's lower case "kshutdown".
	updateIcon();
#endif // KS_NATIVE_KDE

#ifdef KS_PURE_QT
	m_trayIcon = new QSystemTrayIcon(mainWindow);
	#ifdef KS_UNIX
	if (Utils::isKDE_4())
		m_trayIcon->setIcon(U_STOCK_ICON("system-shutdown"));
	else
		m_trayIcon->setIcon(mainWindow->windowIcon());
	#else
	m_trayIcon->setIcon(mainWindow->windowIcon());
	#endif // KS_UNIX

	connect(
		m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		SLOT(onRestore(QSystemTrayIcon::ActivationReason))
	);
#endif // KS_PURE_QT

	U_DEBUG << "USystemTray::isSupported:" << isSupported() U_END;
}

USystemTray:: ~USystemTray() {
	U_DEBUG << "USystemTray::~USystemTray()" U_END;
}

void USystemTray::info(const QString &message) const {
	m_trayIcon->showMessage("KShutdown", message, QSystemTrayIcon::Information, 4000);
}

bool USystemTray::isSupported() const {
	return !Utils::isUnity() && QSystemTrayIcon::isSystemTrayAvailable();
}

void USystemTray::setActive(const bool active, KShutdown::Action *action, KShutdown::Trigger *trigger) const {
	#ifdef KS_NATIVE_KDE
// TODO: Qt 4.6+: http://qt-project.org/doc/qt-4.8/qgraphicscolorizeeffect.html
	if (active) {
		U_ICON defaultIcon = U_STOCK_ICON("system-shutdown");

		QRect iconSize = m_trayIcon->geometry();
		if (iconSize.size().isEmpty()) {
			U_DEBUG << "USystemTray::setActive: empty system tray icon size: " << iconSize U_END;
		}
		else {
			U_DEBUG << "USystemTray::setActive: system tray icon size: " << iconSize U_END;
			int iconW = qBound(24, iconSize.width(), 64);
			int iconH = qBound(24, iconSize.height(), 64);
			QImage i = defaultIcon.pixmap(iconW, iconH).toImage();
			if (Config::blackAndWhiteSystemTrayIcon())
				KIconEffect::deSaturate(i, 0.5f);
			else
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
		
			m_trayIcon->setIcon(QPixmap::fromImage(i));
		}
	}
	else {
		updateIcon();
	}
	#endif // KS_NATIVE_KDE
	
	#ifdef KS_PURE_QT
	Q_UNUSED(active)
	Q_UNUSED(action)
	Q_UNUSED(trigger)
	#endif // KS_PURE_QT
}

void USystemTray::setContextMenu(QMenu *menu) const {
	m_trayIcon->setContextMenu(menu);
}

void USystemTray::setToolTip(const QString &toolTip) const {
	m_trayIcon->setToolTip(toolTip);
}

void USystemTray::setVisible(const bool visible) const {
	if (visible)
		m_trayIcon->show();
	else
		m_trayIcon->hide();
}

void USystemTray::updateIcon() const {
	#ifdef KS_NATIVE_KDE
	if (Config::blackAndWhiteSystemTrayIcon()) {
		U_ICON defaultIcon = U_STOCK_ICON("system-shutdown");
		QImage i = defaultIcon.pixmap(24, 24).toImage();
		KIconEffect::toGray(i, 1.0f);
		m_trayIcon->setIcon(QPixmap::fromImage(i));
	}
	else {
		m_trayIcon->setIcon(U_STOCK_ICON("system-shutdown"));
	}
	#endif // KS_NATIVE_KDE
}

void USystemTray::warning(const QString &message) const {
	m_trayIcon->showMessage("KShutdown", message, QSystemTrayIcon::Warning, 4000);
}

// private slots:

#ifdef KS_PURE_QT
void USystemTray::onRestore(QSystemTrayIcon::ActivationReason reason) {
	U_DEBUG << "USystemTray::onRestore()" U_END;

	if (reason == QSystemTrayIcon::Trigger) {
		MainWindow *mainWindow = MainWindow::self();
		if (mainWindow->isVisible()) {
			mainWindow->hide();
		}
		else {
			mainWindow->show();
			mainWindow->activateWindow();
		}
	}
}
#endif // KS_PURE_QT
