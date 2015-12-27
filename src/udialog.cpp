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

#include "pureqt.h"

#include "udialog.h"
#include "utils.h"

// TODO: clean up all headers

#include <QVBoxLayout>

// public:

UDialog::UDialog(QWidget *parent, const QString &windowTitle, const bool simple) :
	QDialog(parent) {
	//U_DEBUG << "UDialog::UDialog()" U_END;
	setWindowTitle(windowTitle);

#ifdef KS_KF5
	if (simple) {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Close);
	}
	else {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Ok);
	}
#elif defined(KS_NATIVE_KDE)
	m_dialogButtonBox = new KDialogButtonBox(this);
	if (simple) {
		m_acceptButton = m_dialogButtonBox->addButton(KStandardGuiItem::close(), KDialogButtonBox::AcceptRole);
	}
	else {
		m_acceptButton = m_dialogButtonBox->addButton(KStandardGuiItem::ok(), KDialogButtonBox::AcceptRole);
		m_dialogButtonBox->addButton(KStandardGuiItem::cancel(), KDialogButtonBox::RejectRole);
	}
#else
	if (simple) {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Close);
	}
	else {
		m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		m_acceptButton = m_dialogButtonBox->button(QDialogButtonBox::Ok);
	}
#endif // KS_NATIVE_KDE
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

UDialog::~UDialog() {
	//U_DEBUG << "UDialog::~UDialog()" U_END;
}
