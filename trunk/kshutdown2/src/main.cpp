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

#include "commandline.h"
#include "log.h"
#include "mainwindow.h"
#include "mod.h"
#include "utils.h"
#include "version.h"

#include <QLoggingCategory>

#ifdef KS_PURE_QT
	#include <QLibraryInfo>
	#include <QPointer>
	#include <QTranslator>
#else
	#include <KAboutData>
	#include <KCrash>
	#include <KDBusService>
#endif // KS_PURE_QT

// TODO: move more things to commandline.*
bool commonStartup(const QStringList &arguments, const QCommandLineOption &versionOption, const bool first, const bool forceShow) {
	if (!CLI::getArgs()->parse(arguments))
		qWarning() << CLI::getArgs()->errorText();

	if (CLI::getArgs()->isSet(versionOption)) {
		QString text = QApplication::applicationDisplayName() + " " + QApplication::applicationVersion() + " (" KS_RELEASE_DATE ")\n";
		text += "Qt ";
		text += qVersion();
		text += " (compiled using " QT_VERSION_STR ")";
		QTextStream out(stdout);
		out << text << endl;

		return false;
	}

	if (first) {
		Mod::init();
		MainWindow::initActionsAndTriggers();
	}

	if (CLI::check()) {
		if (first)
			qApp->quit();
			
		return false;
	}
		
	bool useTimeOption = TimeOption::isValid() && TimeOption::action();
		
	if (!MainWindow::self()->maybeShow(forceShow)) {
		qApp->quit();

		return false;
	}

	if (useTimeOption)
		TimeOption::setupMainWindow();

// TODO: wayland <http://blog.martin-graesslin.com/blog/2015/07/porting-qt-applications-to-wayland/>
	if (!first)
		MainWindow::self()->raise();

	return true;
}

void initAboutData(const QString &appDescription) {
#ifdef KS_KF5
	KAboutData about(
		"kshutdown", // app name - used in config file name etc.
		"KShutdown", // program display name
		KS_FULL_VERSION
	);

	about.setBugAddress(KS_CONTACT);

	about.setCopyrightStatement(KS_COPYRIGHT);
	about.setHomepage(KS_HOME_PAGE);
	about.setLicense(KAboutLicense::GPL_V2);

// TODO: test dbus compat.
// TODO: update wiki docs
	about.setOrganizationDomain("sf.net"); // changed to "net.sf.kshutdown" because default is "org.kde.kshutdown"

	about.setShortDescription(appDescription);

	about.addAuthor("Konrad Twardowski", i18n("Maintainer"), ""/* email */, KS_CONTACT);
	about.addCredit(i18n("Thanks To All!"), QString(), QString(), "https://sourceforge.net/p/kshutdown/wiki/Credits/");

	KAboutData::setApplicationData(about);
#else
	Q_UNUSED(appDescription)
#endif // KS_KF5
}

void initAppProperties() {
	QApplication::setApplicationName("KShutdown"); // used as file name in ~/.config/kshutdown.sf.net/KShutdown.conf
	QApplication::setApplicationDisplayName("KShutdown"); // no i18n
	QApplication::setApplicationVersion(KS_FULL_VERSION);

	// NOTE: do not modify!
	#ifdef KS_PURE_QT
	QApplication::setOrganizationName("kshutdown.sf.net"); // used as file name in ~/.config/kshutdown.sf.net/KShutdown.conf
	#endif // KS_PURE_QT
}

void initDE() {
	#ifdef KS_UNIX
// TODO: cleanup log texts
	#define KS_DEBUG_SYSTEM(f, d) \
		if (d) qDebug(f ": %s", (d) ? "<FOUND>" : "not detected");
	
	bool e17 = Utils::isEnlightenment();
	KS_DEBUG_SYSTEM("Enlightenment", e17);
	if (e17) {
		qWarning("Enlightenment: Load System->DBus Extension module for Lock Screen action support");
		qWarning("Enlightenment: Load Utilities->Systray module for system tray support");
	}
	
	KS_DEBUG_SYSTEM("Cinnamon", Utils::isCinnamon());
	KS_DEBUG_SYSTEM("GNOME", Utils::isGNOME());
	KS_DEBUG_SYSTEM("KDE Full Session", Utils::isKDEFullSession());
	KS_DEBUG_SYSTEM("KDE", Utils::isKDE());
	KS_DEBUG_SYSTEM("LXDE", Utils::isLXDE());
	KS_DEBUG_SYSTEM("LXQt", Utils::isLXQt());
	KS_DEBUG_SYSTEM("MATE", Utils::isMATE());
	KS_DEBUG_SYSTEM("Openbox", Utils::isOpenbox());
	KS_DEBUG_SYSTEM("Razor-qt", Utils::isRazor());
	KS_DEBUG_SYSTEM("Trinity", Utils::isTrinity());
	KS_DEBUG_SYSTEM("Unity", Utils::isUnity());
	KS_DEBUG_SYSTEM("Xfce", Utils::isXfce());
	#endif // KS_UNIX
}

