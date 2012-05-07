// bookmarks.cpp - Bookmarks
// Copyright (C) 2012  Konrad Twardowski
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

#include "bookmarks.h"
#include "mainwindow.h"

// public:

BookmarkAction::BookmarkAction(QWidget *parent, const QString &actionID, const QString &triggerID, const QString &time)
	: U_ACTION(parent),
	m_actionID(actionID),
	m_triggerID(triggerID),
	m_time(time)
{
	connect(this, SIGNAL(triggered()), SLOT(onAction()));
	
	MainWindow *mainWindow = MainWindow::self();
	Action *action = mainWindow->actionHash()[actionID];
	KShutdown::Trigger *trigger = mainWindow->triggerHash()[triggerID];
	
	QString text = "";
	
	if (action) {
		text += action->originalText();
		setIcon(action->icon());
	}
	else
		text += actionID;
	
	text += " - ";

	if (trigger)
		text += trigger->text();
	else
		text += triggerID;
	
	if (!time.isEmpty())
		text += (" - " + time);
	
	setText(text);
}

BookmarkAction::~BookmarkAction() { }

// private slots:

void BookmarkAction::onAction() {
	MainWindow *mainWindow = MainWindow::self();
	mainWindow->setSelectedAction(m_actionID);
	mainWindow->setSelectedTrigger(m_triggerID);
	
	KShutdown::Action *action = mainWindow->getSelectedAction();
	KShutdown::Trigger *trigger = mainWindow->getSelectedTrigger();
	
	if ((action->id() != m_actionID) || (trigger->id() != m_triggerID))
		return;

	if (!m_time.isEmpty()) {
		QTime time = QTime::fromString(m_time, KShutdown::TIME_PARSE_FORMAT);
		DateTimeTriggerBase *dateTimeTrigger = dynamic_cast<DateTimeTriggerBase *>(trigger);
		if (dateTimeTrigger) {
			QDate date = QDate::currentDate();

			// select next day if time is less than current time !!! common code
			//if (absolute && (time < QTime::currentTime()))
			//	date = date.addDays(1);
			dateTimeTrigger->setDateTime(QDateTime(date, time));
		}
	}
	
	//mainWindow->setActive(true);
}

// public:

BookmarksMenu::BookmarksMenu(QWidget *parent)
	: U_MENU(i18n("&Bookmarks"), parent)
{
	connect(this, SIGNAL(aboutToShow()), SLOT(onUpdateMenu()));
}

BookmarksMenu::~BookmarksMenu() { }

// private slots:

void BookmarksMenu::onToggleBookmark() {
}

void BookmarksMenu::onUpdateMenu() {
	bool canAdd = true;

	QString bookmarkName = "!!!";

	U_ACTION *toggleBookmarkAction = new U_ACTION(this);
	connect(toggleBookmarkAction, SIGNAL(triggered()), SLOT(onToggleBookmark()));
	if (canAdd) {
		toggleBookmarkAction->setIcon(U_ICON("bookmark-new"));
		toggleBookmarkAction->setText(i18n("Add Bookmark: %0").arg(bookmarkName));
	}
	else {
		toggleBookmarkAction->setIcon(U_ICON("edit-delete"));
		toggleBookmarkAction->setText(i18n("Remove Bookmark: %0").arg(bookmarkName));
	}

	clear();
	addAction(toggleBookmarkAction);
	addSeparator();
	
	addAction(new BookmarkAction(this, "shutdown", "time-from-now", "15:00"));
	addAction(new BookmarkAction(this, "lock", "no-delay", ""));
}
