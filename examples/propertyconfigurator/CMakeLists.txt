set(propertyconfigurator_SOURCES
    loggerobject.cpp
    loggerobject.h
    loggerobjectprio.cpp
    loggerobjectprio.h
    loggerstatic.cpp
    loggerstatic.h
    main.cpp
)
qt_add_executable(propertyconfigurator ${propertyconfigurator_SOURCES})
target_link_libraries(propertyconfigurator PRIVATE log4qt)
