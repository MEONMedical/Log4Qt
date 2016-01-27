TEMPLATE = app
CONFIG += testlib \
          c++11

QT += core testlib
QT -= gui

include(tests.pri)

DEPENDPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia
INCLUDEPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia

HEADERS += log4qttest.h
SOURCES += log4qttest.cpp

LIBS += -L../../bin \
        -llog4qt

DESTDIR = ../../bin
