//
// preferences.cpp - Preferences Dialog
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

#include <QCheckBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "preferences.h"

// public

Preferences::Preferences(QWidget *parent) :
	U_DIALOG(parent) {
	U_DEBUG << "Preferences::Preferences()" U_END;

	setWindowTitle(i18n("Preferences") + " UNDER CONSTRUCTION!!!");

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setMargin(5);
	mainLayout->setSpacing(5);
	setLayout(mainLayout);

	m_confirmAction = new QCheckBox(i18n("Confirm Action"));
	m_confirmAction->setChecked(confirmAction());

	QTabWidget *tabs = new QTabWidget(this);
	tabs->addTab(m_confirmAction, i18n("General"));

	mainLayout->addWidget(tabs);
}

Preferences::~Preferences() {
	U_DEBUG << "Preferences::~Preferences()" U_END;
}

void Preferences::apply() {
	setConfirmAction(m_confirmAction->isChecked());
}

bool Preferences::confirmAction() {
	U_CONFIG *config = U_CONFIG_USER;
#ifdef KS_NATIVE_KDE
	KConfigGroup g = config->group("General");

	return g.readEntry("Confirm Action", true);
#else
	config->beginGroup("General");
	bool result = config->value("Confirm Action", true).toBool();
	config->endGroup();

	return result;
#endif // KS_NATIVE_KDE
}

void Preferences::setConfirmAction(const bool value) {
	U_CONFIG *config = U_CONFIG_USER;
#ifdef KS_NATIVE_KDE
	KConfigGroup g = config->group("General");
	g.writeEntry("Confirm Action", value);
#else
	config->beginGroup("General");
	config->setValue("Confirm Action", value);
	config->endGroup();
#endif // KS_NATIVE_KDE
}
