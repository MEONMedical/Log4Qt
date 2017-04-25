set(basic_SRCS
    loggerobject.cpp
    loggerobject.h
    loggerobjectprio.cpp
    loggerobjectprio.h
    loggerstatic.cpp
    loggerstatic.h
    main.cpp
)
add_executable(propertyconfigurator ${basic_SRCS})
target_link_libraries(propertyconfigurator log4qt)
