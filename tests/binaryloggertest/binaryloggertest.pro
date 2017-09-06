include(../tests.pri)

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

LIBS += -L../../bin/ \
         -llog4qt

INCLUDEPATH += ../../src
