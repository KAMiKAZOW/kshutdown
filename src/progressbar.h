// main.h - A progress bar widget
// Copyright (C) 2008  Konrad Twardowski
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

#ifndef __PROGRESSBAR_H__
#define __PROGRESSBAR_H__

#include <QWidget>

class ProgressBar: public QWidget {
	Q_OBJECT
public:
	enum Position { TOP, BOTTOM };
	virtual ~ProgressBar();
	inline static void freeInstance() {
		if (m_instance) {
			delete m_instance;
			m_instance = 0;
		}
	}
	inline static bool isInstance() { return m_instance; }
	inline static ProgressBar *self() {
		if (!m_instance)
			m_instance = new ProgressBar();

		return m_instance;
	}
	inline Position position() const { return m_position; }
	void setPosition(const Position value, const bool updateConfig);
	void setHeight(const int value);
	void setProgress(const int complete);
	void setTotal(const int total);
protected:
	void mousePressEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);
private:
	int m_complete;
	int m_total;
	Position m_position;
	static ProgressBar *m_instance;
	ProgressBar();
private slots:
	void onSetBottomPosition();
	void onSetTopPosition();
};

#endif // __PROGRESSBAR_H__