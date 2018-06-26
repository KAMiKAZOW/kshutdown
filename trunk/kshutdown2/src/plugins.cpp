// plugins.cpp - Plugins
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

#include "plugins.h"

#include "config.h"
#include "actions/extras.h"
#include "actions/lock.h"
#include "actions/test.h"
#include "triggers/idlemonitor.h"
#include "triggers/processmonitor.h"

// private:

QHash<QString, Action*> PluginManager::m_actionMap = QHash<QString, Action*>();
QHash<QString, Trigger*> PluginManager::m_triggerMap = QHash<QString, Trigger*>();

QList<Action*> PluginManager::m_actionList = QList<Action*>();
QList<Trigger*> PluginManager::m_triggerList = QList<Trigger*>();

// public:

void PluginManager::add(Action *action) {
	//qDebug() << "Add Action:" << action->id();

	m_actionMap[action->id()] = action;
	m_actionList.append(action);
}

void PluginManager::add(Trigger *trigger) {
	//qDebug() << "Add Trigger:" << trigger->id();

	m_triggerMap[trigger->id()] = trigger;
	m_triggerList.append(trigger);
}

void PluginManager::init() {
	qDebug() << "PluginManager::init (Action)" U_END;

	add(new ShutDownAction());
	add(new RebootAction());
	add(new HibernateAction());
	add(new SuspendAction());
	add(LockAction::self());
	add(new LogoutAction());
	add(Extras::self());
	add(new TestAction());

	qDebug() << "PluginManager::init (Trigger)" U_END;

	add(new NoDelayTrigger());
	add(new TimeFromNowTrigger());
	add(new DateTimeTrigger());
	add(new ProcessMonitor());

	auto *idleMonitor = new IdleMonitor();
	if (idleMonitor->isSupported())
		add(idleMonitor);
	else
		delete idleMonitor;

	Config *config = Config::user();

	foreach (Action *i, actionList()) {
		config->beginGroup("KShutdown Action " + i->id());
		i->readConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, triggerList()) {
		config->beginGroup("KShutdown Trigger " + i->id());
		i->readConfig(config);
		config->endGroup();
	}
}

void PluginManager::writeConfig() {
	Config *config = Config::user();

	foreach (Action *i, actionList()) {
		config->beginGroup("KShutdown Action " + i->id());
		i->writeConfig(config);
		config->endGroup();
	}

	foreach (Trigger *i, triggerList()) {
		config->beginGroup("KShutdown Trigger " + i->id());
		i->writeConfig(config);
		config->endGroup();
	}
}
