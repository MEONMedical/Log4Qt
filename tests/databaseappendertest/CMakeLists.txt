find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Sql)

set(l4qt_SOURCES
    tst_databaseappender.cpp
)
qt_add_executable(tst_databaseappendertest ${l4qt_SOURCES})
target_link_libraries(tst_databaseappendertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Sql)

add_test(NAME tst_databaseappendertest COMMAND $<TARGET_FILE:tst_databaseappendertest>)
