find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(l4qt_SOURCES
    tst_writerappender.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(tst_writerappendertest ${l4qt_SOURCES})
target_link_libraries(tst_writerappendertest log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_writerappendertest COMMAND $<TARGET_FILE:tst_writerappendertest>)
