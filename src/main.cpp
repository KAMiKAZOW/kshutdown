// main.cpp - An advanced shutdown utility
// Copyright (C) 2007  Konrad Twardowski
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

#ifdef KS_PURE_QT
	#include <QApplication>
	#include <QLocale>
	#ifdef Q_OS_LINUX
		#include <QStyleFactory>
	#endif // Q_OS_LINUX
	#include <QTranslator>
#else
	#include <KAboutData>
	#include <KCmdLineArgs>
	#include <KUniqueApplication>

	#include "version.h"
#endif // KS_PURE_QT

#include "mainwindow.h"
#include "utils.h"

int main(int argc, char **argv) {
	bool isRunning = false;

	qDebug("GDM = %d", Utils::isGDM());
	qDebug("GNOME = %d", Utils::isGNOME());
	qDebug("KDE Full Session = %d", Utils::isKDEFullSession());
	qDebug("KDE 3 = %d", Utils::isKDE_3());
	qDebug("KDE 4 = %d", Utils::isKDE_4());
	qDebug("KDM = %d", Utils::isKDM());

#ifdef KS_PURE_QT

	// Pure Qt startup

	QApplication::setOrganizationName("kshutdown.sf.net"); // do not modify
	QApplication::setApplicationName("KShutdown");
	QApplication a(argc, argv);

	#ifdef Q_OS_LINUX
	if (Utils::isGNOME()) {
		QStyle *gtkStyle = QStyleFactory::create("gtk+");
		if (gtkStyle)
			QApplication::setStyle(gtkStyle);
	}
	#endif // Q_OS_LINUX

	// init i18n
	QString lang = QLocale::system().name();
	QTranslator qt_trans;
	qt_trans.load("qt_" + lang);
	a.installTranslator(&qt_trans);
	QTranslator kshutdown_trans;
	//kshutdown_trans.load("kshutdown_" + lang, QApplication::applicationDirPath());
	kshutdown_trans.load("kshutdown_" + lang, ":/i18n");
	a.installTranslator(&kshutdown_trans);
	
#else

	// Native KDE startup

	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"kshutdown", // catalog name
		ki18n("KShutdown"), // program name
		KS_VERSION
	);
	about.setBugAddress(KS_CONTACT);
	about.setCopyrightStatement(ki18n(KS_COPYRIGHT));
	about.setHomepage(KS_HOME_PAGE);
	about.setLicense(KAboutData::License_GPL_V2);
	about.setShortDescription(ki18n("An advanced shutdown utility"));

	KCmdLineArgs::init(argc, argv, &about);
	
	// add custom command line options
	// NOTE: Sync. with "addCommandLineArg"
	KCmdLineOptions options;
	
	options.add("h");
	options.add("halt", ki18n("Turn Off Computer"));
	
	options.add("s");
	options.add("shutdown", ki18n("Turn Off Computer"));
	
	options.add("k");
	options.add("lock", ki18n("Lock screen"));
	
	options.add("r");
	options.add("reboot", ki18n("Restart Computer"));
	
	options.add("l");
	options.add("logout", ki18n("Logout"));
	
	options.add("H");
	options.add("hibernate", ki18n("Hibernate Computer"));
	
	options.add("S");
	options.add("suspend", ki18n("Suspend Computer"));

	options.add("e");
	options.add("extra <file>", ki18n("Extras"));
	
	options.add("init", ki18n("Do not show main window on startup"));
	
	KCmdLineArgs::addCmdLineOptions(options);
	// BUG: --nofork option does not work like in KShutdown 1.0.x (?)
	// "KUniqueApplication: Can't setup D-Bus service. Probably already running."
	KUniqueApplication::addCmdLineOptions();

	isRunning = !KUniqueApplication::start();
	KUniqueApplication a;

#endif // KS_PURE_QT

	// Common startup
	
	MainWindow::init();
	
	if (MainWindow::checkCommandLine())
		return 0;
	
	if (isRunning) {
		U_DEBUG << "KShutdown is already running" U_END;
// FIXME: show and raise main window
		return 0;
	}
	
	MainWindow::self()->maybeShow();

	return a.exec();
}