void initTranslation() {
	#ifdef KS_PURE_QT
	QString lang = QLocale::system().name();

	#ifdef Q_OS_WIN32
	QPointer<QTranslator> qtTranslator = new QTranslator();
	if (qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		qApp->installTranslator(qtTranslator);
	#endif // Q_OS_WIN32

	QPointer<QTranslator> kshutdownTranslator = new QTranslator();
	// NOTE: It uses QLocale::uiLanguages() which on Windows is not what we really want:
	//if (kshutdownTranslator->load(locale, "kshutdown", "_", ":/i18n"))
	if (kshutdownTranslator->load("kshutdown_" + lang, ":/i18n"))
		qApp->installTranslator(kshutdownTranslator);
	#else
	KLocalizedString::setApplicationDomain("kshutdown");
	#endif // KS_PURE_QT
}

int main(int argc, char **argv) {
/* TODO: GTK style (?)
	#ifdef KS_PURE_QT
	// NOTE: run this before QApplication constructor
	#ifdef KS_UNIX
	bool userStyle = false;
	#endif // KS_UNIX
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			QString arg(argv[i]);
			#ifdef KS_UNIX
			if ((arg == "-style") || (arg == "--style")) {
				userStyle = true;
			
				break; // for
			}
			#endif // KS_UNIX
		}
	}
	#endif // KS_PURE_QT

	qDebug() << QStyleFactory::keys();

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
*/

	// init misc

// TODO: move to Log class
// TODO: logging options
	QLoggingCategory::setFilterRules("default=true");
	qSetMessagePattern("%{appname}[%{time}] %{type}/%{category}: %{message}");

	Utils::init();
	Log::init();
	initAppProperties();
	initDE();

	QApplication program(argc, argv);

	#ifdef KS_KF5
	// CREDITS: based on https://cgit.kde.org/okular.git/commit/?id=1904f7e95e68ded874cdbe7d9685fe2cd05795b0
	KCrash::initialize();
	#endif // KS_KF5

	initTranslation(); // 1.
	QString appDescription = i18n("A graphical shutdown utility"); // 2.

/* TODO: qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, true); #Qt5.4
export QT_SCALE_FACTOR=2
http://doc.qt.io/qt-5/highdpi.html
*/

	initAboutData(appDescription);

/* TEST: KCrash handler
	QObject *crash = nullptr;
	crash->objectName();
*/

// TODO: about.setTranslator(ki18n("Your names"), ki18n("Your emails"));

	// init command line options

	CLI::init(appDescription);
	// NOTE: clash with -h: parser->addHelpOption();
	QCommandLineOption versionOption = CLI::getArgs()->addVersionOption();

	// init single-instance handler

	#ifdef KS_KF5
	KDBusService *dbusService = new KDBusService(KDBusService::Unique | KDBusService::NoExitOnFailure);
// TODO: print message if KShutdown is already running
// qDebug("KShutdown is already running");

	QObject::connect(dbusService, &KDBusService::activateRequested, [&versionOption](const QStringList &arguments, const QString &cwd) {
		Q_UNUSED(cwd)

		commonStartup(arguments, versionOption, false, true);

// FIXME: Qt 5 build
		if (CLI::isArg("cancel"))
			MainWindow::self()->setActive(false);
	} );
	#endif // KS_KF5

	// start the application

	static bool first = true;

	if (!commonStartup(program.arguments(), versionOption, first, false))
		return 0;

	first = false;

	return program.exec();
}
