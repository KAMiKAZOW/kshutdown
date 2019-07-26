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

#include "config.h"

#include "commandline.h"

#ifdef KS_PURE_QT
	#include <QApplication>
	#include <QDebug>
	#include <QDir>
#else
	#include <KConfig>
	#include <KSharedConfig>
#endif // KS_PURE_QT

// Config

// private

bool Config::m_portable = false;
Config *Config::m_user = nullptr;

// public

Var Config::blackAndWhiteSystemTrayIconVar = Var("General", "Black and White System Tray Icon", false);
Var Config::cancelDefaultVar = Var("General", "Cancel Default", true);
Var Config::confirmActionVar = Var("General", "Confirm Action", false);
Var Config::lockScreenBeforeHibernateVar = Var("General", "Lock Screen Before Hibernate", true);
Var Config::minimizeToSystemTrayIconVar = Var("General", "Minimize To System Tray Icon", true);
Var Config::oldActionNamesVar = Var("General", "Old Action Names", false);
Var Config::progressBarEnabledVar = Var("Progress Bar", "Enabled", false);
Var Config::systemTrayIconEnabledVar = Var("General", "System Tray Icon Enabled", true);

Config::~Config() {
	//qDebug() << "Config::~Config()";
#ifdef KS_PURE_QT
	if (m_engine) {
		delete m_engine;
		m_engine = nullptr;
	}
#endif // KS_PURE_QT
}

void Config::beginGroup(const QString &name) {
#ifdef KS_KF5
	m_group = m_engine->group(name);
#else
	m_engine->beginGroup(name);
#endif // KS_KF5
}

void Config::endGroup() {
#ifdef KS_PURE_QT
	m_engine->endGroup();
#endif // KS_PURE_QT
}

void Config::init() {
	#ifdef KS_PORTABLE
	m_portable = true;
	#else
		#ifdef KS_PURE_QT
		// HACK: QCommandLineParser is not initialized at this stage yet
		//m_portable = CLI::isArg("portable");
		const QStringList &args = QApplication::arguments();
		m_portable = args.contains("-portable") || args.contains("--portable");
		#else
		m_portable = false;
		#endif // KS_PURE_QT
	#endif // KS_PORTABLE
}

QVariant Config::read(const QString &key, const QVariant &defaultValue) {
#ifdef KS_KF5
	return m_group.readEntry(key, defaultValue);
#else
	return m_engine->value(key, defaultValue);
#endif // KS_KF5
}

void Config::write(const QString &key, const QVariant &value) {
#ifdef KS_KF5
	m_group.writeEntry(key, value);
#else
	m_engine->setValue(key, value);
#endif // KS_KF5
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
#ifdef KS_KF5
	m_group.deleteGroup();
#else
	m_engine->remove("");
#endif // KS_KF5
}

void Config::sync() {
	m_engine->sync();
}

// private

Config::Config() :
	QObject(nullptr) {

	//qDebug() << "Config::Config()";
	
#ifdef KS_KF5
	m_engine = KSharedConfig::openConfig().data();
#else
	qDebug() << "Config::isPortable(): " << isPortable();
	if (isPortable())
		m_engine = new QSettings(QApplication::applicationDirPath() + QDir::separator() + "kshutdown.ini", QSettings::IniFormat);
	else
		m_engine = new QSettings();
#endif // KS_KF5
}

// Var

// public

QCheckBox *Var::newCheckBox(const QString &text) {
	auto *result = new QCheckBox(text);
	result->setChecked(getBool());

	return result;
}

void Var::write() {
	QVariant value = variant();

	Config *config = Config::user();
	//qDebug() << "WRITE VAR: [" << m_group << "] " << m_key << " = " << value;

	config->beginGroup(m_group);
	config->write(m_key, value);
	config->endGroup();
}

void Var::write(QCheckBox *checkBox) {
	setBool(checkBox->isChecked());
	write();
}

// private

QVariant Var::variant() {
	if (!m_lazyVariant.isValid()) {
		Config *config = Config::user();

		config->beginGroup(m_group);
		m_lazyVariant = config->read(m_key, m_defaultVariant);
		config->endGroup();

		//qDebug() << "Read var: " << m_group << " / " << m_key << " = " << m_lazyVariant;
	}
	
	return m_lazyVariant;
}
