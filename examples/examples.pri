include (../g++.pri)
include (../build.pri)

CONFIG += c++14

msvc {
    QMAKE_CXXFLAGS += -wd4267
}
