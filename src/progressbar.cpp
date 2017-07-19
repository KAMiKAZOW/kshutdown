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

#include "config.h"
#include "mainwindow.h"
#include "mod.h"
#include "password.h"
#include "progressbar.h"
#include "pureqt.h"
#include "utils.h"

#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#ifdef KS_V5
#ifdef KS_DBUS
	#include <QDBusConnection>
	#include <QDBusMessage>
#endif // KS_DBUS
#endif // KS_V5

#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	#include <KColorDialog>
#else
	#include <QColorDialog>
#endif // KS_NATIVE_KDE

// public

ProgressBar::~ProgressBar() {
	//U_DEBUG << "ProgressBar::~ProgressBar()" U_END;
}

void ProgressBar::setAlignment(const Qt::Alignment value, const bool updateConfig) {
	if (updateConfig) {
		Config *config = Config::user();
		config->setProgressBarAlignment(value);
		config->sync();
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
	U_DEBUG << "ProgressBar::setDemo: " << active U_END;

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

	#ifdef KS_V5
	if (m_value == -1) {
		updateTaskbar(-1, -1, false);
	}
	else {
		double progress = 1 - (double)m_value / (double)m_total;//!!!div zero
		progress = qBound(0.0, progress, 1.0);
		updateTaskbar(progress, m_value, (m_value <= 55));//!!!60
	}
	#endif // KS_V5

	if ((m_total == 0) || (m_value == -1))
		return;

	int newCompleteWidth = (int)((float)width() * ((float)(m_total - m_value) / (float)m_total));
	if (newCompleteWidth != m_completeWidth) {
		m_completeWidth = newCompleteWidth;
		repaint();
	}
}

void ProgressBar::updateTaskbar(const double progress, const int seconds, const bool urgent) {
#ifdef KS_V5
	#if defined(Q_OS_LINUX) && defined(KS_DBUS)
	// CREDITS: https://askubuntu.com/questions/65054/unity-launcher-api-for-c/310940
	// DOC: https://wiki.ubuntu.com/Unity/LauncherAPI

	if (!Utils::isUnity() && !Utils::isKDE())
		return;

	QDBusMessage taskbarMessage = QDBusMessage::createSignal(
		"/",
		"com.canonical.Unity.LauncherEntry",
		"Update"
	);

	#ifdef KS_PURE_QT
	taskbarMessage << "application://kshutdown-qt.desktop";
	#else
	taskbarMessage << "application://kshutdown.desktop";
	#endif // KS_PURE_QT

	QVariantMap properties;

	U_DEBUG << "" U_END;
	U_DEBUG << progress U_END;
	U_DEBUG << m_value U_END;
	U_DEBUG << m_total U_END;

	bool countVisible = (seconds >= 0) && (seconds <= 55);//!!!60
	properties["count"] = qint64(countVisible ? seconds : 0);
	properties["count-visible"] = countVisible;

	bool progressVisible = (progress >= 0);
	properties["progress"] = progressVisible ? progress : 0;
	properties["progress-visible"] = progressVisible;

	properties["urgent"] = urgent;

	taskbarMessage << properties;
	QDBusConnection::sessionBus()
		.send(taskbarMessage);
	#endif // defined(Q_OS_LINUX) && defined(KS_DBUS)

	#ifdef Q_OS_WIN32
// TODO: taskbar progress
	Q_UNUSED(progress)
	Q_UNUSED(seconds)
	Q_UNUSED(urgent)
	#endif // Q_OS_WIN32
#else
	Q_UNUSED(progress)
	Q_UNUSED(seconds)
	Q_UNUSED(urgent)
#endif // KS_V5
}

// protected

void ProgressBar::contextMenuEvent(QContextMenuEvent *e) {
	if (
		Utils::isArg("hide-ui") ||
		Utils::isRestricted("kshutdown/progress_bar/menu")
	)
		return;

	// show popup menu
	U_MENU *menu = new U_MENU(this);

	Utils::addTitle(menu, U_APP->windowIcon(),
		i18n("Progress Bar") + " - " +
		#ifdef KS_KF5
		QApplication::applicationDisplayName()
		#elif defined(KS_NATIVE_KDE)
		KGlobal::caption()
		#else
		i18n("KShutdown")
		#endif // KS_KF5
	);

	bool canConfigure = !Utils::isRestricted("action/options_configure");

	menu->addAction(i18n("Hide"), this, SLOT(onHide()));
	menu->addAction(i18n("Set Color..."), this, SLOT(onSetColor()))
		->setEnabled(canConfigure);

	// position

	U_MENU *positionMenu = new U_MENU(i18n("Position"), menu);

	QActionGroup *positionGroup = new QActionGroup(this);

	QAction *a = positionMenu->addAction(i18n("Top"), this, SLOT(onSetTopAlignment()));
	makeRadioButton(a, positionGroup, m_alignment.testFlag(Qt::AlignTop));
		
	a = positionMenu->addAction(i18n("Bottom"), this, SLOT(onSetBottomAlignment()));
	makeRadioButton(a, positionGroup, m_alignment.testFlag(Qt::AlignBottom));

	U_MENU *sizeMenu = new U_MENU(i18n("Size"), menu);

	// size

	QActionGroup *sizeGroup = new QActionGroup(this);
		
	a = sizeMenu->addAction(i18n("Small"), this, SLOT(onSetSizeSmall()));
	makeRadioButton(a, sizeGroup, height() == SmallSize);

	a = sizeMenu->addAction(i18n("Normal"), this, SLOT(onSetSizeNormal()));
	makeRadioButton(a, sizeGroup, height() == NormalSize);

	a = sizeMenu->addAction(i18n("Medium"), this, SLOT(onSetSizeMedium()));
	makeRadioButton(a, sizeGroup, height() == MediumSize);

	a = sizeMenu->addAction(i18n("Large"), this, SLOT(onSetSizeLarge()));
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
	if (!Utils::isArg("hide-ui") && (e->button() == Qt::LeftButton)) {
		MainWindow *mainWindow = MainWindow::self();
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
		0,
		Qt::FramelessWindowHint |
		#if QT_VERSION >= 0x050000
		Qt::NoDropShadowWindowHint |
		Qt::WindowDoesNotAcceptFocus |
		#endif // QT_VERSION
		Qt::WindowStaysOnTopHint |
		Qt::X11BypassWindowManagerHint |
		Qt::Tool
	) {

	//U_DEBUG << "ProgressBar::ProgressBar()" U_END;

	m_demoTimer = new QTimer(this);
	connect(m_demoTimer, SIGNAL(timeout()), SLOT(onDemoTimeout()));

	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setObjectName("progress-bar");

	setWindowTitle(i18n("Progress Bar") + " - " + "KShutdown");
	setToolTip(windowTitle());

	QVariant opacityVariant = Mod::get("ui-progress-bar-opacity", 1.0f);
	bool opacityOK = false;
	qreal opacity = opacityVariant.toReal(&opacityOK);
	if (opacityOK)
		setWindowOpacity(opacity);

	QPalette p;

	QColor background = Mod::getColor("ui-progress-bar-window-color", Qt::black);
	p.setColor(QPalette::Window, background);

	Config *config = Config::user();
	config->beginGroup("Progress Bar");

	QColor defaultForeground = 0xF8FFBF /* lime 1 */;
	m_demoColor = defaultForeground;

	QColor foreground = config->read("Foreground Color", defaultForeground).value<QColor>();
	
	setHeight(qBound(SmallSize, (Size)config->read("Size", NormalSize).toInt(), LargeSize));
	
	config->endGroup();
	p.setColor(QPalette::WindowText, (foreground.rgb() == background.rgb()) ? defaultForeground : foreground);
	
	setPalette(p);

	setAlignment(Config::user()->progressBarAlignment(), false);
	
	QDesktopWidget *desktop = QApplication::desktop();
	connect(desktop, SIGNAL(resized(int)), SLOT(onResize(int)));
}

bool ProgressBar::authorize() {
	return PasswordDialog::authorizeSettings(this);
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

	Config *config = Config::user();
	config->beginGroup("Progress Bar");
	config->write("Size", size);
	config->endGroup();
	config->sync();
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
	#if defined(KS_NATIVE_KDE) && !defined(KS_KF5)
	QColor newColor;
	if (KColorDialog::getColor(newColor, currentColor, this) != KColorDialog::Accepted)
		return;
	#else
	QColor newColor = QColorDialog::getColor(
		currentColor,
		this,
		QString::null // use default title
	);
	#endif // KS_NATIVE_KDE
	if (newColor.isValid()) {
		QPalette p(palette());
		p.setColor(QPalette::WindowText, newColor);
		setPalette(p);
		repaint();

		Config *config = Config::user();
		config->beginGroup("Progress Bar");
		config->write("Foreground Color", newColor);
		config->endGroup();
		config->sync();
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
