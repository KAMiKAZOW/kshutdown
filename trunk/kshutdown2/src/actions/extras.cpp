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

#include "extras.h"

#include "../config.h"
#include "../mainwindow.h"
#include "../password.h"
#include "../utils.h"

#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QPointer>
#include <QUrl>

#ifdef KS_KF5
	#include <QSettings>
#endif // KS_KF5

#ifdef KS_PURE_QT
	#include <QProcess>
	#include <QSettings>
#endif // KS_PURE_QT

#ifdef KS_NATIVE_KDE
	#include <KStandardAction>
	#ifndef KS_KF5
		#include <KDesktopFile> // #kde4
		#include <KRun> // #kde4
		#include <KService> // #kde4
		#include <KStandardDirs> // #kde4
	#endif // !KS_KF5
#endif // KS_NATIVE_KDE

// Extras

// private

Extras *Extras::m_instance = nullptr;

// public

QString Extras::getStringOption() { return m_command; }

void Extras::setStringOption(const QString &option) {
	m_command = option;
	if (m_command.isEmpty()) {
		setCommandAction(nullptr);
	}
	else {
		QFileInfo fileInfo(m_command);
		setCommandAction(createCommandAction(fileInfo, false));
	}
}

QWidget *Extras::getWidget() { return m_menuButton; }

bool Extras::onAction() {
	QFileInfo fileInfo(m_command);
	QString path = fileInfo.filePath();

#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	if (KDesktopFile::isDesktopFile(path)) {
		KDesktopFile desktopFile(m_command);
		KService service(&desktopFile);
		
		if (service.exec().isEmpty()) { // krazy:exclude=crashy
			m_error = i18n("Invalid \"Extras\" command");
		
			return false;
		}
		
		// HACK: chmod +x to avoid "antivirus" security dialog
		if (!fileInfo.isExecutable()) {
			U_DEBUG << "Setting executable permission to: " << path U_END;
			QFile::setPermissions(path, fileInfo.permissions() | QFile::ExeOwner);
		}
		
// FIXME: error detection, double error message box
		if (KRun::run(service, KUrl::List(), qApp->activeWindow()))
			return true;
		
		m_error = i18n("Cannot execute \"Extras\" command");
		
		return false;
	}
	else {
		if (KRun::run("\"" + m_command + "\"", KUrl::List(), qApp->activeWindow()))
			return true;
		
		m_error = i18n("Cannot execute \"Extras\" command");
		
		return false;
	}
#else
	bool ok = false;
	QString suffix = fileInfo.suffix();

	if (suffix == "desktop") {
		QSettings settings(path, QSettings::IniFormat);
		settings.beginGroup("Desktop Entry");
		QString exec = settings.value("Exec", "").toString();
		//U_DEBUG << exec U_END;
		if (!exec.isEmpty()) {
			auto *process = new QProcess(this);
			QString dir = settings.value("Path", "").toString();
			//U_DEBUG << dir U_END;
			if (!dir.isEmpty())
				process->setWorkingDirectory(dir);
			process->start(exec);
			ok = process->waitForStarted(5000);
		}
		settings.endGroup();
	}
	#ifdef Q_OS_WIN32
	else if (suffix == "lnk") { // shortcut
		ok = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
	#endif // Q_OS_WIN32
	else if (fileInfo.isExecutable()) {
		ok = (QProcess::execute(path, QStringList()) == 0);
	}
	else {
		ok = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}

	if (!ok)
		m_error = i18n("Cannot execute \"Extras\" command");

	return ok;
#endif // KS_NATIVE_KDE
}

void Extras::readConfig(Config *config) {
	// do not override command set via "e" command line option
	if (m_command.isNull())
		setStringOption(config->read("Command", "").toString());
}

void Extras::updateMainWindow(MainWindow *mainWindow) {
	if (isEnabled() && getStringOption().isEmpty()) {
		mainWindow->okCancelButton()->setEnabled(false);
		mainWindow->infoWidget()->setText(
			"<qt>" +
			i18n("Please select an Extras command<br>from the menu above.") +
			"</qt>",
			InfoWidget::Type::Warning
		);
	}
	else {
		Action::updateMainWindow(mainWindow);
	}
}

void Extras::writeConfig(Config *config) {
	config->write("Command", m_command);
}

// private

Extras::Extras() :
	Action(i18n("Extras"), "rating", "extras"),
	m_command(QString::null) {

	setCanBookmark(true);
	setMenu(createMenu());
	setShowInMenu(false);
	m_menuButton = new QPushButton();
	m_menuButton->setMenu(menu());
	
	//setCommandAction(0);

	addCommandLineArg("e", "extra");
}

