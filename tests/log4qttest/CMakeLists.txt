find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    log4qttest.cpp
    log4qttest.h
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_log4qttest ${l4qt_SOURCES})
target_link_libraries(tst_log4qttest log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_log4qttest COMMAND $<TARGET_FILE:tst_log4qttest>)
