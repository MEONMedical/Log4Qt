cmake_minimum_required(VERSION 3.3.0)
project(Log4Qt VERSION 1.6.0)


set(LOG4QT_STANDALONE_BUILD OFF)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(LOG4QT_STANDALONE_BUILD ON)
endif()

option(LOG4QT_ENABLE_TESTS "${PROJECT_NAME}: Enable tests" ${LOG4QT_STANDALONE_BUILD})
option(LOG4QT_ENABLE_EXAMPLES "${PROJECT_NAME}: Enable examples" ${LOG4QT_STANDALONE_BUILD})

# in-source builds should be avoided
set(Log4Qt_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${Log4Qt_MODULE_DIR}")
include(MacroEnsureOutOfSourceBuild)

# support for 'make test'
enable_testing()

# Shared or static build of log4qt library
option(BUILD_SHARED_LIBS "Build a dynamic log4qt library (default: on)" ON)
if(BUILD_STATIC_LOG4CXX_LIB)
    set(BUILD_SHARED_LIBS OFF)
    message(WARNING "BUILD_STATIC_LOG4CXX_LIB is deprecated. Use BUILD_SHARED_LIBS=OFF instead.")
endif()

# With Database logging support or without
option(BUILD_WITH_DB_LOGGING "Build with database logging support, link against Qt sql lib (default: on)" OFF)

# With Telnet logging support or without
option(BUILD_WITH_TELNET_LOGGING "Build with telnet logging support, link against Qt network lib (default: on)" OFF)

# Enable documentation generation with doxygen
OPTION(BUILD_WITH_DOCS "Enable documentation generation (default: off)" OFF)

# Helper to be able to properly install into the correct directories (esp. lib/lib64)
include(GNUInstallDirs)

# Automatically include ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR}
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Set include dir src
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LOG4QT_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# We need C++11 and don't want to export all symbols, only the ones we mark for export
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Min. Qt version is 5.12, we want to use cmake automoc feature
# and settings some defines for Qt
set(CMAKE_AUTOMOC TRUE)

if (NOT (Qt5_FOUND OR Qt6_FOUND))
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
    if (Qt6_FOUND)
        message("Qt6 found.")
        message(STATUS "Qt version is ${Qt6Core_VERSION}")
        set(QT_MIN_VERSION 6.0.0)
    elseif(Qt5_FOUND)
        message("Qt5 found.")
        message(STATUS "Qt version is ${Qt5Core_VERSION}")
        set(QT_MIN_VERSION 5.12.0)
    else()
        message(FATAL_ERROR "No Qt found!")
    endif()
endif()

if (Qt${QT_VERSION_MAJOR}Core_VERSION VERSION_LESS ${QT_MIN_VERSION})
    message(FATAL_ERROR "Minimum supported Qt version is ${QT_MIN_VERSION}")
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Concurrent REQUIRED)

if(BUILD_WITH_DB_LOGGING)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Sql REQUIRED)
endif()
if(BUILD_WITH_TELNET_LOGGING)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_TO_ASCII)
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00)
if(NOT MSVC)
    add_definitions(-DQT_STRICT_ITERATORS)
endif()
add_definitions(-DUNICODE -D_UNICODE)

# Pass version as a string to the compiler (for LogManager::version())
add_definitions(-DLOG4QT_VERSION_STR="${Log4Qt_VERSION_MAJOR}.${Log4Qt_VERSION_MINOR}.${Log4Qt_VERSION_PATCH}"
                -DLOG4QT_VERSION_MAJOR=${Log4Qt_VERSION_MAJOR}
                -DLOG4QT_VERSION_MINOR=${Log4Qt_VERSION_MINOR}
                -DLOG4QT_VERSION_PATCH=${Log4Qt_VERSION_PATCH}
                )

