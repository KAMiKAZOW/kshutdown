// config.cpp - Configuration
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

#include "pureqt.h"

#ifdef KS_PURE_QT
	#include <QSettings>
#endif // KS_PURE_QT

#include "config.h"

// private

Config *Config::m_user = 0;

// public

Config::~Config() {
	U_DEBUG << "Config::~Config()" U_END;
#ifdef KS_PURE_QT
	if (m_engine) {
		delete m_engine;
		m_engine = 0;
	}
#endif // KS_PURE_QT
}

void Config::beginGroup(const QString &name) {
#ifdef KS_NATIVE_KDE
	m_group = m_engine->group(name);
#else
	m_engine->beginGroup(name);
#endif // KS_NATIVE_KDE
}

void Config::endGroup() {
#ifdef KS_PURE_QT
	m_engine->endGroup();
#endif // KS_PURE_QT
}

bool Config::blackAndWhiteSystemTrayIcon() {
	return readBool("General", "Black and White System Tray Icon", false);
}

void Config::setBlackAndWhiteSystemTrayIcon(const bool value) {
	return write("General", "Black and White System Tray Icon", value);
}

bool Config::confirmAction() {
	return readBool("General", "Confirm Action", false);
}

void Config::setConfirmAction(const bool value) {
	write("General", "Confirm Action", value);
}

bool Config::lockScreenBeforeHibernate() {
	return readBool("General", "Lock Screen Before Hibernate", true);
}

void Config::setLockScreenBeforeHibernate(const bool value) {
	write("General", "Lock Screen Before Hibernate", value);
}

bool Config::minimizeToSystemTrayIcon() {
	return readBool("General", "Minimize To System Tray Icon", true);
}

void Config::setMinimizeToSystemTrayIcon(const bool value) {
	write("General", "Minimize To System Tray Icon", value);
}

bool Config::progressBarEnabled() {
	return readBool("Progress Bar", "Enabled", false);
}

void Config::setProgressBarEnabled(const bool value) {
	write("Progress Bar", "Enabled", value);
}

Qt::Alignment Config::progressBarAlignment() {
	Config *config = user();
	config->beginGroup("Progress Bar");
	int result = config->read("Alignment", Qt::AlignTop).toInt();
	config->endGroup();

	return (Qt::Alignment)result;
}

void Config::setProgressBarAlignment(const Qt::Alignment value) {
	Config *config = user();
	config->beginGroup("Progress Bar");
	config->write("Alignment", (int)value);
	config->endGroup();
}

bool Config::systemTrayIconEnabled() {
	return readBool("General", "System Tray Icon Enabled", true);
}

void Config::setSystemTrayIconEnabled(const bool value) {
	write("General", "System Tray Icon Enabled", value);
}

QVariant Config::read(const QString &key, const QVariant &defaultValue) {
#ifdef KS_NATIVE_KDE
	return m_group.readEntry(key, defaultValue);
#else
	return m_engine->value(key, defaultValue);
#endif // KS_NATIVE_KDE
}

void Config::write(const QString &key, const QVariant &value) {
#ifdef KS_NATIVE_KDE
	m_group.writeEntry(key, value);
#else
	m_engine->setValue(key, value);
#endif // KS_NATIVE_KDE
}

bool Config::readBool(const QString &group, const QString &key, const bool defaultValue) {
	Config *config = user();
	config->beginGroup(group);
	bool result = config->read(key, defaultValue).toBool();
	config->endGroup();

	return result;
}

void Config::write(const QString &group, const QString &key, const bool value) {
	Config *config = user();
	config->beginGroup(group);
	config->write(key, value);
	config->endGroup();
}

void Config::removeAllKeys() {
#ifdef KS_NATIVE_KDE
	m_group.deleteGroup();
#else
	m_engine->remove("");
#endif // KS_NATIVE_KDE
}

void Config::sync() {
	m_engine->sync();
}

// private

Config::Config() :
	QObject(0) {

	U_DEBUG << "Config::Config()" U_END;
	
#ifdef KS_NATIVE_KDE
	m_engine = KGlobal::config().data();
#else
	#ifdef KS_PORTABLE
		m_engine = new QSettings(QApplication::applicationDirPath() + "\\kshutdown.ini", QSettings::IniFormat);
	#else
		m_engine = new QSettings();
	#endif // KS_PORTABLE
#endif // KS_NATIVE_KDE
}
