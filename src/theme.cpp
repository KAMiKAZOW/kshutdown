//
// theme.cpp - The theme manager
// Copyright (C) 2007  Konrad Twardowski
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

//!!! U_APP
#include <QApplication>
#include <QFile>

#include "mainwindow.h"
#include "theme.h"

// public

Theme::Theme(QObject *parent) :
	QObject(parent) {
	U_DEBUG << "Theme::Theme()";

	setObjectName("theme");
}

Theme::~Theme() {
	U_DEBUG << "Theme::~Theme()";
}

void Theme::load(MainWindow *mainWindow, const QString &name) {
//!!!disable for 4.2-
	QFile style("themes/" + name + "/style.css");

	if (!style.open(QFile::ReadOnly | QFile::Text)) {
		U_ERROR << style.errorString() + ": " + style.fileName();

		return;
	}
	qApp->setStyleSheet(QTextStream(&style).readAll());

	QWidget *widget = mainWindow->getElementById("ok-cancel-button");
	if (widget) {
		U_DEBUG << widget->objectName();
	}
}
