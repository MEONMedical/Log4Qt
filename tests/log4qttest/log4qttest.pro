include(../tests.pri)

TEMPLATE = app
CONFIG += testlib \
          c++14

QT += core testlib network
QT -= gui

INCLUDEPATH += ../../src

HEADERS += log4qttest.h
SOURCES += log4qttest.cpp

LIBS += -L../../bin \
        -llog4qt

DESTDIR = ../../bin
