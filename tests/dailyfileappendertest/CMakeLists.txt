set(TEST_NAME dailyfileappendertest)

set(dfa_SOURCES
    dailyfileappendertest.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${TEST_NAME} ${dfa_SOURCES})
if(QT_VERSION LESS 5.15.0)
    target_link_libraries(log4qttest log4qt Qt::Test)
else()
    target_link_libraries(log4qttest log4qt Qt5::Test)
endif()
add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
