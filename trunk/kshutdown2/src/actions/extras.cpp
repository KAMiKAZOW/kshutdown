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
	#include <QDesktopServices>
	#include <QDir>

	#include <KDesktopFile>
	#include <KRun>
	#include <KService>
	#include <KStandardAction>
	#include <KStandardDirs>
#endif // KS_NATIVE_KDE

#include "extras.h"

// Extras

// private

Extras *Extras::m_instance = 0;

// public

QWidget *Extras::getWidget() { return m_menuButton; }

bool Extras::onAction() {
#ifdef KS_NATIVE_KDE
	QFileInfo fileInfo(m_command);
	if (KDesktopFile::isDesktopFile(fileInfo.filePath())) {
		KDesktopFile desktopFile(m_command);
		KService service(&desktopFile);
		
		if (service.exec().isEmpty()) { // krazy:exclude=crashy
			m_error = i18n("Invalid \"Extras\" command");
		
			return false;
		}
		
// FIXME: error detection, double error message box
		if (KRun::run(service, KUrl::List(), U_APP->activeWindow()))
			return true;
		
		m_error = i18n("Cannot execute \"Extras\" command");
		
		return false;
	}
	else {
		if (KRun::run(m_command, KUrl::List(), U_APP->activeWindow()))
			return true;
		
		m_error = i18n("Cannot execute \"Extras\" command");
		
		return false;
	}
#else
	return false;
#endif // KS_NATIVE_KDE
}

void Extras::readConfig(const QString &group, Config *config) {
	config->beginGroup(group);
	// do not override command set via "e" command line option
	if (m_command.isNull())
		setCommand(config->read("Command", "").toString());
	config->endGroup();
}

void Extras::setCommand(const QString &command) {
	m_command = command;
	if (m_command.isEmpty()) {
		setCommandAction(0);
	}
	else {
		QFileInfo fileInfo(m_command);
		setCommandAction(createCommandAction(fileInfo, false));
	}
}

void Extras::writeConfig(const QString &group, Config *config) {
	config->beginGroup(group);
	config->write("Command", m_command);
	config->endGroup();
}

// private

Extras::Extras() :
	Action(i18n("Extras"), "rating", "extras"),
	m_command(QString::null),
	m_menu(0) {
	
	setMenu(createMenu());
	setShowInMenu(false);
	m_menuButton = new U_PUSH_BUTTON();
	m_menuButton->setMenu(menu());
	
	//setCommandAction(0);

	// NOTE: Sync. with mainwindow.cpp (MainWindow::checkCommandLine())
	addCommandLineArg("e", "extra");
}

CommandAction *Extras::createCommandAction(const QFileInfo &fileInfo, const bool returnNull) {
	#ifdef KS_NATIVE_KDE
	QString text = fileInfo.baseName();

	if (!fileInfo.exists())
		return returnNull ? 0 : new CommandAction(U_STOCK_ICON("dialog-error"), i18n("File not found: %0").arg(text), this, fileInfo.filePath());

	if (!fileInfo.isFile())
		return returnNull ? 0 : new CommandAction(U_STOCK_ICON("dialog-error"), i18n("File not found: %0").arg(text), this, fileInfo.filePath());
	
	if (KDesktopFile::isDesktopFile(fileInfo.filePath())) {
		U_ICON icon = readDesktopInfo(fileInfo, text);

		return new CommandAction(icon, text, this, fileInfo.filePath());
	}
	else if (fileInfo.isExecutable()) {
		QString iconName =
			(fileInfo.suffix() == "sh")
			? "application-x-executable-script"
			: "application-x-executable";
		U_ICON icon = U_STOCK_ICON(iconName);

		return new CommandAction(icon, text, this, fileInfo.filePath());
	}
	else {
		return returnNull ? 0 : new CommandAction(U_STOCK_ICON("dialog-error"), i18n("Error: %0").arg(text), this, fileInfo.filePath());
	}
	#else
	Q_UNUSED(fileInfo)
	Q_UNUSED(returnNull)

	return 0;
	#endif // KS_NATIVE_KDE
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
		else {
			CommandAction *action = createCommandAction(i, true);
			if (action)
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
	emit statusChanged(true);
}

// private slots

void Extras::showHelp() {
#ifdef KS_NATIVE_KDE
	QDesktopServices::openUrl(QUrl("http://sourceforge.net/apps/mediawiki/kshutdown/index.php?title=Extras"));
#endif // KS_NATIVE_KDE
}

void Extras::slotModify() {
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

	QString command = "dolphin \"" + KGlobal::dirs()->saveLocation("data", "kshutdown/extras") + "\"";
	KRun::run(command, KUrl::List(), U_APP->activeWindow());
#endif // KS_NATIVE_KDE
}

void Extras::updateMenu() {
#ifdef KS_NATIVE_KDE
	m_menu->clear();

	QStringList dirs(KGlobal::dirs()->findDirs("data", "kshutdown/extras"));
	foreach (const QString &i, dirs) {
		U_DEBUG << "Found Extras Directory: " << i U_END;
		createMenu(m_menu, i);
	}
	if (!m_menu->isEmpty())
		m_menu->addSeparator();

	U_ACTION *modifyAction = new U_ACTION(i18n("Add or Remove Commands"), this);
	modifyAction->setIcon(U_STOCK_ICON("configure"));
	connect(modifyAction, SIGNAL(triggered()), this, SLOT(slotModify()));
	m_menu->addAction(modifyAction);
	
	U_ACTION *helpAction = KStandardAction::help(this, SLOT(showHelp()), this);
	helpAction->setShortcut(QKeySequence());
	m_menu->addAction(helpAction);
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
