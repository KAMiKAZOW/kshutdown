// bootentry.h - Boot Entry
// Copyright (C) 2014  Konrad Twardowski
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

#ifndef KSHUTDOWN_BOOTENTRY_H
#define KSHUTDOWN_BOOTENTRY_H

#include "../pureqt.h"

class BootEntry final: public QObject {
public:
	static QStringList getList();
	static QString getProblem() { return m_problem; }
private:
	Q_DISABLE_COPY(BootEntry)
	static QString m_problem;
	static QStringList m_list;
};

class BootEntryAction: public U_ACTION {
	Q_OBJECT
public:
	explicit BootEntryAction(const QString &name);
private:
	Q_DISABLE_COPY(BootEntryAction)
	QString m_name;
private slots:
	void onAction();
};

class BootEntryComboBox final: public U_COMBO_BOX {
public:
	explicit BootEntryComboBox();
private:
	Q_DISABLE_COPY(BootEntryComboBox)
};

class BootEntryMenu: public U_MENU {
	Q_OBJECT
public:
	explicit BootEntryMenu(QWidget *parent);
private:
	Q_DISABLE_COPY(BootEntryMenu)
private slots:
	void onProblem();
};

#endif // KSHUTDOWN_BOOTENTRY_H
