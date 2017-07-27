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

#include "config.h"
#include "mainwindow.h"
#include "password.h"
#include "utils.h"

#include <QFormLayout>
#include <QPointer>
#include <QPushButton>

// public:

BookmarkAction::BookmarkAction(
	const QString &text,
	BookmarksMenu *menu,
	const QString &actionID, const QString &triggerID,
	const QString &actionOption, const QString &triggerOption
)
	: U_ACTION(nullptr), // no owner, because clear() will delete action

	m_actionID(actionID),
	m_actionOption(actionOption),
	m_triggerID(triggerID),
	m_triggerOption(triggerOption)
{
	connect(this, SIGNAL(triggered()), SLOT(onAction()));
	
	MainWindow *mainWindow = MainWindow::self();
	auto *action = mainWindow->actionHash()[actionID];
	auto *trigger = mainWindow->triggerHash()[triggerID];
	
	if (action)
		setIcon(action->icon());
	setIconVisibleInMenu(true);
	
	QString actionText = menu->makeText(action, trigger, actionOption, triggerOption);
	m_userText = !text.isEmpty() && (text != actionText);
	m_originalText = m_userText ? text : actionText;

	if (m_userText) {
		setStatusTip(actionText);
		#if QT_VERSION >= 0x050100
		setToolTip(actionText);
		#endif // QT_VERSION
	}

	setText(m_originalText);
}

BookmarkAction::~BookmarkAction() = default;

// private slots:

void BookmarkAction::onAction() {
	MainWindow *mainWindow = MainWindow::self();
	mainWindow->setSelectedAction(m_actionID);
	mainWindow->setSelectedTrigger(m_triggerID);
	
	auto *action = mainWindow->getSelectedAction();
	auto *trigger = mainWindow->getSelectedTrigger();
	
	if ((action->id() != m_actionID) || (trigger->id() != m_triggerID))
		return;

	action->setStringOption(m_actionOption);
	trigger->setStringOption(m_triggerOption);
	
	if (!m_confirmAction)
		mainWindow->setActive(true);
}

// public:

BookmarksMenu::BookmarksMenu(QWidget *parent)
	: U_MENU(i18n("&Bookmarks"), parent),
	m_list(nullptr)
{
	connect(this, SIGNAL(aboutToShow()), SLOT(onUpdateMenu()));

	connect(this, SIGNAL(hovered(QAction *)), SLOT(onMenuHovered(QAction *)));
	#if QT_VERSION >= 0x050100
	setToolTipsVisible(true);
	#endif // QT_VERSION
}

BookmarksMenu::~BookmarksMenu() = default;

QString BookmarksMenu::makeText(KShutdown::Action *action, KShutdown::Trigger *trigger, const QString &actionOption, const QString &triggerOption) const {
	QString text = "";
	
	if (action) {
		text += action->originalText();

		QString option = actionOption.isEmpty() ? action->getStringOption() : actionOption;
		if (!option.isEmpty())
			text += (" - " + Utils::trim(option, 30));
	}
	else {
		text += '?';
	}
	
	text += " - ";

	if (trigger) {
		text += trigger->text();

		QString option = triggerOption.isEmpty() ? trigger->getStringOption() : triggerOption;
		if (!option.isEmpty())
			text += (" - " + Utils::trim(option, 30));
	}
	else {
		text += '?';
	}

	return text;
}

// private:

BookmarkAction *BookmarksMenu::findBookmark(KShutdown::Action *action, KShutdown::Trigger *trigger) {
	QString actionOption = action->getStringOption();
	QString triggerOption = trigger->getStringOption();
	
	foreach (BookmarkAction *i, *list()) {
		if (
			(action->id() == i->m_actionID) && (actionOption == i->m_actionOption) &&
			(trigger->id() == i->m_triggerID) && (triggerOption == i->m_triggerOption)
		) {
			return i;
		}
	}
	
	return nullptr;
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
			QString index = QString::number(i);
			auto *bookmarkAction = new BookmarkAction(
				config->read("Text " + index, "").toString(),
				this,
				config->read("Action " + index, "").toString(),
				config->read("Trigger " + index, "").toString(),
				config->read("Action Option " + index, "").toString(),
				config->read("Trigger Option " + index, "").toString()
			);
			bookmarkAction->m_confirmAction = config->read("Confirm Action " + index, true).toBool();
			m_list->append(bookmarkAction);
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
	
	int i = 0;
	foreach (BookmarkAction *bookmarkAction, *list()) {
		QString index = QString::number(i);
		config->write("Text " + index, bookmarkAction->m_userText ? bookmarkAction->originalText() : "");
		config->write("Action " + index, bookmarkAction->m_actionID);
		config->write("Action Option " + index, bookmarkAction->m_actionOption);
		config->write("Trigger " + index, bookmarkAction->m_triggerID);
		config->write("Trigger Option " + index, bookmarkAction->m_triggerOption);
		config->write("Confirm Action " + index, bookmarkAction->m_confirmAction);
		i++;
	}
	
	config->endGroup();
	
	config->sync();
}

