// main.cpp - A progress bar widget
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

#include "progressbar.h"

#include "commandline.h"
#include "config.h"
#include "mainwindow.h"
#include "mod.h"
#include "password.h"
#include "utils.h"

#include <QColorDialog>
#include <QDebug>
#include <QDesktopWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#ifdef Q_OS_WIN32
	#include <QWinTaskbarProgress>
#endif // Q_OS_WIN32

// public

void ProgressBar::setAlignment(const Qt::Alignment value, const bool updateConfig) {
	if (updateConfig) {
		m_alignmentVar->setInt(value);
		m_alignmentVar->write();
		Config::user()->sync();
	}

	m_alignment = value;
	QDesktopWidget *desktop = QApplication::desktop();

	int margin = 2_px;

	resize(desktop->width() - margin * 2, height());
	if (m_alignment.testFlag(Qt::AlignBottom)) {
		move(margin, desktop->height() - height());
	}
	// Qt::AlignTop
	else {
		move(margin, 0_px);
	}
}


void ProgressBar::setDemo(const bool active) {
	qDebug() << "ProgressBar::setDemo: " << active;

	if (active) {
		m_demoWidth = 0_px;
		m_demoTimer->start(50);
	}
	else {
		m_demoTimer->stop();
	}
}

void ProgressBar::setHeight(const int value) {
	resize(width(), qMax(2_px, value));
}

void ProgressBar::setTotal(const int total) {
	m_total = total;
	m_completeWidth = 0_px; // reset
}

void ProgressBar::setValue(const int value) {
	m_value = value;
	
	//U_DEBUG << "m_total = " << m_total U_END;
	//U_DEBUG << "m_value = " << m_value U_END;

	if ((m_total == 0) || (m_value == -1)) {
		updateTaskbar(-1, -1);

		return;
	}

	double progress = 1 - (double)m_value / (double)m_total;
	progress = qBound(0.0, progress, 1.0);
	updateTaskbar(progress, m_value);

	int newCompleteWidth = (int)((float)width() * ((float)(m_total - m_value) / (float)m_total));
	if (newCompleteWidth != m_completeWidth) {
		m_completeWidth = newCompleteWidth;
		repaint();
	}
}

void ProgressBar::updateTaskbar(const double progress, const int seconds) {
	#if defined(Q_OS_LINUX) && defined(QT_DBUS_LIB)
	// CREDITS: https://askubuntu.com/questions/65054/unity-launcher-api-for-c/310940
	// DOC: https://wiki.ubuntu.com/Unity/LauncherAPI

	if (!Utils::isUnity() && !Utils::isKDE())
		return;

	QDBusMessage taskbarMessage = QDBusMessage::createSignal(
		"/",
		"com.canonical.Unity.LauncherEntry",
		"Update"
	);

	taskbarMessage << "application://kshutdown.desktop";

	QVariantMap properties;

	bool countVisible = (seconds >= 0) && (seconds <= 60);
	bool urgent = seconds <= 60;
	properties["count"] = qint64(countVisible ? seconds : 0);
	properties["count-visible"] = countVisible;

	bool progressVisible = (progress >= 0);
	properties["progress"] = progressVisible ? progress : 0;
	properties["progress-visible"] = progressVisible;

	properties["urgent"] = urgent;

/* TEST:
	qDebug() << "";
	qDebug() << progress;
	qDebug() << m_value << ".." << m_total;
	qDebug() << urgent;
*/

	taskbarMessage << properties;
	QDBusConnection::sessionBus()
		.send(taskbarMessage);
	#elif defined(Q_OS_WIN32)
	Q_UNUSED(seconds)

	QWinTaskbarButton *winTaskbarButton = MainWindow::self()->winTaskbarButton();
	if (winTaskbarButton) {
		QWinTaskbarProgress *taskbarProgress = winTaskbarButton->progress();
		taskbarProgress->setRange(0, 100);
		taskbarProgress->setValue((int)(progress * 100));
		taskbarProgress->setVisible(progress >= 0);
	}
	#else
	Q_UNUSED(progress)
	Q_UNUSED(seconds)
	#endif // defined(Q_OS_LINUX) && defined(QT_DBUS_LIB)
}

// protected

