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

#ifndef KSHUTDOWN_PREFERENCES_H
#define KSHUTDOWN_PREFERENCES_H

#include "pureqt.h"

class QCheckBox;

class Preferences: public U_DIALOG {
	Q_OBJECT
public:
	explicit Preferences(QWidget *parent);
	virtual ~Preferences();
	void apply();
private:
	Q_DISABLE_COPY(Preferences)
	QCheckBox *m_confirmAction;
	QCheckBox *m_lockScreenBeforeHibernate;
	QCheckBox *m_progressBarEnabled;
	//QWidget *createActionsWidget();
	QWidget *createContainerWidget();
	QWidget *createGeneralWidget();
	//QWidget *createTriggersWidget();
private slots:
#ifdef KS_NATIVE_KDE
	void onKDERelatedSettings();
#endif // KS_NATIVE_KDE
};

#endif // KSHUTDOWN_PREFERENCES_H
