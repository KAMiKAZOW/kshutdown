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

#include <QPainter>

// public:

USystemTray::USystemTray(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	U_DEBUG << "USystemTray::USystemTray()" U_END;

#ifdef KS_NATIVE_KDE
	m_trayIcon = new KSystemTrayIcon(mainWindow);
// TODO: "KShutdown" caption in System Tray Settings dialog (Entries tab).
// Currently it's lower case "kshutdown".
#endif // KS_NATIVE_KDE

#ifdef KS_PURE_QT
	m_trayIcon = new QSystemTrayIcon(mainWindow);
	
	connect(
		m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		SLOT(onRestore(QSystemTrayIcon::ActivationReason))
	);
#endif // KS_PURE_QT

	updateIcon(mainWindow);

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

void USystemTray::updateIcon(MainWindow *mainWindow) const {
	bool active = mainWindow->active();
	bool bw = Config::blackAndWhiteSystemTrayIcon();
	
	// get base icon
	
	QIcon icon;
	#ifdef KS_UNIX
	if (Utils::isKDE_4())
		icon = U_STOCK_ICON("system-shutdown");
	else
		icon = mainWindow->windowIcon();
	#else
	icon = mainWindow->windowIcon();
	#endif // KS_UNIX

	// convert base icon to pixmap

	int w = 22;
	int h = 22;
	QRect iconSize = m_trayIcon->geometry();
	// HACK: https://bugreports.qt-project.org/browse/QTBUG-24683
	if (!iconSize.size().isEmpty() && (iconSize.width() <= 64) && (iconSize.height() <= 64)) {
		U_DEBUG << "USystemTray::updateIcon: system tray icon size: " << iconSize U_END;
		w = qBound(22, iconSize.width(), 64);
		h = qBound(22, iconSize.height(), 64);
	}
	else {
		U_DEBUG << "USystemTray::updateIcon: using safe tray icon size; reported geometry = " << iconSize U_END;
	}

	QPixmap pixmap = icon.pixmap(w, h);
	QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

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

	// add overlay icons (active trigger/action)
	
	w = pixmap.width() / 2;
	if (active && (w >= 11)) {
		h = pixmap.height() / 2;

		QPainter p(&image);
		p.setOpacity(0.7);
		
		// left/bottom
		mainWindow->getSelectedAction()->icon().paint(&p, 0, h, w, h);
		// right/bottom
		mainWindow->getSelectedTrigger()->icon().paint(&p, w, h, w, h);
	}

	m_trayIcon->setIcon(QPixmap::fromImage(image));
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
