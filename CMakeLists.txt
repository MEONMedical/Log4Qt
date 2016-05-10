cmake_minimum_required(VERSION 2.6)

# cmake options
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
# postfix for library
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
# variable for project name, used in many places
set (PROJECT Log4Qt)

project(${PROJECT})

string(TOUPPER ${PROJECT_NAME} PROJECT_TO_UPPER)
string(TOLOWER ${PROJECT_NAME} PROJECT_TO_LOWER)

find_package(Qt4 REQUIRED)
# COMPILER customization
  # compilers flags
  if (MSVC)
    add_definitions (/W3 -Zc:wchar_t-)
  elseif (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-Wall)
  else ()
    message ("Unknown compiler")
  endif ()
  #compiler options
  set(OPTIMIZATION_FLAGS "-O2" )
message("Building project ${PROJECT_NAME}")

message("
Configuration
-------------
CMAKE_BUILD_TYPE        = ${CMAKE_BUILD_TYPE}
CXXFLAGS                = $ENV{CXXFLAGS}
CMAKE_CXX_FLAGS         = ${CMAKE_CXX_FLAGS}
LDFLAGS                 = $ENV{LDFLAGS}
CMAKE_EXE_LINKER_FLAGS  = ${CMAKE_EXE_LINKER_FLAGS}
CMAKE_INSTALL_PREFIX    = ${CMAKE_INSTALL_PREFIX}
")

# remove when release
if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	set(CMAKE_VERBOSE_MAKEFILE TRUE)
endif (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")

# SOURCE FILES AND DIRECTORIES
set(LOG4QT_SOURCES)
# Q_SOURCES, var contains files, for which need call qt4_automoc
set(LOG4QT_Q_SOURCES)
set(LOG4QT_HEADERS)
# Q_HEADERS, var contains files, for which qt4_wrap_cpp called
set(LOG4QT_Q_HEADERS)

set(LOG4QT_HEADERS
    src/basicconfigurator.h
    src/helpers/classlogger.h
    src/helpers/datetime.h
    src/helpers/factory.h
    src/hierarchy.h
    src/helpers/initialisationhelper.h
    src/level.h
    src/log4qt.h
    src/log4qtshared.h
    src/loggerrepository.h
    src/loggingevent.h
    src/logmanager.h
    src/helpers/logobjectptr.h
    src/mdc.h
    src/ndc.h
    src/helpers/optionconverter.h
    src/helpers/patternformatter.h
    src/helpers/properties.h
    src/propertyconfigurator.h
    src/helpers/logerror.h
    src/helpers/appenderattachable.h
    src/logstream.h
)

set(LOG4QT_Q_HEADERS src/appender.h
    src/appenderskeleton.h
    src/asyncappender.h
    src/colorconsoleappender.h
    src/consoleappender.h
    src/dailyrollingfileappender.h
    src/fileappender.h
    src/mainthreadappender.h
    src/layout.h
    src/logger.h
    src/patternlayout.h
    src/rollingfileappender.h
    src/signalappender.h
    src/simplelayout.h
    src/simpletimelayout.h
    src/ttcclayout.h
    src/writerappender.h
    src/systemlogappender.h
    src/helpers/configuratorhelper.h
    src/helpers/logobject.h
    src/helpers/dispatcher.h
    src/spi/filter.h
    src/varia/debugappender.h
    src/varia/denyallfilter.h
    src/varia/levelmatchfilter.h
    src/varia/levelrangefilter.h
    src/varia/listappender.h
    src/varia/nullappender.h
    src/varia/stringmatchfilter.h)
    
set(LOG4QT_SOURCES
    src/helpers/classlogger.cpp
    src/helpers/configuratorhelper.cpp
    src/helpers/datetime.cpp
    src/helpers/factory.cpp
    src/helpers/initialisationhelper.cpp
    src/helpers/logerror.cpp
    src/helpers/logobject.cpp
    src/helpers/dispatcher.cpp
    src/helpers/logobjectptr.cpp
    src/helpers/optionconverter.cpp
    src/helpers/patternformatter.cpp
    src/helpers/properties.cpp
    src/helpers/appenderattachable.cpp
    src/spi/filter.cpp
    src/varia/debugappender.cpp
    src/varia/denyallfilter.cpp
    src/varia/levelmatchfilter.cpp
    src/varia/levelrangefilter.cpp
    src/varia/listappender.cpp
    src/varia/nullappender.cpp
    src/varia/stringmatchfilter.cpp
    src/systemlogappender.cpp
    src/appenderskeleton.cpp
    src/basicconfigurator.cpp
    src/colorconsoleappender.cpp
    src/consoleappender.cpp
    src/asyncappender.cpp
    src/dailyrollingfileappender.cpp
    src/fileappender.cpp
    src/mainthreadappender.cpp
    src/hierarchy.cpp
    src/layout.cpp
    src/level.cpp
    src/log4qt.cpp
    src/logger.cpp
    src/loggerrepository.cpp
    src/loggingevent.cpp
    src/logmanager.cpp
    src/mdc.cpp
    src/ndc.cpp
    src/patternlayout.cpp
    src/propertyconfigurator.cpp
    src/rollingfileappender.cpp
    src/signalappender.cpp
    src/simplelayout.cpp
    src/simpletimelayout.cpp
    src/ttcclayout.cpp
    src/writerappender.cpp
    src/logstream.cpp
)

# project version
set(LOG4QT_VERSION_MAJOR 1)
set(LOG4QT_VERSION_MINOR 0)
set(LOG4QT_VERSION_RELEASE 0)
set(LOG4QT_VERSION "${LOG4QT_VERSION_MAJOR}.${LOG4QT_VERSION_MINOR}.${LOG4QT_VERSION_RELEASE}")
set(VERSION ${LOG4QT_VERSION})
set(SOVERSION ${LOG4QT_VERSION})
add_definitions(-DLOG4QT_VERSION=${LOG4QT_VERSION_MAJOR}*65536+${LOG4QT_VERSION_MINOR}*256+${LOG4QT_VERSION_RELEASE})
add_definitions(-DLOG4QT_VERSION_STR="${LOG4QT_VERSION}")

# Make sure LOG4QT_BUILD_STATIC is defined since the variable value is used when configuring
# the project-config.cmake file.
if (NOT DEFINED LOG4QT_BUILD_STATIC)
	set (LOG4QT_BUILD_STATIC FALSE)
endif (NOT DEFINED LOG4QT_BUILD_STATIC)

# QT
set(QT_DONT_USE_QTGUI TRUE)
include(${QT_USE_FILE})

if (QT_USE_QTSQL)
	message("With QT_USE_QTSQL DatabaseAppender, DatabaseLayout compiled")
	list(APPEND LOG4QT_Q_HEADERS src/databaseappender.h src/databaselayout.h)
	list(APPEND LOG4QT_SOURCES src/databaseappender.cpp src/databaselayout.cpp)
endif (QT_USE_QTSQL)

if (QT_USE_QTNETWORK)
    message("With QT_USE_QTNETWORK TelnetAppender compiled")
    list(APPEND LOG4QT_Q_HEADERS src/telnetappender.h )
    list(APPEND LOG4QT_SOURCES src/telnetappender.cpp)
endif (QT_USE_QTNETWORK)

if (WIN32)
	list(APPEND LOG4QT_Q_HEADERS src/wdcappender.h)
	list(APPEND LOG4QT_SOURCES src/wdcappender.cpp)
endif (WIN32)

include_directories(src)

# COMPILATION
# if you use Q_OBJECT
qt4_wrap_cpp(LOG4QT_MOC_SOURCES ${LOG4QT_Q_HEADERS})
qt4_automoc(${LOG4QT_Q_SOURCES})
 
# BUILD STATIC LIBRARY
if (LOG4QT_BUILD_STATIC)
set(LOG4QT_BUILD_CONFIGURATION STATIC)
# BUILD SHARED LIBRARY
else (LOG4QT_BUILD_STATIC)
set(LOG4QT_BUILD_CONFIGURATION SHARED)
endif (LOG4QT_BUILD_STATIC)

add_library(${PROJECT_NAME} ${LOG4QT_BUILD_CONFIGURATION} 
    ${LOG4QT_SOURCES}
    ${LOG4QT_Q_SOURCES}
    ${LOG4QT_MOC_SOURCES}
    ${LOG4QT_HEADERS}
    ${LOG4QT_Q_HEADERS})
    
# LINKING
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    SOVERSION ${LOG4QT_VERSION}
    VERSION ${LOG4QT_VERSION}
    DEFINE_SYMBOL ${PROJECT_TO_UPPER}_EXPORTS)

# Allow install location to be overridden for libraries, includes, binaries and cmake configuration
# files. This provides fine-grained control over where various pieces get installed. The
# LIB_INSTALL_DIR and BIN_INSTALL_DIR locations are particularly useful for platforms where 32-bit
# and 64-bit binaries must be separate. (Idea shamelessly stolen from the QJson project.)
# NOTE: these locations will be used *only* if CMAKE_INSTALL_PREFIX is explicitly defined (see
# comment below).
set (BIN_SUFFIX "" CACHE STRING "Define suffix of binary directory name (32/64)" )
set (BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin${BIN_SUFFIX}" CACHE PATH "The directory where binaries will install")
set (LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${BIN_SUFFIX}" CACHE PATH "The directory where lib will install")
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_TO_LOWER}" CACHE PATH "The directory where headers will install")
set (CMAKECONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}" CACHE PATH "The directory where Log4QtConfig.cmake will install")

if (NOT LOG4QT_DONT_INSTALL)
    
	# The original implementation of this module used hardcoded paths for the install locations on
	# Mac OS X (everything was installed under the /usr directory); or would install directly into
	# the Qt directories on other platforms. CMAKE_INSTALL_PREFIX was ignored. This original
	# behaviour is preserved for anyone who was relying on it. We continue to use those default
	# install locations if CMAKE_INSTALL_PREFIX has *not* been explicitly defined. The rationale is
	# that folks who were satisfied with the old, default install locations would not have been
	# trying to use CMAKE_INSTALL_PREFIX.
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		if (NOT APPLE)
			SET(TARGET_INCLUDE_DIR "${QT_INCLUDE_DIR}/${PROJECT_NAME}")
			SET(TARGET_LIB_DIR "${QT_LIBRARY_DIR}")
			SET(TARGET_BIN_DIR "${QT_BINARY_DIR}")
		else()
			#Qt is the framework but log4qt is not. So it should not be placed into the frameworks location
			#Also the include dir should not be retrieved from the Qt itself, see http://www.paraview.org/Bug/view.php?id=10632
			SET(TARGET_INCLUDE_DIR "/usr/include/${PROJECT_NAME}")
			SET(TARGET_LIB_DIR "/usr/lib")
			SET(TARGET_BIN_DIR "/usr/bin")
		endif()
		# copy project file into qt_libdir and create project.cmake file for use this library in your cmake projects
		set(EXPORT_TARGET_DESTINATION "${QT_MKSPECS_DIR}/cmake/${PROJECT_NAME}")
	else()
		SET(TARGET_INCLUDE_DIR "${INCLUDE_INSTALL_DIR}")
		SET(TARGET_LIB_DIR "${LIB_INSTALL_DIR}")
		SET(TARGET_BIN_DIR "${BIN_INSTALL_DIR}")
		SET(EXPORT_TARGET_DESTINATION "${CMAKECONFIG_INSTALL_DIR}")
	endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    
    # for using library in your cmake projects
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/project-config.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TO_LOWER}-config.cmake"
      IMMEDIATE @ONLY)
    
    # INSTALLATION
    # install and export project target
    set(PROJECT_TARGETS ${PROJECT_NAME}-targets)
    
    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_TARGETS}
      RUNTIME DESTINATION ${TARGET_BIN_DIR}
      LIBRARY DESTINATION ${TARGET_LIB_DIR}
      ARCHIVE DESTINATION ${TARGET_LIB_DIR})
    
    # install exported project target cmake file into export target
    install(EXPORT ${PROJECT_TARGETS} DESTINATION ${EXPORT_TARGET_DESTINATION} COMPONENT Devel)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/" DESTINATION "${TARGET_INCLUDE_DIR}" COMPONENT Devel
        FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/deploy/include/" DESTINATION "${TARGET_INCLUDE_DIR}" COMPONENT Devel)
    
    # install cmake config file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TO_LOWER}-config.cmake DESTINATION ${EXPORT_TARGET_DESTINATION} COMPONENT Devel)
    
    # UNINSTALL TARGET
    # create cmake uninstall files and unregister docs scripts
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    
    add_custom_target(uninstall
        COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif (NOT LOG4QT_DONT_INSTALL)
