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

#include <QDebug>
#include <QPainter>

// TODO: better support for dock(ish) taskbars (Windows 7/Unity)

// public:

USystemTray::USystemTray(MainWindow *mainWindow)
	: QObject(mainWindow)
{
	#ifdef KS_PURE_QT
	m_sessionRestored = qApp->isSessionRestored();
	#endif // KS_PURE_QT

	qDebug() << "USystemTray::isSupported:" << isSupported();
}

void USystemTray::info(const QString &message) const {
	if (m_trayIcon == nullptr)
		return;

	#ifdef KS_KF5
	m_trayIcon->showMessage(QApplication::applicationDisplayName(), message, "dialog-information");
	#else
	m_trayIcon->showMessage(QApplication::applicationDisplayName(), message, QSystemTrayIcon::Information, 4000);
	#endif // KS_KF5
}

bool USystemTray::isSupported() const {
	#ifdef KS_KF5
	return true; // assume the Desktop Environment is sane
	#else
	return
		QSystemTrayIcon::isSystemTrayAvailable() &&
		// HACK: MATE launches KShutdown before system tray panel is created
		!(Utils::isMATE() && m_sessionRestored);
	#endif // KS_KF5
}

void USystemTray::setToolTip(const QString &toolTip) {
	m_toolTip = toolTip; // save "toolTip" value for later use

	if (m_trayIcon == nullptr)
		return;

	#ifdef KS_KF5
	m_trayIcon->setToolTipTitle(QApplication::applicationDisplayName());
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
	#ifdef KS_PURE_QT
	m_sessionRestored = false; // clear flag
	#endif // KS_PURE_QT

	auto *mainWindow = static_cast<MainWindow *>(parent());

	if (visible && (m_trayIcon == nullptr)) {
		qDebug() << "Show system tray";

		#ifdef KS_KF5
// FIXME: Ubuntu 17.10/Unity: mouse click behavior is a total mess
		m_trayIcon = new KStatusNotifierItem(mainWindow);
		m_trayIcon->setCategory(KStatusNotifierItem::ApplicationStatus);
		m_trayIcon->setStandardActionsEnabled(false);
// FIXME: m_trayIcon->setToolTipIconByPixmap(QIcon(":/images/kshutdown.png"));
		m_trayIcon->setToolTipIconByName("kshutdown");
		#else
		m_trayIcon = new QSystemTrayIcon(mainWindow);

		connect(
			m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			SLOT(onRestore(QSystemTrayIcon::ActivationReason))
		);
		#endif // KS_KF5

		auto *menu = new QMenu();
		mainWindow->initFileMenu(menu, false);
		m_trayIcon->setContextMenu(menu);

		updateIcon(mainWindow);

		if (!m_toolTip.isNull()) {
			//qDebug() << "Init tool tip:" << m_toolTip;
			setToolTip(m_toolTip);
		}

		#ifdef KS_PURE_QT
		m_trayIcon->show(); // after setIcon
		#endif // KS_PURE_QT
	}
	else if (!visible && (m_trayIcon != nullptr)) {
		qDebug() << "Hide system tray";

		delete m_trayIcon;
		m_trayIcon = nullptr;
	}
}

// TODO: revisit overlay icons
void USystemTray::updateIcon(MainWindow *mainWindow) {
	if (m_trayIcon == nullptr)
		return;

	#ifndef KS_KF5
	bool active = mainWindow->active();
	bool bw = Config::blackAndWhiteSystemTrayIconVar.getBool();

	// HACK: We need to show an empty system tray icon first
	// to fix wrong icon alignment and background repaint issues...
	if (m_applyIconHack) {
		m_applyIconHack = false;
		if (Utils::isMATE() && Config::systemTrayIconEnabledVar) {
			//qDebug() << "Apply icon hack";
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
	#ifndef Q_OS_WIN32
	if (!active && Config::readBool("General", "Use Theme Icon In System Tray", true)) {
		icon = QIcon::fromTheme("system-shutdown");
		if (icon.isNull()) {
			qDebug() << "System theme icon not found in:" << icon.themeName();
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
	#endif // !Q_OS_WIN32

	QPixmap pixmap = icon.pixmap(64_px);
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
	if (m_trayIcon == nullptr)
		return;

	#ifdef KS_KF5
	m_trayIcon->showMessage(QApplication::applicationDisplayName(), message, "dialog-warning");
	#else
	m_trayIcon->showMessage(QApplication::applicationDisplayName(), message, QSystemTrayIcon::Warning, 4000);
	#endif // KS_KF5
}

// private slots:

#ifdef KS_PURE_QT
void USystemTray::onRestore(QSystemTrayIcon::ActivationReason reason) {
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
