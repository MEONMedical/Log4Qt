QT += core xml network concurrent
include(../../build.pri)
include(../../g++.pri)
include(log4qt.pri)

CONFIG += c++17 \
          hide_symbols

contains(DEFINES, LOG4QT_STATIC) {
    message(Building static log4qt...)
    CONFIG += staticlib
}

TEMPLATE = lib
TARGET = log4qt
QT -= gui

defineReplace(qextGetFirstPath) {
   win32:sep=;
   else:sep=:
   p = $$split(1, $$sep)
   return ($$member(p))
}

# .. is needed for msvc since it is treating '.' as the directory of the current file
# and not the directory where the compiled source is found
INCLUDEPATH += .. .

DESTDIR = ../../bin
DEFINES += NOMINMAX QT_DEPRECATED_WARNINGS QT_NO_CAST_FROM_BYTEARRAY QT_USE_QSTRINGBUILDER
DEFINES += LOG4QT_LIBRARY

# generate feature file by qmake based on this *.in file.
QMAKE_SUBSTITUTES += log4qt.prf.in
OTHER_FILES += log4qt.prf.in

target.path = $$[QT_INSTALL_LIBS]
INSTALLS = target

header_base.files = $$HEADERS_BASE
header_base.path = $$[QT_INSTALL_HEADERS]/log4qt
INSTALLS += header_base
header_helpers.files = $$HEADERS_HELPERS
header_helpers.path = $$[QT_INSTALL_HEADERS]/log4qt/helpers
INSTALLS += header_helpers
header_spi.files = $$HEADERS_SPI
header_spi.path = $$[QT_INSTALL_HEADERS]/log4qt/spi
INSTALLS += header_spi
header_varia.files = $$HEADERS_VARIA
header_varia.path = $$[QT_INSTALL_HEADERS]/log4qt/varia
INSTALLS += header_varia
features.files = log4qt.prf
features.path = $$qextGetFirstPath($$[QMAKE_MKSPECS])/features
INSTALLS += features