void ProgressBar::contextMenuEvent(QContextMenuEvent *e) {
	if (
		CLI::isArg("hide-ui") ||
		Utils::isRestricted("kshutdown/progress_bar/menu")
	)
		return;

	// show popup menu
	auto *menu = new QMenu(this);

	Utils::addTitle(menu, qApp->windowIcon(), i18n("Progress Bar") + " - " + QApplication::applicationDisplayName());

	bool canConfigure = !Utils::isRestricted("action/options_configure");

	menu->addAction(i18n("Hide"), this, SLOT(onHide()));
	menu->addAction(i18n("Set Color..."), this, SLOT(onSetColor()))
		->setEnabled(canConfigure);

	// position

	auto *positionMenu = new QMenu(i18n("Position"), menu);

	auto *positionGroup = new QActionGroup(this);

	auto *a = positionMenu->addAction(i18n("Top"), this, SLOT(onSetTopAlignment()));
	makeIcon(a, Qt::AlignTop, height());
	makeRadioButton(a, positionGroup, m_alignment.testFlag(Qt::AlignTop));
		
	a = positionMenu->addAction(i18n("Bottom"), this, SLOT(onSetBottomAlignment()));
	makeIcon(a, Qt::AlignBottom, height());
	makeRadioButton(a, positionGroup, m_alignment.testFlag(Qt::AlignBottom));

	auto *sizeMenu = new QMenu(i18n("Size"), menu);

	// size

	auto *sizeGroup = new QActionGroup(this);
		
	a = sizeMenu->addAction(i18n("Small"), this, SLOT(onSetSizeSmall()));
	makeIcon(a, m_alignment, SmallSize);
	makeRadioButton(a, sizeGroup, height() == SmallSize);

	a = sizeMenu->addAction(i18n("Normal"), this, SLOT(onSetSizeNormal()));
	makeIcon(a, m_alignment, NormalSize);
	makeRadioButton(a, sizeGroup, height() == NormalSize);

	a = sizeMenu->addAction(i18n("Medium"), this, SLOT(onSetSizeMedium()));
	makeIcon(a, m_alignment, MediumSize);
	makeRadioButton(a, sizeGroup, height() == MediumSize);

	a = sizeMenu->addAction(i18n("Large"), this, SLOT(onSetSizeLarge()));
	makeIcon(a, m_alignment, LargeSize);
	makeRadioButton(a, sizeGroup, height() == LargeSize);

	menu->addSeparator();
	menu->addMenu(positionMenu);
	menu->addMenu(sizeMenu);
	menu->addSeparator();
	menu->addAction(MainWindow::self()->cancelAction());

	menu->popup(e->globalPos());
	e->accept();
}

void ProgressBar::mousePressEvent(QMouseEvent *e) {
	if (!CLI::isArg("hide-ui") && (e->button() == Qt::LeftButton)) {
		auto *mainWindow = MainWindow::self();
		mainWindow->show();
		mainWindow->activateWindow();

		e->accept();

		return;
	}

	QWidget::mousePressEvent(e);
}

void ProgressBar::paintEvent(QPaintEvent *e) {
	Q_UNUSED(e)

	QPainter g(this);

	int x = 0_px;
	int y = 0_px;
	int w = width();
	int h = height();

	if (m_demoTimer->isActive()) {
		g.fillRect(x, y, w, h, Qt::black);
		g.fillRect(x, y, qMin(m_demoWidth, w), h, m_demoColor);
	}
	else {
		g.fillRect(x, y, w, h, palette().window());

		if ((m_completeWidth <= 0_px) || (m_total <= 0) || (m_value <= 0))
			return;

		g.fillRect(x, y, m_completeWidth, h, palette().windowText());
	}
}

// private

