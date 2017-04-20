set(basic_SRCS
    loggerobject.cpp
    loggerobject.h
    loggerobjectprio.cpp
    loggerobjectprio.h
    loggerstatic.cpp
    loggerstatic.h
    main.cpp
)
add_executable(basic ${basic_SRCS})
target_link_libraries(basic log4qt)
