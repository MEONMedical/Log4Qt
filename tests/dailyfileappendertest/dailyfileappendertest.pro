include(../tests.pri)

QT += core testlib
QT -= gui

TARGET = dailyfileappendertest
TEMPLATE = app

SOURCES += dailyfileappendertest.cpp

DESTDIR=../../bin

LIBS += -L../../bin/ \
         -llog4qt

INCLUDEPATH += ../../src
