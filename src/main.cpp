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

#include "commandline.h"
#include "mainwindow.h"
#include "utils.h"

class KShutdownApplication: public
#ifdef KS_NATIVE_KDE
KUniqueApplication
#else
QApplication
#endif // KS_NATIVE_KDE
{
public:
#ifdef KS_PURE_QT
	KShutdownApplication(int &argc, char **argv)
		: QApplication(argc, argv) {
	}
#endif // KS_PURE_QT

#ifdef KS_NATIVE_KDE
	/** http://api.kde.org/4.x-api/kdelibs-apidocs/kdeui/html/classKUniqueApplication.html */
	virtual int newInstance() {
		static bool first = true;
		commonStartup(first);
		first = false;
		
		return 0;
	}
#endif // KS_NATIVE_KDE

	bool commonStartup(const bool first) {
		if (first)
			MainWindow::init();
		
		if (MainWindow::checkCommandLine()) {
			if (first)
				quit();
			
			return false;
		}
		
		bool useTimeOption = TimeOption::isValid() && TimeOption::action();
		
		MainWindow::self()->maybeShow();
		
		if (useTimeOption)
			TimeOption::setupMainWindow();
		
		if (!first)
			MainWindow::self()->raise();
		
		return true;
	}
};

int main(int argc, char **argv) {
	qDebug("GDM = %d", Utils::isGDM());
	qDebug("GNOME = %d", Utils::isGNOME());
	qDebug("Xfce = %d", Utils::isXfce());
	if (Utils::isGDM() || Utils::isGNOME() || Utils::isXfce()) {
		qWarning("WARNING: GNOME/Xfce desktop and/or GDM (login manager) are not supported yet.");
		qWarning("         Some functions may be unavailable.");
	}
	qDebug("KDE Full Session = %d", Utils::isKDEFullSession());
	qDebug("KDE 4 = %d", Utils::isKDE_4());
	qDebug("KDM = %d", Utils::isKDM());

#ifdef KS_PURE_QT

	// Pure Qt startup

	QApplication::setOrganizationName("kshutdown.sf.net"); // do not modify
	QApplication::setApplicationName("KShutdown");
	KShutdownApplication program(argc, argv);

	#ifdef Q_OS_LINUX
	if (Utils::isGNOME() || Utils::isXfce()) {
		QStyle *gtkStyle = QStyleFactory::create("gtk+");
		if (gtkStyle)
			QApplication::setStyle(gtkStyle);
	}
	#endif // Q_OS_LINUX

	// init i18n
	QString lang = QLocale::system().name();
	QTranslator qt_trans;
	qt_trans.load("qt_" + lang);
	program.installTranslator(&qt_trans);
	QTranslator kshutdown_trans;
	kshutdown_trans.load("kshutdown_" + lang, ":/i18n");
	program.installTranslator(&kshutdown_trans);
	
	if (!program.commonStartup(true))
		return 0;

#else

	// Native KDE startup

	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"kshutdown", // catalog name
		ki18n("KShutdown"), // program name
		KS_FULL_VERSION
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
	options.add("extra <file>", ki18n("Run executable file (example: Desktop shortcut or Shell script)"));

	// NOTE: sync. description with mainwindow.cpp/MainWindow::checkCommandLine()
	options.add("hide-ui", ki18n("Hide main window and system tray icon"));
	options.add("init", ki18n("Do not show main window on startup"));
	options.add("+[time]", ki18n("Activate countdown. Examples: 13:37 - absolute time (HH:MM), 10 - number of minutes from now"));
	
	KCmdLineArgs::addCmdLineOptions(options);
	// BUG: --nofork option does not work like in KShutdown 1.0.x (?)
	// "KUniqueApplication: Can't setup D-Bus service. Probably already running."
	KShutdownApplication::addCmdLineOptions();

	if (!KShutdownApplication::start()) {
		U_DEBUG << "KShutdown is already running" U_END;
		
		return 0;
	}
	
	KShutdownApplication program;

#endif // KS_PURE_QT

	return program.exec();
}
