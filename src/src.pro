TEMPLATE = app
TARGET = kshutdown-qt
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT
exists(portable.pri) {
	include(portable.pri)
	message("Building portable version...")
}

contains(QT_MAJOR_VERSION, 5) {
	QT += widgets
}

QMAKE_CXXFLAGS += -std=c++0x

unix {
	haiku-g++ {
		message("Building without D-Bus")
	}
	else {
		contains(QT_MAJOR_VERSION, 5) {
			QT += dbus
		}
		else {
			CONFIG += qdbus
		}
	}
}

win32 {
	LIBS += -lpowrprof
#	QMAKE_LFLAGS = -static-libgcc
}

# Input

HEADERS += bookmarks.h commandline.h config.h infowidget.h kshutdown.h mainwindow.h password.h preferences.h progressbar.h pureqt.h udialog.h usystemtray.h utils.h version.h \
	actions/extras.h \
	actions/test.h \
	triggers/idlemonitor.h \
	triggers/processmonitor.h

SOURCES += bookmarks.cpp commandline.cpp config.cpp infowidget.cpp kshutdown.cpp main.cpp mainwindow.cpp password.cpp preferences.cpp progressbar.cpp udialog.cpp usystemtray.cpp utils.cpp \
	actions/extras.cpp \
	actions/lock.cpp \
	actions/test.cpp \
	triggers/idlemonitor.cpp \
	triggers/processmonitor.cpp

RESOURCES = kshutdown.qrc

unix {
	target.path = /usr/bin

	icon16.path = /usr/share/icons/hicolor/16x16/apps
	icon16.extra = install -m 644 -p images/hi16-app-kshutdown.png /usr/share/icons/hicolor/16x16/apps/kshutdown.png

	icon22.path = /usr/share/icons/hicolor/22x22/apps
	icon22.extra = install -m 644 -p images/hi22-app-kshutdown.png /usr/share/icons/hicolor/22x22/apps/kshutdown.png

	icon32.path = /usr/share/icons/hicolor/32x32/apps
	icon32.extra = install -m 644 -p images/hi32-app-kshutdown.png /usr/share/icons/hicolor/32x32/apps/kshutdown.png

	icon48.path = /usr/share/icons/hicolor/48x48/apps
	icon48.extra = install -m 644 -p images/hi48-app-kshutdown.png /usr/share/icons/hicolor/48x48/apps/kshutdown.png

	icon64.path = /usr/share/icons/hicolor/64x64/apps
	icon64.extra = install -m 644 -p images/hi64-app-kshutdown.png /usr/share/icons/hicolor/64x64/apps/kshutdown.png

	icon128.path = /usr/share/icons/hicolor/128x128/apps
	icon128.extra = install -m 644 -p images/hi128-app-kshutdown.png /usr/share/icons/hicolor/128x128/apps/kshutdown.png

	shortcut.path = /usr/share/applications
	shortcut.files += kshutdown-qt.desktop
	
	INSTALLS += target icon16 icon22 icon32 icon48 icon64 icon128 shortcut
}
