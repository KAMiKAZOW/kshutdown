// config.h - Configuration
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

#ifndef KSHUTDOWN_CONFIG_H
#define KSHUTDOWN_CONFIG_H

#include "pureqt.h"

#include <QColor>

#ifdef KS_PURE_QT
	#include <QSettings>
#endif // KS_PURE_QT

#ifdef KS_KF5
	#include <KConfigGroup>
#endif // KS_KF5

/**
 * A configuration reader/writer.
 *
 * @b WARNING: This class is not thread-safe.
 */
class Config final: public QObject {
public:
	virtual ~Config();

	void beginGroup(const QString &name);
	void endGroup();
	static bool isPortable();

	static bool blackAndWhiteSystemTrayIcon();
	static void setBlackAndWhiteSystemTrayIcon(const bool value);

	static bool confirmAction();
	static void setConfirmAction(const bool value);

	static bool lockScreenBeforeHibernate();
	static void setLockScreenBeforeHibernate(const bool value);

	static bool minimizeToSystemTrayIcon();
	static void setMinimizeToSystemTrayIcon(const bool value);

	static bool progressBarEnabled();
	static void setProgressBarEnabled(const bool value);

	static bool systemTrayIconEnabled();
	static void setSystemTrayIconEnabled(const bool value);

	QVariant read(const QString &key, const QVariant &defaultValue);
	void write(const QString &key, const QVariant &value);
	
	static bool readBool(const QString &group, const QString &key, const bool defaultValue);
	static void write(const QString &group, const QString &key, const bool value);
	
	void removeAllKeys();
	
	static void shutDown() {
		if (m_user) {
			delete m_user;
			m_user = 0;
		}
	}
	void sync();
	static Config *user() {
		if (!m_user)
			m_user = new Config();

		return m_user;
	}
private:
	Q_DISABLE_COPY(Config)
#ifdef KS_KF5
	KConfig *m_engine;
	KConfigGroup m_group;
#else
	QSettings *m_engine;
#endif
	static Config *m_user;
	explicit Config();
};

class Var final {
public:
	explicit Var(const QString &group, const QString &key, const int defaultValue)
		: m_group(group), m_key(key), m_defaultVariant(defaultValue), m_lazyVariant(QVariant()) { }
	explicit Var(const QString &group, const QString &key, const QColor &defaultValue)
		: m_group(group), m_key(key), m_defaultVariant(defaultValue), m_lazyVariant(QVariant()) { }

	QColor getColor() {
		return variant().value<QColor>();
	}

	void set(const QColor &value) {
		m_lazyVariant.setValue(value);
	}

	QVariant getDefault() const { return m_defaultVariant; }

	int getInt() {
		return variant().value<int>();
	}

	void set(const int value) {
		m_lazyVariant.setValue(value);
	}

	void sync();
private:
	QString m_group;
	QString m_key;
	QVariant m_defaultVariant;
	QVariant m_lazyVariant;
	QVariant variant();
};

#endif // KSHUTDOWN_CONFIG_H
