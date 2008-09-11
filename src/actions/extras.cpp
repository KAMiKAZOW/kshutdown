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
	#include <KRun>
	#include <KStandardDirs>
	
	#include "../mainwindow.h"
#endif // KS_NATIVE_KDE

#include "extras.h"

// Extras

// private

Extras *Extras::m_instance = 0;

// public

QWidget *Extras::getWidget() { return m_menuButton; }

bool Extras::onAction() {
#ifdef KS_NATIVE_KDE
//!!!command line
	KDesktopFile desktopFile(m_command);
	QString desktopExec = desktopFile.desktopGroup().readEntry("Exec", "");
	
	if (desktopExec.isEmpty()) {
		m_error = i18n("Invalid \"Extras\" command");
	
		return false;
	}
	
// FIXME: error detection
	if (KRun::run(desktopExec, KUrl::List(), MainWindow::self()))
		return true;
	
	m_error = i18n("Cannot execute \"Extras\" command");
	
	return false;
#else
	return false;
#endif // KS_NATIVE_KDE
}

// private

Extras::Extras() :
	Action(i18n("Extras..."), "rating", "extras"),
	m_menu(0) {
	
	setMenu(createMenu());
	setShowInMenu(false);
	m_menuButton = new U_PUSH_BUTTON();
	m_menuButton->setMenu(menu());
	
	setCommandAction(0);

	addCommandLineArg("e", "extra");//!!!arg value
}

U_MENU *Extras::createMenu() {
#ifdef KS_NATIVE_KDE
	m_menu = new U_MENU();
	connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));

	return m_menu;
#else
	m_menu = new U_MENU();
	
	return m_menu; // return dummy menu
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
		else if (i.isFile() && KDesktopFile::isDesktopFile(i.filePath())) {//!!!allow any executables
			QString text = i.baseName();
			U_ICON icon = readDesktopInfo(i, text);
			CommandAction *action = new CommandAction(icon, text, this, i.filePath());
			parentMenu->addAction(action);
		}
	}
#else
	Q_UNUSED(parentMenu)
	Q_UNUSED(parentDir)
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
	Q_UNUSED(fileInfo)
	Q_UNUSED(text)

	return U_ICON(); // return dummy icon
#endif // KS_NATIVE_KDE
}

void Extras::setCommandAction(const CommandAction *command) {
	if (command) {
		m_command = command->m_command;
		
		U_DEBUG << "Extras::setCommandAction: " << m_command U_END;
		m_menuButton->setIcon(U_ICON(command->icon()));
		m_menuButton->setText(command->text());
		//m_status = (originalText() + " - " + command->text());
	}
	else {
		m_command = QString::null;
	
		U_DEBUG << "Extras::setCommandAction: NULL" U_END;
		m_menuButton->setIcon(U_STOCK_ICON("arrow-down"));
		m_menuButton->setText(i18n("Select a command..."));
		//m_status = QString::null;
	}
}

// private slots

void Extras::slotModify()
{
#ifdef KS_NATIVE_KDE
	#define KS_LI(text) "<li>" + (text) + "</li>" +
	KMessageBox::information(
		0,
		"<qt>" +
		i18n("Use context menu to add/edit/remove actions.") +
		"<ul>" +
			KS_LI(i18n("Use <b>Context Menu</b> to create a new link to application (action)"))
			KS_LI(i18n("Use <b>Create New|Folder...</b> to create a new submenu"))
			KS_LI(i18n("Use <b>Properties</b> to change icon, name, or command"))
		"</ul>" +
		"</qt>",
		originalText(),
		"ModifyExtras"
	);

	QString command = "konqueror \"" + KGlobal::dirs()->saveLocation("data", "kshutdown/extras") + "\"";
	KRun::run(command, KUrl::List(), MainWindow::self());
#endif // KS_NATIVE_KDE
}

void Extras::updateMenu() {
#ifdef KS_NATIVE_KDE
	m_menu->clear();

	QStringList dirs(KGlobal::dirs()->findDirs("data", "kshutdown/extras"));
	foreach (QString i, dirs) {
		U_DEBUG << "Found Extras Directory: " << i U_END;
		createMenu(m_menu, i);
	}
	if (!m_menu->isEmpty())
		m_menu->addSeparator();

	U_ACTION *modifyAction = new U_ACTION(i18n("Add/Remove Commands..."), this);
	connect(modifyAction, SIGNAL(triggered()), this, SLOT(slotModify()));
	m_menu->addAction(modifyAction);
#endif // KS_NATIVE_KDE
}

// CommandAction

// private

CommandAction::CommandAction(const U_ICON &icon, const QString &text, QObject *parent, const QString &command) :
	U_ACTION(icon, text, parent),
	m_command(command) {
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

// private slots

void CommandAction::slotFire() {
	Extras::self()->setCommandAction(this);
}