CommandAction *Extras::createCommandAction(const QFileInfo &fileInfo, const bool returnNull) {
	QString text = fileInfo.fileName();

	if (!fileInfo.exists() || !fileInfo.isFile())
		return returnNull ? nullptr : new CommandAction(QIcon::fromTheme("dialog-error"), i18n("File not found: %0").arg(text), this, fileInfo, "");

	QString statusTip = "";

	if (fileInfo.suffix() == "desktop") {
		QIcon icon = readDesktopInfo(fileInfo, text, statusTip);

		return new CommandAction(icon, text, this, fileInfo, statusTip);
	}

	statusTip = fileInfo.filePath();
	statusTip = Utils::trim(statusTip, 100);

	if (fileInfo.isExecutable()) {
		QString iconName =
			(fileInfo.suffix() == "sh")
			? "application-x-executable-script"
			: "application-x-executable";

		return new CommandAction(QIcon::fromTheme(iconName), text, this, fileInfo, statusTip);
	}

	return new CommandAction(QIcon(), text, this, fileInfo, statusTip);
}

QMenu *Extras::createMenu() {
	m_menu = new QMenu();

	connect(m_menu, SIGNAL(hovered(QAction *)), this, SLOT(onMenuHovered(QAction *)));
	m_menu->setToolTipsVisible(true);

	connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));

	return m_menu;
}

void Extras::createMenu(QMenu *parentMenu, const QString &parentDir) {
	QDir dir(parentDir);
	QFileInfoList entries = dir.entryInfoList(
		QDir::Dirs | QDir::Files,
		QDir::DirsFirst | QDir::Name
	);

	foreach (QFileInfo i, entries) {
		QString fileName = i.fileName();
		if (i.isDir() && (fileName != ".") && (fileName != "..")) {
			QString dirProperties = i.filePath() + "/.directory";

			QMenu *dirMenu;

			QString text = fileName; // do not use "baseName" here
			if (QFile::exists(dirProperties)) {
				QString statusTip = "";
				QIcon icon = readDesktopInfo(dirProperties, text, statusTip);

				dirMenu = new QMenu(text); // use "text" from ".directory" file
				dirMenu->setIcon(icon);
			}
			else {
				dirMenu = new QMenu(text);
			}

			connect(dirMenu, SIGNAL(hovered(QAction *)), this, SLOT(onMenuHovered(QAction *)));
			dirMenu->setToolTipsVisible(true);

			createMenu(dirMenu, i.filePath()); // recursive scan

			if (dirMenu->isEmpty()) {
				auto *emptyAction = new QAction(dirMenu);
				emptyAction->setEnabled(false);
				emptyAction->setText('(' + i18n("Empty") + ')');
				dirMenu->addAction(emptyAction);
			}

			parentMenu->addMenu(dirMenu);
		}
		else {
			CommandAction *action = createCommandAction(i, true);
			if (action)
				parentMenu->addAction(action);
		}
	}
}

QString Extras::getFilesDirectory() const {
#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	return KGlobal::dirs()->saveLocation("data", "kshutdown/extras");
#else
	QDir dir;
	if (Config::isPortable()) {
		dir = QDir(QApplication::applicationDirPath() + QDir::separator() + "extras");
	}
	else {
		dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "extras");
	}

	//U_DEBUG << "Extras dir: " << dir U_END;
	// CREDITS: http://stackoverflow.com/questions/6232631/how-to-recursively-create-a-directory-in-qt ;-)
	if (!dir.mkpath(dir.path())) {
		U_DEBUG << "Could not create Config dir" U_END;
	}
	
	return dir.path();
#endif // KS_NATIVE_KDE
}

QIcon Extras::readDesktopInfo(const QFileInfo &fileInfo, QString &text, QString &statusTip) {
	QString desktopName = "";
	QString desktopComment = "";
	QString exec = "";
	QString icon = "";

#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	KDesktopFile desktopFile(fileInfo.filePath());
	desktopName = desktopFile.readName();
	desktopComment = desktopFile.readComment();
	exec = desktopFile.desktopGroup().readEntry("Exec", "");
	icon = desktopFile.readIcon();
#else
	QSettings settings(fileInfo.filePath(), QSettings::IniFormat);
	settings.beginGroup("Desktop Entry");
	desktopName = settings.value("Name", "").toString();
	desktopComment = settings.value("Comment", "").toString();
	exec = settings.value("Exec", "").toString();
	icon = settings.value("Icon", "").toString();
	settings.endGroup();
#endif // KS_NATIVE_KDE

	if (!exec.isEmpty())
		statusTip = Utils::trim(exec, 100);

	if (!desktopComment.isEmpty()) {
		if (!statusTip.isEmpty())
			statusTip += '\n';
		statusTip += Utils::trim(desktopComment, 100);
	}

	if (!desktopName.isEmpty())
		text = desktopName;

	text = Utils::trim(text, 100);

	return QIcon::fromTheme(icon);
}

