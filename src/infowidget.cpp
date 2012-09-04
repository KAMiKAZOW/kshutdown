// infowidget.cpp - Info Widget
// Copyright (C) 2009  Konrad Twardowski
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

// TODO: Use KMessageWidget in KDE 4 build
// http://agateau.wordpress.com/2011/04/21/kde-ux-2011/
// http://community.kde.org/Sprints/UX2011/KMessageWidget

#include "pureqt.h"

#include <QHBoxLayout>
#include <QLabel>

#include "infowidget.h"
#include "utils.h"

// public

InfoWidget::InfoWidget(QWidget *parent) :
	QFrame(parent) {

	// smaller font
	Utils::setFont(this, -1, false);

	setAutoFillBackground(true);
	setFrameStyle(Panel | Sunken);
	setLineWidth(1);
	setObjectName("info-widget");
	setVisible(false);

	m_icon = new QLabel();
	m_text = new QLabel();
	m_text->setOpenExternalLinks(true);

	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setMargin(5);
	mainLayout->setSpacing(10);
	mainLayout->addWidget(m_icon);
	mainLayout->addWidget(m_text);
	mainLayout->addStretch();
}

InfoWidget::~InfoWidget() { }

void InfoWidget::setText(const QString &text, const Type type) {
	QRgb background; // picked from the Oxygen palette
	switch (type) {
		case ErrorType:
			background = 0xF9CCCA; // brick red 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxCritical);
			#else
			setIcon("dialog-error");
			#endif // Q_OS_WIN32
			break;
		case InfoType:
			background = 0xEEEEEE; // gray 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxInformation);
			#else
			setIcon("dialog-information");
			#endif // Q_OS_WIN32
			break;
		default: // WarningType
			background = 0xF8FFBF; // lime 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxWarning);
			#else
			setIcon("dialog-warning");
			#endif // Q_OS_WIN32
			break;
	}
	
	QPalette p;
	p.setColor(QPalette::Window, QColor(background));
	p.setColor(QPalette::WindowText, Qt::black);
	setPalette(p);
	
	m_text->setText(text);
	setVisible(!text.isEmpty() && (text != "<qt></qt>"));
	if (isVisible())
		repaint(0, 0, width(), height());
}

// private

#ifdef Q_OS_WIN32
void InfoWidget::setIcon(const QStyle::StandardPixmap standardIcon) {
	m_icon->setPixmap(U_APP->style()->standardIcon(standardIcon).pixmap(24, 24));
}
#else
void InfoWidget::setIcon(const QString &iconName) {
	m_icon->setPixmap(U_STOCK_ICON(iconName).pixmap(24, 24));
}
#endif // Q_OS_WIN32
