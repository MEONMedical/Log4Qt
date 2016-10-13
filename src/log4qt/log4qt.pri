HEADERS += $$PWD/appender.h \
           $$PWD/appenderskeleton.h \
           $$PWD/basicconfigurator.h \
           $$PWD/colorconsoleappender.h \
           $$PWD/consoleappender.h \
           $$PWD/dailyrollingfileappender.h \
           $$PWD/asyncappender.h \
           $$PWD/dailyfileappender.h \
           $$PWD/mainthreadappender.h \
           $$PWD/fileappender.h \
           $$PWD/hierarchy.h \
           $$PWD/layout.h \
           $$PWD/level.h \
           $$PWD/log4qt.h \
           $$PWD/log4qtshared.h \
           $$PWD/logger.h \
           $$PWD/loggerrepository.h \
           $$PWD/loggingevent.h \
           $$PWD/logmanager.h \
           $$PWD/mdc.h \
           $$PWD/ndc.h \
           $$PWD/patternlayout.h \
           $$PWD/propertyconfigurator.h \
           $$PWD/rollingfileappender.h \
           $$PWD/signalappender.h \
           $$PWD/simplelayout.h \
           $$PWD/simpletimelayout.h \
           $$PWD/ttcclayout.h \
           $$PWD/telnetappender.h \
           $$PWD/writerappender.h \
           $$PWD/systemlogappender.h \
           $$PWD/helpers/classlogger.h \
           $$PWD//helpers/appenderattachable.h \
           $$PWD/helpers/configuratorhelper.h \
           $$PWD/helpers/datetime.h \
           $$PWD/helpers/factory.h \
           $$PWD/helpers/initialisationhelper.h \
           $$PWD/helpers/logerror.h \
           $$PWD/helpers/optionconverter.h \
           $$PWD/helpers/patternformatter.h \
           $$PWD/helpers/properties.h \
           $$PWD/helpers/dispatcher.h \
           $$PWD/spi/filter.h \
           $$PWD/varia/debugappender.h \
           $$PWD/varia/denyallfilter.h \
           $$PWD/varia/levelmatchfilter.h \
           $$PWD/varia/levelrangefilter.h \
           $$PWD/varia/listappender.h \
           $$PWD/varia/nullappender.h \
           $$PWD/varia/stringmatchfilter.h \
           $$PWD/logstream.h \
           $$PWD/binaryloggingevent.h \
           $$PWD/binarylogger.h \
           $$PWD/varia/binaryeventfilter.h \
           $$PWD/binarytotextlayout.h \
           $$PWD/binarywriterappender.h \
           $$PWD/binaryfileappender.h \
           $$PWD/binarylogstream.h \
           $$PWD/helpers/binaryclasslogger.h \
           $$PWD/rollingbinaryfileappender.h \
           $$PWD/binarylayout.h \
           $$PWD/xmllayout.h \
    	   $$PWD/qmllogger.h \
           $$PWD/log4qtsharedptr.h

SOURCES += $$PWD/appender.cpp \
           $$PWD/appenderskeleton.cpp \
           $$PWD/basicconfigurator.cpp \
           $$PWD/colorconsoleappender.cpp \
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
           $$PWD/telnetappender.cpp \
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
HEADERS += \
    $$PWD/databaseappender.h \
    $$PWD/databaselayout.h

SOURCES += \
    $$PWD/databaseappender.cpp \
    $$PWD/databaselayout.cpp

}

win32 {
    HEADERS+=$$PWD/wdcappender.h
    SOURCES+=$$PWD/wdcappender.cpp
}

!contains(QT, sql) {
message("Skipping databaseappender and -layout")
}
