//
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

Config *Config::m_user = 0;

// public

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
#endif //KS_PURE_QT
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

void Config::sync() {
	m_engine->sync();
}

// private

Config::Config() {
#ifdef KS_NATIVE_KDE
	m_engine = KGlobal::config().data();
#else
	m_engine = new QSettings();
#endif // KS_NATIVE_KDE
}
