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

#include "pureqt.h"

class BookmarkAction: public U_ACTION {
	Q_OBJECT
public:
	explicit BookmarkAction(QWidget *parent, const QString &actionID, const QString &triggerID, const QString &time);
	virtual ~BookmarkAction();
private:
	Q_DISABLE_COPY(BookmarkAction)
	QString m_actionID;
	QString m_triggerID;
	QString m_time;
private slots:
	void onAction();
};

class BookmarksMenu: public U_MENU {
	Q_OBJECT
public:
	explicit BookmarksMenu(QWidget *parent);
	virtual ~BookmarksMenu();
private:
	Q_DISABLE_COPY(BookmarksMenu)
private slots:
	void onToggleBookmark();
	void onUpdateMenu();
};

#endif // KSHUTDOWN_BOOKMARKS_H
