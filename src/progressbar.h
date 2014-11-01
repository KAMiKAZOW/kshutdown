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

#ifndef KSHUTDOWN_PROGRESSBAR_H
#define KSHUTDOWN_PROGRESSBAR_H

#include <QWidget>

class QActionGroup;

class ProgressBar: public QWidget {
	Q_OBJECT
public:
	enum/* non-class */Size { SmallSize = 2, NormalSize = 3, MediumSize = 6, LargeSize = 9 };
	ProgressBar();
	virtual ~ProgressBar();
	inline Qt::Alignment alignment() const { return m_alignment; }
	void setAlignment(const Qt::Alignment value, const bool updateConfig);
	void setHeight(const int value);
	void setTotal(const int total);
	void setValue(const int value);
protected:
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void paintEvent(QPaintEvent *e) override;
private:
	Q_DISABLE_COPY(ProgressBar)
	int m_completeWidth;
	int m_total;
	int m_value;
	Qt::Alignment m_alignment;
	bool authorize();
	void makeRadioButton(QAction *action, QActionGroup *group, const bool checked);
	void setSize(const Size size);
private slots:
	void onHide();
	void onResize(int screen);
	void onSetBottomAlignment();
	void onSetColor();
	void onSetSizeLarge();
	void onSetSizeMedium();
	void onSetSizeNormal();
	void onSetSizeSmall();
	void onSetTopAlignment();
};

#endif // KSHUTDOWN_PROGRESSBAR_H
