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

#include <QVBoxLayout>

// public:

Stats::Stats(QWidget *parent) :
	UDialog(parent, i18n("Statistics"), true) {

	resize(800, 600);

	m_textView = new QPlainTextEdit(this);
	m_textView->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_textView->setPlainText(i18n("Please Wait..."));
	m_textView->setReadOnly(true);
	m_textView->setStyleSheet("QPlainTextEdit { font-family: monospace; }");

	mainLayout()->setSpacing(0);
	mainLayout()->addWidget(m_textView);
	addButtonBox();

// TODO: common code
	m_process = new QProcess(this);
	connect(
		m_process, SIGNAL(error(QProcess::ProcessError)),
		SLOT(onError(QProcess::ProcessError))
	);
	connect(
		m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
		SLOT(onFinished(int, QProcess::ExitStatus))
	);
	connect(
		m_process, SIGNAL(readyReadStandardOutput()),
		SLOT(onReadyReadStandardOutput())
	);

	m_process->start("w");
}

Stats::~Stats() {
	if (m_process->state() == QProcess::Running)
		m_process->terminate();
}

// private slots:

void Stats::onError(QProcess::ProcessError error) {
	m_textView->setPlainText(i18n("Error: %0").arg(error));
}

void Stats::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	if (exitStatus == QProcess::NormalExit)
		m_textView->setPlainText(m_outputBuf);
	else
		m_textView->setPlainText(i18n("Error, exit code: %0").arg(exitCode));
}

void Stats::onReadyReadStandardOutput() {
	m_outputBuf.append(m_process->readAllStandardOutput());
}
