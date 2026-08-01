find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_mainthreadappender.cpp
)
qt_add_executable(tst_mainthreadappendertest ${l4qt_SOURCES})
target_link_libraries(tst_mainthreadappendertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_mainthreadappendertest COMMAND $<TARGET_FILE:tst_mainthreadappendertest>)
