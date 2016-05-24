include(../examples.pri)

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

DEPENDPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia
INCLUDEPATH += ../../src ../../src/helpers ../../src/spi ../../src/varia

DISTFILES += \
    propertyconfigurator.exe.log4qt.properties
