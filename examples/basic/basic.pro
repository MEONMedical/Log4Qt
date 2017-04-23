include(../examples.pri)
include(../../build.pri)

QT       += core network

TEMPLATE = app
DESTDIR = ../../bin

LIBS += -L../../bin \
        -llog4qt

SOURCES += \
    main.cpp \
    loggerobject.cpp \
    loggerobjectprio.cpp \
    loggerstatic.cpp

HEADERS  += \
    loggerobject.h \
    loggerobjectprio.h \
    loggerstatic.h

DEPENDPATH += ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia
INCLUDEPATH += ../../src/ ../../src/log4qt ../../src/log4qt/helpers ../../src/log4qt/spi ../../src/log4qt/varia

