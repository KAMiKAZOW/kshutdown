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

#include <QActionGroup>

class Var;

class ProgressBar: public QWidget {
	Q_OBJECT
public:
	enum/* non-class */Size { SmallSize = 2, NormalSize = 3, MediumSize = 6, LargeSize = 9 };
	explicit ProgressBar();
	virtual ~ProgressBar() = default;
	inline Qt::Alignment alignment() const { return m_alignment; }
	void setAlignment(const Qt::Alignment value, const bool updateConfig);
	void setDemo(const bool active);
	void setHeight(const int value);
	void setTotal(const int total);
	void setValue(const int value);
	void updateTaskbar(const double progress, const int seconds);
protected:
	virtual void contextMenuEvent(QContextMenuEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void paintEvent(QPaintEvent *e) override;
private:
	Q_DISABLE_COPY(ProgressBar)
	int m_completeWidth = 0;
	int m_demoWidth = 0;
	int m_total = 0;
	int m_value = 0;
	Qt::Alignment m_alignment;
	QColor m_demoColor;
	QTimer *m_demoTimer;
	Var *m_alignmentVar;
	Var *m_foregroundColorVar;
	Var *m_sizeVar;
	bool authorize();
	void makeIcon(QAction *action, const Qt::Alignment alignment, const int size);
	void makeRadioButton(QAction *action, QActionGroup *group, const bool checked);
	void setSize(const Size size);
private slots:
	void onDemoTimeout();
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
