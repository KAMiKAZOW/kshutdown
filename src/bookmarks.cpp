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
#include "utils.h"

// public:

BookmarkAction::BookmarkAction(
	BookmarksMenu *menu,
	const QString &actionID, const QString &triggerID,
	const QString &actionOption, const QString &triggerOption
)
	: U_ACTION(0), // no owner, because clear() will delete action

	m_actionID(actionID),
	m_actionOption(actionOption),
	m_triggerID(triggerID),
	m_triggerOption(triggerOption)
{
	connect(this, SIGNAL(triggered()), SLOT(onAction()));
	
	MainWindow *mainWindow = MainWindow::self();
	Action *action = mainWindow->actionHash()[actionID];
	KShutdown::Trigger *trigger = mainWindow->triggerHash()[triggerID];
	
	if (action)
		setIcon(action->icon());
		
	setText(menu->makeText(action, trigger, actionOption, triggerOption));
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

	action->setStringOption(m_actionOption);
	trigger->setStringOption(m_triggerOption);
	
	//mainWindow->setActive(true);
}

// public:

BookmarksMenu::BookmarksMenu(QWidget *parent)
	: U_MENU(i18n("&Bookmarks"), parent),
	m_list(0)
{
	connect(this, SIGNAL(aboutToShow()), SLOT(onUpdateMenu()));
}

BookmarksMenu::~BookmarksMenu() { }

QString BookmarksMenu::makeText(KShutdown::Action *action, KShutdown::Trigger *trigger, const QString &actionOption, const QString &triggerOption) const {
	QString text = "";
	
	if (action) {
		text += action->originalText();

		QString option = actionOption.isEmpty() ? action->getStringOption() : actionOption;
		if (!option.isEmpty())
			text += (" - " + Utils::trim(option, 30));
	}
	else {
		text += "?";
	}
	
	text += " - ";

	if (trigger) {
		text += trigger->text();

		QString option = triggerOption.isEmpty() ? trigger->getStringOption() : triggerOption;
		if (!option.isEmpty())
			text += (" - " + Utils::trim(option, 30));
	}
	else {
		text += "?";
	}

	return text;
}

// private:

int BookmarksMenu::findBookmark(KShutdown::Action *action, KShutdown::Trigger *trigger) {
	int index = 0;
	QString actionOption = action->getStringOption();
	QString triggerOption = trigger->getStringOption();
	
	foreach (BookmarkAction *i, *list()) {
		if (
			(action->id() == i->m_actionID) && (actionOption == i->m_actionOption) &&
			(trigger->id() == i->m_triggerID) && (triggerOption == i->m_triggerOption)
		) {
			return index;
		}
		
		index++;
	}
	
	return -1;
}

QList<BookmarkAction *> *BookmarksMenu::list() {
	if (m_list)
		return m_list;

	m_list = new QList<BookmarkAction *>();

	Config *config = Config::user();
	config->beginGroup("Bookmarks");
	int count = config->read("Count", 0).toInt();
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			m_list->append(new BookmarkAction(
				this,
				config->read("Action " + QString::number(i), "").toString(),
				config->read("Trigger " + QString::number(i), "").toString(),
				config->read("Action Option " + QString::number(i), "").toString(),
				config->read("Trigger Option " + QString::number(i), "").toString()
			));
		}
	}
	config->endGroup();
	
	return m_list;
}

void BookmarksMenu::syncConfig() {
	Config *config = Config::user();
	
	config->beginGroup("Bookmarks");
	config->removeAllKeys();
	config->write("Count", list()->count());
	
	int index = 0;
	foreach (BookmarkAction *i, *list()) {
		config->write("Action " + QString::number(index), i->m_actionID);
		config->write("Action Option " + QString::number(index), i->m_actionOption);
		config->write("Trigger " + QString::number(index), i->m_triggerID);
		config->write("Trigger Option " + QString::number(index), i->m_triggerOption);
		index++;
	}
	
	config->endGroup();
	
	config->sync();
}

// private slots:

// sort alphabetically, by action text
bool compareBookmarkAction(const BookmarkAction *a1, const BookmarkAction *a2) {
	return QString::compare(a1->text(), a2->text(), Qt::CaseInsensitive) < 0;
}

void BookmarksMenu::onAddBookmark() {
	MainWindow *mainWindow = MainWindow::self();
	KShutdown::Action *action = mainWindow->getSelectedAction();
	KShutdown::Trigger *trigger = mainWindow->getSelectedTrigger();

	BookmarkAction *bookmark = new BookmarkAction(
		this,
		action->id(),
		trigger->id(),
		action->getStringOption(),
		trigger->getStringOption()
	);
	list()->append(bookmark);
	
	qSort(list()->begin(), list()->end(), compareBookmarkAction);
	
	syncConfig();
}

void BookmarksMenu::onRemoveBookmark() {
	MainWindow *mainWindow = MainWindow::self();
	KShutdown::Action *action = mainWindow->getSelectedAction();
	KShutdown::Trigger *trigger = mainWindow->getSelectedTrigger();

	int i = findBookmark(action, trigger);
	if (i != -1) {
		list()->removeAt(i);
		syncConfig();
	}
}

void BookmarksMenu::onUpdateMenu() {
	MainWindow *mainWindow = MainWindow::self();
	KShutdown::Action *action = mainWindow->getSelectedAction();
	KShutdown::Trigger *trigger = mainWindow->getSelectedTrigger();

	QString bookmarkName = makeText(action, trigger, QString::null, QString::null);

	U_ACTION *toggleBookmarkAction = new U_ACTION(this);
	int i = findBookmark(action, trigger);
	if (i == -1) {
		toggleBookmarkAction->setEnabled(action->canBookmark() && trigger->canBookmark());
		
		toggleBookmarkAction->setIcon(U_ICON("bookmark-new"));
		toggleBookmarkAction->setText(i18n("Add: %0").arg(bookmarkName));
		connect(toggleBookmarkAction, SIGNAL(triggered()), SLOT(onAddBookmark()));
	}
	else {
		toggleBookmarkAction->setIcon(U_ICON("edit-delete"));
		toggleBookmarkAction->setText(i18n("Remove: %0").arg(bookmarkName));
		connect(toggleBookmarkAction, SIGNAL(triggered()), SLOT(onRemoveBookmark()));
	}

	clear();
	addAction(toggleBookmarkAction);
	
	if (!list()->isEmpty()) {
		addSeparator();
		foreach (BookmarkAction *i, *list()) {
// TODO: select active bookmark (radio button menu item)
			addAction(i);
		}
	}
}
