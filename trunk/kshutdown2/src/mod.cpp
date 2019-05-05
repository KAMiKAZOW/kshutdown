// mod.cpp - Mod Support
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

#include "mod.h"

#include "commandline.h"
#include "config.h"
#include "mainwindow.h"

#include <QDebug>
#include <QMenuBar>
#include <QPushButton>

// private

QHash<QString, QVariant> *Mod::m_map = nullptr;

// public

void Mod::applyMainWindowColors(MainWindow *mainWindow) {
// TODO: document properties
// TODO: layout mod
	QColor windowColor = getColor("ui-window-color", QColor());
	QColor windowText = getColor("ui-window-text", QColor());
	QColor buttonColorActive = getColor("ui-button-color-active", QColor());
	QColor buttonColorInactive = getColor("ui-button-color-inactive", QColor());
	QColor buttonText = QColor();

	QString theme = get("ui-theme", "").toString();
	if (theme == "classic") {
		windowColor = 0xA7DC6B;
		buttonColorActive = 0xDCD86A;
		buttonColorInactive = 0xDC6A6E;
		buttonText = mainWindow->active() ? Qt::black : Qt::white;
	}
	else if (theme == "solarized") {
		windowColor = 0x002B36;
		windowText = 0xEEE8D5;
		buttonColorActive = 0x859900;
		buttonColorInactive = 0xB58900;
	}
	else if (theme == "sky") {
		windowColor = 0xA4C0E4;
		buttonColorActive = 0xD8E8C2;
		buttonColorInactive = 0xFFBFBF;
	}

	QColor buttonColor = mainWindow->active() ? buttonColorActive : buttonColorInactive;
	if (buttonColor.isValid()) {
		QPalette buttonPalette;
		buttonPalette.setColor(QPalette::Button, buttonColor);
		buttonPalette.setColor(QPalette::ButtonText, buttonText.isValid() ? buttonText : getContrastBW(buttonColor));
		mainWindow->okCancelButton()->setPalette(buttonPalette);
	}

	if (windowColor.isValid()) {
		QPalette windowPalette;
		windowPalette.setColor(QPalette::Window, windowColor);
		QColor text = windowText.isValid() ? windowText : getContrastBW(windowColor);
		windowPalette.setColor(QPalette::WindowText, text);
		mainWindow->setPalette(windowPalette);
		
		QPalette palette;
		palette.setColor(QPalette::Button, windowColor);
		palette.setColor(QPalette::ButtonText, text);
		//palette.setColor(QPalette::Base, windowColor);
		//palette.setColor(QPalette::Text, text);
		mainWindow->m_actions->setPalette(palette);
		mainWindow->m_triggers->setPalette(palette);
		mainWindow->menuBar()->setPalette(palette);
	}
}

QVariant Mod::get(const QString &name, const QVariant &defaultValue) {
	return m_map->value(name, defaultValue);
}

bool Mod::getBool(const QString &name, const bool defaultValue) {
	return get(name, defaultValue).toBool();
}

QColor Mod::getColor(const QString &name, const QColor &defaultValue) {
	return get(name, defaultValue).value<QColor>();
}

QString Mod::getString(const QString &name, const QString &defaultValue) {
	return get(name, defaultValue).toString();
}

void Mod::init() {
	//U_DEBUG << "Mod::init()" U_END;

	m_map = new QHash<QString, QVariant>();

	// read command line
	QString cliMod = CLI::getOption("mod");
	
	// read config
	Config *config = Config::user();
	config->beginGroup("Mod");
	QString configMod = config->read("Value", "").toString();
	config->endGroup();

	QString mod = cliMod; // 1. - add value from command line

	if (!configMod.isEmpty()) { // 2. - add value from config - overrides command line entries
		if (!mod.isEmpty())
			mod += ',';
		mod += configMod;
	}

	// parse and initialize the mod list

	mod = mod.trimmed();
	
	if (mod.isEmpty())
		return;
	
	QStringList list = mod.split(',');
	
	if (list.isEmpty())
		return;
	
	qDebug() << "Mod value:" << mod;
	
	foreach(const QString &item, list) {
		QString token = item.trimmed();
		//U_DEBUG << "Mod token:" << token U_END;
		
		QString name;
		QVariant value;
		
		int i = token.indexOf('=');
		if (i == -1) {
			name = token;
			value = true;
		}
		else {
			name = token.mid(0, i);
			value = token.mid(i + 1);
		}

		name = name.trimmed();

		qDebug() << "Mod insert: " << name << "=" << value;
		m_map->insert(name, value);
	}
}

// private

QColor Mod::getContrastBW(const QColor &base) {
	// CREDITS: http://24ways.org/2010/calculating-color-contrast/
	int r = base.red();
	int g = base.green();
	int b = base.blue();
	int yiq = ((r * 299) + (g * 587) + (b * 114)) / 1000;

	return (yiq >= 128) ? Qt::black : Qt::white;
}
