set(basic_SOURCES
    loggerobject.cpp
    loggerobject.h
    loggerobjectprio.cpp
    loggerobjectprio.h
    loggerstatic.cpp
    loggerstatic.h
    main.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(propertyconfigurator ${basic_SOURCES})
target_link_libraries(propertyconfigurator log4qt)
