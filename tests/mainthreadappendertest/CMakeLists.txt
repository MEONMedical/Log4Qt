find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(l4qt_SOURCES
    tst_mainthreadappender.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_mainthreadappendertest ${l4qt_SOURCES})
target_link_libraries(tst_mainthreadappendertest log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_mainthreadappendertest COMMAND $<TARGET_FILE:tst_mainthreadappendertest>)
