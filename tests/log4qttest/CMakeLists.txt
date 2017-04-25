set(l4qt_SRCS
    log4qttest.cpp
    log4qttest.h
)
add_executable(log4qttest ${l4qt_SRCS})
target_link_libraries(log4qttest log4qt Qt5::Test)
add_test(NAME log4qttest COMMAND $<TARGET_FILE:log4qttest>)
