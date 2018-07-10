// bookmarks.h - Bookmarks
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

#ifndef KSHUTDOWN_BOOKMARKS_H
#define KSHUTDOWN_BOOKMARKS_H

#include "version.h" // HACK: check KS_* defines early

#include "config.h"
#include "kshutdown.h"
#include "pureqt.h"

#include <QMenu>

class BookmarksMenu;

class BookmarkAction: public QAction {
	Q_OBJECT
public:
	explicit BookmarkAction(
		const QString &text,
		BookmarksMenu *menu,
		const QString &actionID, const QString &triggerID,
		const QString &actionOption, const QString &triggerOption
	);
	virtual ~BookmarkAction();
	inline QString originalText() const { return m_originalText; }
private:
	Q_DISABLE_COPY(BookmarkAction)
	friend class BookmarksMenu;
	bool m_confirmAction = true;
	bool m_userText;
	QString m_actionID;
	QString m_actionOption;
	QString m_originalText;
	QString m_triggerID;
	QString m_triggerOption;
private slots:
	void onAction();
};

class BookmarksMenu: public QMenu {
	Q_OBJECT
public:
	explicit BookmarksMenu(QWidget *parent);
	virtual ~BookmarksMenu();
	QString makeText(Action *action, Trigger *trigger, const QString &actionOption, const QString &triggerOption) const;
private:
	Q_DISABLE_COPY(BookmarksMenu)
	QList<BookmarkAction *> *m_list;
	BookmarkAction *findBookmark(Action *action, Trigger *trigger);
	QList<BookmarkAction *> *list();
	void syncConfig();
private slots:
	void onAddBookmark();
	void onRemoveBookmark();
	void onMenuHovered(QAction *action);
	void onUpdateMenu();
};

#endif // KSHUTDOWN_BOOKMARKS_H
