cmake_minimum_required(VERSION 3.28)
project(Log4Qt
    VERSION 2.0.0
    DESCRIPTION "Logging for the Qt cross-platform application framework"
    HOMEPAGE_URL "https://github.com/MEONMedical/Log4Qt"
    LANGUAGES CXX
)

# Modern CMake policies
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()
if(POLICY CMP0087)
    cmake_policy(SET CMP0087 NEW)
endif()

# disable C++20 module scanning for clang
# (replaces the deprecated cmake_policy(SET CMP0155 OLD))
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(LOG4QT_ENABLE_TESTS "${PROJECT_NAME}: Enable tests" ${PROJECT_IS_TOP_LEVEL})
option(LOG4QT_ENABLE_EXAMPLES "${PROJECT_NAME}: Enable examples" ${PROJECT_IS_TOP_LEVEL})

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
option(BUILD_WITH_DB_LOGGING "Build with database logging support, link against Qt sql lib (default: off)" OFF)

# With Telnet logging support or without
option(BUILD_WITH_TELNET_LOGGING "Build with telnet logging support, link against Qt network lib (default: on)" ON)

# With qml logging support or without
option(BUILD_WITH_QML_LOGGING "Build with qml logging support, link against Qt qml lib (default: on)" ON)

# Enable documentation generation with doxygen
OPTION(BUILD_WITH_DOCS "Enable documentation generation (default: off)" OFF)

# Automatically include ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR}
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Set include dir src (used by target_include_directories in src/log4qt)
set(LOG4QT_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# We need C++20 and don't want to export all symbols, only the ones we mark for export
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Min. Qt version is 6.5
set(QT_MIN_VERSION 6.5.0)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core)
# Enables AUTOMOC/AUTOUIC, includes GNUInstallDirs, sets install RPATH
# defaults and opts into the Qt CMake policies up to ${QT_MIN_VERSION}.
# Parent projects can opt out via QT_NO_STANDARD_PROJECT_SETUP.
qt_standard_project_setup(REQUIRES ${QT_MIN_VERSION})
message(STATUS "Qt version is ${Qt6Core_VERSION}")
set(QT_VERSION_MAJOR 6)

find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Concurrent)

if(BUILD_WITH_DB_LOGGING)
    find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Sql)
endif()
if(BUILD_WITH_TELNET_LOGGING)
    find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Network)
endif()
if(BUILD_WITH_QML_LOGGING)
    find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Qml)
endif()

# Qt compile definitions - will be applied to targets via target_compile_definitions
set(LOG4QT_QT_DEFINITIONS
    QT_USE_QSTRINGBUILDER
    QT_NO_CAST_TO_ASCII
    #QT_NO_CAST_FROM_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    UNICODE
    _UNICODE
)
if(NOT MSVC)
    list(APPEND LOG4QT_QT_DEFINITIONS QT_STRICT_ITERATORS)
endif()

