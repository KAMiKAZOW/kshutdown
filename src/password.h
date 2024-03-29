// password.h - A basic password protection
// Copyright (C) 2011  Konrad Twardowski
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

#ifndef KSHUTDOWN_PASSWORD_H
#define KSHUTDOWN_PASSWORD_H

#include "udialog.h"

#include <QCheckBox>
#include <QListWidget>

class InfoWidget;

class PasswordDialog: public UDialog {
public:
	explicit PasswordDialog(QWidget *parent);
	virtual ~PasswordDialog();
	void apply();
	static bool authorize(QWidget *parent, const QString &caption, const QString &userAction);
	static bool authorizeSettings(QWidget *parent);
	static void clearPassword(QString &password);
	static QString toHash(const QString &password);
private:
	Q_DISABLE_COPY(PasswordDialog)
	InfoWidget *m_status;
	QLineEdit *m_confirmPassword;
	QLineEdit *m_password;
	void updateStatus();
};

class PasswordPreferences: public QWidget {
	Q_OBJECT
public:
	explicit PasswordPreferences(QWidget *parent);
	virtual ~PasswordPreferences() = default;
	void apply();
private:
	Q_DISABLE_COPY(PasswordPreferences)
	int m_configKeyRole;
	QCheckBox *m_enablePassword;
	QListWidget *m_userActionList;
	QListWidgetItem *addItem(const QString &key, const QString &text, const QIcon &icon);
	void updateWidgets(const bool passwordEnabled);
private slots:
	void onEnablePassword(bool checked);
};

#endif // KSHUTDOWN_PASSWORD_H
