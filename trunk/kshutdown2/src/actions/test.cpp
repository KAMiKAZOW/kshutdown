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

#include "../config.h"
#include "../utils.h"

#include <QFormLayout>

// TestAction

// public

TestAction::TestAction() :
	Action(i18n("Show Message (no shutdown)"), "dialog-ok", "test"),
	m_widget(nullptr) {

// TODO: sound beep

	m_defaultText = "<qt><h1 style=\"background-color: red; color: white\">" + i18n("Test") + "</h1></qt>";
	m_textField = new QLineEdit();
	m_textField->setClearButtonEnabled(true);
	m_textField->setPlaceholderText(i18n("Enter a message"));

	setCanBookmark(true);
	setShowInMenu(false);

	addCommandLineArg(QString::null, "test");
}

QWidget *TestAction::getWidget() {
	if (!m_widget) {
		m_widget = new QWidget();

		auto *layout = new QFormLayout(m_widget);
		layout->setLabelAlignment(Qt::AlignRight);
		layout->setMargin(0_px);
		layout->addRow(i18n("Text:"), m_textField);
	}
	
	return m_widget;
}

bool TestAction::onAction() {
	QString text = m_textField->text();
	text = text.trimmed();
	if (text.isEmpty())
		text = m_defaultText;

	U_INFO_MESSAGE(nullptr, text);
	
	return true;
}

void TestAction::readConfig(Config *config) {
	m_textField->setText(config->read("Text", "").toString());
}

void TestAction::writeConfig(Config *config) {
	config->write("Text", m_textField->text());
}