ProgressBar::ProgressBar() // public
	: QWidget(
		nullptr,
		Qt::FramelessWindowHint |
		Qt::NoDropShadowWindowHint |
		Qt::WindowDoesNotAcceptFocus |
		Qt::WindowStaysOnTopHint |

// FIXME: GNOME - no context menu, no tool tip
		Qt::X11BypassWindowManagerHint |

		Qt::Tool
	) {

	m_alignmentVar = new Var("Progress Bar", "Alignment", Qt::AlignTop);
	m_foregroundColorVar = new Var("Progress Bar", "Foreground Color", QColor(0xF8FFBF/* lime 1 */));
	m_sizeVar = new Var("Progress Bar", "Size", NormalSize);

	//U_DEBUG << "ProgressBar::ProgressBar()" U_END;

	m_demoTimer = new QTimer(this);
	connect(m_demoTimer, SIGNAL(timeout()), SLOT(onDemoTimeout()));

	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setObjectName("progress-bar");

	setWindowTitle(i18n("Progress Bar") + " - " + QApplication::applicationDisplayName());
	setToolTip(windowTitle());

	QVariant opacityVariant = Mod::get("ui-progress-bar-opacity", 1.0f);
	bool opacityOK = false;
	qreal opacity = opacityVariant.toReal(&opacityOK);
	if (opacityOK)
		setWindowOpacity(opacity);

	QPalette p;

	QColor background = Mod::getColor("ui-progress-bar-window-color", Qt::black);
	p.setColor(QPalette::Window, background);

	QColor defaultForeground = m_foregroundColorVar->getDefault().value<QColor>();
	m_demoColor = defaultForeground;

	QColor foreground = m_foregroundColorVar->getColor();
	p.setColor(QPalette::WindowText, (foreground.rgb() == background.rgb()) ? defaultForeground : foreground);
	setPalette(p);

	setHeight(qBound(SmallSize, static_cast<Size>(m_sizeVar->getInt()), LargeSize));

	setAlignment(static_cast<Qt::Alignment>(m_alignmentVar->getInt()), false);
	
	QDesktopWidget *desktop = QApplication::desktop();
	connect(desktop, SIGNAL(resized(int)), SLOT(onResize(int)));
}

bool ProgressBar::authorize() {
	return PasswordDialog::authorizeSettings(this);
}

void ProgressBar::makeIcon(QAction *action, const Qt::Alignment alignment, const int size) {
	int iconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
	QPixmap pixmap(iconSize, iconSize);

	QPainter painter(&pixmap);
	painter.fillRect(0, 0, iconSize, iconSize, palette().window());

	int by;
	switch (alignment) {
		case Qt::AlignBottom:
			by = iconSize - size;
			break;
		default: // Qt::AlignTop
			by = 0;
			break;
	}
	painter.fillRect(0, by, iconSize, size, palette().windowText());

	action->setIcon(pixmap);
}

void ProgressBar::makeRadioButton(QAction *action, QActionGroup *group, const bool checked) {
	action->setActionGroup(group);
	action->setCheckable(true);
	action->setChecked(checked);
	action->setEnabled(!Utils::isRestricted("action/options_configure"));
}

void ProgressBar::setSize(const Size size) {
	setHeight(size);
	setAlignment(m_alignment, false);

	m_sizeVar->setInt(size);
	m_sizeVar->write();
	Config::user()->sync();
}

// private slots

void ProgressBar::onDemoTimeout() {
	m_demoWidth += 5_px;
	if (m_demoWidth > width() / 3) {
		m_demoWidth = 0_px;
		m_demoTimer->stop();
		repaint();

		return;
	}

	int h, s, v;
	m_demoColor.getHsv(&h, &s, &v);
	h = (h + 5) % 360;
	m_demoColor.setHsv(h, s, v);

	repaint();
}

void ProgressBar::onHide() {
	if (!authorize())
		return;

	hide();
}

void ProgressBar::onResize(int screen) {
	Q_UNUSED(screen)

	// update window location on screen size change
	setAlignment(m_alignment, false);
}

void ProgressBar::onSetBottomAlignment() {
	if (!authorize())
		return;

	setAlignment(Qt::AlignBottom, true);
}

void ProgressBar::onSetColor() {
	if (!authorize())
		return;

	QColor currentColor = palette().color(QPalette::WindowText);
	QColor newColor = QColorDialog::getColor(
		currentColor,
		this,
		QString::null // use default title
	);

	if (newColor.isValid()) {
		QPalette p(palette());
		p.setColor(QPalette::WindowText, newColor);
		setPalette(p);
		repaint();

		m_foregroundColorVar->setColor(newColor);
		m_foregroundColorVar->write();
		Config::user()->sync();
	}
}

void ProgressBar::onSetSizeLarge() {
	if (!authorize())
		return;

	setSize(LargeSize);
}

void ProgressBar::onSetSizeMedium() {
	if (!authorize())
		return;

	setSize(MediumSize);
}

void ProgressBar::onSetSizeNormal() {
	if (!authorize())
		return;

	setSize(NormalSize);
}

void ProgressBar::onSetSizeSmall() {
	if (!authorize())
		return;

	setSize(SmallSize);
}

void ProgressBar::onSetTopAlignment() {
	if (!authorize())
		return;

	setAlignment(Qt::AlignTop, true);
}
