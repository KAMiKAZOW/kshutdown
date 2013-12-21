// test.cpp - Test
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

#include "test.h"

#include <QHBoxLayout>
#include <QLabel>

// TestAction

// public

TestAction::TestAction() :
	Action(i18n("Show Message (no shutdown)"), "dialog-ok", "test"),
	m_widget(0) {

// TODO: sound beep

	m_defaultText = "<qt><h1 style=\"background-color: red; color: white\">" + i18n("Test") + "</h1></qt>";
	m_textField = new U_LINE_EDIT();
	m_textField->setPlaceholderText(i18n("Enter a message"));
	#if QT_VERSION >= 0x050200
	m_textField->setClearButtonEnabled(true);
	#endif
	
	setCanBookmark(true);
	setShowInMenu(false);

	addCommandLineArg(QString::null, "test");
}

QWidget *TestAction::getWidget() {
	if (!m_widget) {
		m_widget = new QWidget();

		QHBoxLayout *layout = new QHBoxLayout(m_widget);
		layout->setMargin(0);
		layout->setSpacing(5);
		
		QLabel *label = new QLabel(i18n("Text:"), m_widget);
		label->setBuddy(m_textField);
		
		layout->addWidget(label);
		layout->addWidget(m_textField);
	}
	
	return m_widget;
}

bool TestAction::onAction() {
	QString text = m_textField->text();
	text = text.trimmed();
	if (text.isEmpty())
		text = m_defaultText;

	U_INFO_MESSAGE(0, text);
	
	return true;
}

void TestAction::readConfig(const QString &group, Config *config) {
	config->beginGroup(group);
	m_textField->setText(config->read("Text", "").toString());
	config->endGroup();
}

void TestAction::writeConfig(const QString &group, Config *config) {
	config->beginGroup(group);
	config->write("Text", m_textField->text());
	config->endGroup();
}
