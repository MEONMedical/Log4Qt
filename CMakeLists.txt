cmake_minimum_required(VERSION 3.3.0)
project(Log4Qt VERSION 1.5.1)

#
# in-source builds should be avoided
#
set(Log4Qt_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${Log4Qt_MODULE_DIR}")
include(MacroEnsureOutOfSourceBuild)

#
# support for 'make test'
#
enable_testing()

#
# Shared or static build of log4qt library
#
option(BUILD_STATIC_LOG4CXX_LIB "Build a static log4cxx library (default: off)" OFF)

#
# With Database logging support or without
#
option(BUILD_WITH_DB_LOGGING "Build with database logging support, link against Qt sql lib (default: on)" OFF)

#
# With Database logging support or without
#
option(BUILD_WITH_TELNET_LOGGING "Build with telnet logging support, link against Qt network lib (default: on)" OFF)

#
# Helper to be able to properly install into the correct directories (esp. lib/lib64)
#
include(GNUInstallDirs)

#
# Automatically include ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR}
#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

#
# Set include dir src
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LOG4QT_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#
# We need C++11 and don't want to export all symbols, only the ones we mark for export
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

#
# Min. Qt version is 5.6, we want to use cmake automoc feature
# and settings some defines for Qt
#
set(CMAKE_AUTOMOC TRUE)
set(QT_MIN_VERSION 5.7.0)
# TODO: make Network also optional?
find_package(Qt5 COMPONENTS Core Network Concurrent Test REQUIRED)
if(BUILD_WITH_DB_LOGGING)
  find_package(Qt5 COMPONENTS Sql REQUIRED)
endif()
if(BUILD_WITH_TELNET_LOGGING)
  find_package(Qt5 COMPONENTS Network REQUIRED)
endif()
message(STATUS "Qt version is ${Qt5Core_VERSION_STRING}")
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_TO_ASCII)
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050700)
if(NOT MSVC)
    add_definitions(-DQT_STRICT_ITERATORS)
endif()
add_definitions(-DUNICODE -D_UNICODE)

#
# Pass version as a string to the compiler (for LogManager::version())
#
add_definitions(-DLOG4QT_VERSION_STR="${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH}"
                -DLOG4QT_VERSION_MAJOR=${Log4Qt_VERSION_MAJOR}
                -DLOG4QT_VERSION_MINOR=${Log4Qt_VERSION_MINOR}
                -DLOG4QT_VERSION_PATCH=${Log4Qt_VERSION_PATCH}
                )

#
# We need gcc >= 4.8 or clang >= 3.3 or msvc >= 2015
#
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

#
# Create a proper FindLog4Qt - module
#
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Log4Qt" )
install(EXPORT log4qt_targets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    NAMESPACE Log4Qt::
    FILE Log4QtTargets.cmake
    COMPONENT Common
)
configure_file(
    "${Log4Qt_MODULE_DIR}/Log4QtConfig.cmake.in"
    "${Log4Qt_BINARY_DIR}/Log4QtConfig.cmake"
    @ONLY
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${Log4Qt_BINARY_DIR}/Log4QtConfigVersion.cmake"
    VERSION ${Log4Qt_VERSION_STRING}
    COMPATIBILITY AnyNewerVersion
)
install(FILES
    "${Log4Qt_BINARY_DIR}/Log4QtConfig.cmake"
    "${Log4Qt_BINARY_DIR}/Log4QtConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Common
)

#
# Add support for an uninstall target
#
configure_file(
  "${Log4Qt_MODULE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

