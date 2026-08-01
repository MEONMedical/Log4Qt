find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Network)

set(l4qt_SOURCES
    tst_telnetappender.cpp
)
qt_add_executable(tst_telnetappendertest ${l4qt_SOURCES})
target_link_libraries(tst_telnetappendertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Network)

add_test(NAME tst_telnetappendertest COMMAND $<TARGET_FILE:tst_telnetappendertest>)
