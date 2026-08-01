find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

qt_add_executable(tst_headerfootertest tst_headerfooter.cpp)
target_link_libraries(tst_headerfootertest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_headerfootertest COMMAND $<TARGET_FILE:tst_headerfootertest>)
