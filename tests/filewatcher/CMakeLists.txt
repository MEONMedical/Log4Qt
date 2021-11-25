find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(l4qt_SOURCES
    tst_filewatchertest.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(filewatchertest ${l4qt_SOURCES})
target_link_libraries(filewatchertest log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME filewatchertest COMMAND $<TARGET_FILE:log4qttest>)
