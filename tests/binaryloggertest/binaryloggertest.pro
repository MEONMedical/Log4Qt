include(../tests.pri)

QT += core testlib
QT -= gui

TARGET = binaryloggertest
TEMPLATE = app

HEADERS += \
    testappender.h \
    elementsinarray.h \
    logging.h \
    atscopeexit.h \

SOURCES += binaryloggertest.cpp \
    testappender.cpp \
    logging.cpp




DESTDIR=../../bin
DEPENDPATH+=../../bin

LIBS += -L../../bin/ \
         -llog4qt

DEPENDPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia
INCLUDEPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia
