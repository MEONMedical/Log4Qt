cmake_minimum_required(VERSION 3.3.0)
project(Log4Qt VERSION 1.0.0)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
include(MacroEnsureOutOfSourceBuild)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_AUTOMOC TRUE)
set(QT_MIN_VERSION 5.3.0)
# TODO: make Network also optional?
find_package(Qt5 COMPONENTS Core Network Test REQUIRED)
find_package(Qt5 COMPONENTS Sql)
message(STATUS "Qt version is ${Qt5Core_VERSION_STRING}")
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_TO_ASCII)
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050300)
if(NOT MSVC)
    add_definitions(-DQT_STRICT_ITERATORS)
else()
    add_definitions(-DNOMINMAX)
endif()
# TODO: LOG4QT_VERSION = 0x010000
add_definitions(-DLOG4QT_VERSION_STR="${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH}")
# TODO: check compiler versions
# gcc >= 4.8, clang >= 3.3, msvc >= 2015
add_subdirectory(src/log4qt)
add_subdirectory(tests)
add_subdirectory(examples)
