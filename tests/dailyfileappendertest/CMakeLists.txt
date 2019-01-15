set(TEST_NAME dailyfileappendertest)

set(dfa_SOURCES
    dailyfileappendertest.cpp
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${TEST_NAME} ${dfa_SOURCES})
target_link_libraries(${TEST_NAME} log4qt Qt5::Test)
add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
