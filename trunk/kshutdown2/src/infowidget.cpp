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

#include "infowidget.h"

#include "utils.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QUrl>

#ifdef KS_PURE_QT
	#include <QStyle>
#endif // KS_PURE_QT

// public

InfoWidget::InfoWidget(QWidget *parent) :
	QFrame(parent) {

	setObjectName("info-widget");
	setVisible(false);

	auto *mainLayout = new QHBoxLayout(this);

#ifdef KS_KF5
	m_messageWidget = new KMessageWidget(this);
	m_messageWidget->setCloseButtonVisible(false);
	connect(
		m_messageWidget, SIGNAL(linkActivated(const QString &)),
		SLOT(onLinkActivated(const QString &))
	);

	mainLayout->setMargin(0_px);
	mainLayout->addWidget(m_messageWidget);
#else
	setAutoFillBackground(true);
	setFrameStyle(Panel | Sunken);
	setLineWidth(1_px);
	m_icon = new QLabel();
	m_text = new QLabel();
	m_text->setOpenExternalLinks(true);

	#ifdef Q_OS_WIN32
	mainLayout->setMargin(5_px);
	#else
	mainLayout->setMargin(10_px);
	#endif // Q_OS_WIN32
	mainLayout->setSpacing(10_px);
	mainLayout->addWidget(m_icon);
	mainLayout->addWidget(m_text);
	mainLayout->addStretch();
#endif // KS_KF5
}

InfoWidget::~InfoWidget() = default;

void InfoWidget::setText(const QString &text, const Type type) {
#ifdef KS_KF5
	switch (type) {
		case Type::Error:
			m_messageWidget->setIcon(QIcon::fromTheme("dialog-error"));
			m_messageWidget->setMessageType(KMessageWidget::Error);
			break;
		case Type::Info:
			m_messageWidget->setIcon(QIcon::fromTheme("dialog-information"));
			m_messageWidget->setMessageType(KMessageWidget::Information);
			break;
		case Type::Warning:
			m_messageWidget->setIcon(QIcon::fromTheme("dialog-warning"));
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
			background = 0xbfd9ff; // blue 1
			#ifdef Q_OS_WIN32
			setIcon(QStyle::SP_MessageBoxInformation);
			#else
// FIXME: empty icon in some Desktop Environments
			setIcon("dialog-information");
			#endif // Q_OS_WIN32
			break;
		case Type::Warning:
		default:
			background = 0xffd9b0; // hot orange 1
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

	QColor border = bg.darker(120);;
	p.setColor(QPalette::Light, border);
	p.setColor(QPalette::Dark, border);

	setPalette(p);

	m_text->setText(text);
#endif // KS_KF5

	setVisible(!text.isEmpty() && (text != "<qt></qt>"));
	if (isVisible())
		repaint(0_px, 0_px, width(), height());
}

// private slots

void InfoWidget::onLinkActivated(const QString &contents) {
	QDesktopServices::openUrl(QUrl(contents));
}

// private

#ifdef KS_PURE_QT
#ifdef Q_OS_WIN32
void InfoWidget::setIcon(const QStyle::StandardPixmap standardIcon) {
	int size = qApp->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	m_icon->setPixmap(qApp->style()->standardIcon(standardIcon).pixmap(size));
}
#else
void InfoWidget::setIcon(const QString &iconName) {
	int size = qApp->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	m_icon->setPixmap(QIcon::fromTheme(iconName).pixmap(size));
}
#endif // Q_OS_WIN32
#endif // KS_PURE_QT
