find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Performance test suite
add_executable(tst_performancetest
    performancetest.cpp
    performancetest.h
)
target_link_libraries(tst_performancetest log4qt Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_performancetest COMMAND $<TARGET_FILE:tst_performancetest>)

# QString benchmark
add_executable(tst_qstring_benchmark
    qstring_benchmark.cpp
    qstring_benchmark.h
)
target_link_libraries(tst_qstring_benchmark log4qt Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_qstring_benchmark COMMAND $<TARGET_FILE:tst_qstring_benchmark>)

# LoggingEvent accessor benchmark (investigation target I-004)
add_executable(tst_loggingevent_accessor_benchmark
    loggingevent_accessor_benchmark.cpp
)
target_link_libraries(tst_loggingevent_accessor_benchmark log4qt Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_loggingevent_accessor_benchmark COMMAND $<TARGET_FILE:tst_loggingevent_accessor_benchmark>)
