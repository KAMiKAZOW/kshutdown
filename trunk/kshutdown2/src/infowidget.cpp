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

#include "pureqt.h"

#ifdef KS_NATIVE_KDE
	#include <KMessageWidget>
#endif // KS_NATIVE_KDE

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>

#include "infowidget.h"
#include "utils.h"

// public

InfoWidget::InfoWidget(QWidget *parent) :
	QFrame(parent) {

	setObjectName("info-widget");
	setVisible(false);

	QHBoxLayout *mainLayout = new QHBoxLayout(this);

#ifdef KS_NATIVE_KDE
	m_messageWidget = new KMessageWidget(this);
	m_messageWidget->setCloseButtonVisible(false);
	connect(
		m_messageWidget, SIGNAL(linkActivated(const QString &)),
		SLOT(onLinkActivated(const QString &))
	);
	mainLayout->addWidget(m_messageWidget);
#else
	setAutoFillBackground(true);
// FIXME: too buggy and unpredictable: setFrameStyle(StyledPanel | Plain);
	setFrameStyle(Box | Plain);
	setLineWidth(1);

	m_icon = new QLabel();
	m_text = new QLabel();
	m_text->setOpenExternalLinks(true);

	mainLayout->setMargin(10_px);
	mainLayout->setSpacing(10_px);
	mainLayout->addWidget(m_icon);
	mainLayout->addWidget(m_text);
	mainLayout->addStretch();
#endif // KS_NATIVE_KDE
}

InfoWidget::~InfoWidget() { }

void InfoWidget::setText(const QString &text, const Type type) {
#ifdef KS_NATIVE_KDE
	switch (type) {
		case Type::Error:
			m_messageWidget->setMessageType(KMessageWidget::Error);
			break;
		case Type::Info:
			m_messageWidget->setMessageType(KMessageWidget::Information);
			break;
		case Type::Warning:
			m_messageWidget->setMessageType(KMessageWidget::Warning);
			break;
	}
	m_messageWidget->setText(text);
#else
	QRgb background; // picked from the Oxygen palette
	switch (type) {
		case Type::Error:
			background = 0xF9CCCA; // brick red 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxCritical);
			#else
			setIcon("dialog-error");
			#endif // Q_OS_WIN32
			break;
		case Type::Info:
			background = 0xEEEEEE; // gray 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxInformation);
			#else
// FIXME: empty icon in some Desktop Environments
			setIcon("dialog-information");
			#endif // Q_OS_WIN32
			break;
		default: // Type::Warning
			background = 0xF8FFBF; // lime 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxWarning);
			#else
			setIcon("dialog-warning");
			#endif // Q_OS_WIN32
			break;
	}

	// HACK: icon themes are not supported in some DE (e.g. MATE)
	m_icon->setVisible(m_icon->pixmap() && !m_icon->pixmap()->isNull());

	QPalette p;
	QColor bg = QColor(background);
	QColor fg = bg.darker(300);
	p.setColor(QPalette::Window, bg);
	p.setColor(QPalette::WindowText, fg);
	setPalette(p);
	
	m_text->setText(text);
#endif // KS_NATIVE_KDE

	setVisible(!text.isEmpty() && (text != "<qt></qt>"));
	if (isVisible())
		repaint(0, 0, width(), height());
}

// private slots

void InfoWidget::onLinkActivated(const QString &contents) {
	QDesktopServices::openUrl(QUrl(contents));
}

// private

#ifdef KS_PURE_QT
#ifdef Q_OS_WIN32
void InfoWidget::setIcon(const QStyle::StandardPixmap standardIcon) {
	m_icon->setPixmap(U_APP->style()->standardIcon(standardIcon).pixmap(24, 24));
}
#else
void InfoWidget::setIcon(const QString &iconName) {
	m_icon->setPixmap(U_STOCK_ICON(iconName).pixmap(24, 24));
}
#endif // Q_OS_WIN32
#endif // KS_PURE_QT
