// idlemonitor.h - An inactivity monitor
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

#ifndef KSHUTDOWN_IDLEMONITOR_H
#define KSHUTDOWN_IDLEMONITOR_H

#include "../kshutdown.h"

class IdleMonitor: public KShutdown::DateTimeTriggerBase {
	Q_OBJECT
public:
	explicit IdleMonitor();
	virtual ~IdleMonitor();
	virtual bool canActivateAction() override;
	virtual QString getStringOption() override;
	virtual void setStringOption(const QString &option) override;
	virtual QWidget *getWidget() override;
	inline bool isSupported() const { return m_supported; }
	virtual void setState(const State state) override;
protected:
	virtual QDateTime calcEndTime() override;
	virtual void updateStatus() override;
private:
	Q_DISABLE_COPY(IdleMonitor)
	bool m_supported;
	quint32 m_idleTime;
	quint32 getMaximumIdleTime();
	void getSessionIdleTime();
};

#endif // KSHUTDOWN_IDLEMONITOR_H
