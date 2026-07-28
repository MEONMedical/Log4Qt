set(basic_SOURCES
    loggerobject.cpp
    loggerobject.h
    loggerobjectprio.cpp
    loggerobjectprio.h
    loggerstatic.cpp
    loggerstatic.h
    main.cpp
)
qt_add_executable(basic ${basic_SOURCES})
target_link_libraries(basic PRIVATE log4qt)
