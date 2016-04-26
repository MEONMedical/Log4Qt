Original project

http://log4qt.sourceforge.net/ Log4Qt is a C++ port of the Apache Software Foundation Log4j package using the Trolltech Qt Framework. It is intended to be used by open source and commercial Qt projects.

Requirements
    Minimum Qt version required Qt5.5.
    C++11

License:
Apache License 2, Version 2.0

Build and install

use qmake to build the project
*NIX
  qmake
  make
  make install
WIN*
  qmake
  mingw32-make
  mingw32-make install

cmake (CMakeLists.txt) is out of date for Qt5 and not working at the moment.
Help to fix cmake for Qt5 build welcome.

*NIX
  STATIC
    cmake -DQT_USE_QTSQL=TRUE -DQT_USE_QTNETWORK=TRUE -DCMAKE_BUILD_TYPE=Release -DLOG4QT_BUILD_STATIC=True .
    make
    make install
  SHARED
    cmake -DQT_USE_QTSQL=TRUE -DQT_USE_QTNETWORK=TRUE -DCMAKE_BUILD_TYPE=Release .
    make
    make install

WIN*
  STATIC
    cmake -DQT_USE_QTSQL=TRUE -DQT_USE_QTNETWORK=TRUE -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" -DLOG4QT_BUILD_STATIC=True .
    mingw32-make
    mingw32-make install
  SHARED
    cmake -DQT_USE_QTSQL=TRUE -DQT_USE_QTNETWORK=TRUE -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" .
    mingw32-make
    mingw32-make install

If you dont want to use log4qt sql features you must not define QT_USE_QTSQL.
If you dont want to use log4qt network features you must not define QT_USE_QTNETWORK.

Using in your projects

If your have cmake-based project
  for shared linking you must:
    1. make log4qt in shared mode;
    2. install it;
    1. add log4qt package to your CMakeLists.txt;
    2. include header directory;
    3. link library to target.

    For example:
    find_package(log4qt REQUIRED HINTS "${QT_MKSPECS_DIR}/cmake/Log4Qt" NO_DEFAULT_PATHS)
    include_directories(${LOG4QT_INCLUDE_DIRS})
    target_link_libraries(main ${QT_LIBRARIES} log4qt)

  for static linking:
    1. add subdirectory to your project
    2. include log4qt/src
    3. link library to target

    For example:
    add_subdirectory(../log4qt ${CMAKE_CURRENT_BINARY_DIR}/log4qt)
    include_directories(../log4qt/src)
    target_link_libraries(main ${QT_LIBRARIES} log4qt)
