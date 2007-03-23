
//!!!cleanup

#include <KAboutData>
#include <KLocale>
#include <KCmdLineArgs>
#include <KUniqueApplication>

#include "mainwindow.h"
#include "version.h"

int main(int argc, char **argv) {
	KAboutData about(
		"kshutdown", // internal name - do not modify
		"KShutDown", // no i18n
		KS_VERSION " (" KS_BUILD ")",
		I18N_NOOP("An advanced shut down utility"),
		KAboutData::License_GPL_V2,
		"(C) 2003-3000 Konrad Twardowski",
		0,
		"http://kshutdown.sf.net",
// FIXME: show address in GUI
		"kdtonline@poczta.onet.pl"
	);

	KCmdLineArgs::init(argc, argv, &about);
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start()) {
		kDebug() << "KShutDown is already running" << endl;
// FIXME: show and raise main window
		return 0;
	}

	KUniqueApplication a;
	MainWindow::self()->show();

	return a.exec();
}
