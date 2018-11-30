set(l4qt_SOURCES
    log4qttest.cpp
    log4qttest.h
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(log4qttest ${l4qt_SOURCES})
target_link_libraries(log4qttest log4qt Qt5::Test)
add_test(NAME log4qttest COMMAND $<TARGET_FILE:log4qttest>)
