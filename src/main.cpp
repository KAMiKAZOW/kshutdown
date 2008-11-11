//
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
	qDebug("GDM = %d", Utils::isGDM());
	qDebug("GNOME = %d", Utils::isGNOME());
	qDebug("KDE Full Session = %d", Utils::isKDEFullSession());
	qDebug("KDE 3 = %d", Utils::isKDE_3());
	qDebug("KDE 4 = %d", Utils::isKDE_4());
	qDebug("KDM = %d", Utils::isKDM());

#ifdef KS_PURE_QT
	QApplication::setOrganizationName("kshutdown.sf.net"); // do not modify
	QApplication::setApplicationName("KShutdown");
	QApplication a(argc, argv);
	
	// init i18n
	QString lang = QLocale::system().name();
	QTranslator qt_trans;
	qt_trans.load("qt_" + lang);
	a.installTranslator(&qt_trans);
	QTranslator kshutdown_trans;
	kshutdown_trans.load("kshutdown_" + lang, QApplication::applicationDirPath());
	a.installTranslator(&kshutdown_trans);

	MainWindow::init();

	if (MainWindow::checkCommandLine())
		return 0;

	MainWindow::self()->maybeShow();

	return a.exec();
#else
// FIXME: show email address in GUI
	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"kshutdown",
		ki18n("KShutdown"),
		KS_VERSION,
		ki18n("An advanced shutdown utility"),
		KAboutData::License_GPL_V2,
		ki18n(KS_COPYRIGHT),
		KLocalizedString(), // no extra test
		KS_HOME_PAGE,
		KS_CONTACT
	);

	KCmdLineArgs::init(argc, argv, &about);
	
	// add custom command line options
	//!!!
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
	options.add("hibernate", ki18n("Hibernate Computer"));
	options.add("suspend", ki18n("Suspend Computer"));

	options.add("e");
	options.add("extra <file>", ki18n("Extras"));
	options.add("init", ki18n("Do not show main window on startup"));
	options.add("theme <file>", ki18n("Theme"));
	
	KCmdLineArgs::addCmdLineOptions(options);

	KUniqueApplication::addCmdLineOptions();

	bool isRunning = !KUniqueApplication::start();
	KUniqueApplication a;
	
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
#endif // KS_PURE_QT
}
