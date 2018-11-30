set(blt_SOURCES
    atscopeexit.h
    binaryloggertest.cpp
    elementsinarray.h
    logging.cpp
    logging.h
    testappender.cpp
    testappender.h
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(binaryloggertest ${blt_SOURCES})
target_link_libraries(binaryloggertest log4qt Qt5::Test)
add_test(NAME binaryloggertest COMMAND $<TARGET_FILE:binaryloggertest>)
