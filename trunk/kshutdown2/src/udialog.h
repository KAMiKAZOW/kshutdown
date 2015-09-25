// udialog.h - A dialog base
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

#ifndef KSHUTDOWN_UDIALOG_H
#define KSHUTDOWN_UDIALOG_H

#include "pureqt.h"

#include <QDialog>

#if defined(KS_PURE_QT) || defined(KS_KF5)
	#include <QDialogButtonBox>
#else
	#include <KDialogButtonBox>
#endif // KS_PURE_QT

class QVBoxLayout;

class UDialog: public QDialog {
	Q_OBJECT
public:
	explicit UDialog(QWidget *parent, const QString &windowTitle, const bool simple);
	virtual ~UDialog();
	inline QPushButton *acceptButton() { return m_acceptButton; }
	void addButtonBox();
	inline QVBoxLayout *mainLayout() { return m_mainLayout; }
private:
	Q_DISABLE_COPY(UDialog)
	#if defined(KS_PURE_QT) || defined(KS_KF5)
	QDialogButtonBox *m_dialogButtonBox;
	#else
	KDialogButtonBox *m_dialogButtonBox;
	#endif // KS_PURE_QT
	QPushButton *m_acceptButton;
	QVBoxLayout *m_mainLayout;
};

#endif // KSHUTDOWN_UDIALOG_H
