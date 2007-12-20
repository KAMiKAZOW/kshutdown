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

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(5);

	QTabWidget *tabs = new QTabWidget();
	tabs->addTab(createGeneralWidget(), i18n("General"));
// TODO: actions/triggers config.
	//tabs->addTab(createActionsWidget(), i18n("Actions"));
	//tabs->addTab(createTriggersWidget(), i18n("Triggers"));

#ifdef KS_NATIVE_KDE
	U_PUSH_BUTTON *closeButton = new U_PUSH_BUTTON();
	closeButton->setGuiItem(KStandardGuiItem::close());
#else
	U_PUSH_BUTTON *closeButton = new U_PUSH_BUTTON(i18n("Close"));
#endif // KS_NATIVE_KDE
	connect(closeButton, SIGNAL(clicked()), SLOT(accept()));

	mainLayout->addWidget(tabs);
	mainLayout->addWidget(closeButton);//!!!
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

// private

QWidget *Preferences::createActionsWidget() {
	QWidget *w = createContainerWidget();

	return w;
}

QWidget *Preferences::createContainerWidget() {
	QWidget *w = new QWidget();
	QVBoxLayout *l = new QVBoxLayout(w);
	l->setMargin(5);

	return w;
}

QWidget *Preferences::createGeneralWidget() {
	QWidget *w = createContainerWidget();

	m_confirmAction = new QCheckBox(i18n("Confirm Action"));
	m_confirmAction->setChecked(confirmAction());
	w->layout()->addWidget(m_confirmAction);

	return w;
}

QWidget *Preferences::createTriggersWidget() {
	QWidget *w = createContainerWidget();

	return w;
}
