// udialog.cpp - A dialog base
// Copyright (C) 2011  Konrad Twardowski
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

#include "udialog.h"

#include "utils.h"

#ifdef KS_KF5
	#include <KMessageBox>
#else
	#include <QMessageBox>
#endif // KS_KF5

// public:

UDialog::UDialog(QWidget *parent, const QString &windowTitle, const bool simple) :
	QDialog(parent) {
	//U_DEBUG << "UDialog::UDialog()" U_END;
	setWindowTitle(windowTitle);

// TODO: AA_DisableWindowContextHelpButton #Qt5.10

	if (simple) {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Close);
	}
	else {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Ok);
	}

	connect(m_dialogButtonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(m_dialogButtonBox, SIGNAL(rejected()), SLOT(reject()));

	auto *mainWidget = new QWidget(this);
	m_mainLayout = new QVBoxLayout(mainWidget);
	m_mainLayout->setMargin(0_px);
	m_mainLayout->setSpacing(10_px);

	m_rootLayout = new QVBoxLayout(this);
	m_rootLayout->setMargin(10_px);
	m_rootLayout->setSpacing(10_px);

	m_rootLayout->addWidget(mainWidget);
	m_rootLayout->addWidget(m_dialogButtonBox);
}

// messages

bool UDialog::confirm(QWidget *parent, const QString &text) {
	#ifdef KS_KF5
	return KMessageBox::questionYesNo(
		parent, text, i18n("Confirm"),
		KStandardGuiItem::ok(), KStandardGuiItem::cancel()
	) == KMessageBox::Yes;
	#else
	return QMessageBox::question(
		parent, i18n("Confirm"), text,
		QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok
	) == QMessageBox::Ok;
	#endif // KS_KF5
}

void UDialog::error(QWidget *parent, const QString &text) {
	#ifdef KS_KF5
	KMessageBox::error(parent, text);
	#else
	QMessageBox::critical(parent, i18n("Error"), text);
	#endif // KS_KF5
}

void UDialog::info(QWidget *parent, const QString &text) {
	#ifdef KS_KF5
	KMessageBox::information(parent, text);
	#else
	QMessageBox::information(parent, i18n("Information"), text);
	#endif // KS_KF5
}
