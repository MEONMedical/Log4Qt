find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Sql REQUIRED)

set(l4qt_SOURCES
    tst_databaseappender.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_databaseappendertest ${l4qt_SOURCES})
target_link_libraries(tst_databaseappendertest log4qt Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Sql)

add_test(NAME tst_databaseappendertest COMMAND $<TARGET_FILE:tst_databaseappendertest>)
