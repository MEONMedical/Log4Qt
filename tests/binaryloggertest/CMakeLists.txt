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
if(QT_VERSION LESS 5.15.0)
    target_link_libraries(log4qttest log4qt Qt::Test)
else()
    target_link_libraries(log4qttest log4qt Qt5::Test)
endif()

add_test(NAME binaryloggertest COMMAND $<TARGET_FILE:binaryloggertest>)
