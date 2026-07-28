find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_xmlconfigurator.cpp
)
qt_add_executable(tst_xmltest ${l4qt_SOURCES})
target_link_libraries(tst_xmltest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_xmltest COMMAND $<TARGET_FILE:tst_xmltest>)
