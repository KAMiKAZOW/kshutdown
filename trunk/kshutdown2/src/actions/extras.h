//
// extras.h - Extras
// Copyright (C) 2007  Konrad Twardowski
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


#ifndef __EXTRAS_H__
#define __EXTRAS_H__

#include "../kshutdown.h"

class Extras: public KShutdown::Action {
public:
	virtual QWidget *getWidget();
	virtual bool onAction();
	inline static Extras *self() {
		if (!m_instance)
			m_instance = new Extras();

		return m_instance;
	}
private:
	static Extras *m_instance;
	U_PUSH_BUTTON *m_menuButton;
	Extras();
	U_MENU *createMenu();
};

#endif // __EXTRAS_H__
