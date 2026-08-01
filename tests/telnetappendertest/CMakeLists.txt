find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Network REQUIRED)

set(l4qt_SOURCES
    tst_telnetappender.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_telnetappendertest ${l4qt_SOURCES})
target_link_libraries(tst_telnetappendertest log4qt Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Network)

add_test(NAME tst_telnetappendertest COMMAND $<TARGET_FILE:tst_telnetappendertest>)
