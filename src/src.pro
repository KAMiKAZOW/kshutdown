TEMPLATE = app
TARGET = kshutdown
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += KS_PURE_QT
QT += xml

unix {
	CONFIG += qdbus
}

win32 {
	LIBS += -lpowrprof
}

# Input
HEADERS += config.h kshutdown.h mainwindow.h preferences.h pureqt.h theme.h version.h actions/extras.h
SOURCES += config.cpp kshutdown.cpp main.cpp mainwindow.cpp preferences.cpp theme.cpp actions/extras.cpp
RESOURCES = kshutdown.qrc

# TODO: make install
