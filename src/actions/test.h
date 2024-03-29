// test.h - Test
// Copyright (C) 2010  Konrad Twardowski
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

#ifndef KSHUTDOWN_TEST_H
#define KSHUTDOWN_TEST_H

#include "../kshutdown.h"

#include <QLineEdit>

class TestAction: public Action {
public:
	explicit TestAction();
	virtual QWidget *getWidget() override;
	virtual bool onAction() override;
	virtual void readConfig(Config *config) override;
	virtual void writeConfig(Config *config) override;
private:
	Q_DISABLE_COPY(TestAction)
	QLineEdit *m_textField = nullptr;
	QString m_defaultText = "";
	QWidget *m_widget = nullptr;
};

#endif // KSHUTDOWN_TEST_H
