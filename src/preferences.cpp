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

#include "config.h"
#include "preferences.h"

#ifdef KS_NATIVE_KDE
	#include <KRun>
#endif // KS_NATIVE_KDE

// public

Preferences::Preferences(QWidget *parent) :
	U_DIALOG(parent) {
	U_DEBUG << "Preferences::Preferences()" U_END;

	setWindowTitle(i18n("Preferences"));

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(5);

	QTabWidget *tabs = new QTabWidget();
	tabs->addTab(createGeneralWidget(), i18n("General"));
// TODO: actions/triggers config.
	//tabs->addTab(createActionsWidget(), i18n("Actions"));
	//tabs->addTab(createTriggersWidget(), i18n("Triggers"));

	mainLayout->addWidget(tabs);
	
	U_DIALOG_BUTTON_BOX *dialogButtonBox;
#ifdef KS_NATIVE_KDE
	dialogButtonBox = new U_DIALOG_BUTTON_BOX(this);
	dialogButtonBox->addButton(KStandardGuiItem::ok(), U_DIALOG_BUTTON_BOX::AcceptRole);
	dialogButtonBox->addButton(KStandardGuiItem::cancel(), U_DIALOG_BUTTON_BOX::RejectRole);
#else
	dialogButtonBox = new U_DIALOG_BUTTON_BOX(U_DIALOG_BUTTON_BOX::Ok | U_DIALOG_BUTTON_BOX::Cancel);
#endif // KS_NATIVE_KDE
	connect(dialogButtonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(dialogButtonBox, SIGNAL(rejected()), SLOT(reject()));
	mainLayout->addWidget(dialogButtonBox);
}

Preferences::~Preferences() {
	U_DEBUG << "Preferences::~Preferences()" U_END;
}

void Preferences::apply() {
	Config::setConfirmAction(m_confirmAction->isChecked());
	Config::setLockScreenBeforeHibernate(m_lockScreenBeforeHibernate->isChecked());
	Config::setProgressBarEnabled(m_progressBarEnabled->isChecked());

	Config::user()->sync();
}

// private

/*
QWidget *Preferences::createActionsWidget() {
	QWidget *w = createContainerWidget();

	return w;
}
*/

QWidget *Preferences::createContainerWidget() {
	QWidget *w = new QWidget();
	QVBoxLayout *l = new QVBoxLayout(w);
	l->setMargin(5);

	return w;
}

QWidget *Preferences::createGeneralWidget() {
	QWidget *w = createContainerWidget();

	m_confirmAction = new QCheckBox(i18n("Confirm Action"));
	m_confirmAction->setChecked(Config::confirmAction());
	w->layout()->addWidget(m_confirmAction);

	m_progressBarEnabled = new QCheckBox(i18n("Progress Bar"));
	m_progressBarEnabled->setChecked(Config::progressBarEnabled());
	w->layout()->addWidget(m_progressBarEnabled);

	m_lockScreenBeforeHibernate = new QCheckBox(i18n("Lock Screen Before Hibernate"));
	m_lockScreenBeforeHibernate->setChecked(Config::lockScreenBeforeHibernate());
	w->layout()->addWidget(m_lockScreenBeforeHibernate);
#ifdef Q_WS_WIN
	m_lockScreenBeforeHibernate->hide();
#endif // Q_WS_WIN

#ifdef KS_NATIVE_KDE
	U_PUSH_BUTTON *kdeRelatedSettingsPushButton = new U_PUSH_BUTTON(U_STOCK_ICON("start-here-kde"), i18n("Related KDE Settings..."));
	w->layout()->addWidget(kdeRelatedSettingsPushButton);
	connect(kdeRelatedSettingsPushButton, SIGNAL(clicked()), SLOT(onKDERelatedSettings()));
#endif // KS_NATIVE_KDE

	return w;
}

/*
QWidget *Preferences::createTriggersWidget() {
	QWidget *w = createContainerWidget();

	return w;
}
*/

// private slots

#ifdef KS_NATIVE_KDE
void Preferences::onKDERelatedSettings() {
	KRun::run(
		"kcmshell4 screensaver kcmsmserver energy powerdevilconfig autostart kcmkded kdm kgrubeditor",
		KUrl::List(),
		this
	);
}
#endif // KS_NATIVE_KDE
