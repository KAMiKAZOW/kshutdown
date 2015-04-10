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

#include <QFileInfo>

#include "../kshutdown.h"

class CommandAction;

class Extras: public KShutdown::Action {
	Q_OBJECT
	friend class CommandAction;
public:
	virtual QString getStringOption() override;
	virtual void setStringOption(const QString &option) override;
	virtual QWidget *getWidget() override;
	virtual bool onAction() override;
	virtual void readConfig(const QString &group, Config *config) override;
	static Extras *self() {
		if (!m_instance)
			m_instance = new Extras();

		return m_instance;
	}
	virtual void updateMainWindow(MainWindow *mainWindow) override;
	virtual void writeConfig(const QString &group, Config *config) override;
private:
	Q_DISABLE_COPY(Extras)
	static Extras *m_instance;
	QString m_command;
	U_MENU *m_menu;
	U_PUSH_BUTTON *m_menuButton;
	Extras();
	CommandAction *createCommandAction(const QFileInfo &fileInfo, const bool returnNull);
	U_MENU *createMenu();
	void createMenu(U_MENU *parentMenu, const QString &parentDir);
	QString getFilesDirectory() const;
	U_ICON readDesktopInfo(const QFileInfo &fileInfo, QString &text, QString &statusTip);
	void setCommandAction(const CommandAction *command);
private slots:
	void onMenuHovered(QAction *action);
	void showHelp();
	void slotModify();
	void updateMenu();
};

class CommandAction: private U_ACTION {
	Q_OBJECT
	friend class Extras;
private:
	Q_DISABLE_COPY(CommandAction)
	QString m_command;
	explicit CommandAction(const U_ICON &icon, const QString &text, QObject *parent, const QFileInfo &fileInfo, const QString &statusTip);
private slots:
	void slotFire();
};

#endif // KSHUTDOWN_EXTRAS_H
