// preferences.h - Preferences Dialog
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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include "pureqt.h"

class QCheckBox;

class Preferences: public U_DIALOG {
	Q_OBJECT
public:
	Preferences(QWidget *parent);
	virtual ~Preferences();
	void apply();
private:
	QCheckBox *m_confirmAction;
	QWidget *createActionsWidget();
	QWidget *createContainerWidget();
	QWidget *createGeneralWidget();
	QWidget *createTriggersWidget();
};

#endif // __PREFERENCES_H__
