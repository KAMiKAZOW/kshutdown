TEMPLATE = app
TARGET = kshutdown
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT

exists(portable.pri) {
	include(portable.pri)
	message("Building portable version...")
}

QT += widgets

# HACK: https://bugreports.qt.io/browse/QTBUG-52129
CONFIG += c++14

QMAKE_CXXFLAGS += -Wextra -Wpedantic -Wswitch-enum

unix {
	haiku-g++ {
		message("Building without D-Bus")
	}
	else {
		QT += dbus
	}
}

win32 {
	LIBS += -lpowrprof
	RC_FILE = kshutdown.rc
	QT += winextras
}

HEADERS += \
	bookmarks.h \
	commandline.h \
	config.h \
	infowidget.h \
	kshutdown.h \
	log.h \
	mainwindow.h \
	mod.h \
	password.h \
	plugins.h \
	preferences.h \
	progressbar.h \
	pureqt.h \
	stats.h \
	udialog.h \
	usystemtray.h \
	utils.h \
	version.h \
	actions/bootentry.h \
	actions/extras.h \
	actions/test.h \
	triggers/idlemonitor.h \
	triggers/processmonitor.h

SOURCES += \
	bookmarks.cpp \
	commandline.cpp \
	config.cpp \
	infowidget.cpp \
	kshutdown.cpp \
	log.cpp \
	main.cpp \
	mainwindow.cpp \
	mod.cpp \
	password.cpp \
	plugins.cpp \
	preferences.cpp \
	progressbar.cpp \
	stats.cpp \
	udialog.cpp \
	usystemtray.cpp \
	utils.cpp \
	actions/bootentry.cpp \
	actions/extras.cpp \
	actions/lock.cpp \
	actions/test.cpp \
	triggers/idlemonitor.cpp \
	triggers/processmonitor.cpp

RESOURCES = kshutdown.qrc

unix {
	target.path = /usr/bin
	target.extra = \
		install -m 755 -p kshutdown "$(INSTALL_ROOT)/usr/bin/kshutdown"; \
		ln -fs "$(INSTALL_ROOT)/usr/bin/kshutdown" "$(INSTALL_ROOT)/usr/bin/kshutdown-qt"
	target.uninstall = rm -f "$(INSTALL_ROOT)/usr/bin/kshutdown" "$(INSTALL_ROOT)/usr/bin/kshutdown-qt"

	icon16.path = /usr/share/icons/hicolor/16x16/apps
	icon16.extra = install -m 644 -p images/hi16-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/16x16/apps/kshutdown.png"
	icon16.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/16x16/apps/kshutdown.png"

	icon22.path = /usr/share/icons/hicolor/22x22/apps
	icon22.extra = install -m 644 -p images/hi22-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/22x22/apps/kshutdown.png"
	icon22.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/22x22/apps/kshutdown.png"

	icon32.path = /usr/share/icons/hicolor/32x32/apps
	icon32.extra = install -m 644 -p images/hi32-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/32x32/apps/kshutdown.png"
	icon32.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/32x32/apps/kshutdown.png"

	icon48.path = /usr/share/icons/hicolor/48x48/apps
	icon48.extra = install -m 644 -p images/hi48-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/48x48/apps/kshutdown.png"
	icon48.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/48x48/apps/kshutdown.png"

	icon64.path = /usr/share/icons/hicolor/64x64/apps
	icon64.extra = install -m 644 -p images/hi64-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/64x64/apps/kshutdown.png"
	icon64.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/64x64/apps/kshutdown.png"

	icon128.path = /usr/share/icons/hicolor/128x128/apps
	icon128.extra = install -m 644 -p images/hi128-app-kshutdown.png "$(INSTALL_ROOT)/usr/share/icons/hicolor/128x128/apps/kshutdown.png"
	icon128.uninstall = rm -f "$(INSTALL_ROOT)/usr/share/icons/hicolor/128x128/apps/kshutdown.png"

	shortcut.path = /usr/share/applications
	shortcut.files += kshutdown.desktop
	
	INSTALLS += target icon16 icon22 icon32 icon48 icon64 icon128 shortcut
}
