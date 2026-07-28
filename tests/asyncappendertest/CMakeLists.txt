find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_asyncappender.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
qt_add_executable(tst_asyncappendertest ${l4qt_SOURCES})
target_link_libraries(tst_asyncappendertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_asyncappendertest COMMAND $<TARGET_FILE:tst_asyncappendertest>)
