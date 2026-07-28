find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

qt_add_executable(tst_jsonlayouttest tst_jsonlayout.cpp)
target_link_libraries(tst_jsonlayouttest PRIVATE log4qt Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_jsonlayouttest COMMAND $<TARGET_FILE:tst_jsonlayouttest>)