void Extras::setCommandAction(const CommandAction *command) {
	if (command) {
		m_command = command->m_command;
		
		//U_DEBUG << "Extras::setCommandAction: " << m_command U_END;
		m_menuButton->setIcon(QIcon(command->icon()));
		m_menuButton->setText(command->text());
		//m_status = (originalText() + " - " + command->text());
	}
	else {
		m_command = QString::null;
	
		//U_DEBUG << "Extras::setCommandAction: NULL" U_END;
		m_menuButton->setIcon(QIcon::fromTheme("arrow-down"));
		m_menuButton->setText(i18n("Select a command..."));
		//m_status = QString::null;
	}
	emit statusChanged(true);
}

// private slots

void Extras::onMenuHovered(QAction *action) {
	Utils::showMenuToolTip(action);
}

void Extras::showHelp() {
	QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/kshutdown/wiki/Extras/"));
}

void Extras::slotModify() {
	if (!PasswordDialog::authorizeSettings(MainWindow::self()))
		return;

	QString text =
		"<qt>" +
		i18n("Use context menu to add/edit/remove actions.") +
		"<ul>" +
			"<li>" + i18n("Use <b>Context Menu</b> to create a new link to application (action)") + "</li>" +
// TODO: built-in "New Folder" menu item
			"<li>" + i18n("Use <b>Create New|Folder...</b> to create a new submenu") + "</li>" +
			"<li>" + i18n("Use <b>Properties</b> to change icon, name, or command") + "</li>" +
		"</ul>" +
		"</qt>";

	#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	KMessageBox::information(0, text, originalText(), "ModifyExtras");
	#else
	Config *config = Config::user();
	config->beginGroup("KShutdown Action extras");
	bool showInfo = config->read("Show Info", true).toBool();
	config->endGroup();
	
	if (showInfo) {
// TODO: KMessageBox
		QPointer<QMessageBox> message = new QMessageBox( // krazy:exclude=qclasses
			QMessageBox::Information, // krazy:exclude=qclasses
			originalText(),
			text,
			QMessageBox::Ok // krazy:exclude=qclasses
		);

		QCheckBox *doNotShowAgainCheckBox = new QCheckBox(i18n("Do not show this message again"));
		message->setCheckBox(doNotShowAgainCheckBox);

		message->exec();

		if (doNotShowAgainCheckBox->isChecked()) {
			config->beginGroup("KShutdown Action extras");
			config->write("Show Info", false);
			config->endGroup();
		}

		delete message;
	}
	#endif // KS_NATIVE_KDE
	QUrl url = QUrl::fromLocalFile(getFilesDirectory());
	QDesktopServices::openUrl(url);
}

void Extras::updateMenu() {
	m_menu->clear();

	#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	QStringList dirs(KGlobal::dirs()->findDirs("data", "kshutdown/extras"));
	foreach (const QString &i, dirs) {
		U_DEBUG << "Found Extras Directory: " << i U_END;
		createMenu(m_menu, i);
	}
	#else
		#ifdef KS_KF5
		QStringList dirs = QStandardPaths::locateAll(
			QStandardPaths::GenericDataLocation,
			"kshutdown/extras",
			QStandardPaths::LocateDirectory
		);
		foreach (const QString &i, dirs) {
			U_DEBUG << "Found Extras Directory: " << i U_END;
			createMenu(m_menu, i);
		}
		#else
		createMenu(m_menu, getFilesDirectory());
		#endif // KS_KF5
	#endif // KS_NATIVE_KDE
	
	if (!m_menu->isEmpty())
		m_menu->addSeparator();

	auto *modifyAction = new QAction(i18n("Add or Remove Commands"), this);
	modifyAction->setIcon(QIcon::fromTheme("configure"));
	connect(modifyAction, SIGNAL(triggered()), this, SLOT(slotModify()));
	m_menu->addAction(modifyAction);
	
	#ifdef KS_NATIVE_KDE
	auto *helpAction = KStandardAction::helpContents(this, SLOT(showHelp()), this);
	#else
	auto *helpAction = new QAction(i18n("Help"), this);
	connect(helpAction, SIGNAL(triggered()), SLOT(showHelp()));
	#endif // KS_NATIVE_KDE
	helpAction->setShortcut(QKeySequence());
	m_menu->addAction(helpAction);
}

// CommandAction

// private

CommandAction::CommandAction(
	const QIcon &icon,
	const QString &text,
	QObject *parent,
	const QFileInfo &fileInfo,
	const QString &statusTip
) :
	QAction(icon, text, parent),
	m_command(fileInfo.filePath()) {

	setIconVisibleInMenu(true);
	setStatusTip(statusTip);
	setToolTip(statusTip);
	connect(this, SIGNAL(triggered()), SLOT(slotFire()));
}

// private slots

void CommandAction::slotFire() {
	Extras::self()->setCommandAction(this);
}
