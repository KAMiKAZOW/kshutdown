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

#include "pureqt.h"

class InfoWidget;

class QCheckBox;

class PasswordDialog: public U_DIALOG {
	Q_OBJECT
public:
	explicit PasswordDialog(QWidget *parent, const bool newPasswordMode);
	virtual ~PasswordDialog();
	void apply();
	static bool authorize(QWidget *parent, const QString &caption, const QString &userAction);
	static QString toHash(const QString &password);
private:
	Q_DISABLE_COPY(PasswordDialog)
	bool m_newPasswordMode;
	InfoWidget *m_status;
	QLabel *m_caption;
	QLineEdit *m_confirmPassword;
	QLineEdit *m_password;
	QPushButton *m_acceptButton;
	void updateStatus();
private slots:
	void onConfirmPasswordChange(const QString &text);
	void onPasswordChange(const QString &text);
};

class PasswordPreferences: public QWidget {
	Q_OBJECT
public:
	explicit PasswordPreferences(QWidget *parent);
	virtual ~PasswordPreferences();
	void apply();
private:
	Q_DISABLE_COPY(PasswordPreferences)
	QCheckBox *m_enablePassword;
	U_LIST_WIDGET *m_userActionList;
	void updateWidgets(const bool passwordEnabled);
private slots:
	void onEnablePassword(int state);
};

#endif // KSHUTDOWN_PASSWORD_H
