// stats.cpp - System Statistics
// Copyright (C) 2014  Konrad Twardowski
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

#include "stats.h"

#include "utils.h"

// public:

Stats::Stats(QWidget *parent) :
	UDialog(parent, i18n("Statistics"), true) {

	resize(800_px, 600_px);

	m_textView = new QPlainTextEdit(this);
	m_textView->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_textView->setPlainText(i18n("Please Wait..."));
	m_textView->setReadOnly(true);
	m_textView->setStyleSheet("QPlainTextEdit { font-family: monospace; }");

	mainLayout()->addWidget(m_textView);

	QProcess process;
	process.start("w");
	bool ok;
	m_textView->setPlainText(Utils::read(process, ok));
}

Stats::~Stats() = default;
