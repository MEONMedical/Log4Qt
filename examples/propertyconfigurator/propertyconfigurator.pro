include(../examples.pri)
include(../../build.pri)

QT       += core network
QT -= gui

TEMPLATE = app
DESTDIR = ../../bin

LIBS += -L../../bin \
        -llog4qt

SOURCES += \
    main.cpp \
    loggerobject.cpp \
    loggerobjectprio.cpp \
    loggerstatic.cpp

HEADERS  += \
    loggerobject.h \
    loggerobjectprio.h \
    loggerstatic.h

INCLUDEPATH += ../../src

DISTFILES += \
    propertyconfigurator.exe.log4qt.properties


