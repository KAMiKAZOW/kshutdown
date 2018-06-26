// plugins.h - Plugins
// Copyright (C) 2018  Konrad Twardowski
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

#ifndef KSHUTDOWN_PLUGINS_H
#define KSHUTDOWN_PLUGINS_H

#include "kshutdown.h"

using namespace KShutdown;

class PluginManager final {
public:
	static Action *action(const QString &id) { return m_actionMap[id]; }
	static QList<Action*> actionList() { return m_actionList; }
	static QHash<QString, Action*> actionMap() { return m_actionMap; }
	static void add(Action *action);
	static void add(Trigger *trigger);
	static void init();
	static Trigger *trigger(const QString &id) { return m_triggerMap[id]; }
	static QList<Trigger*> triggerList() { return m_triggerList; }
	static QHash<QString, Trigger*> triggerMap() { return m_triggerMap; }
	static void writeConfig();
private:
	static QHash<QString, Action*> m_actionMap;
	static QHash<QString, Trigger*> m_triggerMap;
	static QList<Action*> m_actionList;
	static QList<Trigger*> m_triggerList;
};

#endif // KSHUTDOWN_PLUGINS_H
