TEMPLATE = app
TARGET = kshutdown
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT
include(portable.pri)
QT += xml

unix {
	CONFIG += qdbus
}

win32 {
	LIBS += -lpowrprof
}

# Input
HEADERS += config.h kshutdown.h mainwindow.h preferences.h progressbar.h pureqt.h theme.h utils.h version.h actions/extras.h triggers/processmonitor.h
SOURCES += config.cpp kshutdown.cpp main.cpp mainwindow.cpp preferences.cpp progressbar.cpp theme.cpp utils.cpp actions/extras.cpp triggers/processmonitor.cpp
RESOURCES = kshutdown.qrc

# TODO: make install
