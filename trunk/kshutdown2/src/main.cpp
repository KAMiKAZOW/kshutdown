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
#else
	#include <KAboutData>
	#include <KCmdLineArgs>
	#include <KUniqueApplication>
#endif // KS_PURE_QT

#include "mainwindow.h"
#include "version.h"

int main(int argc, char **argv) {
#ifdef KS_PURE_QT
	QApplication a(argc, argv);
	MainWindow::self()->show();

	return a.exec();
	//!!!
#else
	KAboutData about(
		"kshutdown", // internal name - do not modify
		"KShutdown", // no i18n
		KS_VERSION " (" KS_BUILD ")",
		I18N_NOOP("An advanced shutdown utility"),
		KAboutData::License_GPL_V2,
		"(C) 2003-3000 Konrad Twardowski",
		0, // no extra text
		"http://kshutdown.sf.net",
// FIXME: show address in GUI
		"kdtonline@poczta.onet.pl"
	);

	KCmdLineArgs::init(argc, argv, &about);
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start()) {
		U_DEBUG << "KShutdown is already running";
// FIXME: show and raise main window
		return 0;
	}

	KUniqueApplication a;
	MainWindow::self()->show();

	return a.exec();
#endif // KS_PURE_QT
}
