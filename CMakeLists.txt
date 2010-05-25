cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
# postfix for library
# if CMAKE_BUILD_TYPE == Debug then library_name become library_name + CMAKE_DEBUG_POSTFIX
set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

# variable for project name, used in many places
set (PROJECT log4qt)

project(${PROJECT})

string(TOUPPER ${PROJECT_NAME} PROJECT_TO_UPPER)

find_package(Qt4 REQUIRED)

# COMPILER customization
  # compilers flags
  if (MSVC)
    add_definitions (/W3)
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
set(SOURCES)
# Q_SOURCES, var contains files, for which need call qt4_automoc
set(Q_SOURCES)
set(HEADERS)
# Q_HEADERS, var contains files, for which qt4_wrap_cpp called
set(Q_HEADERS)

file(GLOB_RECURSE Q_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# project version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 3)
set (VERSION_RELEASE 0)
set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}" )

# QT CUSTOMIZATION
# qt definitions
# dont using gui
set(QT_DONTUSE_QTGUI   TRUE)

# include qt libraries
include(${QT_USE_FILE})

if (NOT QT_USE_QTSQL)
	message("Without QT_USE_QTSQL DatabaseAppender, DatabaseLayout not compiled")
	list(REMOVE_ITEM Q_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/databaseappender.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/databaselayout.h")
	list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/databaseappender.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/databaselayout.cpp")
endif (NOT QT_USE_QTSQL)

# COMPILATION
include_directories(src)
# if you use Q_OBJECT 
qt4_wrap_cpp(MOC_SOURCES ${Q_HEADERS})

qt4_automoc(${Q_SOURCES})

qt4_wrap_ui(UI_HEADERS ${FORMS})

qt4_add_resources(QRC_SOURCES ${RESOURCES})

add_library(${PROJECT_NAME} SHARED 
	${SOURCES}
	${Q_SOURCES}
	${MOC_SOURCES}
	${QRC_SOURCES}
	${HEADERS}
	${Q_HEADERS}
	${UI_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
  # lib version
  VERSION ${VERSION} 
  SOVERSION ${VERSION}
 	# add -D${PROJECT_NAME}_EXPORTS to export symbols for msvc compilers
  DEFINE_SYMBOL ${PROJECT_TO_UPPER}_EXPORTS)

# LINKING
#set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)
# main library
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})


# CMAKE CONFIG FILE
set (INSTALL_PROJECT_INCLUDE_DIR "${QT_INCLUDE_DIR}/${PROJECT_NAME}")
# for using library in your cmake projects
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/project-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  IMMEDIATE @ONLY)

# INSTALLATION
# install and export project target
# copy project file into qt_libdir and create project.cmake file for use this library in your cmake projects
set(EXPORT_TARGET_DESTINATION "${QT_MKSPECS_DIR}/cmake/${PROJECT_NAME}")
set(PROJECT_TARGETS ${PROJECT_NAME}-targets)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_TARGETS}
  RUNTIME DESTINATION ${QT_BINARY_DIR}
  LIBRARY DESTINATION ${QT_LIBRARY_DIR}
  ARCHIVE DESTINATION ${QT_LIBRARY_DIR})

# dublicate dll file for dll-based platform
#if(WIN32)
#  install(TARGETS ${PROJECT_NAME} DESTINATION lib)
#endif(WIN32)

# install exported project target cmake file into qt_mkspec_dir/cmake
install(EXPORT ${PROJECT_TARGETS} DESTINATION ${EXPORT_TARGET_DESTINATION} COMPONENT Devel)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/" DESTINATION "${INSTALL_PROJECT_INCLUDE_DIR}" COMPONENT Devel
	FILES_MATCHING PATTERN "*.h")

# install cmake config file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake DESTINATION ${EXPORT_TARGET_DESTINATION} COMPONENT Devel)

# UNINSTALL TARGET
# create cmake uninstall files and unregister docs scripts
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")