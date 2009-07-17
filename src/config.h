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

#include "progressbar.h"
#include "pureqt.h"

#ifdef KS_NATIVE_KDE
	#include <KConfigGroup>
#else
	class QSettings;
#endif

/**
 * A configuration reader/writer.
 *
 * @b WARNING: This class is not thread-safe.
 */
class Config {
public:
	void beginGroup(const QString &name);
	void endGroup();

	static bool confirmAction();
	static void setConfirmAction(const bool value);

	static bool lockScreenBeforeHibernate();
	static void setLockScreenBeforeHibernate(const bool value);

	static bool progressBarEnabled();
	static void setProgressBarEnabled(const bool value);

	static ProgressBar::Position progressBarPosition();
	static void setProgressBarPosition(const ProgressBar::Position value);

	QVariant read(const QString &key, const QVariant &defaultValue);
	void write(const QString &key, const QVariant &value);
	
	static bool readBool(const QString &group, const QString &key, const bool defaultValue);
	static void write(const QString &group, const QString &key, const bool value);
	
	void sync();
	inline static Config *user() {
		if (!m_user)
			m_user = new Config();

		return m_user;
	}
private:
#ifdef KS_NATIVE_KDE
	KConfig *m_engine;
	KConfigGroup m_group;
#else
	QSettings *m_engine;
#endif
	static Config *m_user;
	Config();
};

#endif // KSHUTDOWN_CONFIG_H
