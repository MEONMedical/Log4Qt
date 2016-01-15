QT += core xml network
include(src/Log4Qt.pri)
include(g++.pri)

CONFIG += c++11 \
          hide_symbols

TEMPLATE = lib
TARGET = log4qt

DESTDIR     = $$PWD/build/lib
MOC_DIR     = $$PWD/build/moc
OBJECTS_DIR = $$PWD/build/obj

CONFIG += link_pkgconfig create_prl no_install_prl create_pc

# installation settings
isEmpty(PREFIX) {
    PREFIX=/usr/local/
}

target.path             = $$PREFIX/bin
includes.path           = $$PREFIX/include/log4qt
includes.files         += $$PWD/src/*.h
includes.files         += $$PWD/deploy/include/*.h

includesHelpers.path    = $$PREFIX/include/log4qt/helpers
includesHelpers.files  += $$PWD/src/helpers/*.h

includesSpi.path        = $$PREFIX/include/log4qt/spi
includesSpi.files      += $$PWD/src/spi/*.h

includesVaria.path      = $$PREFIX/include/log4qt/varia
includesVaria.files    += $$PWD/src/varia/*.h

INSTALLS += target includes includesHelpers  includesSpi includesVaria

LOG4QT_VERSION_MAJOR = 1
LOG4QT_VERSION_MINOR = 1
LOG4QT_VERSION_RELEASE = 0

LOG4QT_VERSION = '\\"$${LOG4QT_VERSION_MAJOR}.$${LOG4QT_VERSION_MINOR}.$${LOG4QT_VERSION_RELEASE}\\"'
DEFINES += LOG4QT_VERSION_STR=\"$${LOG4QT_VERSION}\"
DEFINES += LOG4QT_VERSION=$${LOG4QT_VERSION}

# pc-file params

QMAKE_PKGCONFIG_NAME          = Log4Qt
QMAKE_PKGCONFIG_FILE          = Log4Qt
QMAKE_PKGCONFIG_DESCRIPTION   = Log4Qt is a C++ port of the Apache Software Foundation Log4j package using \
                                the Trolltech Qt Framework. It is intended to be used by open source \
                                and commercial Qt projects.
QMAKE_PKGCONFIG_LIBDIR        = $$target.path
QMAKE_PKGCONFIG_INCDIR        = $$PREFIX/include
QMAKE_PKGCONFIG_DESTDIR       = ../share/pkgconfig

#VERSION = 1.1.0

DEPENDPATH += src src/helpers src/spi src/varia
INCLUDEPATH += src src/helpers src/spi src/varia

#DESTDIR = ../../bin

DEFINES += NOMINMAX
DEFINES += LOG4QT_LIBRARY
