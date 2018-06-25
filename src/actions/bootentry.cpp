// bootentry.cpp - Boot Entry
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

#include "bootentry.h"

#include "../udialog.h"
#include "../utils.h"

#include <QAbstractItemView>
#include <QFileInfo>

// BootEntry

// private:

QString BootEntry::m_problem = "";
QStringList BootEntry::m_list = QStringList();

// public:

QStringList BootEntry::getList() {
	if (!m_list.isEmpty())
		return m_list;

	m_problem = ""; // reset

	QFile grubConfigFile("/boot/grub/grub.cfg");
	QFileInfo grubConfigFileInfo(grubConfigFile);
	if (!grubConfigFileInfo.isReadable()) {
/*
		m_problem += i18n("No permissions to read GRUB menu entries.");
		m_problem += '\n';
		m_problem += grubConfigFile.fileName();
		m_problem += "\n\n";
		m_problem += i18n("Quick fix: %0").arg("sudo chmod 0604 grub.cfg");
*/

		return m_list;
	}

	if (grubConfigFile.open(QFile::ReadOnly)) {
		bool inSubmenu = false;
		QTextStream text(&grubConfigFile);
		QString menuEntryID = "menuentry ";
		QString line;
		while (!(line = text.readLine()).isNull()) {
// FIXME: the parser assumes that the grub.cfg formatting is sane
			if (inSubmenu && (line == "}")/* use non-simplified line */) {
				inSubmenu = false;

				continue; // while
			}

			line = line.simplified();

			if (!inSubmenu && line.startsWith("submenu ")) {
				inSubmenu = true;

				continue; // while
			}

			if (inSubmenu || !line.startsWith(menuEntryID))
				continue; // while
			
			line = line.mid(menuEntryID.length());
			
			QChar quoteChar;
			int quoteStart = -1;
			int quoteEnd = -1;
			for (int i = 0; i < line.length(); i++) {
				QChar c = line[i];
				if (quoteStart == -1) {
					quoteStart = i + 1;
					quoteChar = c;
				}
// FIXME: unescape, quotes (?)
				else if ((quoteEnd == -1) && (c == quoteChar)) {
					quoteEnd = i - 1;
					
					break; // for
				}
			}
			
			if ((quoteStart != -1) && (quoteEnd != -1) && (quoteEnd > quoteStart))
				line = line.mid(quoteStart, quoteEnd);
			else
				qCritical() << "Error parsing menuentry: " << line;

			if (line.contains("(memtest86+")) {
				U_DEBUG << "Skipping Boot Entry: " << line U_END;
			}
			else {
				U_DEBUG << "Adding Boot Entry: " << line U_END;
				m_list << line;
			}
		}
	}
	else {
		qCritical() << "Could not read GRUB menu entries: " << grubConfigFile.fileName();
	}

	if (m_list.isEmpty()) {
/*
		m_problem += i18n("Could not find any boot entries.");
		m_problem += '\n';
*/
		m_problem += grubConfigFile.fileName();
	}

	return m_list;
}

// BootEntryAction

// public:

BootEntryAction::BootEntryAction(const QString &name) :
	QAction(nullptr),
	m_name(name) {
	setText(name);
	connect(this, SIGNAL(triggered()), SLOT(onAction()));
}

// private slots:

void BootEntryAction::onAction() {
	U_DEBUG << m_name U_END;
}

// BootEntryComboBox

// public:

BootEntryComboBox::BootEntryComboBox() :
	QComboBox() {
	//setToolTip(i18n("Select an Operating System you want to use after restart"));
	view()->setAlternatingRowColors(true);

	//addItem('<' + i18n("Default") + '>');
	addItems(BootEntry::getList());
}

// BootEntryMenu

// public:

BootEntryMenu::BootEntryMenu(QWidget *parent) :
	QMenu(i18n("Restart Computer"), parent) {

	QStringList entryList = BootEntry::getList(); // 1.

	if (!BootEntry::getProblem().isEmpty()) { // 2.
		addAction(
			QIcon::fromTheme("dialog-error"), i18n("Error"),
			this, SLOT(onProblem())
		);
	}
	else {
		for (const QString &i : entryList) {
			addAction(new BootEntryAction(i));
		}
	}
}

// private slots:

void BootEntryMenu::onProblem() {
	UDialog::error(this, BootEntry::getProblem());
}
