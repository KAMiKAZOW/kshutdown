TEMPLATE = app
TARGET = kshutdown-qt
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT
exists(portable.pri) {
	include(portable.pri)
	message("Building portable version...")
}

unix {
	CONFIG += qdbus
}

win32 {
	LIBS += -lpowrprof
#	QMAKE_LFLAGS = -static-libgcc
}

# Input

HEADERS += commandline.h config.h infowidget.h kshutdown.h mainwindow.h password.h preferences.h progressbar.h pureqt.h utils.h version.h \
	actions/extras.h \
	actions/test.h \
	triggers/idlemonitor.h \
	triggers/processmonitor.h

SOURCES += commandline.cpp config.cpp infowidget.cpp kshutdown.cpp main.cpp mainwindow.cpp password.cpp preferences.cpp progressbar.cpp utils.cpp \
	actions/extras.cpp \
	actions/lock.cpp \
	actions/test.cpp \
	triggers/idlemonitor.cpp \
	triggers/processmonitor.cpp

RESOURCES = kshutdown.qrc

# TODO: make install
