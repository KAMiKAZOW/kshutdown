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

// public:

BookmarksButton::BookmarksButton(QWidget *parent) :
	U_PUSH_BUTTON(parent)
{
	m_toggleBookmarkAction = new U_ACTION(this);

	m_menu = new U_MENU();
	connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(onUpdateMenu()));

	setIcon(U_ICON("bookmarks"));
	setMenu(m_menu);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	setToolTip(i18n("Bookmarks"));
}

BookmarksButton::~BookmarksButton() { }

// private slots:

void BookmarksButton::onUpdateMenu() {
	m_menu->clear();
	m_menu->addAction(m_toggleBookmarkAction);
	m_menu->addSeparator();
}
