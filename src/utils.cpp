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

#include <QToolTip>

#ifdef KS_NATIVE_KDE
	#include <KAuthorized>
#endif // KS_NATIVE_KDE

// private

#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
	QCommandLineParser *Utils::m_args = nullptr;
	#else
	KCmdLineArgs *Utils::m_args = nullptr;
	#endif // KS_KF5
#else
	QStringList Utils::m_args;
#endif // KS_NATIVE_KDE
QProcessEnvironment Utils::m_env = QProcessEnvironment::systemEnvironment();
QString Utils::m_desktopSession;
QString Utils::m_xdgCurrentDesktop;

// public

void Utils::addTitle(U_MENU *menu, const QIcon &icon, const QString &text) {
	#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	menu->addTitle(icon, text);
	#else
// FIXME: useless & unimplemented decoy API to annoy developers: menu->addSection(icon, text);
	auto *action = new U_ACTION(menu);
	QFont font = action->font();
	font.setBold(true);

	action->setEnabled(false);
	action->setFont(font);
	action->setIcon(icon);
	action->setIconVisibleInMenu(true);
	action->setText(text);
	menu->addAction(action);
/* TODO: new title UI
	int size = font.pointSize();
	if (size != -1) {
		font.setPointSize(qMax(8_px, size - 1));
	}
	else {
		size = font.pixelSize();
		font.setPixelSize(qMax(8_px, size - 1));
	}

	QLabel *titleLabel = new QLabel(text);
	titleLabel->setFont(font);
	titleLabel->setIndent(5_px);
	titleLabel->setMargin(1_px);

	QWidgetAction *widgetAction = new QWidgetAction(menu);
	widgetAction->setDefaultWidget(titleLabel);
	menu->addAction(widgetAction);
*/
	#endif // KS_NATIVE_KDE
}

QString Utils::getOption(const QString &name) {
#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
	QString option = m_args->value(name);

	return option.isEmpty() ? QString::null : option;
	#else
	return m_args->getOption(name.toAscii());
	#endif // KS_KF5
#else
	int i = m_args.indexOf('-' + name, 1);
	if (i == -1) {
		i = m_args.indexOf("--" + name, 1);
		if (i == -1) {
			//U_DEBUG << "Argument not found: " << name U_END;

			return QString::null;
		}
	}

	int argIndex = (i + 1);

	if (argIndex < m_args.size()) {
		//U_DEBUG << "Value of " << name << " is " << m_args[argIndex] U_END;

		return m_args[argIndex];
	}

	//U_DEBUG << "Argument value is not set: " << name U_END;

	return QString::null;
#endif // KS_NATIVE_KDE
}

QString Utils::getTimeOption() {
#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
	QStringList pa = m_args->positionalArguments();

	if (pa.count())
		return pa.at(0);
	
	return QString::null;
	#else
	if (m_args->count())
		return m_args->arg(0);
	
	return QString::null;
	#endif // KS_KF5
#else
	if (m_args.size() > 2) {
		QString timeOption = m_args.last();
// FIXME: clash with --mod and --extra
		if (!timeOption.isEmpty() && (timeOption.at(0) != '-')) {
			//U_DEBUG << timeOption U_END;
			
			return timeOption;
		}
	}
	
	return QString::null;
#endif // KS_NATIVE_KDE
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

void Utils::initArgs() {
#ifdef KS_NATIVE_KDE
	#ifndef KS_KF5
	m_args = KCmdLineArgs::parsedArgs();
	#endif // KS_KF5
#else
	m_args = U_APP->arguments();
#endif // KS_NATIVE_KDE
}

bool Utils::isArg(const QString &name) {
#ifdef KS_NATIVE_KDE
	#ifdef KS_KF5
	return m_args->isSet(name);
	#else
	return m_args->isSet(name.toAscii());
	#endif // KS_KF5
#else
	return (m_args.contains('-' + name) || m_args.contains("--" + name));
#endif // KS_NATIVE_KDE
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

bool Utils::isHelpArg() {
#ifdef KS_KF5
	return isArg("help");
#elif defined(KS_NATIVE_KDE)
	return false; // "--help" argument handled by KDE
#else
	return
		#ifdef Q_OS_WIN32
		m_args.contains("/?") ||
		#endif // Q_OS_WIN32
		isArg("help");
#endif // KS_KF5
}

// FIXME: test GNOME 2
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
#ifdef KS_NATIVE_KDE
	return !KAuthorized::authorize(action);
#else
	Q_UNUSED(action)

	return false;
#endif // KS_NATIVE_KDE
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

	if (!process.waitForStarted(-1))
// TODO: show process.program()/arguments() #5.x
		return process.errorString();

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

	#if QT_VERSION >= 0x050100
	// HACK: hide previous tool tip window if no tool tip...
	if (action->statusTip().isEmpty())
		QToolTip::hideText();
	#else
	QWidget *widget = list.first();
	int magicNumber = 5_px;
	QToolTip::showText(
		QPoint(widget->x() + magicNumber, widget->y() + widget->height() - (magicNumber * 2)),
		//QCursor::pos(),
		action->statusTip()
	);
	#endif // QT_VERSION
}

void Utils::shutDown() {
#ifdef KS_NATIVE_KDE
	#ifndef KS_KF5
	if (m_args)
		m_args->clear();
	#endif // KS_KF5
#endif // KS_NATIVE_KDE
}

QString Utils::trim(QString &text, const int maxLength) {
	if (text.length() > maxLength) {
		text.truncate(maxLength);
		text = text.trimmed();
		text.append("...");
	}
	
	return text;
}
