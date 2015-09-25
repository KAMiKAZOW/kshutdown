// main.cpp - A graphical shutdown utility
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
	#include <QLibraryInfo>
	#include <QLocale>
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

#if defined(KS_PURE_QT) && defined(KS_UNIX)
	#include <QStyleFactory>
#endif // defined(KS_PURE_QT) && defined(KS_UNIX)

class KShutdownApplication: public
#ifdef KS_KF5
QApplication
#elif defined(KS_NATIVE_KDE)
KUniqueApplication
#else
QApplication
#endif // KS_KF5
{
public:
#if defined(KS_PURE_QT) || defined(KS_KF5)
	KShutdownApplication(int &argc, char **argv)
		: QApplication(argc, argv, Utils::isGUI()) {
	}
#endif // KS_PURE_QT

#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	/** http://api.kde.org/4.x-api/kdelibs-apidocs/kdeui/html/classKUniqueApplication.html */
	virtual int newInstance() override {
		static bool first = true;
		commonStartup(first);
		first = false;

		if (Utils::isArg("cancel"))
			MainWindow::self()->setActive(false);

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

// TODO: wayland <http://blog.martin-graesslin.com/blog/2015/07/porting-qt-applications-to-wayland/>
		if (!first)
			MainWindow::self()->raise();

		return true;
	}
};

int main(int argc, char **argv) {
	#ifdef KS_KF5
	#warning **** This build/port (KF5) is experimental and incomplete ****
	qSetMessagePattern("%{appname}[%{time}] %{message}");
	#endif // KS_KF5

	Utils::init();
	#define KS_DEBUG_SYSTEM(f, d) \
		if (d) qDebug("kshutdown: " f ": %s", d ? "<FOUND>" : "not detected");
	
	bool e17 = Utils::isEnlightenment();
	KS_DEBUG_SYSTEM("Enlightenment", e17);
	if (e17) {
		qWarning("kshutdown: Enlightenment: Load System->DBus Extension module for Lock Screen action support");
		qWarning("kshutdown: Enlightenment: Load Utilities->Systray module for system tray support");
	}
	
	KS_DEBUG_SYSTEM("Cinnamon", Utils::isCinnamon());
	KS_DEBUG_SYSTEM("GNOME", Utils::isGNOME());
	KS_DEBUG_SYSTEM("KDE Full Session", Utils::isKDEFullSession());
	KS_DEBUG_SYSTEM("KDE", Utils::isKDE());
	KS_DEBUG_SYSTEM("LXDE", Utils::isLXDE());
	KS_DEBUG_SYSTEM("MATE", Utils::isMATE());
	KS_DEBUG_SYSTEM("Razor-qt", Utils::isRazor());
	KS_DEBUG_SYSTEM("Trinity", Utils::isTrinity());
	KS_DEBUG_SYSTEM("Unity", Utils::isUnity());
	KS_DEBUG_SYSTEM("Xfce", Utils::isXfce());

#ifdef KS_PURE_QT

	// Pure Qt startup

	#ifdef KS_UNIX
	// NOTE: run this before QApplication constructor
	bool userStyle = false;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			QByteArray arg(argv[i]);
			if (arg.startsWith("-style")) {
				userStyle = true;
			
				break; // for
			}
		}
	}
	#endif // KS_UNIX

	QApplication::setOrganizationName("kshutdown.sf.net"); // do not modify
	QApplication::setApplicationName("KShutdown");
	#if QT_VERSION >= 0x050200
	QApplication::setApplicationDisplayName("KShutdown");
	#endif // QT_VERSION
	KShutdownApplication program(argc, argv);

/* TODO: program.setAttribute(Qt::AA_UseHighDpiPixmaps, true); #Qt5.4
http://doc-snapshot.qt-project.org/qt5-5.4/highdpi.html
http://blog.davidedmundson.co.uk/blog/kde_apps_high_dpi
*/

	#ifdef KS_UNIX
	if (
		!userStyle && // do not override user style option
		Utils::isGTKStyle() &&
		!Utils::isHaiku()
	) {
		QStyle *gtkStyle = QStyleFactory::create("gtk+");
		if (gtkStyle)
			QApplication::setStyle(gtkStyle);
	}
	#endif // KS_UNIX

	// init i18n
	QString lang = QLocale::system().name();
	QTranslator qt_trans;
	qt_trans.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	program.installTranslator(&qt_trans);
	QTranslator kshutdown_trans;
	kshutdown_trans.load("kshutdown_" + lang, ":/i18n");
	program.installTranslator(&kshutdown_trans);
	
	if (!program.commonStartup(true))
		return 0;

