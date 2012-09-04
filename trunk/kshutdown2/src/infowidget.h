// infowidget.h - Info Widget
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

#ifndef KSHUTDOWN_INFOWIDGET_H
#define KSHUTDOWN_INFOWIDGET_H

#include <QFrame>

class QLabel;

class InfoWidget: public QFrame {
public:
	enum Type { ErrorType, InfoType, WarningType };
	explicit InfoWidget(QWidget *parent);
	virtual ~InfoWidget();
	void setText(const QString &text, const Type type = InfoType);
private:
	Q_DISABLE_COPY(InfoWidget)
	QLabel *m_icon;
	QLabel *m_text;
	#ifdef Q_OS_WIN32
	void setIcon(const QStyle::StandardPixmap standardIcon);
	#else
	void setIcon(const QString &iconName);
	#endif // Q_OS_WIN32
};

#endif // KSHUTDOWN_INFOWIDGET_H
