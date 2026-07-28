find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

set(l4qt_SOURCES
    tst_filewatchertest.cpp
)
qt_add_executable(tst_filewatchertest ${l4qt_SOURCES})
target_link_libraries(tst_filewatchertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_filewatchertest COMMAND $<TARGET_FILE:tst_filewatchertest>)
