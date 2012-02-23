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

class BookmarksButton: public U_PUSH_BUTTON {
	Q_OBJECT
public:
	explicit BookmarksButton(QWidget *parent);
	virtual ~BookmarksButton();
private:
	U_ACTION *m_toggleBookmarkAction;
	U_MENU *m_menu;
	Q_DISABLE_COPY(BookmarksButton)
private slots:
	void onUpdateMenu();
};

#endif // KSHUTDOWN_BOOKMARKS_H
