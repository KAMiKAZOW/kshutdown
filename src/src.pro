TEMPLATE = app
TARGET = kshutdown
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT
include(portable.pri)
#QT += xml

unix {
	CONFIG += qdbus
}

win32 {
	LIBS += -lpowrprof
}

# Input

HEADERS += commandline.h config.h infowidget.h kshutdown.h mainwindow.h preferences.h progressbar.h pureqt.h utils.h version.h \
	actions/extras.h \
	triggers/idlemonitor.h \
	triggers/processmonitor.h

SOURCES += commandline.cpp config.cpp infowidget.cpp kshutdown.cpp main.cpp mainwindow.cpp preferences.cpp progressbar.cpp utils.cpp \
	actions/extras.cpp actions/lock.cpp \
	triggers/idlemonitor.cpp \
	triggers/processmonitor.cpp

RESOURCES = kshutdown.qrc

# TODO: make install