# Version definitions - will be applied to targets via target_compile_definitions
set(LOG4QT_VERSION_DEFINITIONS
    LOG4QT_VERSION_STR="${PROJECT_VERSION}"
    LOG4QT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    LOG4QT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    LOG4QT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Compiler-specific compile options - will be applied to targets
set(LOG4QT_COMPILE_OPTIONS "")
set(LOG4QT_COMPILE_DEFINITIONS "")

# We need gcc >= 10 or clang >= 10 or msvc >= 19.29 (VS 2019 16.11) for C++20
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "gcc 10.0 or higher required for C++20!")
    endif()
    list(APPEND LOG4QT_COMPILE_OPTIONS
        -Wall
        -Wextra
    )
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.99)
        list(APPEND LOG4QT_COMPILE_OPTIONS
            -Wsuggest-final-types
            -Wsuggest-final-methods
            -Wsuggest-override
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "llvm/clang 10.0 or higher required for C++20!")
    endif()
    list(APPEND LOG4QT_COMPILE_OPTIONS
        -Wall
        -Wextra
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(MSVC_VERSION LESS 1929)
        message(FATAL_ERROR "msvc 19.29 (Visual Studio 2019 16.11) or higher required for C++20!")
    endif()
    # disable common warnings in Qt/stdlib
    # disable 4127: conditional expression is constant
    #         4512: assignment operator could not be generated
    #         4267: conversion from 'size_t' to 'type', possible loss of data
    list(APPEND LOG4QT_COMPILE_OPTIONS
        /W4
        /wd4127
        /wd4512
        /wd4267
        /wd4913
        /w34265
        /we4265
        /permissive-
    )
    list(APPEND LOG4QT_COMPILE_DEFINITIONS
        UNICODE
        NOMINMAX
    )
    set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_subdirectory(src/log4qt)
if (LOG4QT_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
if (LOG4QT_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# github workflow files
add_custom_target(githubworkflows SOURCES
                  ".github/workflows/cmake-ubuntu.yml"
                  ".github/workflows/cmake-windows.yml"
                  ".github/workflows/cmake-macos.yml")

# project documentation
add_custom_target(documentation SOURCES
                  "Readme.md"
                  "ChangeLog.md"
                  "Configuration.md")

# Create a proper FindLog4Qt - module
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Log4Qt" )
install(EXPORT log4qt_targets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    NAMESPACE Log4Qt::
    FILE Log4QtTargets.cmake
    COMPONENT Devel
)

# Include CMakePackageConfigHelpers before using its commands
include(CMakePackageConfigHelpers)

# Qt components consumers of the installed package must be able to resolve:
# all PUBLIC link dependencies, plus the PRIVATE ones for static builds
# (they appear in the export as $<LINK_ONLY:...> references).
set(LOG4QT_CONFIG_QT_COMPONENTS Core)
if(NOT BUILD_SHARED_LIBS)
    list(APPEND LOG4QT_CONFIG_QT_COMPONENTS Concurrent)
endif()
if(BUILD_WITH_DB_LOGGING)
    list(APPEND LOG4QT_CONFIG_QT_COMPONENTS Sql)
endif()
if(BUILD_WITH_TELNET_LOGGING)
    list(APPEND LOG4QT_CONFIG_QT_COMPONENTS Network)
endif()
if(BUILD_WITH_QML_LOGGING)
    list(APPEND LOG4QT_CONFIG_QT_COMPONENTS Qml)
endif()
list(JOIN LOG4QT_CONFIG_QT_COMPONENTS " " LOG4QT_CONFIG_QT_COMPONENTS)

configure_package_config_file(
    "${Log4Qt_MODULE_DIR}/Log4QtConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/Log4QtConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/Log4QtConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES
    "${PROJECT_BINARY_DIR}/Log4QtConfig.cmake"
    "${PROJECT_BINARY_DIR}/Log4QtConfigVersion.cmake"
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


# Documentation generation
if(BUILD_WITH_DOCS)
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
    else()
        message(FATAL_ERROR "Doxygen needs to be installed to generate the doxygen documentation")
    endif()
endif()

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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    if(NOT CPACK_PACKAGE_ARCHITECTURE)
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

    if(CPACK_PACKAGE_ARCHITECTURE STREQUAL "x86_64")
        set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64" )  # DEB doesn't always use the kernel's arch name
    else()
        set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
    endif()

    set( CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT" ) # Use default naming scheme

    set( CPACK_DEBIAN_LIBRARY_PACKAGE_NAME ${CPACK_PACKAGE_NAME} )
    set( CPACK_DEBIAN_LIBRARY_PACKAGE_SHLIBDEPS 1 )

    set( CPACK_DEBIAN_DEVEL_PACKAGE_DEPENDS "cmake (>= ${CMAKE_MINIMUM_REQUIRED_VERSION}), log4qt (>= ${CPACK_PACKAGE_VERSION})" )
    set( CPACK_DEBIAN_DEVEL_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-dev" )

elseif(CMAKE_HOST_WIN32)
    set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
    set( CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME} )
    set( CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME} )
    set( CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_HOMEPAGE_URL} )
endif()

# This must always be last!
include( CPack )
