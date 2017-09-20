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

CONFIG( debug, debug|release ) {
    mac: LIBS += -L../../bin \
            -llog4qt_debug
    win32: LIBS += -L../../bin \
            -llog4qtd
} else {
    LIBS += -L../../bin \
            -llog4qt
}

INCLUDEPATH += ../../src
