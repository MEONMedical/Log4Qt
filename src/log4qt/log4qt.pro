QT += core xml network
include(../../build.pri)
include(../../g++.pri)
include(log4qt.pri)

CONFIG += c++11 \
          hide_symbols

contains(DEFINES, LOG4QT_STATIC) {
    message(Building static log4qt...)
    CONFIG += staticlib
}

TEMPLATE = lib
TARGET = log4qt
QT -= gui

LOG4QT_VERSION_MAJOR = 1
LOG4QT_VERSION_MINOR = 5
LOG4QT_VERSION_PATCH = 0

DEFINES += LOG4QT_VERSION_MAJOR=$${LOG4QT_VERSION_MAJOR}
DEFINES += LOG4QT_VERSION_MINOR=$${LOG4QT_VERSION_MINOR}
DEFINES += LOG4QT_VERSION_PATCH=$${LOG4QT_VERSION_PATCH}
DEFINES += LOG4QT_VERSION_STR='\\"$${LOG4QT_VERSION_MAJOR}.$${LOG4QT_VERSION_MINOR}.$${LOG4QT_VERSION_PATCH}\\"'

# .. is needed for msvc since it is treating '.' as the directory of the current file
# and not the directory where the compiled source is found
INCLUDEPATH += .. .

DESTDIR = ../../bin
DEFINES += NOMINMAX QT_DEPRECATED_WARNINGS QT_NO_CAST_FROM_BYTEARRAY QT_USE_QSTRINGBUILDER
DEFINES += LOG4QT_LIBRARY

target.path = $$INSTALL_PREFIX/lib$$LIB_SUFFIX
INSTALLS = target

header_base.files = $$HEADERS_BASE
header_base.path = $$INSTALL_PREFIX/include/log4qt
INSTALLS += header_base
header_helpers.files = $$HEADERS_HELPERS
header_helpers.path = $$INSTALL_PREFIX/include/log4qt/helpers
INSTALLS += header_helpers
header_spi.files = $$HEADERS_SPI
header_spi.path = $$INSTALL_PREFIX/include/log4qt/spi
INSTALLS += header_spi
header_varia.files = $$HEADERS_VARIA
header_varia.path = $$INSTALL_PREFIX/include/log4qt/varia
INSTALLS += header_varia

