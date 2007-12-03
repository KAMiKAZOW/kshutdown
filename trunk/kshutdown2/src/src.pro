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
HEADERS += kshutdown.h mainwindow.h pureqt.h theme.h version.h
SOURCES += kshutdown.cpp main.cpp mainwindow.cpp theme.cpp
RESOURCES = kshutdown.qrc

# TODO: make install
