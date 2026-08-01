find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_jsonconfigurator.cpp
)
qt_add_executable(tst_jsontest ${l4qt_SOURCES})
target_link_libraries(tst_jsontest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_jsontest COMMAND $<TARGET_FILE:tst_jsontest>)
