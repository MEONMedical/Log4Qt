include(../tests.pri)

QT += core testlib network
QT -= gui

TARGET = tst_filewatchertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR=../../bin

SOURCES += \
        tst_filewatchertest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L../../bin/ \
         -llog4qt

INCLUDEPATH += ../../src
