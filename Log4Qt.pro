QT += core xml network
include(src/Log4Qt.pri)
include(g++.pri)

CONFIG += c++11 \
          hide_symbols

TEMPLATE = lib
TARGET = log4qt

LOG4QT_VERSION_MAJOR = 1
LOG4QT_VERSION_MINOR = 1
LOG4QT_VERSION_RELEASE = 0

LOG4QT_VERSION = '\\"$${LOG4QT_VERSION_MAJOR}.$${LOG4QT_VERSION_MINOR}.$${LOG4QT_VERSION_RELEASE}\\"'
DEFINES += LOG4QT_VERSION_STR=\"$${LOG4QT_VERSION}\"
DEFINES += LOG4QT_VERSION=$${LOG4QT_VERSION}

#VERSION = 1.1.0

DEPENDPATH += src src/helpers src/spi src/varia
INCLUDEPATH += src src/helpers src/spi src/varia

DESTDIR = ../../bin

DEFINES += NOMINMAX
DEFINES += LOG4QT_LIBRARY
