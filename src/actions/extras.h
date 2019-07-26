// extras.h - Extras
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


#ifndef KSHUTDOWN_EXTRAS_H
#define KSHUTDOWN_EXTRAS_H

#include "../kshutdown.h"

#include <QFileInfo>
#include <QMenu>
#include <QPushButton>

class CommandAction;

class Extras: public Action {
	Q_OBJECT
	friend class CommandAction;
public:
	virtual QString getStringOption() override;
	virtual void setStringOption(const QString &option) override;
	virtual QWidget *getWidget() override;
	virtual bool onAction() override;
	virtual bool onCommandLineOption() override;
	virtual void readConfig(Config *config) override;
	static Extras *self() {
		if (!m_instance)
			m_instance = new Extras();

		return m_instance;
	}
	virtual void updateMainWindow(MainWindow *mainWindow) override;
	virtual void writeConfig(Config *config) override;
private:
	Q_DISABLE_COPY(Extras)
	bool m_examplesCreated = false;
	static Extras *m_instance;
	QMenu *m_menu = nullptr;
	QPushButton *m_menuButton;
	QString m_command;
	explicit Extras();
	CommandAction *createCommandAction(const QFileInfo &fileInfo, const bool returnNull);
	QMenu *createMenu();
	void createMenu(QMenu *parentMenu, const QString &parentDir);
	QString getFilesDirectory();
	QIcon readDesktopInfo(const QFileInfo &fileInfo, QString &text, QString &statusTip);
	void setCommandAction(const CommandAction *command);
private slots:
	void onMenuHovered(QAction *action);
	void showHelp();
	void slotModify();
	void updateMenu();
};

class CommandAction: private QAction {
	Q_OBJECT
	friend class Extras;
private:
	Q_DISABLE_COPY(CommandAction)
	QString m_command;
	explicit CommandAction(const QIcon &icon, const QString &text, QObject *parent, const QFileInfo &fileInfo, const QString &statusTip);
private slots:
	void slotFire();
};

#endif // KSHUTDOWN_EXTRAS_H
