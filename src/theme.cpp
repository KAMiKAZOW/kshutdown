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

#include <QFile>

#include "mainwindow.h"
#include "theme.h"

// public

Theme::Theme(QObject *parent) :
	QObject(parent),
	m_inElement(false) {
	U_DEBUG << "Theme::Theme()" U_END;
}

Theme::~Theme() {
	U_DEBUG << "Theme::~Theme()" U_END;
}

//!!!review, doc, and test
void Theme::load(MainWindow *mainWindow, const QString &name) {
	QFile theme("themes/" + name + "/theme.xml");

	if (!theme.open(QFile::ReadOnly | QFile::Text)) {
		U_ERROR << "Cannot open theme file: " + theme.fileName() U_END;

		return;
	}

	m_inElement = false;
	bool inTheme = false;
	QXmlStreamReader xml(&theme);
	while (!xml.atEnd()) {
		xml.readNext();

		if (xml.isStartElement()) {
			if (xml.name() == "theme") {
				if (inTheme) {
					U_ERROR << "ERROR: Nested \"theme\" element: " << xml.lineNumber() U_END;

					break; // while
				}
				else {
					U_DEBUG << "Start \"theme\"" U_END;
					inTheme = true;
				}
			}
			else if (inTheme && (xml.name() == "style")) {
				QString href = xml.attributes().value("href").toString();
				if (href.isEmpty()) {
					U_ERROR << "ERROR: Empty or missing \"href\" attribute: " << href << ": " << xml.lineNumber() U_END;

					break; // while
				}
				else {
					U_DEBUG << "Loading external style sheet: " << href U_END;
					QFile style("themes/" + name + "/" + href);
					if (!style.open(QFile::ReadOnly | QFile::Text)) {
						U_ERROR << "Cannot open style file: " + style.fileName() U_END;

						break; // while
					}
					U_APP->setStyleSheet(QTextStream(&style).readAll());
				}
			}
			else if (inTheme && (xml.name() == "element")) {
				if (m_inElement) {
					U_ERROR << "ERROR: Nested \"element\" element: " << xml.lineNumber() U_END;

					break; // while
				}
				else {
					U_DEBUG << "Start \"element\"" U_END;
					m_inElement = true;

					QString id = xml.attributes().value("id").toString();
					QWidget *widget = mainWindow->getElementById(id);
					if (!widget) {
						U_ERROR << "ERROR: Invalid or missing \"id\" attribute: " << id << ": " << xml.lineNumber() U_END;

						break; // while
					}
					else {
						if (!processElement(xml, widget))
							break; // while
					}
				}
			}
			else {
				U_ERROR << "ERROR: Unknown element: " << xml.name().toString() << ": " << xml.lineNumber() U_END;

				break; // while
			}
		}
		else if (xml.isEndElement()) {
			if (inTheme && (xml.name() == "theme")) {
				U_DEBUG << "End \"theme\"" U_END;
				inTheme = false;
			}
		}
	}
}

// private

bool Theme::processElement(QXmlStreamReader &xml, QWidget *widget) {
	U_DEBUG << "\tProcessing " << widget->objectName() << " element" U_END;

	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			if (xml.name() == "property") {
				if (!processProperty(xml, widget))
					return false;
			}
			else if (xml.name() == "style") {//!!!
				if (!processStyle(xml, widget))
					return false;
			}
			else {
				U_ERROR << "ERROR: Unknown element: " << xml.name().toString() << ": " << xml.lineNumber() U_END;

				return false;
			}
		}
		else if (xml.isEndElement() && (xml.name() == "element")) {
			U_DEBUG << "End \"element\"" U_END;
			m_inElement = false;

			return true;
		}
	}

	return true;
}

bool Theme::processProperty(QXmlStreamReader &xml, QWidget *widget) {
	QString name = xml.attributes().value("name").toString();
	if (name.isEmpty()) {
		U_ERROR << "\tERROR: Empty or missing \"name\" attribute" U_END;

		return false;
	}

	QString value = xml.attributes().value("value").toString();
	if (value.isEmpty()) {
		U_ERROR << "\tERROR: Empty or missing \"value\" attribute" U_END;

		return false;
	}

	U_DEBUG << "\tProcessing " << name << " property" U_END;

	widget->setProperty(name.toLatin1(), value);

	return true;
}

bool Theme::processStyle(QXmlStreamReader &xml, QWidget *widget) {
	U_DEBUG << "\tProcessing style" U_END;

//!!!clear all styles before load
	QString style = xml.readElementText();
	if (!style.isEmpty())
		widget->setStyleSheet(style);

	//U_DEBUG << style U_END;

	return true;
}
