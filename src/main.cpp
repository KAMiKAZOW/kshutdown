//
// main.cpp - An advanced shut down utility
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
		0, // no extra text
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
