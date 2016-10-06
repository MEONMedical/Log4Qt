include (../g++.pri)
include (../build.pri)

CONFIG += c++11 \
          testcase

mac {
    CONFIG -= app_bundle
}

win32 {
    CONFIG   += console
}

DEFINES += QT_DEPRECATED_WARNINGS

msvc {
    QMAKE_CXXFLAGS += -wd4267
}
