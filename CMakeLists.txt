cmake_minimum_required(VERSION 3.3.0)
project(Log4Qt VERSION 1.0.0)

enable_testing()

option(BUILD_STATIC_LOG4CXX_LIB "Build a static log4cxx library (default: off)" OFF)
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
endif()

# TODO: LOG4QT_VERSION = 0x010000
add_definitions(-DLOG4QT_VERSION_STR="${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8.0)
    message(FATAL_ERROR "gcc 4.8 or higher required!")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.99)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3.0)
    message(FATAL_ERROR "llvm/clang 3.3 or higher required!")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "msvc 2015 or higher required!")
  endif()
  # disable common warnings in Qt/stdlib
  # disable 4127: conditional expression is constant
  #         4512: assignment operator could not be generated
  #         4267: conversion from 'size_t' to 'type', possible loss of data
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4127 -wd4512 -wd4267 -we4265")

  list(REMOVE_ITEM CMAKE_CXX_FLAGS "-W3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")

  add_definitions(-DUNICODE -DNOMINMAX)

  set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_subdirectory(src/log4qt)
add_subdirectory(tests)
add_subdirectory(examples)
