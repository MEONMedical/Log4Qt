set(blt_SRCS
    atscopeexit.h
    binaryloggertest.cpp
    elementsinarray.h
    logging.cpp
    logging.h
    testappender.cpp
    testappender.h
)
add_executable(binaryloggertest ${blt_SRCS})
target_link_libraries(binaryloggertest log4qt Qt5::Test)

