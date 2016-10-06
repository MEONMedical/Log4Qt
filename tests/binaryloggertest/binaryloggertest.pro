include(../tests.pri)
include(../../build.pri)

QT += core testlib network
QT -= gui

TARGET = binaryloggertest
TEMPLATE = app

HEADERS += \
    testappender.h \
    elementsinarray.h \
    atscopeexit.h \

SOURCES += binaryloggertest.cpp \
    testappender.cpp

DESTDIR=../../bin
DEPENDPATH+=../../bin

LIBS += -L../../bin/ \
         -llog4qt

DEPENDPATH += ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia
INCLUDEPATH += ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia
