*-g++* {
  NORMAL_CFLAGS = -Wno-long-long -ansi
  NORMAL_CXXFLAGS = \
    -Wnon-virtual-dtor -Wundef -Wcast-align \
    -Wchar-subscripts -Wpointer-arith \
    -Wwrite-strings -Wpacked -Wformat-security \
    -Wmissing-format-attribute -Woverloaded-virtual \
#    -Wzero-as-null-pointer-constant


  # NORMAL_CXXFLAGS += -Wconversion

  NORMAL_CFLAGS += -pedantic

  USABLE_CXXFLAGS = -Wold-style-cast
  HARD_CXXFLAGS = -Weffc++ -Wshadow
  PITA_CXXFLAGS = -Wunreachable-code

  QMAKE_CFLAGS   += $$NORMAL_CFLAGS
  QMAKE_CXXFLAGS += $$NORMAL_CFLAGS $$NORMAL_CXXFLAGS

  # Not needed - warn_on.prf adds QMAKE_CXXFLAGS_WARN_ON to QMAKE_CXXFLAGS
  #QMAKE_CFLAGS_WARN_ON   += $$NORMAL_CFLAGS
  #QMAKE_CXXFLAGS_WARN_ON += $$NORMAL_CFLAGS $$NORMAL_CXXFLAGS

  QMAKE_CXXFLAGS_WARN_ON += $$USABLE_CXXFLAGS
  #QMAKE_CXXFLAGS_WARN_ON += $$HARD_CXXFLAGS # headers must compile with this, code doesn't need to; needs patched Qt
  #QMAKE_CXXFLAGS_WARN_ON += $$PITA_CXXFLAGS # header would be nice, but it's probably pointless, due to noise from Qt and libstdc++

}

msvc {
    QMAKE_CFLAGS_WARN_ON -= -W3
    QMAKE_CFLAGS_WARN_ON += -W4

    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4

    # disable common warnings in Qt/stdlib
    # disable 4127: conditional expression is constant
    #         4512: assignment operator could not be generated
    #         4267: conversion from 'size_t' to 'type', possible loss of data
    QMAKE_CXXFLAGS += -wd4127 -wd4512 -wd4267 -we4265 -wd4913

# /WX ... treat all warnings as errors
# /we<n> ... Treat warning <n> as error

}