# We need gcc >= 4.8 or clang >= 3.3 or msvc >= 2015
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4127 -wd4512 -wd4267 -wd4913 -we4265")

  string(REGEX REPLACE "/W[0-3]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  add_definitions(-DUNICODE -DNOMINMAX)

  set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_subdirectory(src/log4qt)
if (LOG4QT_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
if (LOG4QT_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Create a proper FindLog4Qt - module
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Log4Qt" )
install(EXPORT log4qt_targets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    NAMESPACE Log4Qt::
    FILE Log4QtTargets.cmake
    COMPONENT Devel
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
    COMPONENT Devel
)

# Add support for an uninstall target
configure_file(
  "${Log4Qt_MODULE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# Documentation generations
IF (BUILD_WITH_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
      message(FATAL "Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
ENDIF()

# Packaging
# https://cmake.org/cmake/help/latest/module/CPack.html
set( CPACK_PACKAGE_NAME ${PROJECT_NAME} )
set( CPACK_PACKAGE_VENDOR "MEON Medical" ) # Github project owner
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Logging for the Qt cross-platform application framework" )
set( CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/MEONMedical/Log4Qt" )
set( CPACK_PACKAGE_CONTACT      "https://github.com/MEONMedical/Log4Qt" )
set( CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH} )
set( CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME} )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" )
set( CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md" )

set( CPACK_COMPONENT_Library_DISPLAY_NAME "Log4Qt Library" )
set( CPACK_COMPONENT_Library_DESCRIPTION "The Log4Qt binary library." )
set( CPACK_COMPONENT_Library_REQUIRED 1 )
set( CPACK_COMPONENT_Devel_DISPLAY_NAME "Log4Qt Development Files" )
set( CPACK_COMPONENT_Devel_DESCRIPTION "Development files for compiling against Log4Qt." )
set( CPACK_COMPONENT_Devel_REQUIRED 0 )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )

    if ( "${CPACK_PACKAGE_ARCHITECTURE}" STREQUAL "" )
        # Note: the architecture should default to the local architecture, but it
        # in fact comes up empty.  We call `uname -m` to ask the kernel instead.
        EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE )
    endif()

    set( CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1 )
    set( CPACK_PACKAGE_RELEASE 1 )


    # RPM - https://cmake.org/cmake/help/latest/cpack_gen/rpm.html
    set( CPACK_RPM_PACKAGE_RELEASE ${CPACK_PACKAGE_RELEASE} )
    set( CPACK_RPM_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
    set( CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY} )
    set( CPACK_RPM_PACKAGE_URL ${CPACK_PACKAGE_HOMEPAGE_URL} )
    set( CPACK_RPM_PACKAGE_LICENSE "APACHE-2" )
    set( CPACK_RPM_COMPONENT_INSTALL 1 )
    set( CPACK_RPM_MAIN_COMPONENT "Library" )
    set( CPACK_RPM_COMPRESSION_TYPE "xz" )
    set( CPACK_RPM_PACKAGE_AUTOPROV 1 )

    set( CPACK_RPM_Library_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
    set( CPACK_RPM_Library_PACKAGE_NAME ${CPACK_PACKAGE_NAME} )
    set( CPACK_RPM_Library_FILE_NAME
         "${CPACK_RPM_Library_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CPACK_RPM_Library_PACKAGE_ARCHITECTURE}.rpm" )
    set( CPACK_RPM_Library_PACKAGE_SUMMARY ${CPACK_COMPONENT_Library_DESCRIPTION} )

    set( CPACK_RPM_Devel_PACKAGE_REQUIRES "cmake >= ${CMAKE_MINIMUM_REQUIRED_VERSION},log4qt >= ${CPACK_PACKAGE_VERSION}" )
    set( CPACK_RPM_Devel_PACKAGE_SUMMARY ${CPACK_COMPONENT_Devel_DESCRIPTION} )
    set( CPACK_RPM_Devel_PACKAGE_ARCHITECTURE "noarch" )  # only contains headers and cmake files
    set( CPACK_RPM_Devel_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-devel" )
    set( CPACK_RPM_Devel_FILE_NAME
         "${CPACK_RPM_Devel_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CPACK_RPM_Devel_PACKAGE_ARCHITECTURE}.rpm" )


    # DEB - https://cmake.org/cmake/help/latest/cpack_gen/deb.html
    set( CPACK_DEBIAN_PACKAGE_RELEASE ${CPACK_PACKAGE_RELEASE} )
    set( CPACK_DEBIAN_PACKAGE_HOMEPAGE ${CPACK_PACKAGE_HOMEPAGE_URL} )
    set( CPACK_DEB_COMPONENT_INSTALL 1 )
    set( CPACK_DEBIAN_COMPRESSION_TYPE "xz")

    if ( ${CPACK_PACKAGE_ARCHITECTURE} STREQUAL "x86_64" )
        set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64" )  # DEB doesn't always use the kernel's arch name
    else()
        set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
    endif()

    set( CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT" ) # Use default naming scheme

    set( CPACK_DEBIAN_LIBRARY_PACKAGE_NAME ${CPACK_PACKAGE_NAME} )
    set( CPACK_DEBIAN_LIBRARY_PACKAGE_SHLIBDEPS 1 )

    set( CPACK_DEBIAN_DEVEL_PACKAGE_DEPENDS "cmake (>= ${CMAKE_MINIMUM_REQUIRED_VERSION}), log4qt (>= ${CPACK_PACKAGE_VERSION})" )
    set( CPACK_DEBIAN_DEVEL_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-dev" )

elseif( ${CMAKE_HOST_WIN32} )
    set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
    set( CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME} )
    set( CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME} )
    set( CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_HOMEPAGE_URL} )
endif()


# This must always be last!
include( CPack )
