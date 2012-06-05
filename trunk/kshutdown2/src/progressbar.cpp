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
#include "utils.h"

#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>

#ifdef KS_NATIVE_KDE
	#include <KColorDialog>
#else
	#include <QColorDialog>
#endif // KS_NATIVE_KDE

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
}

void ProgressBar::setHeight(const int value) {
	int newHeight = (value < 2) ? 2 : value;
	resize(width(), newHeight);
}

void ProgressBar::setTotal(const int total) {
	m_total = total;
	m_completeWidth = 0; // reset
}

void ProgressBar::setValue(const int value) {
	m_value = value;
	
	if (m_total == 0)
		return;

	int newCompleteWidth = (int)((float)width() * ((float)(m_total - m_value) / (float)m_total));
	if (newCompleteWidth != m_completeWidth) {
		m_completeWidth = newCompleteWidth;
		repaint();
	}
}

// protected

void ProgressBar::mousePressEvent(QMouseEvent *e) {
	if (Utils::isRestricted("kshutdown/progress_bar/menu"))
		return;

	if (e->button() == Qt::RightButton) {
		// show popup menu
		U_MENU *menu = new U_MENU(this);

		#ifdef KS_NATIVE_KDE
		menu->addTitle(U_APP->windowIcon(), KGlobal::caption());
		#endif // KS_NATIVE_KDE

		bool canConfigure = !Utils::isRestricted("action/options_configure");

		menu->addAction(i18n("Hide"), this, SLOT(hide()));
		menu->addAction(i18n("Set Color..."), this, SLOT(onSetColor()))
			->setEnabled(canConfigure);

		#ifdef KS_NATIVE_KDE
		menu->addTitle(i18n("Position"));
		#else
		menu->addSeparator();
		#endif // KS_NATIVE_KDE

		QActionGroup *ag = new QActionGroup(this);

		QAction *a = menu->addAction(i18n("Top"), this, SLOT(onSetTopAlignment()));
		a->setActionGroup(ag);
		a->setCheckable(true);
		a->setChecked(m_alignment.testFlag(Qt::AlignTop));
		a->setEnabled(canConfigure);
		
		a = menu->addAction(i18n("Bottom"), this, SLOT(onSetBottomAlignment()));
		a->setActionGroup(ag);
		a->setCheckable(true);
		a->setChecked(m_alignment.testFlag(Qt::AlignBottom));
		a->setEnabled(canConfigure);
		
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

	if ((m_completeWidth <= 0) || (m_total <= 0) || (m_value <= 0))
		return;

	g.fillRect(0, 0, m_completeWidth, h, palette().windowText());
}

// private

ProgressBar::ProgressBar() // public
	: QWidget(
		0,
		Qt::FramelessWindowHint |
		Qt::WindowStaysOnTopHint |
		Qt::X11BypassWindowManagerHint |
		Qt::Tool
	),
	m_completeWidth(0),
	m_total(0),
	m_value(0) {
	
	U_DEBUG << "ProgressBar::ProgressBar()" U_END;

	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setObjectName("progress-bar");

	QPalette p;
	QColor background = QColor(Qt::black);
	p.setColor(QPalette::Window, background);
	
	Config *config = Config::user();
	config->beginGroup("Progress Bar");
	QColor defaultForeground = QColor(0xF8FFBF /* lime 1 */);
	QColor foreground = config->read("Foreground Color", defaultForeground).value<QColor>();
	config->endGroup();
	p.setColor(QPalette::WindowText, (foreground.rgb() == background.rgb()) ? defaultForeground : foreground);
	
	setPalette(p);

// TODO: size configuration
	setHeight(3);

// TODO: auto align on screen size change
	setAlignment(Config::user()->progressBarAlignment(), false);
}

// private slots

void ProgressBar::onSetBottomAlignment() {
	setAlignment(Qt::AlignBottom, true);
}

void ProgressBar::onSetColor() {
	QColor currentColor = palette().color(QPalette::WindowText);
	#ifdef KS_NATIVE_KDE
	QColor newColor;
	if (KColorDialog::getColor(newColor, currentColor, this) != KColorDialog::Accepted)
		return;
	#else
	QColor newColor = QColorDialog::getColor( // krazy:exclude=qclasses
		currentColor,
		this,
		QString::null // use default title
	);
	#endif // KS_NATIVE_KDE
	if (newColor.isValid()) {
		QPalette p(palette());
		p.setColor(QPalette::WindowText, newColor);
		setPalette(p);
		repaint();

		Config *config = Config::user();
		config->beginGroup("Progress Bar");
		config->write("Foreground Color", newColor);
		config->endGroup();
		config->sync();
	}
}

void ProgressBar::onSetTopAlignment() {
	setAlignment(Qt::AlignTop, true);
}