// private slots:

// sort alphabetically, by original action text
bool compareBookmarkAction(const BookmarkAction *a1, const BookmarkAction *a2) {
	return QString::compare(a1->originalText(), a2->originalText(), Qt::CaseInsensitive) < 0;
}

void BookmarksMenu::onAddBookmark() {
	if (!PasswordDialog::authorizeSettings(MainWindow::self()))
		return;

	MainWindow *mainWindow = MainWindow::self();
	auto *action = mainWindow->getSelectedAction();
	auto *trigger = mainWindow->getSelectedTrigger();

	QPointer<UDialog> dialog = new UDialog(mainWindow, i18n("Add Bookmark"), false);
	dialog->acceptButton()->setText(i18n("Add"));

	U_LINE_EDIT *nameField = new U_LINE_EDIT(makeText(action, trigger, QString::null, QString::null));
	#if QT_VERSION >= 0x050200
	nameField->setClearButtonEnabled(true);
	#endif

	auto *confirmActionField = new QCheckBox(i18n("Confirm Action"));
	confirmActionField->setChecked(true);

	auto *layout = new QFormLayout();
	layout->setLabelAlignment(Qt::AlignRight);
	layout->setVerticalSpacing(20_px);
	layout->addRow(i18n("Name:"), nameField);
	layout->addRow(confirmActionField);
	dialog->mainLayout()->addLayout(layout);

	nameField->setFocus();
	nameField->selectAll();

	if (dialog->exec()) {
		BookmarkAction *bookmark = new BookmarkAction(
			nameField->text().trimmed(),
			this,
			action->id(),
			trigger->id(),
			action->getStringOption(),
			trigger->getStringOption()
		);
		bookmark->m_confirmAction = confirmActionField->isChecked();
		list()->append(bookmark);
		
		qSort(list()->begin(), list()->end(), compareBookmarkAction);
		
		syncConfig();
	}
	
	delete dialog;
}

void BookmarksMenu::onRemoveBookmark() {
	if (!PasswordDialog::authorizeSettings(MainWindow::self()))
		return;

	MainWindow *mainWindow = MainWindow::self();
	auto *action = mainWindow->getSelectedAction();
	auto *trigger = mainWindow->getSelectedTrigger();

	auto bookmark = findBookmark(action, trigger);
	if (bookmark) {
		list()->removeOne(bookmark);
		syncConfig();
	}
}

void BookmarksMenu::onMenuHovered(QAction *action) {
	Utils::showMenuToolTip(action);
}

void BookmarksMenu::onUpdateMenu() {
	MainWindow *mainWindow = MainWindow::self();
	auto *action = mainWindow->getSelectedAction();
	auto *trigger = mainWindow->getSelectedTrigger();

	U_ACTION *toggleBookmarkAction = new U_ACTION(this);
	auto *bookmark = findBookmark(action, trigger);
	if (!bookmark) {
		toggleBookmarkAction->setEnabled(action->canBookmark() && trigger->canBookmark());
		
		toggleBookmarkAction->setIcon(U_ICON("bookmark-new"));
		QString text = makeText(action, trigger, QString::null, QString::null);
		toggleBookmarkAction->setText(i18n("Add: %0").arg(text));
		connect(toggleBookmarkAction, SIGNAL(triggered()), SLOT(onAddBookmark()));
	}
	else {
		toggleBookmarkAction->setIcon(U_ICON("edit-delete"));
		toggleBookmarkAction->setText(i18n("Remove: %0").arg(bookmark->originalText()));
		connect(toggleBookmarkAction, SIGNAL(triggered()), SLOT(onRemoveBookmark()));
	}

	clear();
	addAction(toggleBookmarkAction);
	
	if (!list()->isEmpty()) {
		addSeparator();

		QString actionOption = action->getStringOption();
		QString triggerOption = trigger->getStringOption();

		QActionGroup *group = new QActionGroup(this);
		foreach (BookmarkAction *i, *list()) {
			bool current =
				(action->id() == i->m_actionID) && (actionOption == i->m_actionOption) &&
				(trigger->id() == i->m_triggerID) && (triggerOption == i->m_triggerOption);

			i->setActionGroup(group);
			i->setCheckable(true);
			i->setChecked(current);
			addAction(i);
		}
	}
}
