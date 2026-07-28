find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_xmlconfigurator.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_xmltest ${l4qt_SOURCES})
target_link_libraries(tst_xmltest log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_xmltest COMMAND $<TARGET_FILE:tst_xmltest>)
