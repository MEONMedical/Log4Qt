HEADERS_BASE += \
           $$PWD/appender.h \
           $$PWD/appenderskeleton.h \
           $$PWD/asyncappender.h \
           $$PWD/basicconfigurator.h \
           $$PWD/binaryfileappender.h \
           $$PWD/binarylayout.h \
           $$PWD/binarylogger.h \
           $$PWD/binaryloggingevent.h \
           $$PWD/binarylogstream.h \
           $$PWD/binarytotextlayout.h \
           $$PWD/binarywriterappender.h \
           $$PWD/consoleappender.h \
           $$PWD/dailyfileappender.h \
           $$PWD/dailyrollingfileappender.h \
           $$PWD/fileappender.h \
           $$PWD/hierarchy.h \
           $$PWD/layout.h \
           $$PWD/level.h \
           $$PWD/log4qt.h \
           $$PWD/log4qtshared.h \
           $$PWD/log4qtsharedptr.h \
           $$PWD/logger.h \
           $$PWD/loggerrepository.h \
           $$PWD/loggingevent.h \
           $$PWD/logmanager.h \
           $$PWD/logstream.h \
           $$PWD/mainthreadappender.h \
           $$PWD/mdc.h \
           $$PWD/ndc.h \
           $$PWD/patternlayout.h \
           $$PWD/propertyconfigurator.h \
           $$PWD/qmllogger.h \
           $$PWD/rollingbinaryfileappender.h \
           $$PWD/rollingfileappender.h \
           $$PWD/signalappender.h \
           $$PWD/simplelayout.h \
           $$PWD/simpletimelayout.h \
           $$PWD/systemlogappender.h \
           $$PWD/ttcclayout.h \
           $$PWD/writerappender.h \
           $$PWD/xmllayout.h
HEADERS_HELPERS += \
           $$PWD/helpers/appenderattachable.h \
           $$PWD/helpers/binaryclasslogger.h \
           $$PWD/helpers/classlogger.h \
           $$PWD/helpers/configuratorhelper.h \
           $$PWD/helpers/datetime.h \
           $$PWD/helpers/dispatcher.h \
           $$PWD/helpers/factory.h \
           $$PWD/helpers/initialisationhelper.h \
           $$PWD/helpers/logerror.h \
           $$PWD/helpers/optionconverter.h \
           $$PWD/helpers/patternformatter.h \
           $$PWD/helpers/properties.h
HEADERS_SPI += \
           $$PWD/spi/filter.h
HEADERS_VARIA += \
           $$PWD/varia/binaryeventfilter.h \
           $$PWD/varia/debugappender.h \
           $$PWD/varia/denyallfilter.h \
           $$PWD/varia/levelmatchfilter.h \
           $$PWD/varia/levelrangefilter.h \
           $$PWD/varia/listappender.h \
           $$PWD/varia/nullappender.h \
           $$PWD/varia/stringmatchfilter.h
SOURCES += $$PWD/appender.cpp \
           $$PWD/appenderskeleton.cpp \
           $$PWD/basicconfigurator.cpp \
           $$PWD/consoleappender.cpp \
           $$PWD/dailyrollingfileappender.cpp \
           $$PWD/asyncappender.cpp \
           $$PWD/dailyfileappender.cpp \
           $$PWD/mainthreadappender.cpp \
           $$PWD/fileappender.cpp \
           $$PWD/hierarchy.cpp \
           $$PWD/layout.cpp \
           $$PWD/level.cpp \
           $$PWD/logger.cpp \
           $$PWD/loggerrepository.cpp \
           $$PWD/loggingevent.cpp \
           $$PWD/logmanager.cpp \
           $$PWD/mdc.cpp \
           $$PWD/ndc.cpp \
           $$PWD/patternlayout.cpp \
           $$PWD/propertyconfigurator.cpp \
           $$PWD/rollingfileappender.cpp \
           $$PWD/signalappender.cpp \
           $$PWD/simplelayout.cpp \
           $$PWD/simpletimelayout.cpp \
           $$PWD/ttcclayout.cpp \
           $$PWD/writerappender.cpp \
           $$PWD/systemlogappender.cpp \
           $$PWD/helpers/classlogger.cpp \
           $$PWD/helpers/appenderattachable.cpp \
           $$PWD/helpers/configuratorhelper.cpp \
           $$PWD/helpers/datetime.cpp \
           $$PWD/helpers/factory.cpp \
           $$PWD/helpers/initialisationhelper.cpp \
           $$PWD/helpers/logerror.cpp \
           $$PWD/helpers/optionconverter.cpp \
           $$PWD/helpers/patternformatter.cpp \
           $$PWD/helpers/properties.cpp \
           $$PWD/helpers/dispatcher.cpp \
           $$PWD/spi/filter.cpp \
           $$PWD/varia/debugappender.cpp \
           $$PWD/varia/denyallfilter.cpp \
           $$PWD/varia/levelmatchfilter.cpp \
           $$PWD/varia/levelrangefilter.cpp \
           $$PWD/varia/listappender.cpp \
           $$PWD/varia/nullappender.cpp \
           $$PWD/varia/stringmatchfilter.cpp \
           $$PWD/logstream.cpp \
           $$PWD/binaryloggingevent.cpp \
           $$PWD/binarylogger.cpp \
           $$PWD/varia/binaryeventfilter.cpp \
           $$PWD/binarytotextlayout.cpp \
           $$PWD/binarywriterappender.cpp \
           $$PWD/binaryfileappender.cpp \
           $$PWD/binarylogstream.cpp \
           $$PWD/helpers/binaryclasslogger.cpp \
           $$PWD/rollingbinaryfileappender.cpp \
           $$PWD/binarylayout.cpp \
           $$PWD/xmllayout.cpp \
           $$PWD/qmllogger.cpp

msvc {
    QMAKE_CXXFLAGS_WARN_ON -= -w34100
    QMAKE_CXXFLAGS += -wd4100
}

# add databaseappender and -layout if QT contains sql
contains(QT, sql) {
message("Including databaseappender and -layout")
HEADERS_BASE += \
    $$PWD/databaseappender.h \
    $$PWD/databaselayout.h

SOURCES += \
    $$PWD/databaseappender.cpp \
    $$PWD/databaselayout.cpp
}

contains(QT, network) {
message("Including telnetappender")
HEADERS_BASE += \
    $$PWD/telnetappender.h

SOURCES += \
    $$PWD/telnetappender.cpp
}

win32 {
    HEADERS_BASE+=$$PWD/wdcappender.h
    SOURCES+=$$PWD/wdcappender.cpp
    HEADERS_BASE+=$$PWD/colorconsoleappender.h
    SOURCES+=$$PWD/colorconsoleappender.cpp
}
HEADERS += $$HEADERS_BASE \
           $$HEADERS_HELPERS \
           $$HEADERS_SPI \
           $$HEADERS_VARIA

!contains(QT, sql) {
message("Skipping databaseappender and -layout")
}
!contains(QT, network) {
message("Skipping telnetappender")
}
