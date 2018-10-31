// utils.cpp - Misc. utilities
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

#include "utils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QToolTip>
#include <QWidgetAction>

#ifdef KS_KF5
	#include <KAuthorized>
#endif // KS_KF5

// private

QProcessEnvironment Utils::m_env = QProcessEnvironment::systemEnvironment();
QString Utils::m_desktopSession;
QString Utils::m_xdgCurrentDesktop;

// public

void Utils::addTitle(QMenu *menu, const QIcon &icon, const QString &text) {
	// NOTE: "QMenu::addSection" is fully implemented only in Oxygen style...

	auto *titleWidget = new QWidget(menu);
	auto *titleLayout = new QHBoxLayout(titleWidget);
	titleLayout->setMargin(5_px);
	titleLayout->setSpacing(5_px);

	if (!icon.isNull()) {
		auto *iconLabel = new QLabel(titleWidget);
		iconLabel->setPixmap(icon.pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
		titleLayout->addWidget(iconLabel);
	}

	auto *textLabel = new QLabel(text, titleWidget);
	setFont(textLabel, -1, true);
	titleLayout->addWidget(textLabel);

	titleLayout->addStretch();

	auto *widgetAction = new QWidgetAction(menu);
	widgetAction->setDefaultWidget(titleWidget);
	menu->addAction(widgetAction);
}

QString Utils::getUser() {
	QString LOGNAME = m_env.value("LOGNAME");
	
	if (!LOGNAME.isEmpty())
		return LOGNAME;

	QString USER = m_env.value("USER");
	
	if (!USER.isEmpty())
		return USER;
		
	return QString::null;
}

void Utils::init() {
	m_desktopSession = m_env.value("DESKTOP_SESSION");
	m_xdgCurrentDesktop = m_env.value("XDG_CURRENT_DESKTOP");

	#ifdef Q_OS_LINUX
	if (m_desktopSession.isEmpty() && m_xdgCurrentDesktop.isEmpty()) {
		qWarning("kshutdown: WARNING: \"DESKTOP_SESSION\" and \"XDG_CURRENT_DESKTOP\" environment variables not set (unknown or unsupported Desktop Environment). Good luck.");
	}
	else {
		qDebug(
			"kshutdown: DESKTOP_SESSION=%s | XDG_CURRENT_DESKTOP=%s",
			m_desktopSession.toLocal8Bit().constData(),
			m_xdgCurrentDesktop.toLocal8Bit().constData()
		);
	}
	#endif // Q_OS_LINUX
}

bool Utils::isCinnamon() {
// TODO: test
	return
		m_desktopSession.contains("cinnamon", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("cinnamon", Qt::CaseInsensitive);
}

bool Utils::isEnlightenment() {
	return
		m_desktopSession.contains("enlightenment", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("enlightenment", Qt::CaseInsensitive);
}

bool Utils::isGNOME() {
	return
		m_desktopSession.contains("gnome", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("gnome", Qt::CaseInsensitive);
}

bool Utils::isGTKStyle() {
	#ifdef Q_OS_WIN32
	return false;
	#elif defined(Q_OS_HAIKU)
	return true;
	#else
	return isGNOME() || isLXDE() || isMATE() || isXfce() || isUnity() || isCinnamon();
	#endif // Q_OS_HAIKU
}

bool Utils::isHaiku() {
	#ifdef Q_OS_HAIKU
	return true;
	#else
	return false;
	#endif // Q_OS_HAIKU
}

bool Utils::isKDEFullSession() {
	return m_env.value("KDE_FULL_SESSION") == "true";
}

bool Utils::isKDE() {
	return
		isKDEFullSession() &&
		(
			m_desktopSession.contains("kde", Qt::CaseInsensitive) ||
			m_xdgCurrentDesktop.contains("kde", Qt::CaseInsensitive) ||
			(m_env.value("KDE_SESSION_VERSION").toInt() >= 4)
		);
}

bool Utils::isLXDE() {
	return
		m_desktopSession.contains("LXDE", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("LXDE", Qt::CaseInsensitive);
}

bool Utils::isLXQt() {
// TODO: inline Utils::isDesktop(QString)
	return
		m_desktopSession.contains("LXQT", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("LXQT", Qt::CaseInsensitive);
}

bool Utils::isMATE() {
	return
		m_desktopSession.contains("mate", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("mate", Qt::CaseInsensitive);
}

bool Utils::isOpenbox() {
	// NOTE: Use "contains" instead of "compare"
	// to correctly detect "DESKTOP_SESSION=/usr/share/xsessions/openbox".
	// BUG: https://sourceforge.net/p/kshutdown/bugs/31/
	return
		m_desktopSession.contains("openbox", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("openbox", Qt::CaseInsensitive);
}

bool Utils::isRazor() {
// TODO: test
	return
		m_desktopSession.contains("RAZOR", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("RAZOR", Qt::CaseInsensitive);
}

bool Utils::isRestricted(const QString &action) {
#ifdef KS_KF5
	return !KAuthorized::authorize(action);
#else
	Q_UNUSED(action)

	return false;
#endif // KS_KF5
}

bool Utils::isTrinity() {
	return
		m_desktopSession.contains("TRINITY", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("TRINITY", Qt::CaseInsensitive);
}

bool Utils::isUnity() {
	return
		m_desktopSession.contains("UBUNTU", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("UNITY", Qt::CaseInsensitive);
}

bool Utils::isXfce() {
	return
		m_desktopSession.contains("xfce", Qt::CaseInsensitive) ||
		m_xdgCurrentDesktop.contains("xfce", Qt::CaseInsensitive);
}

QString Utils::read(QProcess &process, bool &ok) {
	ok = false;

	if (!process.waitForStarted(-1)) {
		QString message = i18n("Error: %0").arg(
			process.program() + " " + process.arguments().join(" ") + "\n" +
			process.errorString()
		);

		return message;
	}

	QString err = "";
	QString out = "";

	while (process.waitForReadyRead(-1)) {
		out += QString::fromUtf8(process.readAllStandardOutput());
		err += QString::fromUtf8(process.readAllStandardError());
	}

	if (!err.isEmpty())
		return err;

	ok = true;

	return out;
}

void Utils::setFont(QWidget *widget, const int relativeSize, const bool bold) {
	QFont newFont(widget->font());
	if (bold)
		newFont.setBold(bold);
	int size = newFont.pointSize();
	if (size != -1) {
		newFont.setPointSize(qMax(8, size + relativeSize));
	}
	else {
		size = newFont.pixelSize();
		newFont.setPixelSize(qMax(8_px, size + relativeSize));
	}
	widget->setFont(newFont);
}


void Utils::showMenuToolTip(QAction *action) {
	QList<QWidget *> list = action->associatedWidgets();

	if (list.count() != 1)
		return;

	// HACK: hide previous tool tip window if no tool tip...
	if (action->statusTip().isEmpty())
		QToolTip::hideText();
}

QString Utils::trim(QString &text, const int maxLength) {
	if (text.length() > maxLength) {
		text.truncate(maxLength);
		text = text.trimmed();
		text.append("...");
	}
	
	return text;
}
