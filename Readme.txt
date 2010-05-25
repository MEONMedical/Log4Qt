Build and install

*NIX
cmake -DQT_USE_QTSQL=TRUE -DCMAKE_BUILD_TYPE=Release .
make
make install

WIN*
cmake -DQT_USE_QTSQL=TRUE -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" .
mingw32-make
mingw32-make install

If you dont want to use log4qt sql features you must not define QT_USE_QTSQL.

cmake-based
in your CMakeLists.txt
find_package(log4qt REQUIRED HINTS "${QT_MKSPECS_DIR}/cmake" NO_DEFAULT_PATHS)
include_directories(${LOG4QT_INCLUDE_DIRS})
target_link_libraries(main ${QT_LIBRARIES} log4qt)
