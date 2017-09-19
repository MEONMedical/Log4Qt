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

CONFIG( debug, debug|release ) {
    LIBS += -L../../bin \
            -llog4qtd
} else {
    LIBS += -L../../bin \
            -llog4qt
}

INCLUDEPATH += ../../src
