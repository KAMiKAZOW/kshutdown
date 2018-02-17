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

#include "usystemtray.h"

#include "config.h"
#include "mainwindow.h"
#include "utils.h"

#include <QPainter>

// TODO: better support for dock(ish) taskbars (Windows 7/Unity)

// public:

USystemTray::USystemTray(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	//U_DEBUG << "USystemTray::USystemTray()" U_END;

#ifdef KS_NATIVE_KDE
	m_sessionRestored = false;
	#ifdef KS_KF5
// FIXME: Ubuntu 17.10/Unity: mouse click behavior is a total mess
	m_trayIcon = new KStatusNotifierItem(mainWindow);
	m_trayIcon->setCategory(KStatusNotifierItem::ApplicationStatus);
	m_trayIcon->setStandardActionsEnabled(false);
// FIXME: m_trayIcon->setToolTipIconByPixmap(QIcon(":/images/kshutdown.png"));
	m_trayIcon->setToolTipIconByName("kshutdown");
	#else
	m_trayIcon = new KSystemTrayIcon(mainWindow);
	#endif // KS_KF5
#endif // KS_NATIVE_KDE

#ifdef KS_PURE_QT
	m_sessionRestored = U_APP->isSessionRestored();
	m_trayIcon = new QSystemTrayIcon(mainWindow);

	connect(
		m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		SLOT(onRestore(QSystemTrayIcon::ActivationReason))
	);
#endif // KS_PURE_QT
	
	updateIcon(mainWindow);

	U_DEBUG << "USystemTray::isSupported:" << isSupported() U_END;
}

void USystemTray::info(const QString &message) const {
	#ifdef KS_KF5
	m_trayIcon->showMessage("KShutdown", message, "dialog-information");
	#else
	m_trayIcon->showMessage("KShutdown", message, QSystemTrayIcon::Information, 4000);
	#endif // KS_KF5
}

bool USystemTray::isSupported() const {
	#ifdef KS_KF5
// TODO: use classic QSystemTrayIcon as fallback if KStatusNotifierItem is unsupported
	return true; // assume the Desktop Environment is sane
	#else
	return
		QSystemTrayIcon::isSystemTrayAvailable() &&
		// HACK: MATE launches KShutdown before system tray panel is created
		!(Utils::isMATE() && m_sessionRestored);
	#endif // KS_KF5
}

void USystemTray::setContextMenu(QMenu *menu) const {
	m_trayIcon->setContextMenu(menu);
}

void USystemTray::setToolTip(const QString &toolTip) const {
	#ifdef KS_KF5
	m_trayIcon->setToolTipTitle("KShutdown");
	m_trayIcon->setToolTipSubTitle(
		(toolTip == m_trayIcon->toolTipTitle())
		? ""
		: QString(toolTip).replace("<qt>KShutdown<br><br>", "<qt>") // remove leading/duplicated "KShutdown" text
	);
	#else
	m_trayIcon->setToolTip(toolTip);
	#endif // KS_KF5
}

void USystemTray::setVisible(const bool visible) {
	m_sessionRestored = false; // clear flag
	#ifdef KS_KF5
	Q_UNUSED(visible)
	#else
	if (visible)
		m_trayIcon->show();
	else
		m_trayIcon->hide();
	#endif // KS_KF5
}

void USystemTray::updateIcon(MainWindow *mainWindow) {
	#ifndef KS_KF5
	bool active = mainWindow->active();
	bool bw = Config::blackAndWhiteSystemTrayIcon();

	// HACK: We need to show an empty system tray icon first
	// to fix wrong icon alignment and background repaint issues...
	if (m_applyIconHack) {
		m_applyIconHack = false;
		if (Utils::isMATE() && Config::systemTrayIconEnabled()) {
			m_trayIcon->setIcon(mainWindow->windowIcon()); // suppress Qt warning
			m_trayIcon->show();
		}
	}
	#endif // !KS_KF5
	
	// get base icon

	#ifdef KS_KF5
	Q_UNUSED(mainWindow)
// TODO: option
// FIXME: setIconByPixmap does not work...
	if (Utils::isKDE())
		m_trayIcon->setIconByName("system-shutdown");
	else
		m_trayIcon->setIconByName("kshutdown");

/* TODO: hide if inactive #5.x
	bool active = mainWindow->active();
	m_trayIcon->setStatus(active ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
*/
	#else
	// convert base icon to pixmap

	QIcon icon;
	#ifdef KS_UNIX
	if (!active && Config::readBool("General", "Use Theme Icon In System Tray", true)) {
		icon = QIcon::fromTheme("system-shutdown");
		if (icon.isNull()) {
			U_DEBUG << "System theme icon not found in: " << icon.themeName() U_END;
			icon = mainWindow->windowIcon(); // fallback
		}
		// HACK: fixes https://sourceforge.net/p/kshutdown/bugs/27/
		else {
			m_trayIcon->setIcon(icon);

			return; // no image effects
		}
	}
	else {
		icon = mainWindow->windowIcon();
	}
	#else
	icon = QIcon(":/images/hi16-app-kshutdown.png");
	#endif // KS_UNIX

	int w = 64_px;
	int h = 64_px;
	QPixmap pixmap = icon.pixmap(w, h);
	QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);

	// add some effects

	// CREDITS: http://stackoverflow.com/questions/2095039/qt-qimage-pixel-manipulation
	if (active || bw) {
		QRgb *line = (QRgb *)image.bits();
		int c = image.width() * image.height();
		int h, s, l;
		QColor temp;
		for (int i = 0; i < c; i++) {
			QRgb rgb = *line;
			
			// invisible pixel, skip
			if (rgb == 0) {
				line++;
			
				continue; // for
			}

			// convert RGB to HSL
			temp.setRgba(rgb);
			temp.getHsl(&h, &s, &l);
			
			if (active) {
				//h = 200; // ~blue
				h = 42; // ~orange
				//l = qMin(255, (int)(l * 1.2));
			}
			else if (bw) {
				s *= 0.2; // desaturate
			}

			// convert back to RGBA
			temp.setHsl(h, s, l, qAlpha(rgb));
			*line = temp.rgba();
			line++;
		}
	}

	m_trayIcon->setIcon(QPixmap::fromImage(image));
	#endif // KS_KF5
}

void USystemTray::warning(const QString &message) const {
	#ifdef KS_KF5
// TODO: "KShutdown" -> QApplication::applicationDisplayName()
	m_trayIcon->showMessage("KShutdown", message, "dialog-warning");
	#else
	m_trayIcon->showMessage("KShutdown", message, QSystemTrayIcon::Warning, 4000);
	#endif // KS_KF5
}

// private slots:

#ifdef KS_PURE_QT
void USystemTray::onRestore(QSystemTrayIcon::ActivationReason reason) {
	//U_DEBUG << "USystemTray::onRestore()" U_END;

	if (reason == QSystemTrayIcon::Trigger) {
		MainWindow *mainWindow = MainWindow::self();
		if (mainWindow->isVisible() && !mainWindow->isMinimized()) {
			mainWindow->hide();
		}
		else {
			#ifdef Q_OS_WIN32
			if (mainWindow->isMinimized()) {
				// HACK: activateWindow() does not work and causes funny bugs
				mainWindow->showNormal();
			}
			else {
				mainWindow->show();
				mainWindow->activateWindow();
			}
			#else
			mainWindow->show();
			mainWindow->activateWindow();
			#endif // Q_OS_WIN32
		}
	}
}
#endif // KS_PURE_QT