#else

	// Native KDE startup

	#ifdef KS_KF5
	QApplication::setApplicationDisplayName("KShutdown");
	#endif // KS_KF5

	#define KS_EMAIL \
		"twardowski" \
		"@" \
		"gmail" \
		".com"
	#ifdef KS_KF5
	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"KShutdown", // program display name
		KS_FULL_VERSION
	);
	about.setBugAddress(KS_EMAIL);
	about.setCopyrightStatement(i18n(KS_COPYRIGHT));
	about.setHomepage(KS_HOME_PAGE);
	about.setLicense(KAboutLicense::GPL_V2);
	about.setShortDescription(i18n("A graphical shutdown utility"));

	about.addAuthor("Konrad Twardowski", i18n("Maintainer"), KS_EMAIL, KS_CONTACT);
	about.addCredit(i18n("Thanks To All!"), QString(), QString(), "http://sourceforge.net/p/kshutdown/wiki/Credits/");
	#else
	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"kshutdown", // catalog name
		ki18n("KShutdown"), // program name
		KS_FULL_VERSION
	);
	about.setBugAddress(KS_EMAIL);
	about.setCopyrightStatement(ki18n(KS_COPYRIGHT));
	about.setHomepage(KS_HOME_PAGE);
	about.setLicense(KAboutData::License_GPL_V2);
	about.setShortDescription(ki18n("A graphical shutdown utility"));

	about.addAuthor(ki18n("Konrad Twardowski"), ki18n("Maintainer"), KS_EMAIL, KS_CONTACT);
	about.addCredit(ki18n("Thanks To All!"), KLocalizedString(), QByteArray(), "http://sourceforge.net/p/kshutdown/wiki/Credits/");
	#endif // KS_KF5

	// NOTE: "kshutdown.sf.net" produces too long DBus names
	// (net.sf.kshutdown.kshutdown)
	about.setOrganizationDomain("sf.net");

	// DOC: http://api.kde.org/4.8-api/kdelibs-apidocs/kdecore/html/classKAboutData.html
// TODO: about.setTranslator(ki18n("Your names"), ki18n("Your emails"));

	#ifdef KS_KF5
	KAboutData::setApplicationData(about);
	QCommandLineParser *options = Utils::parser();
//	options->addOption(QCommandLineOption(QStringList() << "h" << "halt", i18n("Turn Off Computer")));
	#else
	KCmdLineArgs::init(argc, argv, &about);

	// add custom command line options
	// NOTE: Sync. with "addCommandLineArg"
	KCmdLineOptions options;
	
	options.add(":", ki18n("Actions:"));
	
	options.add("h");
	options.add("halt", ki18n("Turn Off Computer"));
	
	options.add("s");
	options.add("shutdown", ki18n("Turn Off Computer"));

	options.add("r");
	options.add("reboot", ki18n("Restart Computer"));

	options.add("H");
	options.add("hibernate", ki18n("Hibernate Computer"));

	options.add("S");
	options.add("suspend", ki18n("Suspend Computer"));

	options.add("k");
	options.add("lock", ki18n("Lock screen"));

	options.add("l");
	options.add("logout", ki18n("Logout"));

	options.add("e");
	options.add("extra <file>", ki18n("Run executable file (example: Desktop shortcut or Shell script)"));
	
	options.add("test", ki18n("Test Action (does nothing)"));
	
	// NOTE: sync. description with mainwindow.cpp/MainWindow::checkCommandLine()
	
	options.add(":", ki18n("Triggers:"));

	options.add("i");
	options.add(
		"inactivity",
		ki18n(
			"Detect user inactivity. Example:\n"
			"--logout --inactivity 90 - automatically logout after 90 minutes of user inactivity"
		)
	);

	options.add(":", ki18n("Other Options:"));

	options.add("cancel", ki18n("Cancel an active action"));
	options.add("confirm", ki18n("Confirm command line action"));
	options.add("hide-ui", ki18n("Hide main window and system tray icon"));
	options.add("init", ki18n("Do not show main window on startup"));
	options.add("mod <value>", ki18n("A list of modifications"));
// TODO: better AM/PM format support
	options.add(
		"+[time]",
		ki18n(
			"Activate countdown. Examples:\n"
			"13:37 - absolute time (HH:MM)\n"
			"10 or 10m - number of minutes from now\n"
			"2h - two hours"
		)
	);
	
	options.add("", ki18n("More Info...\nhttp://sourceforge.net/p/kshutdown/wiki/Command%20Line/")); // krazy:exclude=i18ncheckarg
	
	KCmdLineArgs::addCmdLineOptions(options);
	// BUG: --nofork option does not work like in KShutdown 1.0.x (?)
	// "KUniqueApplication: Can't setup D-Bus service. Probably already running."
	KShutdownApplication::addCmdLineOptions();
	#endif // KS_KF5

	#ifndef KS_KF5
	if (!KShutdownApplication::start()) {
		// HACK: U_DEBUG is unavailble here
		//U_DEBUG << "KShutdown is already running" U_END;
		qDebug("KShutdown is already running");
		
		return 0;
	}
	#endif // KS_KF5
	
	#ifdef KS_KF5
	KShutdownApplication program(argc, argv);
	//!!!Utils::parser()->process(program);

	if (!program.commonStartup(true))
		return 0;
	#else
	KShutdownApplication program;
	#endif // KS_KF5
#endif // KS_PURE_QT

	return program.exec();
}
