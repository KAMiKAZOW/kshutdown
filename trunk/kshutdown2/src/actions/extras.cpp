//
// extras.cpp - Extras
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

#include "../pureqt.h"

#ifdef KS_NATIVE_KDE
	#include <QDir>

	#include <KDesktopFile>
	#include <KStandardDirs>
#endif // KS_NATIVE_KDE

#include "extras.h"

Extras *Extras::m_instance = 0;

// public

QWidget *Extras::getWidget() {
	if (!m_menuButton) {
		m_menuButton = new U_PUSH_BUTTON();
		m_menuButton->setMenu(menu());
	}

	return m_menuButton;
}

bool Extras::onAction() {
	//!!!
	return false;
}

// private

Extras::Extras() :
	Action(i18n("Extras..."), "rating", "extras"),
	m_menuButton(0) {
	//setShowInMenu(false);
	setMenu(createMenu());
}

U_MENU *Extras::createMenu() {
#ifdef KS_NATIVE_KDE
	U_MENU *menu = new U_MENU();

	QStringList dirs(KGlobal::dirs()->findDirs("data", "kshutdown/extras"));
	foreach (QString i, dirs) {
		U_DEBUG << "Found Extras Directory: " << i U_END;
		createMenu(menu, i);
	}
	if (!menu->isEmpty())
		menu->addSeparator();

	//!!!modify command

	return menu;
#else
	return new U_MENU(); // return dummy menu
#endif // KS_NATIVE_KDE
}

void Extras::createMenu(U_MENU *parentMenu, const QString &parentDir) {
#ifdef KS_NATIVE_KDE
	QDir dir(parentDir);
	QFileInfoList entries = dir.entryInfoList(
		QDir::Dirs | QDir::Files,
		QDir::DirsFirst | QDir::Name
	);
	QString fileName;
	foreach (QFileInfo i, entries) {
		fileName = i.fileName();
		if (i.isDir() && (fileName != ".") && (fileName != "..")) {
			QString dirProperties = i.filePath() + "/.directory";
			QString text = i.baseName();
			U_MENU *dirMenu;
			if (QFile::exists(dirProperties)) {
				U_ICON icon = readDesktopInfo(dirProperties, text);
				dirMenu = new U_MENU(text);
				dirMenu->setIcon(icon);
			}
			else {
				dirMenu = new U_MENU(text);
			}
			createMenu(dirMenu, i.filePath()); // recursive scan
			parentMenu->addMenu(dirMenu);
		}
		else if (i.isFile() && KDesktopFile::isDesktopFile(i.filePath())) {
			QString text = i.baseName();
			U_ICON icon = readDesktopInfo(i, text);
			U_ACTION *action = new U_ACTION(icon, text, this);
			action->setData(i.filePath()); // user data = file to execute
			parentMenu->addAction(action);
		}
	}
#endif // KS_NATIVE_KDE
}

U_ICON Extras::readDesktopInfo(const QFileInfo &fileInfo, QString &text) {
#ifdef KS_NATIVE_KDE
	KDesktopFile desktopFile(fileInfo.filePath());

	QString desktopName = desktopFile.readName();
	if (!desktopName.isEmpty())
		text = desktopName;

	QString desktopComment = desktopFile.readComment();
	if (!desktopComment.isEmpty())
		text += (" - " + desktopComment);

/* TODO: show actual "Exec" command
	QString desktopExec = desktopFile.desktopGroup().readEntry("Exec", "");
	if (!desktopExec.isEmpty())
		text += (" - " + desktopExec);
*/

	return U_STOCK_ICON(desktopFile.readIcon());
#else
	return U_ICON(); // return dummy icon
#endif // KS_NATIVE_KDE
}
