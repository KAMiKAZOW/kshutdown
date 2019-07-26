// udialog.h - A dialog base
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

#ifndef KSHUTDOWN_UDIALOG_H
#define KSHUTDOWN_UDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class UDialog: public QDialog {
	Q_OBJECT
public:
	explicit UDialog(QWidget *parent, const QString &windowTitle, const bool simple);
	virtual ~UDialog() = default;
	inline QPushButton *acceptButton() { return m_acceptButton; }
	QDialogButtonBox *buttonBox() { return m_dialogButtonBox; }
	inline QVBoxLayout *mainLayout() { return m_mainLayout; }
	inline QVBoxLayout *rootLayout() { return m_rootLayout; }

	// messages

	static bool confirm(QWidget *parent, const QString &text);
	static void error(QWidget *parent, const QString &text);
	static void info(QWidget *parent, const QString &text);

	// misc

	static void plainText(QWidget *parent, const QString &text, const QString &windowTitle);

private:
	Q_DISABLE_COPY(UDialog)
	QDialogButtonBox *m_dialogButtonBox;
	QPushButton *m_acceptButton;
	QVBoxLayout *m_mainLayout;
	QVBoxLayout *m_rootLayout;
};

#endif // KSHUTDOWN_UDIALOG_H
