TEMPLATE = app
CONFIG += testlib \
          c++11

QT += core testlib
QT -= gui

include(../tests.pri)

DEPENDPATH += ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia
INCLUDEPATH += ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia

HEADERS += log4qttest.h
SOURCES += log4qttest.cpp

LIBS += -L../../bin \
        -llog4qt

DESTDIR = ../../bin
