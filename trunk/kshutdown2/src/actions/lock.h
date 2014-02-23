// lock.h - Lock
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

#ifndef KSHUTDOWN_LOCK_H
#define KSHUTDOWN_LOCK_H

#include "../kshutdown.h"

#ifdef KS_DBUS
class QDBusInterface;
#endif // KS_DBUS

class LockAction: public KShutdown::Action {
public:
	virtual bool onAction();
	#ifdef KS_DBUS
	static QDBusInterface *getQDBusInterface();
	#endif // KS_DBUS
	inline static LockAction *self() {
		if (!m_instance)
			m_instance = new LockAction();

		return m_instance;
	}
private:
	Q_DISABLE_COPY(LockAction)
	static LockAction *m_instance;
	#ifdef KS_DBUS
	static QDBusInterface *m_qdbusInterface;
	#endif // KS_DBUS
	LockAction();
};

#endif // KSHUTDOWN_LOCK_H
