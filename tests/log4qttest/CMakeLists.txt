set(l4qt_SOURCES
    log4qttest.cpp
    log4qttest.h
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(log4qttest ${l4qt_SOURCES})
if(Qt5_FOUND)
    target_link_libraries(log4qttest log4qt Qt5::Test)
else()
    target_link_libraries(log4qttest log4qt Qt::Test)
endif()

add_test(NAME log4qttest COMMAND $<TARGET_FILE:log4qttest>)
