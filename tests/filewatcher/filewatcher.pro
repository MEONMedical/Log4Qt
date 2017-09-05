include(../tests.pri)

QT       += testlib
QT       -= gui

TARGET = tst_filewatchertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
        tst_filewatchertest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L../../bin/ \
         -llog4qt

INCLUDEPATH += ../../src
