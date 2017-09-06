include (../g++.pri)
include (../build.pri)

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS

msvc {
    QMAKE_CXXFLAGS += -wd4267
}
