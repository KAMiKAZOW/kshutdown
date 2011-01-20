// main.cpp - A progress bar widget
// Copyright (C) 2008  Konrad Twardowski
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

#include "config.h"
#include "progressbar.h"
#include "pureqt.h"

#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>

ProgressBar *ProgressBar::m_instance = 0;

// public

ProgressBar::~ProgressBar() {
	U_DEBUG << "ProgressBar::~ProgressBar()" U_END;
}

void ProgressBar::setAlignment(const Qt::Alignment value, const bool updateConfig) {
	if (updateConfig) {
		Config *config = Config::user();
		config->setProgressBarAlignment(value);
		config->sync();
	}

	m_alignment = value;
	QDesktopWidget *desktop = QApplication::desktop();
	resize(desktop->width() - 4, height());
	if (m_alignment.testFlag(Qt::AlignBottom)) {
		move(2, desktop->height() - height());
	}
	// Qt::AlignTop
	else {
		move(2, 0);
	}
	show();
}

void ProgressBar::setHeight(const int value) {
	int newHeight = (value < 2) ? 2 : value;
	resize(width(), newHeight);
}

void ProgressBar::setProgress(const int value) {
	int complete = m_total - value;
	if (m_complete != complete) {
		m_complete = complete;
		repaint();
	}
}

void ProgressBar::setTotal(const int total) {
	if (m_total != total) {
		m_complete = 0; // reset
		m_total = total;
		repaint();
	}
}

// protected

void ProgressBar::mousePressEvent(QMouseEvent *e) {
	if (e->button() == Qt::RightButton) {
		// show popup menu
		U_MENU *menu = new U_MENU(this);
#ifdef KS_NATIVE_KDE
		menu->addTitle(U_APP->windowIcon(), KGlobal::caption());
#endif // KS_PURE_QT
		menu->addAction(i18n("Hide"), this, SLOT(hide()));
#ifdef KS_NATIVE_KDE
		menu->addTitle(i18n("Position"));
#else
		menu->addSeparator();
#endif // KS_PURE_QT

		QActionGroup *ag = new QActionGroup(this);

		QAction *a = menu->addAction(i18n("Top"), this, SLOT(onSetTopAlignment()));
		a->setActionGroup(ag);
		a->setCheckable(true);
		a->setChecked(m_alignment.testFlag(Qt::AlignTop));
		
		a = menu->addAction(i18n("Bottom"), this, SLOT(onSetBottomAlignment()));
		a->setActionGroup(ag);
		a->setCheckable(true);
		a->setChecked(m_alignment.testFlag(Qt::AlignBottom));
		
		menu->popup(e->globalPos());
		e->accept();
	}
	QWidget::mousePressEvent(e);
}

void ProgressBar::paintEvent(QPaintEvent *e) {
	Q_UNUSED(e)

	QPainter g(this);
	int w = width();
	int h = height();
	g.fillRect(0, 0, w, h, palette().window());

	if ((m_complete <= 0) || (m_total <= 0))
		return;

	w = (int)((float)w * ((float)m_complete * 100.0f / (float)m_total) / 100.0f);
	g.fillRect(0, 0, w, h, palette().windowText());
}

// private

ProgressBar::ProgressBar()
	: QWidget(
		0,
		Qt::FramelessWindowHint |
		Qt::WindowStaysOnTopHint |
		Qt::X11BypassWindowManagerHint |
		Qt::Tool
	),
	m_complete(0),
	m_total(0) {
	
	U_DEBUG << "ProgressBar::ProgressBar()" U_END;

	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setObjectName("progress-bar");

// TODO: color configuration
	QPalette p;
	p.setColor(QPalette::Window, Qt::black);
	p.setColor(QPalette::WindowText, QColor(0xF8FFBF /* lime 1 */));
	setPalette(p);

// TODO: size configuration
	setHeight(3);

	setAlignment(Config::user()->progressBarAlignment(), false);
}

// private slots

void ProgressBar::onSetBottomAlignment() {
	setAlignment(Qt::AlignBottom, true);
}

void ProgressBar::onSetTopAlignment() {
	setAlignment(Qt::AlignTop, true);
}